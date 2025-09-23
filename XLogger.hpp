#pragma once
#ifndef XLOGGER_HPP
#define XLOGGER_HPP
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <time.h>

#include "spdlog/async.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h" // or "../stdout_sinks.h" if no color needed
#include "spdlog/spdlog.h"

#ifndef OS_WINDOWS
#if (_WIN32 || _WIN64)
#define OS_WINDOWS 1
#endif
#endif // !OS_WINDOWS

#define LOGBEGIN "begin..."
#define LOGEND "end..."
#define LOGRERURENVALUE "ret:{:d}"

static inline int NowDateToInt()
{
	time_t now;
	time(&now);

	// choose thread save version in each platform
	tm p;
#ifdef OS_WINDOWS
	localtime_s(&p, &now);
#else
	localtime_r(&now, &p);
#endif // OS_WINDOWS
	int now_date = (1900 + p.tm_year) * 10000 + (p.tm_mon + 1) * 100 + p.tm_mday;
	return now_date;
}

static inline int NowTimeToInt()
{
	time_t now;
	time(&now);
	// choose thread save version in each platform
	tm p;
#ifdef OS_WINDOWS
	localtime_s(&p, &now);
#else
	localtime_r(&now, &p);
#endif // OS_WINDOWS

	int now_int = p.tm_hour * 10000 + p.tm_min * 100 + p.tm_sec;
	return now_int;
}

class XLogger
{
  public:
	static XLogger *getInstance()
	{
		static XLogger xlogger;
		return &xlogger;
	}

	std::shared_ptr<spdlog::logger> getLogger()
	{
		return m_logger;
	}
	static void setLogPath(const std::string &logDir)
	{
		getConfig().log_dir = logDir;
	}
	static void setLogPrefixName(const std::string &prefixName)
	{
		getConfig().logger_name_prefix = prefixName;
	}
	static void setConfPath(const std::string &confPath)
	{
		getConfig().conf_path = confPath;
	}
	static void setLogLevel(const std::string &level)
	{
		getConfig().level = level;
		updateLogLevel();
	}
	static void setLogLevel(const int level)
	{
		std::string levelStr;
		switch (level)
		{
		case -1:
			levelStr = "off";
			break;
		case 0:
			levelStr = "trace"; // all
			break;
		case 1:
			levelStr = "debug";
			break;
		case 2:
			levelStr = "info";
			break;
		case 3:
			levelStr = "warn";
			break;
		case 4:
			levelStr = "error";
			break;
		case 5:
			levelStr = "critical";
			break;
		default:
			levelStr = "error";
			break;
		}
		getConfig().level = levelStr;
		updateLogLevel();
	}
	static void setLogMaxFiles(int maxFiles)
	{
		getConfig().max_files = maxFiles;
	}
	static void setLogMaxSize(int maxSize)
	{
		getConfig().max_size = maxSize;
	}
	static void setLogConsole(bool isConsole)
	{
		getConfig().console = isConsole;
	}
	static const std::string &getLogLevel()
	{
		return getConfig().level;
	}
	static void updateLogLevel()
	{
		const std::string& level = getConfig().level;
		if (level == "trace" || level == "all")
		{
			XLogger::getInstance()->m_logger->set_level(spdlog::level::trace);
			XLogger::getInstance()->m_logger->flush_on(spdlog::level::trace);
		}
		else if (level == "debug")
		{
			XLogger::getInstance()->m_logger->set_level(spdlog::level::debug);
			XLogger::getInstance()->m_logger->flush_on(spdlog::level::debug);
		}
		else if (level == "info")
		{
			XLogger::getInstance()->m_logger->set_level(spdlog::level::info);
			XLogger::getInstance()->m_logger->flush_on(spdlog::level::info);
		}
		else if (level == "warn")
		{
			XLogger::getInstance()->m_logger->set_level(spdlog::level::warn);
			XLogger::getInstance()->m_logger->flush_on(spdlog::level::warn);
		}
		else if (level == "error")
		{
			XLogger::getInstance()->m_logger->set_level(spdlog::level::err);
			XLogger::getInstance()->m_logger->flush_on(spdlog::level::err);
		}
		else if (level == "off")
		{
			XLogger::getInstance()->m_logger->set_level(spdlog::level::off);
			XLogger::getInstance()->m_logger->flush_on(spdlog::level::off);
		}
		else // 2023年6月6日17:04:52 沈雪冰 add ，默认error
		{
			XLogger::getInstance()->m_logger->set_level(spdlog::level::err);
			XLogger::getInstance()->m_logger->flush_on(spdlog::level::err);
		}
	}

