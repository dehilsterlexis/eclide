#include "StdAfx.h"

#include "EclCC.h"
#include "comms.h"
#include <cmdProcess.h>
#include <Logger.h>
#include <cache.h>
#include <EclErrorParser.h>
#include "Repository.h"
#include "SMC.h"
#include <UtilFilesystem.h>
#include "DiskAttribute.h"
#include "ModuleHelper.h"

namespace algo = boost::algorithm;

const TCHAR * const ECLCC_ECLBUNDLE_PATH = _T("ECLCC_ECLBUNDLE_PATH");
typedef std::map<std::_tstring, std::_tstring> StringPathMap;

namespace SMC
{
IVersion * CreateVersion(const CString & url, const CString & version);
}

TRI_BOOL g_EnableCompiler = TRI_BOOL_UNKNOWN;
TRI_BOOL g_EnableRemoteDali = TRI_BOOL_UNKNOWN;
TRI_BOOL g_EnableRemoteQueue = TRI_BOOL_UNKNOWN;

class CEclCC : public IEclCC, public clib::CLockableUnknown
{
protected:
    CComPtr<IConfig> m_config;
    CComPtr<IConfig> m_pluginConfig;
    std::_tstring m_compilerFile;
    boost::filesystem::wpath m_compilerFilePath;
    boost::filesystem::wpath m_compilerFolderPath;
    boost::filesystem::wpath m_IDEPluginsFolderPath;

    std::_tstring m_arguments;

    std::_tstring m_workingFolder;
    boost::filesystem::wpath m_workingFolderPath;

    WPathVector m_eclFolders;

    mutable CComPtr<SMC::IVersion> m_compilerVersion;
    mutable CEclMeta m_eclMeta;

public:
    mutable std::_tstring m_version;
    std::_tstring m_errors;
    std::_tstring m_warnings;

    BEGIN_CLOCKABLEUNKNOWN
    END_CUNKNOWN(CUnknown)

    CEclCC(IConfig * config, const std::_tstring & compilerFile) : m_config(config), m_compilerFile(compilerFile)
    {
        ATLASSERT(m_config);
        EclFoldersFromConfig();
        GetBuild();
    }

    void EclFoldersFromConfig() {

        m_eclFolders.clear();

        for (int i = 0; i < 10; ++i)
        {
            CString text;
            switch (i)
            {
            case 0:
                text = m_config->Get(GLOBAL_COMPILER_ECLFOLDER00);
                break;
            case 1:
                text = m_config->Get(GLOBAL_COMPILER_ECLFOLDER01);
                break;
            case 2:
                text = m_config->Get(GLOBAL_COMPILER_ECLFOLDER02);
                break;
            case 3:
                text = m_config->Get(GLOBAL_COMPILER_ECLFOLDER03);
                break;
            case 4:
                text = m_config->Get(GLOBAL_COMPILER_ECLFOLDER04);
                break;
            case 5:
                text = m_config->Get(GLOBAL_COMPILER_ECLFOLDER05);
                break;
            case 6:
                text = m_config->Get(GLOBAL_COMPILER_ECLFOLDER06);
                break;
            case 7:
                text = m_config->Get(GLOBAL_COMPILER_ECLFOLDER07);
                break;
            case 8:
                text = m_config->Get(GLOBAL_COMPILER_ECLFOLDER08);
                break;
            case 9:
                text = m_config->Get(GLOBAL_COMPILER_ECLFOLDER09);
                break;
            }

            if (text.GetLength() > 0 && clib::filesystem::exists(static_cast<const TCHAR *>(text)) && clib::filesystem::is_directory(static_cast<const TCHAR *>(text)))
                m_eclFolders.push_back(std::make_pair(static_cast<const TCHAR *>(text), true));
        }

        m_compilerFilePath = boost::filesystem::wpath(m_compilerFile, boost::filesystem::native);

        //  Patch for 3.10 temporary weirdness (directory structure changes).
        if (!clib::filesystem::exists(m_compilerFilePath) && boost::algorithm::icontains(m_compilerFile, "\\bin\\ver_3_6")) 
        {
            boost::algorithm::replace_all(m_compilerFile, "\\bin\\ver_3_6", "\\ver_3_6\\bin");
            m_compilerFilePath = boost::filesystem::wpath(m_compilerFile, boost::filesystem::native);
        }
        m_compilerFolderPath = m_compilerFilePath.parent_path();
        m_IDEPluginsFolderPath = m_compilerFolderPath.parent_path() / _T("IDEPlugins");

        m_arguments = CString(m_config->Get(GLOBAL_COMPILER_ARGUMENTS));

        m_workingFolder = CString(m_config->Get(GLOBAL_COMPILER_ECLWORKINGFOLDER));
        m_workingFolderPath = boost::filesystem::wpath(m_workingFolder, boost::filesystem::native);

        boost::filesystem::wpath stdLibPath = m_compilerFolderPath / _T("ecllibrary");

        //  Patch for 3.10 temporary weirdness (directory structure changes).
        boost::filesystem::wpath clientToolsFolderPath =  m_compilerFolderPath.parent_path();

        if (!clib::filesystem::exists(stdLibPath))
            stdLibPath = clientToolsFolderPath / _T("share") / _T("ecllibrary");
        if (clib::filesystem::exists(stdLibPath))
            m_eclFolders.push_back(std::make_pair(pathToWString(stdLibPath), false));

        boost::filesystem::wpath pluginsPath = clientToolsFolderPath / PLUGINS_FOLDER;
        if (clib::filesystem::exists(pluginsPath))
            m_eclFolders.push_back(std::make_pair(pathToWString(pluginsPath), false));

        StringPathMap paths;
        GetPaths(paths);
        StringPathMap::const_iterator found = paths.find(ECLCC_ECLBUNDLE_PATH);
        if (found != paths.end())
            m_eclFolders.push_back(std::make_pair(found->second, false));
    }

