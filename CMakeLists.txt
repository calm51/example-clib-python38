﻿cmake_minimum_required(VERSION 3.5)

project(pythondemo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Network Concurrent Charts Multimedia MultimediaWidgets Sql Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Network Concurrent Charts Multimedia MultimediaWidgets Sql Svg)


set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    resource.qrc

)

qt5_add_resources(QT_RESOURCE_FILES ${PROJECT_DIR}/resource.qrc)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(${PROJECT_NAME} SHARED
            ${PROJECT_SOURCES}
        )
    set(ANDROID_MIN_SDK_VERSION 21)
else()
    add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
    )
endif()
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Concurrent Qt${QT_VERSION_MAJOR}::Charts Qt${QT_VERSION_MAJOR}::Multimedia Qt${QT_VERSION_MAJOR}::MultimediaWidgets)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()

# =============================================


set_property(TARGET ${PROJECT_NAME} PROPERTY QT_ANDROID_PRIVATE_KEY_SKIP_VALIDATION ON)

if (WIN32)

elseif (ANDROID)
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS AndroidExtras)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS AndroidExtras)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::AndroidExtras)

elseif (UNIX AND NOT APPLE AND NOT WIN32 AND NOT ANDROID)
    set(CMAKE_CXX_FLAGS "-fno-pie -no-pie ${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "-fno-pie -no-pie ${CMAKE_CXX_FLAGS}")

endif()

add_definitions(-D_HAS_STD_BYTE=0)

# =============================================

#add_definitions(-DPYQTDEPLOY_FROZEN_MAIN=PYQTDEPLOY_OPTIMIZED)
#add_definitions(-DHAVE_EXPAT_CONFIG_H)
#add_definitions(-DPy_BUILD_CORE_MODULE)
#add_definitions(-DUSE_PYEXPAT_CAPI)
#add_definitions(-DXML_DEV_URANDOM)
#add_definitions(-DXML_POOR_ENTROPY)
#add_definitions(-DXML_STATIC)
add_definitions(-D__ANDROID_API__=21)


if (WIN32)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(python38_platform "win32msvc")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(python38_platform "win32mingw")
    endif()
    set(python38_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../clib-python38/output/lib/${python38_platform}/python3.8)
    target_link_directories(${PROJECT_NAME} PRIVATE ${python38_lib_dir})
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(python38_lib1 ${python38_lib_dir}/libcrypto.lib) # TODO
        set(python38_lib2 ${python38_lib_dir}/libssl.lib)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${python38_lib1})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${python38_lib2})
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(python38_lib1 ${python38_lib_dir}/libcrypto-1_1.dll)
        set(python38_lib2 ${python38_lib_dir}/libssl-1_1.dll)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${python38_lib1})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${python38_lib2})
    endif()
elseif (ANDROID)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-llog")
    set(python38_include_3rd ${CMAKE_CURRENT_SOURCE_DIR}/../clib-python38/output/sysroot/include)
    include_directories(${python38_include_3rd})
    #    set(python38_lib_3rd_dir ${CMAKE_CURRENT_SOURCE_DIR}/../clib-python38/output/sysroot/lib)
    #    target_link_directories(${PROJECT_NAME} PRIVATE ${python38_lib_3rd_dir})
    #    set(python38_lib_3rd_so1 ${python38_lib_3rd_dir}/libgdbm.a)
    #    target_link_libraries(${PROJECT_NAME} PRIVATE "gdbm")
    #    target_link_libraries(${PROJECT_NAME} PRIVATE ${python38_lib_3rd_so1})

    set(python38_platform "android")
    set(python38_include ${CMAKE_CURRENT_SOURCE_DIR}/../clib-python38/output/include/${python38_platform}/python3.8)
    include_directories(${python38_include})
    set(python38_include__internal ${CMAKE_CURRENT_SOURCE_DIR}/../clib-python38/output/include/${python38_platform}/python3.8/internal)
    include_directories(${python38_include__internal})

    set(python38_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../clib-python38/output/lib/${python38_platform}/python3.8)
    set(python38_lib1 ${python38_lib_dir}/libpython3.8.so)
    set(python38_lib2 ${python38_lib_dir}/libpython3.so)
    target_link_directories(${PROJECT_NAME} PRIVATE ${python38_lib_dir})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${python38_lib1}" "${CMAKE_CURRENT_BINARY_DIR}/android-build/libs/${ANDROID_ABI}/libpython3.8.so")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${python38_lib2}" "${CMAKE_CURRENT_BINARY_DIR}/android-build/libs/${ANDROID_ABI}/libpython3.so")
    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD COMMAND python3 "${CMAKE_SOURCE_DIR}/script/change_android_settings.py") # PRE_BUILD | PRE_LINK| POST_BUILD
    target_link_libraries(${PROJECT_NAME} PRIVATE ${python38_lib1})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${python38_lib2})

    #    target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_DL_LIBS})
    #    set(python38_liba ${python38_lib_dir}/libpython3.8.a)
    #    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD COMMAND python3 "${CMAKE_SOURCE_DIR}/script/change_android_settings.py") # PRE_BUILD | PRE_LINK| POST_BUILD
    #    target_link_libraries(${PROJECT_NAME} PRIVATE ${python38_liba})


