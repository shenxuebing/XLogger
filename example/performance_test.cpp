#include "../XLogger.hpp"
#include <chrono>
#include <vector>
#include <random>
#include <numeric>

void performanceTest()
{
#ifdef _WIN32
	system("CHCP 65001"); // 改变UTF-8编码
#endif					  // _WIN32
	std::cout << "=== 性能测试示例 ===" << std::endl;

	// 配置日志
	XLogger::setLogPath("./logs");
	XLogger::setLogPrefixName("performance");
	XLogger::setLogLevel("info");
	XLogger::setLogConsole(false); // 禁用控制台输出以提高性能

	const int numLogs = 100000;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, 1000);

	std::cout << "开始性能测试，将记录 " << numLogs << " 条日志..." << std::endl;

	auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < numLogs; ++i)
	{
		int value = dis(gen);
		XLOG_INFO("性能测试 #{}: 随机值 = {}", i, value);

		if (i % 10000 == 0)
		{
			XLOG_DEBUG("进度: {}/{}", i, numLogs);
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	XLOG_INFO("性能测试完成");

	std::cout << "性能测试结果:" << std::endl;
	std::cout << "- 总日志数: " << numLogs << std::endl;
	std::cout << "- 总耗时: " << duration.count() << " 毫秒" << std::endl;
	std::cout << "- 平均每条日志: " << (double)duration.count() / numLogs << " 毫秒" << std::endl;
	std::cout << "- 日志速率: " << (double)numLogs / duration.count() * 1000 << " 条/秒" << std::endl;
}

void memoryUsageTest()
{
	std::cout << "\n=== 内存使用测试 ===" << std::endl;

	// 测试大量内存数据记录
	std::vector<unsigned char> largeBuffer(1024 * 1024); // 1MB缓冲区
	std::iota(largeBuffer.begin(), largeBuffer.end(), 0);

	XLOG_INFO("开始内存使用测试");

	auto start = std::chrono::high_resolution_clock::now();

	// 记录内存数据（只记录前64字节）
	XLOG_MEMHEX(largeBuffer.data(), 64);

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	XLOG_INFO("内存使用测试完成，耗时: {} 微秒", duration.count());

	std::cout << "内存测试完成，耗时: " << duration.count() << " 微秒" << std::endl;
}

int main()
{
	performanceTest();
	memoryUsageTest();

	return 0;
}