    void PopulateMeta(IAttribute *attribute)
    {
        if (!attribute) return;
        CModuleHelper modHelper(attribute->GetQualifiedLabel());
        int level = modHelper.GetQualifiedDepth() >= 3 ? modHelper.GetQualifiedDepth() - 2 : 0;
        m_eclMeta.PopulateMeta(attribute->GetPath(), modHelper.GetModuleLabelNoRoot(), level);
    }

    const TCHAR * GetCacheID() const
    {
        clib::recursive_mutex::scoped_lock proc(m_mutex);
        return m_compilerFile.c_str();
    }

    const TCHAR* GetVersionString(std::_tstring& version) const {
        return m_compilerVersion->GetString(version);
    }

    const bool IsExecutableBad() const {
        return m_compilerVersion->IsExecutableBad();
    }

    bool GetAutoC(IAttribute *attr, const std::_tstring & partialLabel, StdStringVector &set)
    {
        m_eclMeta.GetAutoC(attr, partialLabel, set);
        return false;
    }

    const TCHAR * GetToolTip(const std::_tstring & key, std::_tstring & tooltip)
    {
        return m_eclMeta.GetFunctionTooltip(key, tooltip);
    }

    SMC::IVersion * GetBuild() const
    {
        clib::recursive_mutex::scoped_lock proc(m_mutex);
        if (!m_compilerVersion) {
            CString version;
            m_compilerVersion = SMC::CreateVersion(m_compilerFile.c_str(), version);
        }

        return m_compilerVersion;
    }

    const TCHAR * GetPrefWarnings(std::_tstring & warnings) const
    {
        warnings = m_warnings;
        return warnings.c_str();
    }

    const TCHAR * GetPrefErrors(std::_tstring & errors) const
    {
        clib::recursive_mutex::scoped_lock proc(m_mutex);
        errors = m_errors;
        if (!errors.empty())
            errors += _T("\r\n");
        if (!clib::filesystem::exists(m_compilerFilePath))
            errors += (boost::_tformat(_T("Compiler path invalid:  %1%")) % m_compilerFile).str() + _T("\r\n");
        else if (clib::filesystem::is_directory(m_compilerFilePath))
            errors += (boost::_tformat(_T("Compiler path does not specify eclcc.exe:  %1%")) % m_compilerFile).str() + _T("\r\n");

        if (!m_workingFolder.empty() && !clib::filesystem::exists(m_workingFolder))
        {
            std::_tstring error_msg = (boost::_tformat(_T("Working folder invalid:  %1%")) % m_workingFolder).str() + _T("\r\nCreate?");
            if (::MessageBox(NULL, error_msg.c_str(), _T("ECL IDE"), MB_ICONASTERISK | MB_YESNO) == IDYES)
            {
                try {
                    boost::filesystem::create_directories(m_workingFolder);
                } catch (const boost::filesystem::filesystem_error &) {
                    errors += (boost::_tformat(_T("Failed to create folder:  %1%")) % m_workingFolder).str() + _T("\r\n");
                }

            }
            else
                errors += (boost::_tformat(_T("Working folder invalid:  %1%")) % m_workingFolder).str() + _T("\r\n");
        }

        return errors.c_str();
    }

    const TCHAR * GetWorkingFolder() const
    {
        clib::recursive_mutex::scoped_lock proc(m_mutex);
        return m_workingFolder.c_str();
    }

