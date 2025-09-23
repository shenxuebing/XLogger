#include "../XLogger.hpp"
#include <iomanip>
#include <sstream>
#include <fstream>

void testCustomFormat()
{
	std::cout << "=== 自定义格式示例 ===" << std::endl;

	// 配置日志
	XLogger::setLogPath("./logs");
	XLogger::setLogPrefixName("custom_format");
	XLogger::setLogLevel("info");
	XLogger::setLogConsole(true);

	// 获取logger实例来自定义格式
	auto logger = XLogger::getInstance()->getLogger();

	// 自定义格式：时间戳 + 级别 + 消息
	logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

	XLOG_INFO("使用自定义格式的日志消息");
	XLOG_WARN("警告消息");
	XLOG_ERROR("错误消息");

	// 更详细的格式：包含线程ID和文件名
	logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%P] [%t] [%^%l%$] [%!] %v (%s:%#)");

	XLOG_INFO("包含线程ID和文件名的详细格式");
	XLOG_DEBUG("调试信息");

	// 简单格式
	logger->set_pattern("%^%l%$: %v");

	XLOG_INFO("简单格式的日志");
	XLOG_WARN("警告");
	XLOG_ERROR("错误");
	XLOG_CRITICAL("严重错误");
}

void testStructuredLogging()
{
	std::cout << "\n=== 结构化日志示例 ===" << std::endl;

	// 模拟结构化数据记录
	struct UserInfo
	{
		int id;
		std::string name;
		std::string email;
	};

	UserInfo user1 = {1, "张三", "zhangsan@example.com"};
	UserInfo user2 = {2, "李四", "lisi@example.com"};

	XLOG_INFO("用户登录: ID={}, 姓名={}, 邮箱={}", user1.id, user1.name, user1.email);
	XLOG_INFO("用户登录: ID={}, 姓名={}, 邮箱={}", user2.id, user2.name, user2.email);

	// 模拟API调用日志
	XLOG_INFO("API调用: GET /api/users/{}", user1.id);
	XLOG_DEBUG("API响应: 状态码=200, 响应时间=150ms");

	XLOG_INFO("API调用: POST /api/users");
	XLOG_DEBUG("API响应: 状态码=201, 响应时间=200ms");
}

void testErrorHandling()
{
	std::cout << "\n=== 错误处理示例 ===" << std::endl;

	try
	{
		// 模拟一个可能出错的操作
		int divisor = 0;
		if (divisor == 0)
		{
			XLOG_ERROR("除零错误: 尝试除以零");
			throw std::runtime_error("除零错误");
		}
	}
	catch (const std::exception &e)
	{
		XLOG_ERROR("捕获异常: {}", e.what());
		XLOG_CRITICAL("严重错误: 程序可能无法继续运行");
	}

	// 模拟文件操作错误
	try
	{
		std::ifstream file("不存在的文件.txt");
		if (!file.is_open())
		{
			XLOG_WARN("文件不存在: 不存在的文件.txt");
			XLOG_INFO("使用默认配置继续运行");
		}
	}
	catch (const std::exception &e)
	{
		XLOG_ERROR("文件操作异常: {}", e.what());
	}
}

int main()
{
#ifdef _WIN32
	system("CHCP 65001"); // 改变UTF-8编码
#endif					  // _WIN32
	testCustomFormat();
	testStructuredLogging();
	testErrorHandling();

	std::cout << "\n自定义格式示例完成！" << std::endl;

	return 0;
}
