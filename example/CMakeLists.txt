# 基本示例
add_executable(basic_example basic_example.cpp)
target_link_libraries(basic_example XLogger)

# 多线程示例
add_executable(multithread_example multithread_example.cpp)
target_link_libraries(multithread_example XLogger)

# 文件日志示例
add_executable(file_logging_example file_logging_example.cpp)
target_link_libraries(file_logging_example XLogger)

# 性能测试示例
add_executable(performance_test performance_test.cpp)
target_link_libraries(performance_test XLogger)

# 自定义格式示例
add_executable(custom_format_example custom_format_example.cpp)
target_link_libraries(custom_format_example XLogger)

# 日志级别测试示例
add_executable(level_test_example level_test_example.cpp)
target_link_libraries(level_test_example XLogger)

# 基于配置文件的示例（需配合 -DXLOGGER_ENABLE_CONFIG=ON）
add_executable(config_example config_example.cpp)
target_link_libraries(config_example XLogger)
