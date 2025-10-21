cmake_minimum_required(VERSION 3.14)
set(PROJECT_NAME Internal1-Sun)
set(PROJECT_VERSION 1.0.0)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)


# --- 定位仓库根：优先用根目录传下来的 MONO_ROOT；否则向上两级猜测（apps_internal/<app> -> .. -> repo） ---
if(NOT DEFINED MONO_ROOT)
  get_filename_component(_app_dir  "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)
  get_filename_component(_apps_dir "${_app_dir}/.." ABSOLUTE)   # apps_internal
  get_filename_component(MONO_ROOT "${_apps_dir}/.." ABSOLUTE)  # repo root
endif()

set(_APP_OUT "${MONO_ROOT}/out/apps/${PROJECT_NAME}")


# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 强制让 vs 将源文件以 utf-8 格式解码
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()


add_definitions(-DWINNT)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)

# 设置输出目录
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../../out/apps/${PROJECT_NAME})

# Windows平台特定设置
if(WIN32)
    # 确保插件能找到主程序导出的符号
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    # 启用异常处理
    add_compile_options(/EHsc)
endif()


set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(QT_ROOT "C:/Qt/6.7.3/msvc2022_64" CACHE STRING "Qt root directory" FORCE)
# set(QT_ROOT "C:/Qt/6.9.1/msvc2022_64" CACHE STRING "Qt root directory" FORCE) #测试切换
set(CMAKE_PREFIX_PATH "${QT_ROOT}/lib/cmake")
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core Core5Compat)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core Core5Compat)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(
    ../common/
)

add_executable(
    ${PROJECT_NAME}

    main.cpp

    ../common/pack_definition.h
    ../common/ConfigManager.h  ../common/ConfigManager.cpp

    net_mainwindow.cpp
    net_mainwindow.h
    net_mainwindow.ui
    
   SingleInstance_SharedMemory.h
)

# 统一把多配置生成器的 Debug/Release 等也指到同一目录（不再额外生成子目录）
set_target_properties(${PROJECT_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY                "${_APP_OUT}"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${_APP_OUT}"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${_APP_OUT}"
  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${_APP_OUT}"
  RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${_APP_OUT}"
  LIBRARY_OUTPUT_DIRECTORY                "${_APP_OUT}"
  LIBRARY_OUTPUT_DIRECTORY_DEBUG          "${_APP_OUT}"
  LIBRARY_OUTPUT_DIRECTORY_RELEASE        "${_APP_OUT}"
  ARCHIVE_OUTPUT_DIRECTORY                "${_APP_OUT}"
  ARCHIVE_OUTPUT_DIRECTORY_DEBUG          "${_APP_OUT}"
  ARCHIVE_OUTPUT_DIRECTORY_RELEASE        "${_APP_OUT}"
)


target_link_libraries(${PROJECT_NAME}
    PRIVATE       
    Qt${QT_VERSION_MAJOR}::Widgets
     Qt${QT_VERSION_MAJOR}::Core 
      Qt${QT_VERSION_MAJOR}::Core5Compat
)




# qt deploy
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${QT_ROOT}/bin/windeployqt.exe --no-compiler-runtime --no-system-d3d-compiler --no-quick-import --no-translations $<TARGET_FILE:${PROJECT_NAME}>
    COMMENT "Running windeployqt for ${PROJECT_NAME}"
)


# 拷贝，pos zs软件
add_custom_target(copy_20pos ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_APP_OUT}/apps/20pos-zx
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${MONO_ROOT}/3rd_app/20pos-zx"
            "${_APP_OUT}/apps/20pos-zx"
)

add_custom_target(copy_30jihe ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_APP_OUT}/apps/30jihe
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${MONO_ROOT}/3rd_app/30jihe"
            "${_APP_OUT}/apps/30jihe"
)


