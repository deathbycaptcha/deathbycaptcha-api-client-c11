cmake_minimum_required(VERSION 3.21)

project(deathbycaptcha-c VERSION 4.7.0 LANGUAGES C)

include(GNUInstallDirs)

option(DBC_BUILD_SHARED "Build the shared library" ON)
option(DBC_BUILD_CLIENT "Build the command-line client" ON)
option(DBC_BUILD_EXAMPLE "Build the dynamic loading example" ON)
option(DBC_BUILD_TESTS "Build unit tests" ON)
option(DBC_BUILD_INTEGRATION_TESTS "Build and register live integration tests that require real DBC credentials" OFF)
option(DBC_ENABLE_HTTPS "Enable HTTPS transport via libcurl when available" ON)
option(DBC_ENABLE_COVERAGE "Enable gcov coverage flags" OFF)

set(DBC_PUBLIC_HEADERS
    src/base64.h
    src/cJSON.h
    src/deathbycaptcha.h
)

set(DBC_CORE_SOURCES
    src/base64.c
    src/cJSON.c
    src/deathbycaptcha.c
)

add_library(dbc_warnings INTERFACE)
target_compile_features(dbc_warnings INTERFACE c_std_11)

if(MSVC)
    target_compile_options(dbc_warnings INTERFACE /W4 /permissive-)
