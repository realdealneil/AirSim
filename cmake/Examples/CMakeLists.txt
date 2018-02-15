cmake_minimum_required(VERSION 3.5.0)
project(Examples)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake-modules") 
INCLUDE("${CMAKE_CURRENT_LIST_DIR}/../cmake-modules/CommonSetup.cmake")
CommonSetup()

IncludeEigen()

SetupConsoleBuild()

## GStreamer 1.0 package required
# This website was helpful in finding the right way to use pkgconfig in CMake for gstreamer:
# https://lists.freedesktop.org/archives/gstreamer-devel/2016-October/061111.html

find_package(PkgConfig)

pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.0
							   gstreamer-app-1.0>=1.0)

find_package( OpenCV REQUIRED )

## Specify additional locations of header files
include_directories(
  ${GST_INCLUDE_DIRS}
  ${AIRSIM_ROOT}/Examples
  ${AIRSIM_ROOT}/AirLib/include
  ${RPC_LIB_INCLUDES}
  ${AIRSIM_ROOT}/Unreal/Plugins/AirSim/Source/AirLib/include
  ${AIRSIM_ROOT}/MavLinkCom/include
  ${AIRSIM_ROOT}/MavLinkCom/common_utils
)

AddExecutableSource()
			
CommonTargetLink()
target_link_libraries(${PROJECT_NAME} 
	AirLib
	${RPC_LIB}
	${GST_LIBRARIES}
	${OpenCV_LIBS}
)