    int GetEclFolderCount() const
    {
        clib::recursive_mutex::scoped_lock proc(m_mutex);
        return m_eclFolders.size();
    }

    const TCHAR * GetEclFolder(int i) const
    {
        clib::recursive_mutex::scoped_lock proc(m_mutex);
        ATLASSERT(i >= 0 && i < (int)m_eclFolders.size());
        return m_eclFolders[i].first.c_str();
    }

    std::_tstring GetCompilerFilePath() const
    {
        return pathToWString(m_compilerFilePath);
    }

    bool LocateIDEPlugin(const std::string & batchFile, boost::filesystem::path & foundFolder) const
    {
        boost::filesystem::directory_iterator end_itr;
        for (boost::filesystem::directory_iterator itr(m_IDEPluginsFolderPath); itr != end_itr; ++itr)
        {
            if (clib::filesystem::is_directory(*itr) && clib::filesystem::exists(*itr / batchFile))
            {
                foundFolder = *itr;
                return true;
            }
        }
        return false;
    }

    void ParseErrors(const std::_tstring & sourcePath, const std::_tstring & _err, bool &hasErrors, Dali::CEclExceptionVector & errors, bool markAllAsErrors) const
    {
        clib::recursive_mutex::scoped_lock proc(m_mutex);
        typedef std::vector<std::_tstring> split_vector_type;
        split_vector_type SplitVec; 
        algo::split(SplitVec, _err, algo::is_any_of("\r\n"), algo::token_compress_on);
        for(split_vector_type::iterator itr = SplitVec.begin(); itr != SplitVec.end(); ++itr)
        {
            if (itr->empty())
                continue;
            ParsedEclError err;
            if (ParseEclError(*itr, err))
            {
                StlLinked<Dali::CEclException> e = new Dali::CEclException();
                e->m_lineNo = err.row;
                e->m_code = err.code;
                e->m_message = err.message.c_str();
                e->m_column = err.col;
                if (markAllAsErrors || algo::iequals(err.type, "error"))
                {
                    e->m_severity = _T("Error");
                    hasErrors = true;
                }
                else if (algo::iequals(err.type, "warning"))
                    e->m_severity = _T("Warning");
                else 
                    e->m_severity = err.type.c_str();
                if (algo::iends_with(sourcePath, err.location))
                    e->m_fileName = _T("");
                else
                    e->m_fileName = err.location.c_str();
                errors.push_back(e);
            }
            else if (!itr->empty())
            {
                StlLinked<Dali::CEclException> e = new Dali::CEclException();
                e->m_lineNo = 0;
                e->m_code = 0;
                e->m_message = itr->c_str();
                e->m_column = 0;
                e->m_severity = markAllAsErrors ? _T("Error") : _T("Info");
                errors.push_back(e);
            }
        }
    }

    const TCHAR * CallEclCC(const std::_tstring & _module, const std::_tstring & _attribute, const std::_tstring & path, const std::_tstring & ecl, const StdStringVector & args, std::_tstring & out, std::_tstring & err, bool &hasErrors, Dali::CEclExceptionVector & errors) const
    {
        clib::recursive_mutex::scoped_lock proc(m_mutex);
        hasErrors = false;

        std::_tstring command = _T("\"");
        command += m_compilerFile;
        command += _T("\"");

        CAtlTemporaryFile temp;
        std::_tstring sourcePath;
        if (_module.empty() && _attribute.empty() && path.empty())
        {
            temp.Create();
            sourcePath = temp.TempFileName();
            std::string necl = CT2A(ecl.c_str(), CP_UTF8);
            temp.Write(necl.c_str(), necl.length());
            temp.HandsOff();
            if (GetBuild()->GetMajor() >= 5)
                command += _T(" --nosourcepath");
        }
        else if (path.empty())
        {
            CComPtr<IRepository> rep = AttachRepository();
            CComPtr<IAttribute> attr = rep->GetAttributeFast(_module.c_str(), _attribute.c_str(), CreateIAttributeECLType());
            if (attr != NULL)
                sourcePath = attr->GetPath();
        }
        else
        {
            sourcePath = path;
        }

        if (!m_arguments.empty())
        {
            command += _T(" ");
            command += m_arguments;
        }
        for(StdStringVector::const_iterator itr = args.begin(); itr != args.end(); ++itr)
        {
            command += _T(" -");
            command += *itr;
        }
        if (!m_workingFolder.empty())
        {
            command += _T(" -P\"");
            command += m_workingFolder;
            command += _T("\"");
        }
        for(WPathVector::const_iterator itr = m_eclFolders.begin(); itr != m_eclFolders.end(); ++itr)
        {
            if (itr->second)	//  Exclude ecllibrary
            {
                command += _T(" -I\"");
                command += itr->first;
                command += _T("\"");
            }
        }
        command += _T(" \"");
        command += sourcePath;
        command += _T("\"");
        std::_tstring runPath = pathToWString(m_compilerFolderPath);
        std::_tstring in = _T("");

        std::_tstring folder = m_workingFolder;
        bool exists = boost::filesystem::exists(folder);
        if (folder.empty() || !exists)
        {
            _TCHAR buf[MAX_PATH];
            if (GetTempPath(MAX_PATH, buf)) {
                folder = buf;
            }
        }
        runProcess(command, folder, _T(""), in, out, err);

        ParseErrors(sourcePath, err, hasErrors, errors, false);

        if (_module.empty() && _attribute.empty() && path.empty())
        {
            temp.HandsOn();
        }

        return out.c_str();
    }

