cmake_minimum_required(VERSION 3.10)
project(XLogger VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 添加spdlog子模块
add_subdirectory(third_party/spdlog)

# 创建XLogger库目标（头文件库）
add_library(XLogger INTERFACE)
target_include_directories(XLogger INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# 链接spdlog
target_link_libraries(XLogger INTERFACE spdlog::spdlog)

# 选项：是否启用在头文件中读取配置文件（INI）
option(XLOGGER_ENABLE_CONFIG "Enable reading ini config in XLogger.hpp" OFF)
if(XLOGGER_ENABLE_CONFIG)
    target_compile_definitions(XLogger INTERFACE XLOGGER_ENABLE_CONFIG=1)
endif()

# 添加示例
option(BUILD_EXAMPLES "Build examples" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

# 安装/打包暂时关闭，避免导出依赖子模块目标造成错误
# 如需安装，请改为使用 find_package(spdlog REQUIRED) 并在安装包配置中调用 find_dependency(spdlog)
