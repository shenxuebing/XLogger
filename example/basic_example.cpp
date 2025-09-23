#include "XLogger.hpp"
#include <iostream>
#include <thread>
#include <chrono>

void testConsoleLogging()
{
	std::cout << "=== 控制台日志测试 ===" << std::endl;

	// 配置为控制台输出
	XLogger::setLogConsole(true);
	XLogger::setLogLevel("trace");

	XLOG_INFO("控制台信息日志");
	XLOG_WARN("控制台警告日志");
	XLOG_ERROR("控制台错误日志");
	XLOG_CRITICAL("控制台严重错误日志");

	// 测试函数日志
	XLOG_BEGIN;
	XLOG_INFO("控制台函数开始执行");
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	XLOG_INFO("控制台函数执行完成");
	XLOG_END;

	// 测试内存日志
	unsigned char buffer[16];
	for (int i = 0; i < 16; ++i)
	{
		buffer[i] = i;
	}
	XLOG_MEMHEX(buffer, 16);
}

void testFileLogging()
{
	std::cout << "\n=== 文件日志测试 ===" << std::endl;

	// 配置为文件输出
	XLogger::setLogConsole(false);
	XLogger::setLogLevel("trace");
	XLogger::setLogPath("./logs");
	XLogger::setLogPrefixName("basic_test");

	XLOG_TRACE("文件跟踪日志");
	XLOG_DEBUG("文件调试日志，参数: {}", 123);
	XLOG_INFO("文件信息日志");
	XLOG_WARN("文件警告日志");
	XLOG_ERROR("文件错误日志");
	XLOG_CRITICAL("文件严重错误日志");

	// 测试函数日志
	XLOG_BEGIN;
	XLOG_INFO("文件函数开始执行");
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	XLOG_INFO("文件函数执行完成");
	XLOG_END;

	// 测试内存日志
	unsigned char buffer[16];
	for (int i = 0; i < 16; ++i)
	{
		buffer[i] = i + 16;
	}
	XLOG_MEMHEX(buffer, 16);
}

void testDifferentLevels()
{
	std::cout << "\n=== 不同日志级别测试 ===" << std::endl;

	// 启用控制台输出进行级别测试
	XLogger::setLogConsole(true);

	// 测试不同级别的日志
	XLogger::setLogLevel("trace");
	XLOG_TRACE("TRACE级别日志");

	XLogger::setLogLevel("debug");
	XLOG_DEBUG("DEBUG级别日志");

	XLogger::setLogLevel("trace");
	XLOG_INFO("INFO级别日志");

	XLogger::setLogLevel("warn");
	XLOG_WARN("WARN级别日志");

	XLogger::setLogLevel("error");
	XLOG_ERROR("ERROR级别日志");

	XLogger::setLogLevel("critical");
	XLOG_CRITICAL("CRITICAL级别日志");
}

int main()
{
#ifdef _WIN32
	system("CHCP 65001"); // 改变UTF-8编码
#endif					  // _WIN32
	std::cout << "XLogger 基本示例程序" << std::endl;
	std::cout << "===================" << std::endl;

	// 初始配置
	XLogger::setLogMaxFiles(5);
	XLogger::setLogMaxSize(1); // 1MB

	std::cout << "日志配置:" << std::endl;
	std::cout << "- 最大文件数: 5" << std::endl;
	std::cout << "- 最大文件大小: 1MB" << std::endl;

	// 运行独立测试
	testConsoleLogging();
	testFileLogging();
	testDifferentLevels();

	std::cout << "\n基本示例程序执行完成！" << std::endl;
	std::cout << "控制台日志已显示在上方" << std::endl;
	std::cout << "文件日志已保存到 ./logs 目录" << std::endl;

	return 0;
}