    void CheckSyntax(const std::_tstring & module, const std::_tstring & attribute, const std::_tstring & path, const std::_tstring & ecl, Dali::CEclExceptionVector & errors) const
    {
        clib::recursive_mutex::scoped_lock proc(m_mutex);
        StdStringVector args;
        args.push_back(_T("f\"syntaxcheck=1\""));
        if(!m_config->Get(GLOBAL_COMPILER_DISABLEMETA))
        {
            args.push_back(_T("M"));
        }

        std::_tstring out, err;
        bool hasErrors = false;
        std::_tstring xmlMeta = CallEclCC(module, attribute, path, ecl, args, out, err, hasErrors, errors);
        if(!m_config->Get(GLOBAL_COMPILER_DISABLEMETA))
        {
            m_eclMeta.PopulateMetaUpwards(m_eclFolders, path);
            m_eclMeta.Update(m_eclFolders, xmlMeta);
        }
        m_eclMeta.LoadImports(path,m_eclFolders);
        m_eclMeta.LoadFunctions(path);

#ifdef _DEBUG
        // Temporary file saving for developmemnt
        if (xmlMeta.size() > 0) {
            std::_tstring filePath = pathToWString(m_workingFolderPath / _T("metadata.xml"));
            CUnicodeFile file;
            if (file.Create(filePath.c_str()))
                file.Write(xmlMeta);
        }
#endif
    }

    const TCHAR * GetWorkunitXML(const std::_tstring & wuid, std::_tstring & wuXml) const
    {
        clib::recursive_mutex::scoped_lock proc(m_mutex);
        std::_tstring filePath = pathToWString((m_workingFolderPath / (wuid + _T(".xml"))));
        if (!clib::filesystem::exists(filePath))
        {
            std::_tstring command = _T("wuget.exe \"");
            command += pathToWString((m_workingFolderPath / (wuid + _T(".exe"))));
            command += _T("\"");
            std::_tstring err;
            runProcess(command, m_workingFolder, pathToWString(m_compilerFolderPath), _T(""), wuXml, err);
            CUnicodeFile file;
            if (file.Create(filePath.c_str()))
                file.Write(wuXml);
        }
        else
        {
            CUnicodeFile file;
            file.Open(filePath.c_str());
            file.Read(wuXml);
        }
        return wuXml.c_str();
    }

    const TCHAR * SaveWorkunitXML(const std::_tstring & wuid, std::_tstring & filePath) const
    {
        clib::recursive_mutex::scoped_lock proc(m_mutex);
        filePath = pathToWString((m_workingFolderPath / (wuid + _T(".xml"))));
        if (!clib::filesystem::exists(filePath))
        {
            std::_tstring wuXml;
            GetWorkunitXML(wuid, wuXml);
            if (!wuXml.empty())
            {
                CUnicodeFile file;
                if (file.Create(filePath.c_str()))
                    file.Write(wuXml);
            }
        }
        return filePath.c_str();
    }

    const TCHAR * GetWorkunitResults(const std::_tstring & wuid, bool compileOnly, std::_tstring & results, bool & hasErrors, Dali::CEclExceptionVector & errors) const
    {
        clib::recursive_mutex::scoped_lock proc(m_mutex);
        boost::filesystem::wpath resultPath = m_workingFolderPath / (wuid + _T("-result.xml"));
        boost::filesystem::wpath exePath = m_workingFolderPath / (wuid + _T(".exe"));
        if (clib::filesystem::exists(exePath) && !clib::filesystem::exists(resultPath))
        {
            if (compileOnly)
            {
                results = _T("");//_T("<Dataset/>");
            }
            else
            {
                std::_tstring command = pathToWString(exePath);
                command += _T(" ");
                command += (const TCHAR *)CString(m_config->Get(GLOBAL_COMPILER_WUARGUMENTS));
                command += _T(" -xml");
                std::_tstring err;
                runProcess(command, m_workingFolder, pathToWString(m_compilerFolderPath), _T(""), results, err);
                ParseErrors(_T("POIOIUPOIPOIPOIPOIPOIP"), err, hasErrors, errors, true);
            }
            CUnicodeFile file;
            file.Create(pathToWString(resultPath).c_str());
            file.Write(results);
        }
        else if (clib::filesystem::exists(resultPath))
        {
            CUnicodeFile file;
            file.Open(pathToWString(resultPath).c_str());
            file.Read(results);
        }
        return results.c_str();
    }

