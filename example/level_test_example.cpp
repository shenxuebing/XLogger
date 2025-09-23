#include "../XLogger.hpp"
#include <iostream>

void testNumericLevels()
{
#ifdef _WIN32
	system("CHCP 65001"); // 改变UTF-8编码
#endif					  // _WIN32
	std::cout << "=== 数值日志级别测试 ===" << std::endl;

	// 配置日志
	XLogger::setLogPath("./logs");
	XLogger::setLogPrefixName("numeric_levels");
	XLogger::setLogLevel("trace");
	XLogger::setLogConsole(true);

	std::cout << "测试所有数值级别的日志输出:" << std::endl;

	// 测试所有数值级别
	std::vector<std::pair<int, std::string>> levels = {
		{-1, "off"},
		{0, "trace"},
		{1, "debug"},
		{2, "info"},
		{3, "warn"},
		{4, "error"},
		{5, "critical"}};

	for (const auto &level : levels)
	{
		std::cout << "\n--- 设置级别为 " << level.first << " (" << level.second << ") ---" << std::endl;

		// 使用数值设置级别
		XLogger::setLogLevel(level.first);

		// 输出所有级别的日志
		XLOG_TRACE("这是TRACE级别日志");
		XLOG_DEBUG("这是DEBUG级别日志");
		XLOG_INFO("这是INFO级别日志");
		XLOG_WARN("这是WARN级别日志");
		XLOG_ERROR("这是ERROR级别日志");
		XLOG_CRITICAL("这是CRITICAL级别日志");

		std::cout << "当前级别: " << XLogger::getLogLevel() << std::endl;
	}
}

void testStringLevels()
{
	std::cout << "\n=== 字符串日志级别测试 ===" << std::endl;

	// 测试字符串级别设置
	std::vector<std::string> levelNames = {
		"off", "trace", "debug", "info", "warn", "error", "critical"};

	for (const auto &levelName : levelNames)
	{
		std::cout << "\n--- 设置级别为 " << levelName << " ---" << std::endl;

		XLogger::setLogLevel(levelName);

		XLOG_TRACE("TRACE日志");
		XLOG_DEBUG("DEBUG日志");
		XLOG_INFO("INFO日志");
		XLOG_WARN("WARN日志");
		XLOG_ERROR("ERROR日志");
		XLOG_CRITICAL("CRITICAL日志");

		std::cout << "当前级别: " << XLogger::getLogLevel() << std::endl;
	}
}

void testLevelComparison()
{
	std::cout << "\n=== 级别比较测试 ===" << std::endl;

	// 设置不同级别并观察输出
	std::cout << "设置级别为 info (数值: 2)，应该只显示 info, warn, error, critical" << std::endl;
	XLogger::setLogLevel(2);

	XLOG_TRACE("这条TRACE日志不应该显示");
	XLOG_DEBUG("这条DEBUG日志不应该显示");
	XLOG_INFO("这条INFO日志应该显示");
	XLOG_WARN("这条WARN日志应该显示");
	XLOG_ERROR("这条ERROR日志应该显示");
	XLOG_CRITICAL("这条CRITICAL日志应该显示");

	std::cout << "\n设置级别为 error (数值: 4)，应该只显示 error, critical" << std::endl;
	XLogger::setLogLevel(4);

	XLOG_TRACE("这条TRACE日志不应该显示");
	XLOG_DEBUG("这条DEBUG日志不应该显示");
	XLOG_INFO("这条INFO日志不应该显示");
	XLOG_WARN("这条WARN日志不应该显示");
	XLOG_ERROR("这条ERROR日志应该显示");
	XLOG_CRITICAL("这条CRITICAL日志应该显示");
}

int main()
{
	std::cout << "XLogger 日志级别测试程序" << std::endl;
	std::cout << "=========================" << std::endl;

	testNumericLevels();
	testStringLevels();
	testLevelComparison();

	std::cout << "\n日志级别测试完成！" << std::endl;

	return 0;
}