elseif (UNIX AND NOT APPLE AND NOT WIN32 AND NOT ANDROID)
    set(python38_platform "linux64")
    set(python38_include ${CMAKE_CURRENT_SOURCE_DIR}/../clib-python38/output/include/${python38_platform}/python3.8)
    include_directories(${python38_include})
    set(python38_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../clib-python38/output/lib/${python38_platform}/python3.8)
    set(python38_lib1 ${python38_lib_dir}/libpython3.8.so.1.0)
    set(python38_lib2 ${python38_lib_dir}/libpython3.so)
    target_link_directories(${PROJECT_NAME} PRIVATE ${python38_lib_dir})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${python38_lib1})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${python38_lib2})
endif()


# =============================================

if (ANDROID)
    set(qtandroidcls_include ${CMAKE_CURRENT_SOURCE_DIR}/include)
    include_directories(${qtandroidcls_include})

    set(qtandroidcls_platform "android")
    set(qtandroidcls_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/lib/${qtandroidcls_platform}/qtandroidcls)
    set(qtandroidcls_lib ${qtandroidcls_lib_dir}/libqtandroidcls_armeabi-v7a.so)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${qtandroidcls_lib}" "${CMAKE_CURRENT_BINARY_DIR}/android-build/libs/${ANDROID_ABI}/libqtandroidcls_armeabi-v7a.so")
    target_link_directories(${PROJECT_NAME} PRIVATE ${qtandroidcls_lib_dir})
    target_link_libraries(${PROJECT_NAME} PRIVATE "qtandroidcls_armeabi-v7a")
endif()

# =============================================

if (ANDROID)
    set(qtpyandassets_include ${CMAKE_CURRENT_SOURCE_DIR}/../qtpyandassets/output/include)
    include_directories(${qtpyandassets_include})

    set(qtpyandassets_platform "android")
    set(qtpyandassets_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../qtpyandassets/output/lib/${qtpyandassets_platform}/qtpyandassets)
    set(qtpyandassets_lib ${qtpyandassets_lib_dir}/libqtpyandassets_armeabi-v7a.so)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${qtpyandassets_lib}" "${CMAKE_CURRENT_BINARY_DIR}/android-build/libs/${ANDROID_ABI}/libqtpyandassets_armeabi-v7a.so")
    target_link_directories(${PROJECT_NAME} PRIVATE ${qtpyandassets_lib_dir})
    target_link_libraries(${PROJECT_NAME} PRIVATE "qtpyandassets_armeabi-v7a")
endif()

# =============================================

set(openssl_include ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${openssl_include})

if (ANDROID)
    set(openssl_platform "android")
    set(openssl_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/lib/${openssl_platform}/openssl)
    set(openssl_lib1 ${openssl_lib_dir}/libcrypto.so.1.1)
    set(openssl_lib2 ${openssl_lib_dir}/libssl.so.1.1)
    target_link_directories(${PROJECT_NAME} PRIVATE ${openssl_lib_dir})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${openssl_lib1}" "${CMAKE_CURRENT_BINARY_DIR}/android-build/libs/${ANDROID_ABI}/libcrypto_1_1.so")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${openssl_lib2}" "${CMAKE_CURRENT_BINARY_DIR}/android-build/libs/${ANDROID_ABI}/libssl_1_1.so")
    #    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME} POST_BUILD COMMAND python3 "${CMAKE_SOURCE_DIR}/script/change_android_settings.py") # PRE_BUILD | PRE_LINK| POST_BUILD
    target_link_libraries(${PROJECT_NAME} PRIVATE ${openssl_lib1})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${openssl_lib2})
endif()

# =============================================

