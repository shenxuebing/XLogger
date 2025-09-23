#include "../XLogger.hpp"
#include <fstream>
#include <iostream>

void testFileLogging()
{
#ifdef _WIN32
	system("CHCP 65001"); // 改变UTF-8编码
#endif					  // _WIN32
	std::cout << "=== 文件日志示例 ===" << std::endl;

	// 配置日志 - 只输出到文件，不输出到控制台
	XLogger::setLogPath("./logs");
	XLogger::setLogPrefixName("file_test");
	XLogger::setLogLevel("trace");
	XLogger::setLogConsole(false); // 禁用控制台输出
	XLogger::setLogMaxFiles(3);
	XLogger::setLogMaxSize(1); // 1MB

	std::cout << "日志配置:" << std::endl;
	std::cout << "- 输出路径: ./logs" << std::endl;
	std::cout << "- 文件前缀: file_test" << std::endl;
	std::cout << "- 日志级别: debug" << std::endl;
	std::cout << "- 控制台输出: 禁用" << std::endl;
	std::cout << "- 最大文件数: 3" << std::endl;
	std::cout << "- 最大文件大小: 1MB" << std::endl;

	// 生成大量日志来测试文件轮转
	for (int i = 0; i < 1000; ++i)
	{
		XLOG_DEBUG("调试信息 #{}: 这是一条很长的调试消息，用来测试文件大小限制和轮转功能", i);
		XLOG_INFO("信息日志 #{}: 处理数据包 {}", i, i * 100);
		XLOG_WARN("警告日志 #{}: 内存使用率较高", i);

		if (i % 100 == 0)
		{
			XLOG_INFO("已处理 {} 条记录", i);
		}
	}

	XLOG_INFO("文件日志测试完成");

	// C++11 环境下不使用 std::filesystem，这里仅提示用户查看目录
	std::cout << "\n已生成日志，请手动查看 ./logs 目录中的 .log 文件。" << std::endl;
}

int main()
{
	std::cout << "XLogger 文件日志示例程序" << std::endl;
	std::cout << "=========================" << std::endl;

	testFileLogging();

	std::cout << "\n文件日志示例完成！" << std::endl;
	std::cout << "请查看 ./logs 目录中的日志文件。" << std::endl;

	return 0;
}