else()
    target_compile_options(dbc_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

if(DBC_ENABLE_COVERAGE AND NOT MSVC)
    target_compile_options(dbc_warnings INTERFACE --coverage -O0 -g)
    target_link_options(dbc_warnings INTERFACE --coverage)
endif()

find_package(Threads REQUIRED)

set(DBC_PLATFORM_LIBS Threads::Threads)
set(DBC_OPTIONAL_LIBS)

if(DBC_ENABLE_HTTPS)
    find_package(CURL QUIET)
    if(CURL_FOUND)
        list(APPEND DBC_OPTIONAL_LIBS CURL::libcurl)
        add_compile_definitions(DBC_HAS_CURL=1)
        message(STATUS "DBC HTTPS transport enabled (libcurl found)")
    else()
        add_compile_definitions(DBC_HAS_CURL=0)
        message(STATUS "DBC HTTPS transport disabled (libcurl not found)")
    endif()
else()
    add_compile_definitions(DBC_HAS_CURL=0)
endif()

if(WIN32)
    list(APPEND DBC_PLATFORM_LIBS ws2_32)
else()
    list(APPEND DBC_PLATFORM_LIBS m)
    list(APPEND DBC_PLATFORM_LIBS ${CMAKE_DL_LIBS})
endif()

function(dbc_configure_target target_name)
    target_link_libraries(${target_name} PRIVATE dbc_warnings ${DBC_PLATFORM_LIBS} ${DBC_OPTIONAL_LIBS})
    target_include_directories(${target_name}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    if(NOT WIN32)
        target_compile_definitions(${target_name} PRIVATE _POSIX_C_SOURCE=200112L)
    endif()

    set_target_properties(${target_name} PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
        POSITION_INDEPENDENT_CODE ON
    )
endfunction()

if(DBC_BUILD_SHARED)
    add_library(deathbycaptcha_shared SHARED ${DBC_CORE_SOURCES})
    dbc_configure_target(deathbycaptcha_shared)
    target_compile_definitions(deathbycaptcha_shared PRIVATE DBC_BUILDING_DLL=1)
    set_target_properties(deathbycaptcha_shared PROPERTIES
        OUTPUT_NAME deathbycaptcha
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    install(TARGETS deathbycaptcha_shared
        EXPORT deathbycaptchaTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

if(DBC_BUILD_CLIENT)
    add_executable(deathbycaptcha_cli src/client.c ${DBC_CORE_SOURCES})
    dbc_configure_target(deathbycaptcha_cli)
    set_target_properties(deathbycaptcha_cli PROPERTIES
        OUTPUT_NAME deathbycaptcha
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    install(TARGETS deathbycaptcha_cli
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if(DBC_BUILD_EXAMPLE)
    add_executable(deathbycaptcha_socket_example doc/lib_usage_socket_example.c)
    dbc_configure_target(deathbycaptcha_socket_example)
    target_link_libraries(deathbycaptcha_socket_example PRIVATE ${CMAKE_DL_LIBS})
    set_target_properties(deathbycaptcha_socket_example PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    add_executable(deathbycaptcha_https_example doc/lib_usage_https_example.c)
    dbc_configure_target(deathbycaptcha_https_example)
    target_link_libraries(deathbycaptcha_https_example PRIVATE ${CMAKE_DL_LIBS})
    set_target_properties(deathbycaptcha_https_example PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

if(DBC_BUILD_TESTS)
    enable_testing()

    add_executable(deathbycaptcha_tests tests/deathbycaptcha_tests.c ${DBC_CORE_SOURCES})
    dbc_configure_target(deathbycaptcha_tests)
    set_target_properties(deathbycaptcha_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )

    add_test(NAME deathbycaptcha_tests COMMAND deathbycaptcha_tests)

    if(DBC_ENABLE_COVERAGE)
        add_executable(deathbycaptcha_lib_coverage_harness
            tests/deathbycaptcha_coverage_harness.c
            src/base64.c
            src/cJSON.c
        )
        target_include_directories(deathbycaptcha_lib_coverage_harness PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/fake_curl
        )
        target_compile_definitions(deathbycaptcha_lib_coverage_harness PRIVATE DBC_HAS_CURL=1)
        target_link_libraries(deathbycaptcha_lib_coverage_harness PRIVATE dbc_warnings ${DBC_PLATFORM_LIBS})
        if(NOT WIN32)
            target_compile_definitions(deathbycaptcha_lib_coverage_harness PRIVATE _POSIX_C_SOURCE=200112L)
        endif()
        set_target_properties(deathbycaptcha_lib_coverage_harness PROPERTIES
            C_STANDARD 11
            C_STANDARD_REQUIRED YES
            C_EXTENSIONS NO
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
        add_test(NAME deathbycaptcha_lib_coverage_harness COMMAND deathbycaptcha_lib_coverage_harness)

        add_executable(deathbycaptcha_cli_coverage_harness tests/cli_coverage_harness.c)
        target_include_directories(deathbycaptcha_cli_coverage_harness PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
        target_link_libraries(deathbycaptcha_cli_coverage_harness PRIVATE dbc_warnings ${DBC_PLATFORM_LIBS})
        if(NOT WIN32)
            target_compile_definitions(deathbycaptcha_cli_coverage_harness PRIVATE _POSIX_C_SOURCE=200112L)
        endif()
        set_target_properties(deathbycaptcha_cli_coverage_harness PROPERTIES
            C_STANDARD 11
            C_STANDARD_REQUIRED YES
            C_EXTENSIONS NO
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
        add_test(NAME deathbycaptcha_cli_coverage_harness COMMAND deathbycaptcha_cli_coverage_harness)
    endif()

    if(DBC_BUILD_INTEGRATION_TESTS)
        if(DBC_BUILD_CLIENT)
            if(UNIX)
                add_test(
                    NAME deathbycaptcha_cli_integration
                    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration_cli_test.sh
                            $<TARGET_FILE:deathbycaptcha_cli>
                            ${CMAKE_CURRENT_SOURCE_DIR}
                            ${CMAKE_CURRENT_SOURCE_DIR}/.env
                )
                set_tests_properties(deathbycaptcha_cli_integration PROPERTIES
                    SKIP_RETURN_CODE 77
                )
            else()
                message(STATUS "DBC integration test registration is currently supported on UNIX hosts only")
            endif()
        else()
            message(STATUS "DBC_BUILD_INTEGRATION_TESTS=ON ignored because DBC_BUILD_CLIENT=OFF")
        endif()
    endif()
endif()

install(FILES ${DBC_PUBLIC_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/deathbycaptcha
)

if(TARGET deathbycaptcha_shared)
    install(EXPORT deathbycaptchaTargets
        FILE deathbycaptchaTargets.cmake
        NAMESPACE deathbycaptcha::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/deathbycaptcha
    )
endif()