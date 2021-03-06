set ROOT_DIR=D:\hpccsystems
set SRC_DIR=%ROOT_DIR%\src
set ECLIDE_DIR=%SRC_DIR%\eclide
set BUILD_DIR=%ROOT_DIR%\build

set STAGE_DIR=%ROOT_DIR%\stage
set PF_DIR=%STAGE_DIR%\PF
set HPCC_DIR=%PF_DIR%\HPCC
set BIN_DIR=%HPCC_DIR%\bin
set VER_3_0_SRCDIR=%ECLIDE_DIR%\ver_3_0
set VER_3_0_DIR=%BIN_DIR%\ver_3_0
set VER_3_6_DIR=%HPCC_DIR%\ver_3_6
set VER_4_0_DIR=%HPCC_DIR%\ver_4_0

set CD_DIR=%STAGE_DIR%\CD
set CD_ECL_DIR=%CD_DIR%\ECL
set SAMPLES_DIR=%CD_ECL_DIR%\Samples

set release=release
set builddir=%TEMP%
set externaldir=%ROOT_DIR%\externals
set lndir=%SRC_DIR%\ln
set build_lndir=%BUILD_DIR%\LN
set hpccdir=%SRC_DIR%\HPCC-Platform
set build_hpccdir=%BUILD_DIR%\HPCC-Platform

set HPCC_SAMPLES_DIR=%hpccdir%\initfiles\examples
set HPCC_GITSAMPLES_DIR=%SRC_DIR%\ecl-samples

mkdir %STAGE_DIR%
mkdir %PF_DIR%
mkdir %HPCC_DIR%
mkdir %BIN_DIR%
mkdir %VER_DIR%

mkdir %CD_DIR%
mkdir %CD_ECL_DIR%
mkdir %SAMPLES_DIR%

set LN=0

CALL :DOCLEAN
set cmakedir=%build_lndir%
call %lndir%\build\windows\release_plugins.bat

set cmakedir=%build_hpccdir%
call %lndir%\build\windows\release_eclcc.bat
call %lndir%\build\windows\release_dfu.bat

rem robocopy /S "%VER_3_0_SRCDIR%" "%VER_3_0_DIR%"
robocopy /S %TEMP%\release\bin "%VER_4_0_DIR%\bin"
robocopy /S %TEMP%\release\libs "%VER_4_0_DIR%\bin"
robocopy /S %TEMP%\release\dfuplus "%VER_4_0_DIR%\bin"
robocopy /S %TEMP%\release\eclcc "%VER_4_0_DIR%\bin"
copy "%cmakedir%\bin\release\ecl.exe" "%VER_4_0_DIR%\bin\ecl.exe"

if not %LN%==1 rmdir /Q /S "%VER_4_0_DIR%\ecllibrary\ln"
if %LN%==1 robocopy /S "%TEMP%\release\plugins\metaphone" "%VER_4_0_DIR%\plugins"
if %LN%==1 robocopy /S "%TEMP%\release\plugins\saltlib" "%VER_4_0_DIR%\plugins"
copy "%build_lndir%\bin\release\ecl-package.exe" "%VER_4_0_DIR%\bin\ecl-package.exe"
copy "%build_lndir%\bin\release\ecl-queries.exe" "%VER_4_0_DIR%\bin\ecl-queries.exe"

robocopy /S /MOVE "%VER_4_0_DIR%\bin\cl" "%VER_4_0_DIR%\componentfiles\cl"
robocopy /S /MOVE "%VER_4_0_DIR%\bin\ecllibrary" "%VER_4_0_DIR%\share\ecllibrary"
robocopy /S /MOVE "%VER_4_0_DIR%\bin\plugins" "%VER_4_0_DIR%\plugins"
move /Y "%VER_4_0_DIR%\bin\*.?pp" "%VER_4_0_DIR%\componentfiles"
rmdir /S /Q "%VER_4_0_DIR%\bin\vc2008"

robocopy "%HPCC_SAMPLES_DIR%" "%SAMPLES_DIR%" *.ecl /S 
robocopy "%HPCC_GITSAMPLES_DIR%" "%SAMPLES_DIR%" *.* /S /xd ".git" 

CALL :DOCLEAN
GOTO :EOF

:DOCLEAN
rmdir /Q /S temp 2>nul
rmdir /Q /S %TEMP%\release 2>nul
rmdir /Q /S bin 2>nul
rmdir /Q /S samples 2>nul
GOTO :EOF

