#----------------------------------------------------------------------------
# Set up colors
if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(ColourBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
  set(BoldRed     "${Esc}[1;31m")
  set(BoldGreen   "${Esc}[1;32m")
  set(BoldYellow  "${Esc}[1;33m")
  set(BoldBlue    "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan    "${Esc}[1;36m")
  set(BoldWhite   "${Esc}[1;37m")
endif()

#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(HigsFrame)

#----------------------------------------------------------------------------
# finde required packages
#find_package(X11 REQUIRED)

#----------------------------------------------------------------------------
# find the root package (and add COMPONENTS)
find_package(ROOT)# REQUIRED COMPONENTS Gui GuiHtml Minuit Spectrum OPTIONAL_COMPONENTS MathMore)

# check the root version (ROOT_VERSION=6.32.02, ROOT_VERSION_COUNT=3, ROOT_VERSION_MAJOR=6, ROOT_VERSION_MINOR=32, ROOT_VERSION_PATCH=2, ROOT_VERSION_TWEAK=0)
if(NOT ${ROOT_VERSION_MAJOR} EQUAL "6")
	message(FATAL_ERROR "The ROOT version installed (${ROOT_VERSION}) is wront, the major version needs to be 6 for this to work (ROOT_VERSION_MAJOR=${ROOT_VERSION_MAJOR})!")
endif()

#----------------------------------------------------------------------------
# set c++ standard to the one used for root (adopted from agataselector's CMakeLists.txt)
if(NOT ROOT_CXX_VERSION)
	if(ROOT_CXX_STANDARD)
      set(ROOT_CXX_VERSION ${ROOT_CXX_STANDARD})
	else()
		string(REGEX REPLACE ".*-std=.+\\+\\+([0-9][0-9]).*" "\\1" ROOT_CXX_VERSION ${ROOT_CXX_FLAGS})
		if(${CMAKE_MATCH_COUNT} EQUAL "1")
			message("Got ROOT_CXX_VERSION=${ROOT_CXX_VERSION} from \"${ROOT_CXX_FLAGS}\"")
		else()
			message(FATAL_ERROR "The ROOT cxx flags seem to be misformed (${ROOT_CXX_FLAGS}) (CMAKE_MATCH_COUNT=${CMAKE_MATCH_COUNT}), consider providing a cxx version by hand using \"-DROOT_CXX_VERSION=??\"")
		endif()
	endif()
else()
	message("Got user provided ROOT_CXX_VERSION=${ROOT_CXX_VERSION}")
endif()

if(${ROOT_CXX_VERSION} LESS 14)
	message(FATAL_ERROR "${CMAKE_PROJECT_NAME} requires at least c++14, please consider installing a newer ROOT version that was compiled with at least c++14")
endif()

set(CMAKE_CXX_STANDARD ${ROOT_CXX_VERSION})
message("${BoldBlue}C++ version set to: ${CMAKE_CXX_STANDARD}\n${ColourReset}")
set(CMAKE_CXX_STANDARD_REQUIRED True)

#----------------------------------------------------------------------------
# configure the include directories and set other variables (could maybe have just one include_directories for both linux and macOS?)
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	include_directories(${PROJECT_SOURCE_DIR}/include ${ROOT_INCLUDE_DIRS} ${X11_INCLUDE_DIR} /opt/local/include)
	add_compile_options(-DOS_DARWIN)
else()
	include_directories(${PROJECT_SOURCE_DIR}/include ${ROOT_INCLUDE_DIRS})
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_INSTALL_LIBDIR ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_INSTALL_BINDIR ${CMAKE_BINARY_DIR}/bin)

#----------------------------------------------------------------------------
# copy some necessary text files
# this will always overwrite the files, including any changes in them
# tried using file(COPY, file(GENERATE, and add_custom_command to get around this for grsirc, no success
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/thishigs.sh)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/thishigs.sh  ${CMAKE_CURRENT_BINARY_DIR}/thishigs.sh  COPYONLY)
else()
	message("${CMAKE_CURRENT_SOURCE_DIR}/thishigs.sh not found?")
endif()

#----------------------------------------------------------------------------
# copy includes, and examples
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/examples DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

#----------------------------------------------------------------------------
# define all libraries and root dictionaries
# for some we also create dependencies on other libraries to remove linking errors later on

add_library(Higs SHARED
	${PROJECT_SOURCE_DIR}/src/BasicHelper.cxx
	${PROJECT_SOURCE_DIR}/src/BasicFrame.cxx
	${PROJECT_SOURCE_DIR}/src/DataFrameLibrary.cxx
	${PROJECT_SOURCE_DIR}/src/Calibration.cxx
	)
	root_generate_dictionary(G__Higs BasicHelper.h BasicFrame.h DataFrameLibrary.h Calibration.h CustomMap.h Globals.h Options.h Redirect.h Singleton.h MODULE Higs LINKDEF ${PROJECT_SOURCE_DIR}/src/LinkDef.h)
target_link_libraries(Higs ${ROOT_LIBRARIES})

#----------------------------------------------------------------------------
# Add the executable, and link it to the generated libraries and the ROOT libraries
add_executable(HigsFrame ${PROJECT_SOURCE_DIR}/src/HigsFrame.cxx)

target_link_libraries(HigsFrame Higs ${ROOT_LIBRARIES} ${X11_LIBRARIES} ${X11_Xpm_LIB})

#----------------------------------------------------------------------------
# clean up all copied files and directories
# we're using grsisort as target here, because most (all?) of these do not belong to a specific target
set_property(TARGET HigsFrame APPEND PROPERTY ADDITIONAL_CLEAN_FILES
	${CMAKE_CURRENT_BINARY_DIR}/thishigs.sh
	${CMAKE_CURRENT_BINARY_DIR}/module.modulemap
	${CMAKE_INSTALL_BINDIR}
	${CMAKE_INSTALL_LIBDIR}
	)

#----------------------------------------------------------------------------
# create a compile_commands.json file which can be used for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
if(NOT DEFINED ENV{CMAKE_EXPORT_COMPILE_COMMANDS})
	message("ENV(CMAKE_EXPORT_COMPILE_COMMANDS) NOT DEFINED, setting it to TRUE")
	set($ENV{CMAKE_EXPORT_COMPILE_COMMANDS} TRUE)
endif()
