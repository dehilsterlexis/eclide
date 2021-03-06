project( COMMS )

set ( SRCS
		Account.cpp
		Account.h
		#Active.cpp
		#Active.h
		#Analysis.cpp
		#Analysis.h
		#attrHelper.cpp
		#attrHelper.h
		Attribute.cpp
		Attribute.h
		AttributeImpl.h
		AttributeHistory.cpp
		AttributeHistory.h
		AttributeType.cpp
		AttributeType.h
		Autoupdate.cpp
		Autoupdate.h
		Cluster.cpp
		Cluster.h
		comms.rc
		comms.cpp
		comms.h
		Dali.cpp
		Dali.h
		Depository.h
		dfu.cpp
		dfu.h
		dfuFile.cpp
		dfuFile.h
		dfuServer.cpp
		dfuServer.h
		#DfuWorkunit.cpp
		#DfuWorkunit.h
		DiskAttribute.h
		DiskAttribute.cpp
		DiskAttributeHistory.cpp
		DiskModule.h
		DiskModule.cpp
		DiskMonitor.h
		DiskMonitor.cpp
		DiskRepository.cpp
		DiskRepository.h
		DropZone.cpp
		DropZone.h
		EclCC.cpp
		EclCC.h
		EclServer.cpp
		EclServer.h
		#Entity.cpp
		#Entity.h
		#EntityInstance.cpp
		#EntityInstance.h
		#FileSpray.cpp
		#FileSpray.h
		#FileSprayFile.cpp
		#FileSprayFile.h
		#FileSprayPath.cpp
		#FileSprayPath.h
		Graph.cpp
		Graph.h
		Group.cpp
		Group.h
		GSoapUtil.cpp
		GSoapUtil.h
		#HtmlHelper.cpp
		#HtmlHelper.h
		LocalFile.cpp
		LocalFile.h
		LocalResult.cpp
		LocalResult.h
		LocalWorkunit.cpp
		LocalWorkunit.h
		machine.h
		meta.h
		Migration.cpp
		Migration.h
		ModFileAttribute.cpp
		ModFileAttributeHistory.cpp
		ModFileModule.cpp
		ModFileRepository.cpp
		ModFileRepository.h
		Module.cpp
		Module.h
		ModuleHelper.cpp
		ModuleHelper.h
		Repository.cpp
		Repository.h
		RepositoryImpl.h
		RequestQueue.h
		resource.h
		Result.cpp
		Result.h
		ResultRequestQueue.cpp
		ResultRequestQueue.h
		SecureSoapSocketClient.cpp
		SecureSoapSocketClient.h
		SMC.cpp
		SMC.h
		SMCCluster.cpp
		SMCCluster.h
		SMCVersion.cpp
		SMCVersion.h
		Topology.cpp
		Topology.h
		WebService.cpp
		#WebPage.cpp
		#WebPage.h
		WebService.h
		Workspace.cpp
		Workspace.h
		WorkspaceItem.cpp
		WorkspaceItem.h
		Workunit.cpp
		Workunit.h
		#WsAccount.h
		#WsAnalysis.h
		#WsAttributes.h
		#WsDfu.h
		#WsEclNet.h
		#WsEclNetHelper.cpp
		#WsEclNetHelper.h
		#WsECLWorkunits.h
		#WsEntityClassify.h
		#WsEntityEnhance.h
		#WsEnvironment.h
		#WsFileSpray.h
		#WsRepository.h
		#WsSEEP.h
		#WsSMC.h
		#WsTopology.h
		#WsUpdate.h
		#WsUpdateHelper.cpp
		#WsUpdateHelper.h
		#WsUsers.h
		#WsWorkunits.h
		#WsWorkunitsEx.h
		XGMMLWorkunit.cpp
		XGMMLWorkunit.h
	)

include_directories (
	../clib
	../ecllib

	${WTL_INCLUDE_DIR}/include

	${GSOAP_INCLUDE_DIRS}
	${GSOAP_INCLUDE_DIRS}/extras
	${GSOAP_INCLUDE_DIRS}/mod_gsoap/gsoap_win/wininet
	)

if (Boost_FOUND)
	include_directories ( ${Boost_INCLUDE_DIRS} )
	link_directories ( ${Boost_LIBRARY_DIRS} )
endif (Boost_FOUND)

add_definitions( -DWITH_SOAPDEFS_H )
ADD_MSVC_PRECOMPILED_HEADER("stdafx.h" "stdafx.cpp" SRCS)
add_library ( COMMS SHARED 
		${SRCS}

		${GSOAP_INCLUDE_DIRS}/stdsoap2.cpp
		${GSOAP_INCLUDE_DIRS}/mod_gsoap/gsoap_win/wininet/gsoapWinInet.cpp
		${GSOAP_INCLUDE_DIRS}/mod_gsoap/gsoap_win/wininet/gsoapWinInet.h
	)

add_dependencies ( COMMS
		CLIB
	)

target_link_libraries( COMMS
		CLIB
		ECLLIB
		msxml2.lib
		comsupp.lib
		authz.lib
	)

install ( TARGETS COMMS RUNTIME DESTINATION bin )	