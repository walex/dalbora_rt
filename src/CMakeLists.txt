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
	add_compile_options(/MP) # multi process compilation
	add_compile_options(/Zc:__cplusplus) # get C++ version
endif (MSVC)

add_subdirectory(./core)
add_subdirectory(./os)
add_subdirectory(./rhi)
add_subdirectory(./api)
add_subdirectory(./tests)

set(RT_RENDER_HEADERS ${CORE_HEADERS} ${OS_HEADERS} ${RHI_HEADERS})
set(RT_RENDER_SOURCES ${CORE_SOURCES} ${OS_SOURCES} ${RHI_SOURCES})
set(RT_RENDER_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${API_INCLUDE_DIRS} ${CORE_INCLUDE_DIRS} ${OS_INCLUDE_DIRS} ${RHI_INCLUDE_DIRS})

set(CG_LIBS "")
# DX12 system libraries (Windows)
if (MSVC AND WIN32)
    # link against system DirectX 12 libraries (no .lib extension needed)
    set(CG_LIBS ${CG_LIBS} d3d12 dxgi dxcompiler)
    
    # Find dxcompiler.dll in Windows SDK
    find_file(DXCOMPILER_DLL 
        NAMES dxcompiler.dll
        PATHS 
            "$ENV{WindowsSdkDir}Redist/D3D/$ENV{WindowsSDKVersion}x64"
            "$ENV{WindowsSdkDir}bin/$ENV{WindowsSDKVersion}x64"
            "C:/Program Files (x86)/Windows Kits/10/bin/*/x64"
            "C:/Program Files (x86)/Windows Kits/10/Redist/D3D/*/x64"
        NO_DEFAULT_PATH
    )
else()
    
endif()

set(LINK_LIBS ${CG_LIBS})

# Find all HLSL files in tests directory
file(GLOB HLSL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hlsl")

add_executable(rtrender engine_test_main.cpp ${API_HEADERS} ${RT_RENDER_SOURCES} ${RT_RENDER_HEADERS})
target_include_directories(rtrender PRIVATE ${RT_RENDER_INCLUDE_DIRS})
target_link_libraries(rtrender PRIVATE ${LINK_LIBS})

if (MSVC AND WIN32 AND DXCOMPILER_DLL)
    add_custom_command(TARGET rtrender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DXCOMPILER_DLL}" $<TARGET_FILE_DIR:rtrender>
    )
endif()

# Copy HLSL files to rtrender output directory
foreach(HLSL_FILE ${HLSL_FILES})
    add_custom_command(TARGET rtrender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${HLSL_FILE}" $<TARGET_FILE_DIR:rtrender>
    )
endforeach()

add_executable(api_tests api_tests_main.cpp ${API_HEADERS} ${TEST_SOURCES} ${TEST_HEADERS} ${RT_RENDER_SOURCES} ${RT_RENDER_HEADERS})
target_include_directories(api_tests PRIVATE ${TEST_INCLUDE_DIRS} ${RT_RENDER_INCLUDE_DIRS})
target_link_libraries(api_tests PRIVATE ${LINK_LIBS})

if (MSVC AND WIN32 AND DXCOMPILER_DLL)
    add_custom_command(TARGET api_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DXCOMPILER_DLL}" $<TARGET_FILE_DIR:api_tests>
    )
endif()

# Copy HLSL files to api_tests output directory/shaders
foreach(HLSL_FILE ${HLSL_FILES})
    add_custom_command(TARGET api_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:api_tests>/shaders
        COMMAND ${CMAKE_COMMAND} -E copy
        "${HLSL_FILE}" $<TARGET_FILE_DIR:api_tests>/shaders
    )
endforeach()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TEST_SOURCES} ${API_HEADERS} ${RT_RENDER_SOURCES} ${RT_RENDER_HEADERS})