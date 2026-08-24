#include "XLogger.hpp"

#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>

namespace
{
int g_failures = 0;

void expectTrue(bool condition, const char *message)
{
	if (!condition)
	{
		++g_failures;
		std::cerr << "FAILED: " << message << std::endl;
	}
}

bool isHexString(const std::string &value)
{
	for (std::string::const_iterator it = value.begin(); it != value.end(); ++it)
	{
		const char ch = *it;
		if (!((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')))
		{
			return false;
		}
	}
	return true;
}

size_t expectedSensitiveLength(size_t plainLength)
{
	const size_t paddedLength = ((plainLength / 16) + 1) * 16;
	return paddedLength * 2 + 16;
}

void expectSensitiveOutput(const std::string &encrypted, size_t plainLength, const char *caseName)
{
	std::ostringstream prefix;
	prefix << caseName << ": ";
	const std::string label = prefix.str();

	expectTrue(!encrypted.empty(), (label + "output should not be empty").c_str());
	expectTrue(encrypted != "[xlog_sm4_encrypt_failed]", (label + "encryption should not fail").c_str());
	if (encrypted.size() <= 16)
	{
		expectTrue(false, (label + "output should contain ciphertext before random factor").c_str());
		return;
	}

	expectTrue(encrypted.size() == expectedSensitiveLength(plainLength),
			   (label + "output length should match SM4 hex plus random factor").c_str());
	expectTrue((encrypted.size() % 2) == 0, (label + "output length should be even").c_str());
	expectTrue(isHexString(encrypted), (label + "output should be hexadecimal").c_str());
	expectTrue(isHexString(encrypted.substr(encrypted.size() - 16)),
			   (label + "trailing 16 chars should be hexadecimal random factor").c_str());
}

void testEmptyInput()
{
	expectTrue(XLogger::encryptSensitiveData(static_cast<const unsigned char *>(NULL), 0).empty(),
			   "null byte input should return empty");
	expectTrue(XLogger::encryptSensitiveData(std::string()).empty(), "empty string input should return empty");
	expectTrue(XLogger::encryptSensitiveData("").empty(), "empty C string input should return empty");
	expectTrue(XLogger::encryptSensitiveData(static_cast<const char *>(NULL)).empty(),
			   "null C string input should return empty");
}

void testStringInput()
{
	const std::string plain = "password-123";
	const std::string first = XLogger::encryptSensitiveData(plain);
	const std::string second = XLogger::encryptSensitiveData(plain);

	expectSensitiveOutput(first, plain.size(), "string input");
	expectSensitiveOutput(second, plain.size(), "string input second pass");
	expectTrue(first != second, "same plaintext should produce different outputs on consecutive calls");
}

void testCStringInput()
{
	const char *plain = "token-value";
	const std::string encrypted = XLogger::encryptSensitiveData(plain);
	expectSensitiveOutput(encrypted, std::string(plain).size(), "C string input");
}

void testValueOverloads()
{
	const int intValue = 123456;
	const std::string intEncrypted = XLogger::encryptSensitiveValue(intValue);
	expectSensitiveOutput(intEncrypted, std::string("123456").size(), "integer value overload");

	const double doubleValue = 3.5;
	const std::string doublePlain = fmt::format("{}", doubleValue);
	const std::string doubleEncrypted = XLogger::encryptSensitiveValue(doubleValue);
	expectSensitiveOutput(doubleEncrypted, doublePlain.size(), "double value overload");
}

void testBinaryInput()
{
	const unsigned char bytes[] = {0x00, 0x01, 0x02, 0x7F, 0x80, 0xFE, 0xFF, 0x00, 0x42};
	const std::string encrypted = XLogger::encryptSensitiveData(bytes, sizeof(bytes));
	expectSensitiveOutput(encrypted, sizeof(bytes), "binary input");
}
} // namespace

int main()
{
	testEmptyInput();
	testStringInput();
	testCStringInput();
	testValueOverloads();
	testBinaryInput();

	if (g_failures != 0)
	{
		std::cerr << g_failures << " sensitive data test(s) failed" << std::endl;
		return 1;
	}

	std::cout << "All sensitive data tests passed" << std::endl;
	return 0;
}
