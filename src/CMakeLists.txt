cmake_minimum_required(VERSION 3.10) # Or your required version

# Enable C/C++ and Assembly languages
project(rtrender)

set(rtrender_VERSION 1.0.0)

# c++ 20 as standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (MSVC)
	enable_language(ASM_MASM)
else(MSVC)
	enable_language(ASM_NASM)
endif(MSVC)
enable_language(C)

# settings for gnu and clang compilers (used on linux and android)
if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )

	if (${CMAKE_BUILD_TYPE} MATCHES "Debug")

		add_definitions(-DDEBUG)
	else (${CMAKE_BUILD_TYPE} MATCHES "Release")
	
		add_definitions(-DNDEBUG)
	endif (${CMAKE_BUILD_TYPE} MATCHES "Debug")
	 
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	
endif (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )

# settings for ms compiler
if (MSVC)
	
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4 /DDEBUG /EHsc")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W4 /EHsc")		
	add_compile_options(/MP)
endif (MSVC)

add_subdirectory(./core)
add_subdirectory(./os)
add_subdirectory(./rhi)
add_subdirectory(./api)
add_subdirectory(./tests)

# eigen
set(Eigen3_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/eigen-5.0.1/share/eigen3/cmake)
find_package(Eigen3 REQUIRED NO_MODULE)

#slang
set(slang_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../slang-2026.7-windows-x86_64/cmake)
find_package(slang REQUIRED)

set(RT_RENDER_HEADERS ${CORE_HEADERS} ${OS_HEADERS} ${RHI_HEADERS})
set(RT_RENDER_SOURCES ${CORE_SOURCES} ${OS_SOURCES} ${RHI_SOURCES})
set(RT_RENDER_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${API_INCLUDE_DIRS} ${CORE_INCLUDE_DIRS} ${OS_INCLUDE_DIRS} ${RHI_INCLUDE_DIRS})

set(CG_LIBS "")
# DX12 system libraries (Windows)
if (MSVC AND WIN32)
    # link against system DirectX 12 libraries (no .lib extension needed)
    set(CG_LIBS ${CG_LIBS} d3d12 dxgi dxcompiler)
else()
    
endif()

set(LINK_LIBS Eigen3::Eigen slang::slang ${CG_LIBS})

add_executable(rtrender main.cpp ${API_HEADERS} ${RT_RENDER_SOURCES} ${RT_RENDER_HEADERS})
target_include_directories(rtrender PRIVATE ${RT_RENDER_INCLUDE_DIRS} slang::slang)
target_link_libraries(rtrender PRIVATE ${LINK_LIBS})

add_executable(api_tests api_tests_main.cpp ${API_HEADERS} ${TEST_SOURCES} ${TEST_HEADERS} ${RT_RENDER_SOURCES} ${RT_RENDER_HEADERS})
target_include_directories(api_tests PRIVATE ${TEST_INCLUDE_DIRS} ${RT_RENDER_INCLUDE_DIRS} slang::slang)
target_link_libraries(api_tests PRIVATE ${LINK_LIBS})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TEST_SOURCES} ${API_HEADERS} ${RT_RENDER_SOURCES} ${RT_RENDER_HEADERS})