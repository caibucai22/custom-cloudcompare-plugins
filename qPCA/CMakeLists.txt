# CloudCompare example for standard plugins

# REPLACE ALL 'ExamplePlugin' OCCURENCES BY YOUR PLUGIN NAME
# AND ADAPT THE CODE BELOW TO YOUR OWN NEEDS!

# Add an option to CMake to control whether we build this plugin or not
option( PLUGIN_qPCA "Install PCA plugin" OFF )

if ( PLUGIN_qPCA )
	project( QPCA_PLUGIN )
	 
	AddPlugin( NAME ${PROJECT_NAME} )
		
	add_subdirectory( include )
	add_subdirectory( src )
	add_subdirectory( ui )
	include_directories("${CMAKE_CURRENT_LIST_DIR}/extern/eigen3")
	# set dependencies to necessary libraries
	# target_link_libraries( ${PROJECT_NAME} LIB1 )
endif()
