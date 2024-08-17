include_directories("${CMAKE_CURRENT_LIST_DIR}/../extern/eigen3")

target_sources( ${PROJECT_NAME}
	PRIVATE
		#${CMAKE_CURRENT_LIST_DIR}/ActionA.h
		${CMAKE_CURRENT_LIST_DIR}/qPCA.h
		${CMAKE_CURRENT_LIST_DIR}/ccPCADlg.h

		
)

target_include_directories( ${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)