  private:
	// 配置结构体 - Meyer's Singleton 模式
	struct Config {
		std::string log_dir = "./Log";
		std::string logger_name_prefix = "log";
		std::string conf_path = "defConf.ini";
		std::string level = "all";
		bool console = true;
		int max_size = 10;
		int max_files = 30;
		
		Config() {
#ifdef _DEBUG
			console = true;
#else
			console = false;
#endif
		}
	};
	
	// 获取配置实例
	static Config& getConfig() {
		static Config config;
		return config;
	}
	XLogger()
	{
		try
		{
#if defined(XLOGGER_ENABLE_CONFIG)
			// 从配置文件读取: [CAT] 下的 logLevel/logPath/logMaxSize
			char levelStr[32] = {0};
			char logPath[260] = {0};
			int maxSizeFromConf = 0;
			if (readConfigFile_String(getConfig().conf_path.c_str(), "CAT", "logLevel", levelStr) == 0)
			{
				getConfig().level = levelStr;
			}
			if (readConfigFile_String(getConfig().conf_path.c_str(), "CAT", "logPath", logPath) == 0)
			{
				getConfig().log_dir = logPath;
			}
			if (readConfigFile_Int(getConfig().conf_path.c_str(), "CAT", "logMaxSize", &maxSizeFromConf) == 0)
			{
				if (maxSizeFromConf > 0)
				{
					getConfig().max_size = maxSizeFromConf;
				}
			}
#endif // XLOGGER_ENABLE_CONFIG

			// logger name with timestamp
			int date = NowDateToInt();
			int time = NowTimeToInt();
			const std::string logger_name = getConfig().logger_name_prefix; // +std::to_string(date) + "_" + std::to_string(time); //wfrest20211231_135411.log
			if (getConfig().console)
				m_logger = spdlog::stdout_color_mt("console"); // single thread console output faster
			else
			// m_logger = spdlog::create_async<spdlog::sinks::basic_file_sink_mt>(logger_name, log_dir + "/" + logger_name + ".log"); // only one log file
#ifdef ASYNC_LOG
				m_logger = spdlog::create_async<spdlog::sinks::rotating_file_sink_mt>(getConfig().logger_name_prefix, getConfig().log_dir + "/" + logger_name + ".log", getConfig().max_size * 1024 * 1024, getConfig().max_files); // multi part log files, with every part 500M, max 1000 files
#else
				m_logger = spdlog::daily_logger_mt(getConfig().logger_name_prefix, getConfig().log_dir + "/" + logger_name + ".log", 0, 0, false, getConfig().max_files); // 0点重新创建日志文件,multi part log files, with every part 500M, max 1000 files
#endif // ASYNC_LOG \
	   //m_logger = spdlog::rotating_logger_mt(logger_name, log_dir + "/" + logger_name + ".log", 1024 * 1024 * maxSize, 3);
			/*spdlog::init_thread_pool(8192, 1);
		auto daily_sink = std::make_shared<spdlog::sinks::daily_file_format_sink_mt >(log_dir + "/" + logger_name + ".log", 0, 0, false, 30);
		auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_dir + "/" + logger_name + ".log1", 1024 * 1024 * maxSize, 3);
		std::vector<spdlog::sink_ptr> sinks{ daily_sink, rotating_sink };
		m_logger = std::make_shared<spdlog::async_logger>(logger_name, sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
		spdlog::register_logger(m_logger);*/
#ifndef _DEBUG
			// custom format
			m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [pid:%P] [thread:%t][%!] [%l]  %v(%s:%#)"); // with timestamp, thread_id, filename and line number
#endif
			const std::string& level = getConfig().level;
			if (level == "trace" || level == "all")
			{
				m_logger->set_level(spdlog::level::trace);
				m_logger->flush_on(spdlog::level::trace);
			}
			else if (level == "debug")
			{
				m_logger->set_level(spdlog::level::debug);
				m_logger->flush_on(spdlog::level::debug);
			}
			else if (level == "info")
			{
				m_logger->set_level(spdlog::level::info);
				m_logger->flush_on(spdlog::level::info);
			}
			else if (level == "warn")
			{
				m_logger->set_level(spdlog::level::warn);
				m_logger->flush_on(spdlog::level::warn);
			}
			else if (level == "error")
			{
				m_logger->set_level(spdlog::level::err);
				m_logger->flush_on(spdlog::level::err);
			}
			else if (level == "off")
			{
				m_logger->set_level(spdlog::level::off);
				m_logger->flush_on(spdlog::level::off);
			}
			else // 2023年6月6日17:04:52 沈雪冰 add ，什么都没有就error
			{
				m_logger->set_level(spdlog::level::err);
				m_logger->flush_on(spdlog::level::err);
			}
		}
		catch (const spdlog::spdlog_ex &ex)
		{
			std::cout << "Log initialization failed: " << ex.what() << std::endl;
		}
	}

	~XLogger()
	{
		spdlog::drop_all(); // must do this
	}

	void *operator new(size_t size)
	{
	}

	XLogger(const XLogger &) = delete;
	XLogger &operator=(const XLogger &) = delete;

  private:
	std::shared_ptr<spdlog::logger> m_logger;

  private:
	void err_handler_example()
	{
		// can be set globally or per logger(logger->set_error_handler(..))
		spdlog::set_error_handler([](const std::string &msg)
								  { spdlog::get("console")->error("*** LOGGER ERROR ***: {}", msg); });
		spdlog::get("console")->info("some invalid message to trigger an error {}{}{}{}", 3);
	}

	// 从配置文件读取字符串类型数据
	int readConfigFile_String(const char *filename, const char *title, const char *key, char *value)
	{
		int ret = -1;
		FILE *fp = NULL;
		char szLine[260] = {0};
		static char tmpstr[260] = {0};
		int rtnval = 0;
		int i = 0;
		int flag = 0;
		char *tmp = NULL;
		bool isFirst = true; // 是否为第一次扫描配置文件
		bool isEnd = false;	 // 是否扫描完配置文件
		if ((fp = fopen(filename, "r")) == NULL)
		{
			// printf("没有找到配置文件：%s\n",filename);
			perror(filename);
			ret = -1;
			goto end;
		}
		while (!feof(fp))
		{
			if (isEnd)
				break; // 扫描配置文件结束
			rtnval = fgetc(fp);
			if (rtnval == EOF)
			{
				isEnd = true;
			}
			else
			{
				szLine[i++] = rtnval;
			}
			if (rtnval == '\n' || isFirst == true || isEnd == true) // 第一次扫描配置文件 第一行不用是\n
			{

				if (isFirst == false && isEnd != true) // 是否为第一次扫描文件
				{
					szLine[--i] = '\0';
					i = 0;
				}
				tmp = strchr(szLine, '=');

				if ((tmp != NULL) && (flag == 1))
				{
					if (strstr(szLine, key) != NULL)
					{
						// 注释行
						if ('#' == szLine[0]) // #注释 如#href=0.0.0.0
						{
						}
						else if (0x47 == szLine[0] && 0x47 == szLine[1]) // #注释 如//age=25
						{
						}
						else
						{
							// 找到key对应变量
							strcpy(value, tmp + 1);
							ret = 0;
							goto end;
						}
					}
					else
					{
						memset(szLine, 0, 260);
					}
				}
				else
				{
					strcpy(tmpstr, "[");
					strcat(tmpstr, title);
					strcat(tmpstr, "]");
					if (strncmp(tmpstr, szLine, strlen(tmpstr)) == 0)
					{
						// 找到title
						flag = 1;
					}
				}
				isFirst = false;
				if (szLine[strlen(szLine) - 1] == '\n' || i == 0) // 2019年11月22日19:45:13 沈雪冰 update ||i==0 上边在0a (\n) 时把 0a置为 00了导致不能走到这里，所以加了这个条件
				{
					memset(szLine, 0, 260);
					i = 0;
				}
			}
		}
	end:
		if (fp != NULL)
		{
			fclose(fp);
			fp = NULL;
		}
		while (strcmp(&value[strlen(value) - 1], "\r") == 0 || strcmp(&value[strlen(value) - 1], "\n") == 0) // 去掉/r /n
		{
			value[strlen(value) - 1] = '\0';
		}
		return ret;
	}
	// 从配置文件读取整类型数据
	int readConfigFile_Int(const char *filename, const char *title, const char *key, int *value)
	{
		char value_string[260] = {0};
		if (readConfigFile_String(filename, title, key, value_string) == 0) // 成功
		{
			long lValue;
			lValue = strtoul(value_string, NULL, 0); // base为0可根据value字符串进行转换，0x（零x）开头16进制处理0（零）开头8进制处理否则当成10进制处理
			*value = lValue;
			return 0;
		}
		else // 失败
		{
			return -1;
		}
	}
};

/******************************************
#规定
TRACE：		记录堆栈信息
DEBUG：		记录参数、变量信息
INFO：		记录过程信息
WARN：		记录警告信息
ERROR：		记录错误信息
CRITICAL：	记录致命错误信息
*******************************************/
// use embedded macro to support file and line number
#define VNAME(value) (#value)
/***************************************************
// 可以使用多种类型的 std::container<char> 类型。
// 也支持范围。
// 格式标志：
// {:X} - 以大写形式打印。
// {:s} - 不要用空格分隔每个字节。
// {:p} - 不要在每一行开始处打印位置。
// {:n} - 不要将输出拆分为行。
// {:a} - 如果 :n 未设置，则显示 ASCII。
****************************************************/
#define XLOG_MEMHEX(value, len) XLOG_TRACE("\n{:s}:{:p}(len={}):{:a}\n", VNAME(value), fmt::ptr(value), len, spdlog::to_hex(value, value + len, 16))
#define XLOG_TRACE(...) SPDLOG_LOGGER_CALL(XLogger::getInstance()->getLogger().get(), spdlog::level::trace, __VA_ARGS__)
#define XLOG_DEBUG(...) SPDLOG_LOGGER_CALL(XLogger::getInstance()->getLogger().get(), spdlog::level::debug, __VA_ARGS__)
#define XLOG_INFO(...) SPDLOG_LOGGER_CALL(XLogger::getInstance()->getLogger().get(), spdlog::level::info, __VA_ARGS__)
#define XLOG_WARN(...) SPDLOG_LOGGER_CALL(XLogger::getInstance()->getLogger().get(), spdlog::level::warn, __VA_ARGS__)
#define XLOG_ERROR(...) SPDLOG_LOGGER_CALL(XLogger::getInstance()->getLogger().get(), spdlog::level::err, __VA_ARGS__)
#define XLOG_CRITICAL(...) SPDLOG_LOGGER_CALL(XLogger::getInstance()->getLogger().get(), spdlog::level::critical, __VA_ARGS__)
#define XLOG_BEGIN XLOG_INFO(LOGBEGIN)
#define XLOG_END XLOG_INFO(LOGEND)
/*
int main()
{
	// print log test, you can transfer any param to do format
	int param = 1;

	unsigned char buf[200];
	unsigned char* pBuf=buf;
	int pBufLen=100;
	srand(time(NULL));
	for (size_t i = 0; i < 100; i++)
	{
		buf[i] = rand();
	}
	XLOG_MEMHEX(buf, 80);
	XLOG_MEMHEX(pBuf, pBufLen);
	XLOG_TRACE("this is trace log record, param: {}", ++param); // int type param is ok
	XLOG_DEBUG("this is debug log record, param: {}", ++param);
	XLOG_INFO("this is info log record, param: {}", ++param);
	XLOG_WARN("this is warn log record, param: {}", double(++param)); // double type param is ok
	XLOG_ERROR("this is error log record, param: {}", std::to_string(++param)); // string type param is ok
	XLOG_CRITICAL("this is critical log record, param: {}", std::to_string(++param)); // string type param is ok

	return 0;
}*/
#endif // XLOGGER_HPP
