cmake_minimum_required(VERSION 3.13)

project(magicseteditor VERSION 2.0.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(wxWidgets 3 REQUIRED COMPONENTS core base net html)
find_package(Boost REQUIRED COMPONENTS regex)

# find hunspell
#find_package(HUNSPELL)
find_path(HUNSPELL_INCLUDE_DIRS hunspell/hunspell.hxx)
find_library(HUNSPELL_LIBRARIES NAMES hunspell libhunspell)


include_directories("${PROJECT_BINARY_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src")
include(${wxWidgets_USE_FILE})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${HUNSPELL_INCLUDE_DIRS})

# Magic Set Editor executable

add_executable(magicseteditor WIN32)
target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${HUNSPELL_LIBRARIES})

file(GLOB_RECURSE sources src/*.cpp)
list(FILTER sources EXCLUDE REGEX win32_cli_wrapper.cpp)
target_sources(magicseteditor PRIVATE ${sources})
target_precompile_headers(magicseteditor PRIVATE src/util/prec.hpp)

configure_file(src/config.hpp.in src/config.hpp)

# resource file

target_sources(magicseteditor PRIVATE resource/win32_res.rc)

# magicseteditor.com: wrapper to enable command line executable on windows

if(WIN32)
  add_executable(magicseteditor-cli)
  target_sources(magicseteditor-cli PRIVATE src/cli/win32_cli_wrapper.cpp)
  add_custom_command(TARGET magicseteditor-cli POST_BUILD COMMAND
    ${CMAKE_COMMAND} -E copy "${PROJECT_BINARY_DIR}/magicseteditor-cli.exe" "${PROJECT_BINARY_DIR}/magicseteditor.com"
  )
endif()


# warnings
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wsuggest-override -Wstrict-null-sentinel -Wno-comment -Wno-unused-parameter")
endif()

# visual studio debugger
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT magicseteditor)

# Static linking on windows
if (${VCPKG_TARGET_TRIPLET} MATCHES ".*-static")
  message("Static linking")
  set(CompilerFlags CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE)
  foreach(CompilerFlag ${CompilerFlags})
    string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
  endforeach()
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /GL")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
  # Need explicit dependencies on wx's dependencies
  find_package(png REQUIRED)
  find_package(tiff REQUIRED)
  find_package(jpeg REQUIRED)
  find_package(zlib REQUIRED)
  target_link_libraries(${PROJECT_NAME} ${PNG_LIBRARIES})
  target_link_libraries(${PROJECT_NAME} ${JPEG_LIBRARIES})
  target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES})
  target_link_libraries(${PROJECT_NAME} ${TIFF_LIBRARIES})
  # Defines
  add_definitions(-DSTATIC)
  add_definitions(-DHUNSPELL_STATIC)
endif()

# Test suite
include(test/tests.cmake)