    const TCHAR * Compile(const std::_tstring & path, const std::_tstring & ecl, const std::_tstring & wuid, int resultLimit, const std::_tstring & debugString, std::_tstring & wuXml, bool & hasErrors, Dali::CEclExceptionVector & errors) const
    {
        clib::recursive_mutex::scoped_lock proc(m_mutex);
        StdStringVector args;
        args.push_back(_T("q"));
        if (resultLimit > 0)
        {
            args.push_back(_T("fapplyInstantEclTransformations=1"));
            args.push_back((boost::_tformat(_T("fapplyInstantEclTransformationsLimit=%1%")) % resultLimit).str());
        }
        typedef std::vector<std::_tstring> split_vector_type;
        split_vector_type SplitVec; 
        algo::split(SplitVec, debugString, algo::is_any_of("\r\n"), algo::token_compress_on);
        for(split_vector_type::iterator itr = SplitVec.begin(); itr != SplitVec.end(); ++itr)
        {
            algo::trim(*itr);
            if (!itr->empty())
                args.push_back(_T("f") + *itr);
        }
        args.push_back((boost::_tformat(_T("o\"%1%\"")) % wuid).str());

        std::_tstring out, err;
        hasErrors = false;
        CallEclCC(_T(""), _T(""), path, ecl, args, out, err, hasErrors, errors);

        if (!hasErrors)
        {
            GetWorkunitXML(wuid, wuXml);
        }

        return wuXml.c_str();
    }

    bool autoFindManifest(const boost::filesystem::path & file, boost::filesystem::path & manifestPath) const
    {
        manifestPath = file;
        manifestPath.replace_extension("manifest");
        if (clib::filesystem::exists(manifestPath))
            return true;

        manifestPath = manifestPath.remove_filename() / "files" / "manifest.xml";
        if (clib::filesystem::exists(manifestPath))
            return true;

        return false;
    }

    bool GetArchive(const std::_tstring & path, const std::_tstring & ecl, std::_tstring & archive, Dali::CEclExceptionVector & errors) const
    {
        clib::recursive_mutex::scoped_lock proc(m_mutex);
        StdStringVector args;
        args.push_back(_T("E"));

        boost::filesystem::path filePath = stringToPath(path);
        boost::filesystem::path manifestPath;
        if(autoFindManifest(filePath, manifestPath))
            args.push_back(_T("manifest=") + pathToWString(manifestPath));

        std::_tstring err;
        bool hasErrors = false;
        CallEclCC(_T(""), _T(""), path, ecl, args, archive, err, hasErrors, errors);

        return hasErrors == false;
    }

    const TCHAR * GetAttributeLabel(IAttribute * attr, std::_tstring & label) const
    {
        clib::recursive_mutex::scoped_lock proc(m_mutex);
        label = attr->GetQualifiedLabel();
        algo::replace_all(label, _T("."), _T("/"));
        label += _T(".");
        label += attr->GetType()->GetRepositoryCode();
        return label.c_str();
    }

    unsigned int GetPaths(StringPathMap & paths) const
    {
        clib::recursive_mutex::scoped_lock proc(m_mutex);
        std::_tstring command = m_compilerFile + _T(" -showpaths");
        boost::filesystem::path tempPath;
        GetTempFolder(tempPath);
        std::_tstring in, out, err;
        runProcess(command, pathToWString(tempPath), _T(""), in, out, err);
        typedef std::vector<std::_tstring> split_vector_type;
        split_vector_type lines;
        boost::algorithm::split(lines, out, boost::algorithm::is_any_of(_T("\r\n")), boost::algorithm::token_compress_on);
        for (split_vector_type::const_iterator itr = lines.begin(); itr != lines.end(); ++itr)
        {
            split_vector_type assignment;
            boost::algorithm::split(assignment, *itr, boost::algorithm::is_any_of(_T("=")), boost::algorithm::token_compress_on);
            if (assignment.size() == 2)
            {
                boost::algorithm::replace_all(assignment[1], _T("/"), _T("\\"));	//  Workaround for HPCC-10111
                boost::algorithm::trim_right_if(assignment[1], boost::algorithm::is_any_of("\\"));
                paths[assignment[0]] = assignment[1];
            }
        }
        return paths.size();
    }

