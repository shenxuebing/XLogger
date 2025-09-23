#include "../XLogger.hpp"
#include <thread>
#include <vector>
#include <chrono>

void workerThread(int threadId)
{
	for (int i = 0; i < 5; ++i)
	{
		XLOG_INFO("线程 {} 执行第 {} 次操作", threadId, i + 1);

		// 模拟一些工作
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		XLOG_DEBUG("线程 {} 完成第 {} 次操作", threadId, i + 1);
	}
}

int main()
{
#ifdef _WIN32
	system("CHCP 65001"); // 改变UTF-8编码
#endif					  // _WIN32
	std::cout << "=== 多线程日志示例 ===" << std::endl;

	// 配置日志
	XLogger::setLogPath("./logs");
	XLogger::setLogPrefixName("multithread");
	XLogger::setLogLevel("info");
	XLogger::setLogConsole(true);

	XLOG_INFO("开始多线程测试");

	// 创建多个线程
	std::vector<std::thread> threads;
	const int numThreads = 4;

	for (int i = 0; i < numThreads; ++i)
	{
		threads.emplace_back(workerThread, i + 1);
	}

	// 等待所有线程完成
	for (auto &thread : threads)
	{
		thread.join();
	}

	XLOG_INFO("多线程测试完成");

	return 0;
}
