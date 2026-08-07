#include "XLogger.hpp"
#include <iostream>
#include <string>

// 模拟一次登录：口令、证件号属于敏感信息，不应直接落到日志
void mockLogin(const std::string &user, const std::string &password, long long idCard)
{
	XLOG_INFO("用户登录: user={}, password(sm4)={}, idCard(sm4)={}",
			  user, XLOG_SM4_EN(password), XLOG_SM4_EN(idCard));

	// 使用专用宏自动输出 "变量名(sm4)=密文"，便于在日志中定位字段
	XLOG_SENSITIVE_FIELD(password);
	XLOG_SENSITIVE_FIELD(idCard);
}

// 演示直接调用静态接口：把脱敏后的密文用于非日志场景（例如落库、上报）
void saveMaskedToken(const std::string &token)
{
	std::string masked = XLogger::encryptSensitiveData(token);
	if (masked.empty())
	{
		XLOG_WARN("token 为空，未生成密文");
		return;
	}
	XLOG_INFO("token 已脱敏，密文长度={}", masked.size());
	// 实际项目中可在此处将 masked 写入数据库或转发给后端
}

// 演示内存检查：以十六进制 + ASCII 形式打印二进制缓冲区
void dumpBuffer()
{
	unsigned char buffer[32];
	for (int i = 0; i < 32; ++i)
	{
		buffer[i] = static_cast<unsigned char>('A' + i); // ABCDEFG... 循环可见字符
	}
	XLOG_MEMHEX(buffer, sizeof(buffer));
}

// 演示 DLL / 进程退出场景下的安全访问
void safeAccessAfterShutdown()
{
	if (XLogger::isDestroyed())
	{
		std::cout << "logger 已析构，跳过日志输出" << std::endl;
		return;
	}
	XLOG_INFO("logger 仍在生命周期内，安全输出本条日志");
}

int main()
{
#ifdef _WIN32
	system("CHCP 65001"); // 切换控制台为 UTF-8
#endif					  // _WIN32

	std::cout << "XLogger 敏感数据脱敏与内存检查示例" << std::endl;
	std::cout << "================================" << std::endl;

	// 配置日志：开启控制台、DEBUG 级别，便于观察脱敏密文
	XLogger::setLogConsole(true);
	XLogger::setLogLevel("debug");
	XLogger::setLogPath("./logs");
	XLogger::setLogPrefixName("sensitive_test");

	// 1. 敏感字段脱敏打印
	mockLogin("alice", "P@ssw0rd!", 110101199001011234LL);

	// 2. 直接获取脱敏密文
	saveMaskedToken("Bearer abcdef-1234-5678");

	// 3. 内存十六进制检查
	dumpBuffer();

	// 4. 安全访问检查
	safeAccessAfterShutdown();

	// 5. 同一字段连续两次脱敏：密文每次都不同（含随机盐），可用于验证随机性
	std::string pwd = "same-password";
	std::cout << "\n验证随机性（同一明文两次脱敏密文应不同）:\n"
			  << "  第1次: " << XLogger::encryptSensitiveData(pwd) << "\n"
			  << "  第2次: " << XLogger::encryptSensitiveData(pwd) << std::endl;

	std::cout << "\n敏感数据脱敏示例执行完成！" << std::endl;
	std::cout << "文件日志已保存到 ./logs 目录" << std::endl;
	return 0;
}