    bool PluginFolderExists(const std::string & attrTypeStr, const std::string & batchFile, boost::filesystem::path & foundFolder, int level, bool pluginFolder) const
    {
        boost::filesystem::path folder, path;
        GetProgramFolder(folder);

        for (int i = 0; i < level; i++)
        {
            if (folder.has_parent_path())
            {
                folder = folder.parent_path();
            }
        }
        if (pluginFolder)
        {
            folder /= "plugin";
        }
        else
        {
            folder /= stringToPath(attrTypeStr);
        }
        path = folder / batchFile;
        if (clib::filesystem::exists(path))
        {
            foundFolder = folder;
            return true;
        }
        return false;
    }

    bool LocatePlugin(const std::string & attrTypeStr, const std::string & batchFile, boost::filesystem::path & foundFolder) const
    {
        if (PluginFolderExists(attrTypeStr, batchFile, foundFolder, 0, true))
        {
            return true;
        }
        else if (PluginFolderExists(attrTypeStr, batchFile, foundFolder, 2, false))
        {
            return true;
        }
        else if (PluginFolderExists(attrTypeStr, batchFile, foundFolder, 3, false))
        {
            return true;
        }
        else if (LocateIDEPlugin(batchFile, foundFolder)) {
            return true;
        }
        if (!boost::algorithm::iequals(batchFile, "ecl.bat")) {
            _DBGLOG(LEVEL_WARNING, (boost::format("Plugin not found - %1%") % batchFile).str().c_str());
        }
        return false;
    }

    PREPROCESS_TYPE StringToProcessType(std::_tstring processTypeStr) const
    {
        PREPROCESS_TYPE processType = PREPROCESS_UNKNOWN;
        if (::boost::iequals(processTypeStr, "SyntaxCheck"))
        {
            processType = PREPROCESS_SYNTAXCHECK;
        }
        return processType;
    }

    boost::filesystem::wpath GetIDEPluginFolder() const
    {
        return m_IDEPluginsFolderPath;
    }
};
typedef StlLinked<CEclCC> CEclCCAdapt;
typedef std::vector<CEclCCAdapt> CEclCCVector;
class CEclCCCompare
{
protected:
    SMC::IVersionCompare m_versionCompare;

public:
    bool operator ()(CEclCCAdapt & l, CEclCCAdapt & r)
    {
        return m_versionCompare(l->GetBuild(), r->GetBuild());
    }
};
void EnableLocalRepository(TRI_BOOL enable)
{
    g_EnableCompiler = enable;
}

TRI_BOOL IsLocalRepositoryEnabled()
{
    return g_EnableCompiler;
}

void EnableRemoteDaliEnabled(TRI_BOOL enable)
{
    g_EnableRemoteDali = enable;
}

TRI_BOOL IsRemoteDaliEnabled()
{
    return g_EnableRemoteDali;
}

void EnableRemoteQueueEnabled(TRI_BOOL enable)
{
    g_EnableRemoteQueue = enable;
}

TRI_BOOL IsRemoteQueueEnabled()
{
    return g_EnableRemoteQueue;
}

CacheT<std::_tstring, CEclCC> EclCCCache;
CEclCCVector g_foundCompilers;
CComPtr<IEclCC> g_eclcc;
CComPtr<IEclCC> g_eclccBest;
CComPtr<IEclCC> g_eclccCombo;
void ClearEclCCSingletons()
{
    EclCCCache.Clear();
    g_eclcc = NULL;
}

CEclCC * CreateEclCCRaw(IConfig * config, const std::_tstring & compilerFile)
{
    return EclCCCache.Get(new CEclCC(config, compilerFile));
}

boost::filesystem::path EclCCSubPath(const boost::filesystem::path & directory)
{
    return directory / "clienttools" / "bin" / "eclcc.exe";
}

void PushUniqueEclCC(CEclCCVector& results, const boost::filesystem::path & eclccPath)
{
    bool found = std::any_of(results.begin(), results.end(), [&eclccPath](const CEclCCAdapt& obj) {return (_tcsicmp(obj.get()->GetCompilerFilePath().c_str(), eclccPath.c_str()) == 0); });
    if (!found) {
        CComPtr<IConfig> config = GetIConfig(QUERYBUILDER_CFG);
        results.push_back(CreateEclCCRaw(config, pathToWString(eclccPath)));
    }
}

