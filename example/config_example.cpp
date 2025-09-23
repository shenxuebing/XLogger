#include "../XLogger.hpp"
#include <iostream>
#include <chrono>
#include <thread>

int main()
{
#ifdef _WIN32
	system("CHCP 65001"); // 改变UTF-8编码，便于中文输出
#endif					  // _WIN32

	std::cout << "XLogger 配置文件示例" << std::endl;
	std::cout << "===================" << std::endl;

#ifndef XLOGGER_ENABLE_CONFIG
	std::cout << "未启用配置读取功能。请在CMake配置时添加 -DXLOGGER_ENABLE_CONFIG=ON 后重试。" << std::endl;
	return 0;
#else
	// 指定配置文件路径（默认已为 defConf.ini，可省略）
	XLogger::setConfPath("defConf.ini");

	// 可选：先设置一个默认的输出位置与级别，读取到配置后会覆盖
	XLogger::setLogPath("./logs");
	XLogger::setLogPrefixName("config_example");
	XLogger::setLogLevel("info");

	// 访问logger以便触发构造与配置读取（单例构造在首次使用时完成）
	XLogger::getInstance()->getLogger();

	std::cout << "当前日志级别: " << XLogger::getLogLevel() << std::endl;

	// 记录一些日志，便于在不同配置下观察输出
	XLOG_TRACE("[config] 这是一条TRACE日志");
	XLOG_DEBUG("[config] 这是一条DEBUG日志");
	XLOG_INFO("[config] 这是一条INFO日志");
	XLOG_WARN("[config] 这是一条WARN日志");
	XLOG_ERROR("[config] 这是一条ERROR日志");
	XLOG_CRITICAL("[config] 这是一条CRITICAL日志");

	// 演示一次函数进入/退出
	XLOG_BEGIN;
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	XLOG_END;

	std::cout << "示例结束，请查看控制台或 ./logs 下的日志文件。" << std::endl;
	return 0;
#endif // XLOGGER_ENABLE_CONFIG
}
