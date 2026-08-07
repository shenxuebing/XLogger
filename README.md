# XLogger

基于 [spdlog](https://github.com/gabime/spdlog) 封装的C++单例日志库，提供简单易用的日志记录功能。

## 特性

- 🚀 **单头文件设计** - 只需包含一个 `XLogger.hpp` 文件即可使用
- 🎯 **单例模式** - 全局唯一的日志实例，线程安全
- 📁 **多种输出方式** - 支持控制台输出和文件输出
- 🔄 **日志轮转** - 支持按大小和按日期轮转日志文件
- 📊 **多级别日志** - 支持 TRACE、DEBUG、INFO、WARN、ERROR、CRITICAL 级别
- ⚡ **高性能** - 基于 spdlog 的异步日志记录
- 🛠️ **易于配置** - 提供丰富的配置选项
- 🔐 **敏感数据脱敏** - 内置国密 SM3/SM4 及 MD5 实现，提供 `encryptSensitiveData` 系列接口与 `XLOG_SM4_EN` / `XLOG_SENSITIVE_FIELD` 宏，日志中的口令、证件号等可一键加密脱敏
- 🧰 **内存检查辅助** - 提供 `XLOG_MEMHEX` 宏，以十六进制 + ASCII 形式输出缓冲区内容，便于排查二进制数据
- 🛡️ **DLL 安全检查** - 日志宏内置空指针保护，并提供 `isDestroyed()` 接口，避免 DLL 卸载或进程退出阶段访问已析构的 logger 导致崩溃

## 项目结构

```
XLogger/
├── XLogger.hpp              # 单头文件日志库
├── third_party/
│   └── spdlog/              # spdlog 子模块
├── .gitmodules              # Git 子模块配置
└── README.md                # 项目说明文档
```

## 快速开始

### 1. 克隆项目

```bash
git clone https://github.com/shenxuebing/XLogger
cd XLogger
git submodule update --init --recursive
```

### 2. 基本使用

```cpp
#include "XLogger.hpp"

int main() {
    // 配置日志
    XLogger::setLogPath("./logs");        // 设置日志文件路径
    XLogger::setLogPrefixName("app");     // 设置日志文件前缀
    XLogger::setLogLevel("info");         // 设置日志级别
    XLogger::setLogConsole(true);         // 启用控制台输出
    
    // 使用日志宏
    XLOG_INFO("应用程序启动");
    XLOG_DEBUG("调试信息: {}", 123);
    XLOG_WARN("警告信息: {}", "内存使用率较高");
    XLOG_ERROR("错误信息: {}", "文件读取失败");
    
    return 0;
}
```

## API 参考

### 配置方法

| 方法 | 说明 | 示例 |
|------|------|------|
| `setLogPath(path)` | 设置日志文件保存路径 | `setLogPath("./logs")` |
| `setLogPrefixName(name)` | 设置日志文件前缀名 | `setLogPrefixName("app")` |
| `setLogLevel(level)` | 设置日志级别 | `setLogLevel("info")` |
| `setLogConsole(enable)` | 启用/禁用控制台输出 | `setLogConsole(true)` |
| `setLogMaxFiles(count)` | 设置最大日志文件数量 | `setLogMaxFiles(30)` |
| `setLogMaxSize(size)` | 设置单个日志文件最大大小(MB) | `setLogMaxSize(10)` |

### 日志级别

| 级别 | 数值 | 说明 |
|------|------|------|
| `off` | -1 | 关闭日志 |
| `trace` | 0 | 跟踪级别（最详细） |
| `debug` | 1 | 调试级别 |
| `info` | 2 | 信息级别 |
| `warn` | 3 | 警告级别 |
| `error` | 4 | 错误级别 |
| `critical` | 5 | 严重错误级别 |

### 日志宏

| 宏 | 说明 | 示例 |
|----|------|------|
| `XLOG_TRACE(...)` | 跟踪级别日志 | `XLOG_TRACE("函数调用: {}", func_name)` |
| `XLOG_DEBUG(...)` | 调试级别日志 | `XLOG_DEBUG("变量值: {}", value)` |
| `XLOG_INFO(...)` | 信息级别日志 | `XLOG_INFO("操作完成")` |
| `XLOG_WARN(...)` | 警告级别日志 | `XLOG_WARN("性能警告")` |
| `XLOG_ERROR(...)` | 错误级别日志 | `XLOG_ERROR("操作失败: {}", error_msg)` |
| `XLOG_CRITICAL(...)` | 严重错误日志 | `XLOG_CRITICAL("系统崩溃")` |
| `XLOG_BEGIN` | 函数开始标记 | `XLOG_BEGIN` |
| `XLOG_END` | 函数结束标记 | `XLOG_END` |
| `XLOG_MEMHEX(data, len)` | 内存十六进制输出（TRACE 级别） | `XLOG_MEMHEX(buffer, 100)` |
| `XLOG_SM4_EN(value)` | 对敏感数据执行 SM4 加密脱敏，返回十六进制密文字符串 | `XLOG_SM4_EN(password)` |
| `XLOG_SENSITIVE_FIELD(value)` | 以 DEBUG 级别输出 `变量名(sm4)=密文` | `XLOG_SENSITIVE_FIELD(idCard)` |

> 所有 `XLOG_*` 宏在调用前会检查 logger 是否存在（含 DLL 卸载场景），无需在析构后再额外判空。
>
> `XLOG_SM4_EN` / `XLOG_SENSITIVE_FIELD` 仅对数据做单向加密脱敏，**不可逆**，仅用于在日志中隐藏明文，不适合作为存储加密。

### 敏感数据脱敏接口

| 方法 | 说明 |
|------|------|
| `XLogger::encryptSensitiveData(const unsigned char*, size_t)` | 对原始字节流执行 SM4 加密，返回十六进制密文（含尾部 16 字符随机盐） |
| `XLogger::encryptSensitiveData(const std::string&)` | 对字符串执行脱敏，重载便捷方法 |
| `XLogger::encryptSensitiveData(const char*)` | 对 C 字符串执行脱敏，自动识别 `nullptr` |
| `XLogger::encryptSensitiveValue(const T&)` | 模板方法，对任意可被 `fmt::format` 的类型执行脱敏 |

### 生命周期检查接口

| 方法 | 说明 |
|------|------|
| `XLogger::isDestroyed()` | 返回单例 logger 是否已析构（如 DLL 卸载后），用于安全访问前的判断 |

## 编译要求

- C++11 或更高版本

**注意**: 示例程序 `file_logging_example` 在 C++11 环境下不会自动列出日志文件目录，需要手动查看 `./logs` 目录中的 `.log` 文件。

**默认行为**:

- 编译时定义了 `LOGCONSOLE` 宏时，默认输出到控制台；未定义时默认输出到文件。
- 可随时通过 `XLogger::setLogConsole(true/false)` 覆盖默认行为。
- 如需在 Debug 构建自动启用控制台输出，可在 CMake 中通过 `target_compile_definitions` 按构建类型添加 `LOGCONSOLE`。

### CMake 编译选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `XLOGGER_ENABLE_CONFIG` | `OFF` | 启用从 INI 配置文件读取日志设置 |
| `BUILD_EXAMPLES` | `ON` | 是否编译 `example/` 目录下的示例程序 |

**启用配置文件支持**:
```bash
cmake -DXLOGGER_ENABLE_CONFIG=ON ..
```

启用后，XLogger 会在构造时尝试从 `defConf.ini` 读取以下配置：
- `logLevel` - 日志级别
- `logPath` - 日志文件路径  
- `logMaxSize` - 单个日志文件最大大小(MB)

## 依赖项

本项目使用 Git 子模块方式引入 [spdlog](https://github.com/gabime/spdlog) 库。

### 更新子模块

```bash
# 更新到最新版本
git submodule update --remote third_party/spdlog

# 或者更新所有子模块
git submodule update --remote
```

## 示例代码

### 基本使用

```cpp
#include "XLogger.hpp"

int main() {
    // 配置日志
    XLogger::setLogPath("./logs");
    XLogger::setLogLevel("info");
    XLogger::setLogConsole(true);
    
    // 使用日志宏
    XLOG_INFO("应用程序启动");
    XLOG_DEBUG("调试信息: {}", 123);
    XLOG_WARN("警告信息: {}", "内存使用率较高");
    XLOG_ERROR("错误信息: {}", "文件读取失败");
    
    return 0;
}
```

### 数值级别设置

```cpp
#include "XLogger.hpp"

void testNumericLevels() {
    // 使用数值设置级别
    XLogger::setLogLevel(2);  // info级别
    XLOG_INFO("这条日志会显示");
    XLOG_DEBUG("这条日志不会显示");
    
    XLogger::setLogLevel(4);  // error级别
    XLOG_ERROR("这条日志会显示");
    XLOG_WARN("这条日志不会显示");
}
```

### 函数日志记录

```cpp
#include "XLogger.hpp"

void processData(const std::string& data) {
    XLOG_BEGIN
    XLOG_INFO("开始处理数据，长度: {}", data.length());
    
    try {
        // 处理逻辑
        XLOG_DEBUG("数据处理中...");
        
        XLOG_INFO("数据处理完成");
    } catch (const std::exception& e) {
        XLOG_ERROR("数据处理失败: {}", e.what());
        throw;
    }
    
    XLOG_END
}
```

### 内存调试

```cpp
#include "XLogger.hpp"

void debugMemory() {
    unsigned char buffer[256];
    // 填充一些数据
    for (int i = 0; i < 256; ++i) {
        buffer[i] = i % 256;
    }
    
    // 输出内存内容
    XLOG_MEMHEX(buffer, 64);  // 输出前64字节的十六进制
}
```

### 敏感数据脱敏

```cpp
#include "XLogger.hpp"
#include <string>

void login(const std::string& user, const std::string& password, int idCard) {
    // 方式一：使用宏直接打印脱敏字段，输出形如 password(sm4)=xxxx（密文，每次随机）
    XLOG_SENSITIVE_FIELD(password);

    // 方式二：先取密文，再按需输出
    XLOG_INFO("用户 {} 登录，证件号(sm4)={}", user, XLOG_SM4_EN(idCard));

    // 方式三：直接调用静态接口（适用于非打印场景，例如写入业务表前）
    std::string encrypted = XLogger::encryptSensitiveData(password);
    // ...将 encrypted 落库或传输
}

int main() {
    XLogger::setLogConsole(true);
    XLogger::setLogLevel("debug");
    login("alice", "P@ssw0rd!", 110101199001011234LL);

    // 安全访问示例：DLL 卸载或进程退出后调用日志宏不会崩溃
    if (!XLogger::isDestroyed()) {
        XLOG_INFO("仍在生命周期内，安全输出");
    }
    return 0;
}
```

### 配置文件读取

```cpp
#include "XLogger.hpp"

int main() {
    // 启用配置文件支持（需要在CMake中设置 -DXLOGGER_ENABLE_CONFIG=ON）
#ifdef XLOGGER_ENABLE_CONFIG
    // 指定配置文件路径（默认为 defConf.ini）
    XLogger::setConfPath("defConf.ini");
    
    // 访问logger触发配置读取
    XLogger::getInstance()->getLogger();
    
    // 记录日志，配置会从文件读取
    XLOG_INFO("从配置文件读取的日志设置");
#else
    std::cout << "未启用配置读取功能" << std::endl;
#endif
    return 0;
}
```

**配置文件格式** (`defConf.ini`):
```ini
[CAT]
logLevel=info
logPath=./logs
logMaxSize=10
```

## 示例程序

项目包含多个示例程序，位于 `example/` 目录：

- `basic_example` - 基本使用示例
- `multithread_example` - 多线程日志示例
- `file_logging_example` - 文件日志和轮转示例
- `performance_test` - 性能测试示例
- `custom_format_example` - 自定义格式示例
- `level_test_example` - 日志级别测试示例
- `sensitive_example` - 敏感数据脱敏（SM4）与内存检查示例
- `config_example` - 配置文件读取示例（需启用 `XLOGGER_ENABLE_CONFIG`）

### 编译和运行示例

```bash
# 创建构建目录
mkdir build && cd build

# 配置CMake（普通模式）
cmake ..

# 配置CMake（启用配置文件支持）
cmake -DXLOGGER_ENABLE_CONFIG=ON ..

# 编译
cmake --build .

# 运行示例
./example/basic_example
./example/level_test_example
./example/config_example  # 需要启用 XLOGGER_ENABLE_CONFIG
```

## 许可证

本项目基于 spdlog 库构建，遵循相应的开源许可证。

## 贡献

欢迎提交 Issue 和 Pull Request 来改进这个项目。

## 更新日志

- **v1.1.0** - 新增敏感数据脱敏能力（内置 SM3/SM4/MD5，`encryptSensitiveData` / `encryptSensitiveValue` 及 `XLOG_SM4_EN` / `XLOG_SENSITIVE_FIELD` 宏）；新增 `XLOG_MEMHEX` 内存检查宏；日志宏加入空指针保护并提供 `isDestroyed()` DLL 安全检查；控制台默认行为改由 `LOGCONSOLE` 宏控制
- **v1.0.0** - 初始版本，基于 spdlog 的单例日志库