unsigned int FindAllCEclCC(const boost::filesystem::path & progFiles, CEclCCVector & results)
{
    if (exists(progFiles)) {
        using namespace boost::filesystem;
        {   //  Locate clienttools installs  ---
            path testFolder = progFiles / "HPCCSystems";
            if (exists(testFolder)) {
                directory_iterator end_itr;
                for (directory_iterator itr(testFolder); itr != end_itr; ++itr)
                {
#if (BOOST_FILESYSTEM_VERSION == 3)
                    path eclccPath = EclCCSubPath(wpathToPath(*itr));
#else
                    path eclccPath = EclCCSubPath(itr->path());
#endif
                    if (exists(eclccPath))
                        PushUniqueEclCC(results, eclccPath);
                }
            }
        }

        {   //  Locate 6.x ECL IDE installs  (older than 4.x) ---
            path testFolder = progFiles / "HPCC Systems" / "HPCC" / "bin";
            if (exists(testFolder)) {
                directory_iterator end_itr;
                for (directory_iterator itr(testFolder); itr != end_itr; ++itr)
                {
#if (BOOST_FILESYSTEM_VERSION == 3)
                    path eclccPath = *itr / "eclcc.exe";
#else
                    path eclccPath = itr->path() / "eclcc.exe";
#endif
                    if (exists(eclccPath))
                        PushUniqueEclCC(results, eclccPath);
                }
            }
        }
        std::sort(results.begin(), results.end(), CEclCCCompare());
    }
    return results.size();
}

unsigned int FindRelative(const boost::filesystem::path & progFiles, CEclCCVector & results)
{
    using namespace boost::filesystem;
    boost::filesystem::path current = boost::filesystem::current_path();
    path eclccPath = current / "eclcc.exe";
    CComPtr<IConfig> config = GetIConfig(QUERYBUILDER_CFG);

    for (int i = 0; i < 3; i++)
    {
        current = current.parent_path();
        path eclccPath = EclCCSubPath(current);
        if (exists(eclccPath)) {
            _DBGLOG(LEVEL_INFO, (boost::_tformat(_T("eclcc.exe relative path:  %1%")) % eclccPath.c_str()).str().c_str());
            PushUniqueEclCC(results,eclccPath);
        }
    }
    return results.size();
}

unsigned int FindAllCEclCC(CEclCCVector & results)
{
    boost::filesystem::path progFiles;
    GetProgramFilesX86Folder(progFiles);
    FindAllCEclCC(progFiles, results);
    GetProgramFilesFolder(progFiles);
    FindAllCEclCC(progFiles, results);
    FindRelative(progFiles, results);
    return results.size();
}

void RescanClients() {
    g_foundCompilers.clear();
    FindAllCEclCC(g_foundCompilers);
}

const TCHAR * const warningTpl = _T("Compiler/Server mismatch:\r\nCompiler:\t%1%\r\nServer:\t%2%");
IEclCC * CreateIEclCC(bool force)
{
    if (!force && g_EnableCompiler != TRI_BOOL_TRUE)
        return NULL;

    if (g_eclcc)
        return g_eclcc;

    if (!g_foundCompilers.size())
        FindAllCEclCC(g_foundCompilers);

    GetBestMatch();

    CComPtr<IConfig> config = GetIConfig(QUERYBUILDER_CFG);
    CString client = CString(config->Get(GLOBAL_COMPILER_LOCATION));
    bool override = config->Get(GLOBAL_COMPILER_OVERRIDEDEFAULTSELECTION);
    if (override && !client.IsEmpty()) {
        g_eclcc = CreateEclCCRaw(config, (const TCHAR*)client);
    }
    else {
        g_eclcc = g_eclccBest;
    }
    return g_eclcc;
}

IEclCC* GetBestMatch() {
    if (!g_eclccBest) {
        g_eclccBest = MatchVersion();
    }
    return g_eclccBest;
}

IEclCC* SetComboToBest() {
    if (g_eclccBest) {
        g_eclccCombo = g_eclccBest;
    }
    return g_eclccCombo;
}


IEclCC* GetCurrentCompiler() {
    return g_eclcc;
}

IEclCC* SetCurrentClient() {
    CComPtr<IConfig> config = GetIConfig(QUERYBUILDER_CFG);
    CString client = CString(config->Get(GLOBAL_COMPILER_LOCATION));
    bool override = config->Get(GLOBAL_COMPILER_OVERRIDEDEFAULTSELECTION);

    if (!override && g_eclccBest)
        g_eclcc = g_eclccBest;
    else if (override && client.GetLength()) {
        CComPtr<IEclCC> eclcc = CompilerFromExePath(client.GetString());
        if (!eclcc)
            g_eclcc = CreateEclCCRaw(config, (const TCHAR*)client);
    }

    return g_eclcc;
}

IEclCC * MatchVersion() {
    CComPtr<IConfig> config = GetIConfig(QUERYBUILDER_CFG);
    CComPtr<SMC::ISMC> smc = SMC::AttachSMC(GetIConfig(QUERYBUILDER_CFG)->Get(GLOBAL_SERVER_SMC), _T("SMC"));
    CComPtr<SMC::IVersion> serverVersion = smc->GetBuild();
    bool serverEmpty = serverVersion->IsZeroVersion();
    if (g_foundCompilers.size())
    {
        SMC::IVersionCompare versionCompare;
        StlLinked<CEclCC> bestMatchEclCC_before;
        StlLinked<CEclCC> bestMatchEclCC_after;
        StlLinked<CEclCC> latestGoodEclCC;
        StlLinked<CEclCC> matchedEclCC;

        for (CEclCCVector::const_iterator itr = g_foundCompilers.begin(); itr != g_foundCompilers.end(); ++itr)
        {
            if (!itr->get()->IsExecutableBad()) {
                latestGoodEclCC = itr->get();
                if (!serverEmpty && versionCompare.equals(serverVersion.p, itr->get()->GetBuild()))
                {
                    matchedEclCC = itr->get();
                    break;
                }
                if (!serverEmpty && versionCompare(serverVersion.p, itr->get()->GetBuild()))
                {
                    bestMatchEclCC_after = itr->get();
                    break;
                }
            }
            bestMatchEclCC_before = itr->get();
        }
        if (matchedEclCC) {
            g_eclccBest= matchedEclCC;
            return g_eclccBest;
        }
            
        if (bestMatchEclCC_after && !bestMatchEclCC_after->IsExecutableBad() && versionCompare.distance(serverVersion, bestMatchEclCC_after->GetBuild()) < SMC::IVersionCompare::DISTANCE_POINT)
            matchedEclCC = bestMatchEclCC_after;

        if (bestMatchEclCC_before && !bestMatchEclCC_before->IsExecutableBad() && versionCompare.distance(serverVersion, bestMatchEclCC_before->GetBuild()) < SMC::IVersionCompare::DISTANCE_POINT)
            matchedEclCC = bestMatchEclCC_before;

        if (matchedEclCC)
        {
            std::_tstring eclccBuildStr, serverVersionStr;
            matchedEclCC->m_warnings = (boost::_tformat(warningTpl) % matchedEclCC->GetBuild()->GetString(eclccBuildStr) % serverVersion->GetString(serverVersionStr)).str();
            g_eclccBest = matchedEclCC;
            return g_eclccBest;
        }

        //  No good match, just return latest  ---
        matchedEclCC = latestGoodEclCC;
        std::_tstring eclccBuildStr, serverVersionStr;
        const std::_tstring errors = (boost::_tformat(warningTpl) % matchedEclCC->GetBuild()->GetString(eclccBuildStr) % serverVersion->GetString(serverVersionStr)).str();
        matchedEclCC->m_errors = errors;
        matchedEclCC->m_errors += _T("\r\n(To prevent this message from showing, either download and install the matching client tools package or override the default compiler settings in the preferences window)\r\n");
        g_eclccBest = matchedEclCC;
        return g_eclccBest;
    }
    return NULL;
}

std::wstring GetCompilers(std::vector<std::wstring> &strVec) {
    StlLinked<CEclCC> matchedEclCC;
    if (g_foundCompilers.size() > 0) {
        std::wstring version;
        for (CEclCCVector::const_iterator itr = g_foundCompilers.begin(); itr != g_foundCompilers.end(); ++itr)
        {
            itr->get()->GetVersionString(version);
            if (version.length()) {
                boost::algorithm::trim(version);
                strVec.push_back(version);
            }
        }
        g_eclcc->GetVersionString(version);
        boost::algorithm::trim(version);
        std::sort(strVec.begin(), strVec.end());
        return version;
    }
    return _T("");
}

IEclCC* CompilerFromVersion(const std::_tstring& versionStr) {
    std::wstring version;
    for (CEclCCVector::const_iterator itr = g_foundCompilers.begin(); itr != g_foundCompilers.end(); ++itr)
    {
        itr->get()->GetVersionString(version);
        if (version.length() && _tcsicmp(version.c_str(), versionStr.c_str()) == 0) {
            g_eclccCombo = *itr;
            return *itr;
        }
    }
    return NULL;
}

IEclCC* CompilerFromExePath(const std::_tstring& exePath) {
    std::wstring path;
    for (CEclCCVector::const_iterator itr = g_foundCompilers.begin(); itr != g_foundCompilers.end(); ++itr)
    {
        path = itr->get()->GetCompilerFilePath();
        if (path.length() && _tcsicmp(path.c_str(), exePath.c_str()) == 0) {
            return *itr;
        }
    }
    return NULL;
}