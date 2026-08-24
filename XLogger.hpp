#pragma once
#ifndef XLOGGER_HPP
#define XLOGGER_HPP
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>
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

// ============================================================
// 日志脱敏支撑：MD5 / SM3 / SM4 纯软件实现（无外部依赖）
// 供 XLOG_SENSITIVE_FIELD 宏加密敏感字段后输出，
// 密文可用相同密钥派生逻辑还原，便于事后排查日志。
// ============================================================

struct XLogMd5Context
{
	uint32_t count[2];
	uint32_t state[4];
	unsigned char buffer[64];
};

struct XLogSm3Context
{
	uint32_t total[2];
	uint32_t state[8];
	unsigned char buffer[64];
	unsigned char ipad[64];
	unsigned char opad[64];
};

static inline uint32_t XLogRotl32(uint32_t value, uint32_t shift)
{
	shift &= 31;
	if (shift == 0)
	{
		return value;
	}
	return (value << shift) | (value >> (32 - shift));
}

static inline std::string XLogBytesToHex(const unsigned char *data, size_t len)
{
	static const char *kHexChars = "0123456789abcdef";
	if (data == nullptr || len == 0)
	{
		return "";
	}
	std::string hex;
	hex.resize(len * 2);
	for (size_t i = 0; i < len; ++i)
	{
		hex[i * 2] = kHexChars[(data[i] >> 4) & 0x0F];
		hex[i * 2 + 1] = kHexChars[data[i] & 0x0F];
	}
	return hex;
}

static inline void XLogMd5Encode(unsigned char *output, const uint32_t *input, unsigned int len)
{
	for (unsigned int i = 0, j = 0; j < len; ++i, j += 4)
	{
		output[j] = static_cast<unsigned char>(input[i] & 0xFF);
		output[j + 1] = static_cast<unsigned char>((input[i] >> 8) & 0xFF);
		output[j + 2] = static_cast<unsigned char>((input[i] >> 16) & 0xFF);
		output[j + 3] = static_cast<unsigned char>((input[i] >> 24) & 0xFF);
	}
}

static inline void XLogMd5Decode(uint32_t *output, const unsigned char *input, unsigned int len)
{
	for (unsigned int i = 0, j = 0; j < len; ++i, j += 4)
	{
		output[i] = static_cast<uint32_t>(input[j]) | (static_cast<uint32_t>(input[j + 1]) << 8) |
					(static_cast<uint32_t>(input[j + 2]) << 16) | (static_cast<uint32_t>(input[j + 3]) << 24);
	}
}

static inline uint32_t XLogMd5F(uint32_t x, uint32_t y, uint32_t z)
{
	return (x & y) | (~x & z);
}
static inline uint32_t XLogMd5G(uint32_t x, uint32_t y, uint32_t z)
{
	return (x & z) | (y & ~z);
}
static inline uint32_t XLogMd5H(uint32_t x, uint32_t y, uint32_t z)
{
	return x ^ y ^ z;
}
static inline uint32_t XLogMd5I(uint32_t x, uint32_t y, uint32_t z)
{
	return y ^ (x | ~z);
}

static inline void XLogMd5Step(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac,
							   uint32_t (*func)(uint32_t, uint32_t, uint32_t))
{
	a += func(b, c, d) + x + ac;
	a = XLogRotl32(a, s);
	a += b;
}

static inline void XLogMd5Transform(uint32_t state[4], const unsigned char block[64])
{
	uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
	uint32_t x[16] = {0};
	XLogMd5Decode(x, block, 64);

	XLogMd5Step(a, b, c, d, x[0], 7, 0xd76aa478, XLogMd5F);
	XLogMd5Step(d, a, b, c, x[1], 12, 0xe8c7b756, XLogMd5F);
	XLogMd5Step(c, d, a, b, x[2], 17, 0x242070db, XLogMd5F);
	XLogMd5Step(b, c, d, a, x[3], 22, 0xc1bdceee, XLogMd5F);
	XLogMd5Step(a, b, c, d, x[4], 7, 0xf57c0faf, XLogMd5F);
	XLogMd5Step(d, a, b, c, x[5], 12, 0x4787c62a, XLogMd5F);
	XLogMd5Step(c, d, a, b, x[6], 17, 0xa8304613, XLogMd5F);
	XLogMd5Step(b, c, d, a, x[7], 22, 0xfd469501, XLogMd5F);
	XLogMd5Step(a, b, c, d, x[8], 7, 0x698098d8, XLogMd5F);
	XLogMd5Step(d, a, b, c, x[9], 12, 0x8b44f7af, XLogMd5F);
	XLogMd5Step(c, d, a, b, x[10], 17, 0xffff5bb1, XLogMd5F);
	XLogMd5Step(b, c, d, a, x[11], 22, 0x895cd7be, XLogMd5F);
	XLogMd5Step(a, b, c, d, x[12], 7, 0x6b901122, XLogMd5F);
	XLogMd5Step(d, a, b, c, x[13], 12, 0xfd987193, XLogMd5F);
	XLogMd5Step(c, d, a, b, x[14], 17, 0xa679438e, XLogMd5F);
	XLogMd5Step(b, c, d, a, x[15], 22, 0x49b40821, XLogMd5F);

	XLogMd5Step(a, b, c, d, x[1], 5, 0xf61e2562, XLogMd5G);
	XLogMd5Step(d, a, b, c, x[6], 9, 0xc040b340, XLogMd5G);
	XLogMd5Step(c, d, a, b, x[11], 14, 0x265e5a51, XLogMd5G);
	XLogMd5Step(b, c, d, a, x[0], 20, 0xe9b6c7aa, XLogMd5G);
	XLogMd5Step(a, b, c, d, x[5], 5, 0xd62f105d, XLogMd5G);
	XLogMd5Step(d, a, b, c, x[10], 9, 0x02441453, XLogMd5G);
	XLogMd5Step(c, d, a, b, x[15], 14, 0xd8a1e681, XLogMd5G);
	XLogMd5Step(b, c, d, a, x[4], 20, 0xe7d3fbc8, XLogMd5G);
	XLogMd5Step(a, b, c, d, x[9], 5, 0x21e1cde6, XLogMd5G);
	XLogMd5Step(d, a, b, c, x[14], 9, 0xc33707d6, XLogMd5G);
	XLogMd5Step(c, d, a, b, x[3], 14, 0xf4d50d87, XLogMd5G);
	XLogMd5Step(b, c, d, a, x[8], 20, 0x455a14ed, XLogMd5G);
	XLogMd5Step(a, b, c, d, x[13], 5, 0xa9e3e905, XLogMd5G);
	XLogMd5Step(d, a, b, c, x[2], 9, 0xfcefa3f8, XLogMd5G);
	XLogMd5Step(c, d, a, b, x[7], 14, 0x676f02d9, XLogMd5G);
	XLogMd5Step(b, c, d, a, x[12], 20, 0x8d2a4c8a, XLogMd5G);

	XLogMd5Step(a, b, c, d, x[5], 4, 0xfffa3942, XLogMd5H);
	XLogMd5Step(d, a, b, c, x[8], 11, 0x8771f681, XLogMd5H);
	XLogMd5Step(c, d, a, b, x[11], 16, 0x6d9d6122, XLogMd5H);
	XLogMd5Step(b, c, d, a, x[14], 23, 0xfde5380c, XLogMd5H);
	XLogMd5Step(a, b, c, d, x[1], 4, 0xa4beea44, XLogMd5H);
	XLogMd5Step(d, a, b, c, x[4], 11, 0x4bdecfa9, XLogMd5H);
	XLogMd5Step(c, d, a, b, x[7], 16, 0xf6bb4b60, XLogMd5H);
	XLogMd5Step(b, c, d, a, x[10], 23, 0xbebfbc70, XLogMd5H);
	XLogMd5Step(a, b, c, d, x[13], 4, 0x289b7ec6, XLogMd5H);
	XLogMd5Step(d, a, b, c, x[0], 11, 0xeaa127fa, XLogMd5H);
	XLogMd5Step(c, d, a, b, x[3], 16, 0xd4ef3085, XLogMd5H);
	XLogMd5Step(b, c, d, a, x[6], 23, 0x04881d05, XLogMd5H);
	XLogMd5Step(a, b, c, d, x[9], 4, 0xd9d4d039, XLogMd5H);
	XLogMd5Step(d, a, b, c, x[12], 11, 0xe6db99e5, XLogMd5H);
	XLogMd5Step(c, d, a, b, x[15], 16, 0x1fa27cf8, XLogMd5H);
	XLogMd5Step(b, c, d, a, x[2], 23, 0xc4ac5665, XLogMd5H);

	XLogMd5Step(a, b, c, d, x[0], 6, 0xf4292244, XLogMd5I);
	XLogMd5Step(d, a, b, c, x[7], 10, 0x432aff97, XLogMd5I);
	XLogMd5Step(c, d, a, b, x[14], 15, 0xab9423a7, XLogMd5I);
	XLogMd5Step(b, c, d, a, x[5], 21, 0xfc93a039, XLogMd5I);
	XLogMd5Step(a, b, c, d, x[12], 6, 0x655b59c3, XLogMd5I);
	XLogMd5Step(d, a, b, c, x[3], 10, 0x8f0ccc92, XLogMd5I);
	XLogMd5Step(c, d, a, b, x[10], 15, 0xffeff47d, XLogMd5I);
	XLogMd5Step(b, c, d, a, x[1], 21, 0x85845dd1, XLogMd5I);
	XLogMd5Step(a, b, c, d, x[8], 6, 0x6fa87e4f, XLogMd5I);
	XLogMd5Step(d, a, b, c, x[15], 10, 0xfe2ce6e0, XLogMd5I);
	XLogMd5Step(c, d, a, b, x[6], 15, 0xa3014314, XLogMd5I);
	XLogMd5Step(b, c, d, a, x[13], 21, 0x4e0811a1, XLogMd5I);
	XLogMd5Step(a, b, c, d, x[4], 6, 0xf7537e82, XLogMd5I);
	XLogMd5Step(d, a, b, c, x[11], 10, 0xbd3af235, XLogMd5I);
	XLogMd5Step(c, d, a, b, x[2], 15, 0x2ad7d2bb, XLogMd5I);
	XLogMd5Step(b, c, d, a, x[9], 21, 0xeb86d391, XLogMd5I);

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
}

static inline void XLogMd5Init(XLogMd5Context *context)
{
	context->count[0] = 0;
	context->count[1] = 0;
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
}

static inline void XLogMd5Update(XLogMd5Context *context, const unsigned char *input, unsigned int inputLen)
{
	unsigned int index = (context->count[0] >> 3) & 0x3F;
	unsigned int partLen = 64 - index;
	unsigned int i = 0;
	context->count[0] += inputLen << 3;
	if (context->count[0] < (inputLen << 3))
	{
		context->count[1]++;
	}
	context->count[1] += inputLen >> 29;

	if (inputLen >= partLen)
	{
		memcpy(&context->buffer[index], input, partLen);
		XLogMd5Transform(context->state, context->buffer);
		for (i = partLen; i + 63 < inputLen; i += 64)
		{
			XLogMd5Transform(context->state, input + i);
		}
		index = 0;
	}
	memcpy(&context->buffer[index], input + i, inputLen - i);
}

static inline void XLogMd5Final(XLogMd5Context *context, unsigned char digest[16])
{
	static unsigned char padding[64] = {0x80};
	unsigned char bits[8] = {0};
	unsigned int index = (context->count[0] >> 3) & 0x3F;
	unsigned int padLen = (index < 56) ? (56 - index) : (120 - index);
	XLogMd5Encode(bits, context->count, 8);
	XLogMd5Update(context, padding, padLen);
	XLogMd5Update(context, bits, 8);
	XLogMd5Encode(digest, context->state, 16);
}

static inline void XLogMd5(const unsigned char *data, size_t dataLen, unsigned char digest[16])
{
	XLogMd5Context ctx;
	XLogMd5Init(&ctx);
	XLogMd5Update(&ctx, data, static_cast<unsigned int>(dataLen));
	XLogMd5Final(&ctx, digest);
}

static inline uint32_t XLogGetUint32Be(const unsigned char *b, int i)
{
	return (static_cast<uint32_t>(b[i]) << 24) | (static_cast<uint32_t>(b[i + 1]) << 16) |
		   (static_cast<uint32_t>(b[i + 2]) << 8) | static_cast<uint32_t>(b[i + 3]);
}

static inline void XLogPutUint32Be(uint32_t n, unsigned char *b, int i)
{
	b[i] = static_cast<unsigned char>(n >> 24);
	b[i + 1] = static_cast<unsigned char>(n >> 16);
	b[i + 2] = static_cast<unsigned char>(n >> 8);
	b[i + 3] = static_cast<unsigned char>(n);
}

static inline uint32_t XLogSm3P0(uint32_t x)
{
	return x ^ XLogRotl32(x, 9) ^ XLogRotl32(x, 17);
}
static inline uint32_t XLogSm3P1(uint32_t x)
{
	return x ^ XLogRotl32(x, 15) ^ XLogRotl32(x, 23);
}
static inline uint32_t XLogSm3FF0(uint32_t x, uint32_t y, uint32_t z)
{
	return x ^ y ^ z;
}
static inline uint32_t XLogSm3FF1(uint32_t x, uint32_t y, uint32_t z)
{
	return (x & y) | (x & z) | (y & z);
}
static inline uint32_t XLogSm3GG0(uint32_t x, uint32_t y, uint32_t z)
{
	return x ^ y ^ z;
}
static inline uint32_t XLogSm3GG1(uint32_t x, uint32_t y, uint32_t z)
{
	return (x & y) | (~x & z);
}

static inline void XLogSm3Init(XLogSm3Context *ctx)
{
	ctx->total[0] = 0;
	ctx->total[1] = 0;
	ctx->state[0] = 0x7380166F;
	ctx->state[1] = 0x4914B2B9;
	ctx->state[2] = 0x172442D7;
	ctx->state[3] = 0xDA8A0600;
	ctx->state[4] = 0xA96F30BC;
	ctx->state[5] = 0x163138AA;
	ctx->state[6] = 0xE38DEE4D;
	ctx->state[7] = 0xB0FB0E4E;
}

static inline void XLogSm3Process(XLogSm3Context *ctx, const unsigned char data[64])
{
	uint32_t w[68] = {0};
	uint32_t w1[64] = {0};
	uint32_t t[64] = {0};
	for (int j = 0; j < 16; ++j)
		t[j] = 0x79CC4519;
	for (int j = 16; j < 64; ++j)
		t[j] = 0x7A879D8A;
	for (int j = 0; j < 16; ++j)
		w[j] = XLogGetUint32Be(data, j * 4);
	for (int j = 16; j < 68; ++j)
	{
		uint32_t temp = w[j - 16] ^ w[j - 9] ^ XLogRotl32(w[j - 3], 15);
		w[j] = XLogSm3P1(temp) ^ XLogRotl32(w[j - 13], 7) ^ w[j - 6];
	}
	for (int j = 0; j < 64; ++j)
		w1[j] = w[j] ^ w[j + 4];

	uint32_t a = ctx->state[0], b = ctx->state[1], c = ctx->state[2], d = ctx->state[3];
	uint32_t e = ctx->state[4], f = ctx->state[5], g = ctx->state[6], h = ctx->state[7];
	for (int j = 0; j < 64; ++j)
	{
		uint32_t ss1 = XLogRotl32((XLogRotl32(a, 12) + e + XLogRotl32(t[j], j)) & 0xFFFFFFFF, 7);
		uint32_t ss2 = ss1 ^ XLogRotl32(a, 12);
		uint32_t tt1 = ((j < 16 ? XLogSm3FF0(a, b, c) : XLogSm3FF1(a, b, c)) + d + ss2 + w1[j]) & 0xFFFFFFFF;
		uint32_t tt2 = ((j < 16 ? XLogSm3GG0(e, f, g) : XLogSm3GG1(e, f, g)) + h + ss1 + w[j]) & 0xFFFFFFFF;
		d = c;
		c = XLogRotl32(b, 9);
		b = a;
		a = tt1;
		h = g;
		g = XLogRotl32(f, 19);
		f = e;
		e = XLogSm3P0(tt2);
	}
	ctx->state[0] ^= a;
	ctx->state[1] ^= b;
	ctx->state[2] ^= c;
	ctx->state[3] ^= d;
	ctx->state[4] ^= e;
	ctx->state[5] ^= f;
	ctx->state[6] ^= g;
	ctx->state[7] ^= h;
}

static inline void XLogSm3Update(XLogSm3Context *ctx, const unsigned char *input, int ilen)
{
	if (ilen <= 0)
		return;
	uint32_t left = ctx->total[0] & 0x3F;
	int fill = 64 - static_cast<int>(left);
	ctx->total[0] += static_cast<uint32_t>(ilen);
	ctx->total[0] &= 0xFFFFFFFF;
	if (ctx->total[0] < static_cast<uint32_t>(ilen))
		ctx->total[1]++;
	if (left && ilen >= fill)
	{
		memcpy(ctx->buffer + left, input, fill);
		XLogSm3Process(ctx, ctx->buffer);
		input += fill;
		ilen -= fill;
		left = 0;
	}
	while (ilen >= 64)
	{
		XLogSm3Process(ctx, input);
		input += 64;
		ilen -= 64;
	}
	if (ilen > 0)
	{
		memcpy(ctx->buffer + left, input, ilen);
	}
}

static inline void XLogSm3Final(XLogSm3Context *ctx, unsigned char output[32])
{
	static const unsigned char padding[64] = {0x80};
	unsigned char msglen[8] = {0};
	uint32_t high = (ctx->total[0] >> 29) | (ctx->total[1] << 3);
	uint32_t low = (ctx->total[0] << 3);
	XLogPutUint32Be(high, msglen, 0);
	XLogPutUint32Be(low, msglen, 4);
	uint32_t last = ctx->total[0] & 0x3F;
	uint32_t padn = (last < 56) ? (56 - last) : (120 - last);
	XLogSm3Update(ctx, padding, static_cast<int>(padn));
	XLogSm3Update(ctx, msglen, 8);
	for (int i = 0; i < 8; ++i)
		XLogPutUint32Be(ctx->state[i], output, i * 4);
}

static inline void XLogSm3(const unsigned char *input, int ilen, unsigned char output[32])
{
	XLogSm3Context ctx;
	XLogSm3Init(&ctx);
	XLogSm3Update(&ctx, input, ilen);
	XLogSm3Final(&ctx, output);
	memset(&ctx, 0, sizeof(ctx));
}

static inline void XLogMakeRandomBytes(unsigned char *out, size_t len)
{
	std::random_device rd;
	for (size_t i = 0; i < len; ++i)
	{
		out[i] = static_cast<unsigned char>(rd() & 0xFF);
	}
}

static inline bool XLogMakeRandomHex16(std::string &randomHex)
{
	unsigned char randomBytes[8] = {0};
	XLogMakeRandomBytes(randomBytes, sizeof(randomBytes));
	randomHex = XLogBytesToHex(randomBytes, sizeof(randomBytes));
	return randomHex.size() == 16;
}

static inline uint32_t XLogSms4ByteSub(uint32_t value)
{
	static const uint8_t sbox[256] = {
		0xd6, 0x90, 0xe9, 0xfe, 0xcc, 0xe1, 0x3d, 0xb7, 0x16, 0xb6, 0x14, 0xc2, 0x28, 0xfb, 0x2c, 0x05, 0x2b, 0x67,
		0x9a, 0x76, 0x2a, 0xbe, 0x04, 0xc3, 0xaa, 0x44, 0x13, 0x26, 0x49, 0x86, 0x06, 0x99, 0x9c, 0x42, 0x50, 0xf4,
		0x91, 0xef, 0x98, 0x7a, 0x33, 0x54, 0x0b, 0x43, 0xed, 0xcf, 0xac, 0x62, 0xe4, 0xb3, 0x1c, 0xa9, 0xc9, 0x08,
		0xe8, 0x95, 0x80, 0xdf, 0x94, 0xfa, 0x75, 0x8f, 0x3f, 0xa6, 0x47, 0x07, 0xa7, 0xfc, 0xf3, 0x73, 0x17, 0xba,
		0x83, 0x59, 0x3c, 0x19, 0xe6, 0x85, 0x4f, 0xa8, 0x68, 0x6b, 0x81, 0xb2, 0x71, 0x64, 0xda, 0x8b, 0xf8, 0xeb,
		0x0f, 0x4b, 0x70, 0x56, 0x9d, 0x35, 0x1e, 0x24, 0x0e, 0x5e, 0x63, 0x58, 0xd1, 0xa2, 0x25, 0x22, 0x7c, 0x3b,
		0x01, 0x21, 0x78, 0x87, 0xd4, 0x00, 0x46, 0x57, 0x9f, 0xd3, 0x27, 0x52, 0x4c, 0x36, 0x02, 0xe7, 0xa0, 0xc4,
		0xc8, 0x9e, 0xea, 0xbf, 0x8a, 0xd2, 0x40, 0xc7, 0x38, 0xb5, 0xa3, 0xf7, 0xf2, 0xce, 0xf9, 0x61, 0x15, 0xa1,
		0xe0, 0xae, 0x5d, 0xa4, 0x9b, 0x34, 0x1a, 0x55, 0xad, 0x93, 0x32, 0x30, 0xf5, 0x8c, 0xb1, 0xe3, 0x1d, 0xf6,
		0xe2, 0x2e, 0x82, 0x66, 0xca, 0x60, 0xc0, 0x29, 0x23, 0xab, 0x0d, 0x53, 0x4e, 0x6f, 0xd5, 0xdb, 0x37, 0x45,
		0xde, 0xfd, 0x8e, 0x2f, 0x03, 0xff, 0x6a, 0x72, 0x6d, 0x6c, 0x5b, 0x51, 0x8d, 0x1b, 0xaf, 0x92, 0xbb, 0xdd,
		0xbc, 0x7f, 0x11, 0xd9, 0x5c, 0x41, 0x1f, 0x10, 0x5a, 0xd8, 0x0a, 0xc1, 0x31, 0x88, 0xa5, 0xcd, 0x7b, 0xbd,
		0x2d, 0x74, 0xd0, 0x12, 0xb8, 0xe5, 0xb4, 0xb0, 0x89, 0x69, 0x97, 0x4a, 0x0c, 0x96, 0x77, 0x7e, 0x65, 0xb9,
		0xf1, 0x09, 0xc5, 0x6e, 0xc6, 0x84, 0x18, 0xf0, 0x7d, 0xec, 0x3a, 0xdc, 0x4d, 0x20, 0x79, 0xee, 0x5f, 0x3e,
		0xd7, 0xcb, 0x39, 0x48};
	return (static_cast<uint32_t>(sbox[(value >> 24) & 0xFF]) << 24) ^
		   (static_cast<uint32_t>(sbox[(value >> 16) & 0xFF]) << 16) ^
		   (static_cast<uint32_t>(sbox[(value >> 8) & 0xFF]) << 8) ^ static_cast<uint32_t>(sbox[value & 0xFF]);
}

static inline uint32_t XLogSms4L1(uint32_t b)
{
	return b ^ XLogRotl32(b, 2) ^ XLogRotl32(b, 10) ^ XLogRotl32(b, 18) ^ XLogRotl32(b, 24);
}
static inline uint32_t XLogSms4L2(uint32_t b)
{
	return b ^ XLogRotl32(b, 13) ^ XLogRotl32(b, 23);
}
static inline uint32_t XLogSms4SwapEndian(uint32_t x)
{
	x = XLogRotl32(x, 16);
	return ((x & 0x00FF00FF) << 8) ^ ((x & 0xFF00FF00) >> 8);
}

static inline void XLogSms4KeyExt(const unsigned char *key, uint32_t rk[32], bool decrypt)
{
	static const uint32_t ck[32] = {0x00070e15, 0x1c232a31, 0x383f464d, 0x545b6269, 0x70777e85, 0x8c939aa1, 0xa8afb6bd,
									0xc4cbd2d9, 0xe0e7eef5, 0xfc030a11, 0x181f262d, 0x343b4249, 0x50575e65, 0x6c737a81,
									0x888f969d, 0xa4abb2b9, 0xc0c7ced5, 0xdce3eaf1, 0xf8ff060d, 0x141b2229, 0x30373e45,
									0x4c535a61, 0x686f767d, 0x848b9299, 0xa0a7aeb5, 0xbcc3cad1, 0xd8dfe6ed, 0xf4fb0209,
									0x10171e25, 0x2c333a41, 0x484f565d, 0x646b7279};
	const uint32_t *p = reinterpret_cast<const uint32_t *>(key);
	uint32_t x0 = XLogSms4SwapEndian(p[0]) ^ 0xa3b1bac6;
	uint32_t x1 = XLogSms4SwapEndian(p[1]) ^ 0x56aa3350;
	uint32_t x2 = XLogSms4SwapEndian(p[2]) ^ 0x677d9197;
	uint32_t x3 = XLogSms4SwapEndian(p[3]) ^ 0xb27022dc;
	for (uint32_t r = 0; r < 32; r += 4)
	{
		uint32_t mid = XLogSms4ByteSub(x1 ^ x2 ^ x3 ^ ck[r]);
		rk[r] = x0 ^= XLogSms4L2(mid);
		mid = XLogSms4ByteSub(x2 ^ x3 ^ x0 ^ ck[r + 1]);
		rk[r + 1] = x1 ^= XLogSms4L2(mid);
		mid = XLogSms4ByteSub(x3 ^ x0 ^ x1 ^ ck[r + 2]);
		rk[r + 2] = x2 ^= XLogSms4L2(mid);
		mid = XLogSms4ByteSub(x0 ^ x1 ^ x2 ^ ck[r + 3]);
		rk[r + 3] = x3 ^= XLogSms4L2(mid);
	}
	if (decrypt)
	{
		for (int r = 0; r < 16; ++r)
			std::swap(rk[r], rk[31 - r]);
	}
}

static inline void XLogSms4CryptBlock(const unsigned char *input, unsigned char *output, const uint32_t rk[32])
{
	const uint32_t *p = reinterpret_cast<const uint32_t *>(input);
	uint32_t x0 = XLogSms4SwapEndian(p[0]);
	uint32_t x1 = XLogSms4SwapEndian(p[1]);
	uint32_t x2 = XLogSms4SwapEndian(p[2]);
	uint32_t x3 = XLogSms4SwapEndian(p[3]);
	for (uint32_t r = 0; r < 32; r += 4)
	{
		uint32_t mid = XLogSms4ByteSub(x1 ^ x2 ^ x3 ^ rk[r]);
		x0 ^= XLogSms4L1(mid);
		mid = XLogSms4ByteSub(x2 ^ x3 ^ x0 ^ rk[r + 1]);
		x1 ^= XLogSms4L1(mid);
		mid = XLogSms4ByteSub(x3 ^ x0 ^ x1 ^ rk[r + 2]);
		x2 ^= XLogSms4L1(mid);
		mid = XLogSms4ByteSub(x0 ^ x1 ^ x2 ^ rk[r + 3]);
		x3 ^= XLogSms4L1(mid);
	}
	uint32_t out0 = XLogSms4SwapEndian(x3), out1 = XLogSms4SwapEndian(x2), out2 = XLogSms4SwapEndian(x1),
			 out3 = XLogSms4SwapEndian(x0);
	memcpy(output, &out0, 4);
	memcpy(output + 4, &out1, 4);
	memcpy(output + 8, &out2, 4);
	memcpy(output + 12, &out3, 4);
}

static inline bool XLogSm4EncryptEcbPkcs7(const unsigned char *key, const unsigned char *plaintext, size_t plaintextLen,
										  std::vector<unsigned char> &ciphertext)
{
	if (key == nullptr || (plaintext == nullptr && plaintextLen != 0))
	{
		return false;
	}
	std::vector<unsigned char> padded(plaintext, plaintext + plaintextLen);
	unsigned char pad = static_cast<unsigned char>(16 - (plaintextLen % 16));
	if (pad == 0)
		pad = 16;
	padded.insert(padded.end(), pad, pad);
	ciphertext.assign(padded.size(), 0);
	uint32_t rk[32] = {0};
	XLogSms4KeyExt(key, rk, false);
	for (size_t i = 0; i < padded.size(); i += 16)
	{
		XLogSms4CryptBlock(padded.data() + i, ciphertext.data() + i, rk);
	}
	return true;
}

static inline bool XLogMakeEnDataKey(const unsigned char *pwd, size_t pwdLen, const unsigned char *random16,
									 unsigned char key[16])
{
	if (pwd == nullptr || random16 == nullptr || key == nullptr)
	{
		return false;
	}
	unsigned char strDigest[32] = {0};
	unsigned char dzStrDigest[32] = {0};
	unsigned char dzStrDigestNot[32] = {0};
	unsigned char md5Digest[16] = {0};
	unsigned char md5DigestNot[16] = {0};
	unsigned char md5Folded[16] = {0};
	unsigned char mk[32] = {0};
	unsigned char eKey[16] = {0};

	XLogSm3(pwd, static_cast<int>(pwdLen), strDigest);
	memcpy(dzStrDigest, strDigest + 16, 16);
	memcpy(dzStrDigest + 16, strDigest, 16);
	for (size_t i = 0; i < sizeof(dzStrDigest); ++i)
		dzStrDigestNot[i] = static_cast<unsigned char>(~dzStrDigest[i]);
	XLogMd5(dzStrDigestNot, sizeof(dzStrDigestNot), md5Digest);
	for (size_t i = 0; i < sizeof(md5Digest); ++i)
		md5DigestNot[i] = static_cast<unsigned char>(~md5Digest[i]);
	memcpy(md5Folded, md5DigestNot + 8, 8);
	memcpy(md5Folded + 8, md5DigestNot, 8);
	XLogSm3(md5Folded, sizeof(md5Folded), mk);
	XLogMd5(mk, sizeof(mk), eKey);
	for (size_t i = 0; i < 16; ++i)
		key[i] = static_cast<unsigned char>(eKey[i] ^ random16[i]);
	return true;
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

	// ====== 敏感字段脱敏（SM4 加密后输出，配合 XLOG_SENSITIVE_FIELD 宏使用）======
	// 输出 = SM4(数据, 会话密钥) 的十六进制 + 16 字符随机因子；
	// 会话密钥由固定根密钥与随机因子派生（SM3/MD5 混合），可用同流程还原明文排查日志。
	static std::string encryptSensitiveData(const unsigned char *data, size_t dataLen)
	{
		static const unsigned char kYKey[] = {0x30, 0x82, 0x0a, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
											  0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b,
											  0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00};

		if (data == nullptr || dataLen == 0)
		{
			return "";
		}

		std::string randomHex;
		unsigned char mKey[16] = {0};
		std::vector<unsigned char> cipherData;
		if (!XLogMakeRandomHex16(randomHex) ||
			!XLogMakeEnDataKey(kYKey, sizeof(kYKey), reinterpret_cast<const unsigned char *>(randomHex.data()), mKey) ||
			!XLogSm4EncryptEcbPkcs7(mKey, data, dataLen, cipherData))
		{
			return "[xlog_sm4_encrypt_failed]";
		}
		return XLogBytesToHex(cipherData.data(), cipherData.size()) + randomHex;
	}

	static std::string encryptSensitiveData(const std::string &value)
	{
		return encryptSensitiveData(reinterpret_cast<const unsigned char *>(value.data()), value.size());
	}

	static std::string encryptSensitiveData(const char *value)
	{
		if (value == nullptr)
		{
			return "";
		}
		return encryptSensitiveData(reinterpret_cast<const unsigned char *>(value), strlen(value));
	}

	template <typename T>
	static std::string encryptSensitiveValue(const T &value)
	{
		return encryptSensitiveData(fmt::format("{}", value));
	}

	// 检查 logger 是否已析构（DLL 卸载时安全检查）
	static bool isDestroyed()
	{
		return getInstance()->m_destroyed;
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
		auto logger = XLogger::getInstance()->m_logger;
		if (!logger)
			return;
		const std::string &level = getConfig().level;
		if (level == "trace" || level == "all")
		{
			logger->set_level(spdlog::level::trace);
			logger->flush_on(spdlog::level::trace);
		}
		else if (level == "debug")
		{
			logger->set_level(spdlog::level::debug);
			logger->flush_on(spdlog::level::debug);
		}
		else if (level == "info")
		{
			logger->set_level(spdlog::level::info);
			logger->flush_on(spdlog::level::info);
		}
		else if (level == "warn")
		{
			logger->set_level(spdlog::level::warn);
			logger->flush_on(spdlog::level::warn);
		}
		else if (level == "error")
		{
			logger->set_level(spdlog::level::err);
			logger->flush_on(spdlog::level::err);
		}
		else if (level == "off")
		{
			logger->set_level(spdlog::level::off);
			logger->flush_on(spdlog::level::off);
		}
		else
		{
			logger->set_level(spdlog::level::err);
			logger->flush_on(spdlog::level::err);
		}
	}

  private:
	// 配置结构体 - Meyer's Singleton 模式
	struct Config
	{
		std::string log_dir = "./Log";
		std::string logger_name_prefix = "log";
		std::string conf_path = "defConf.ini";
		std::string level = "all";
		bool console = true;
		int max_size = 10;
		int max_files = 30;

		Config()
		{
#ifdef LOGCONSOLE
			console = true;
#else
			console = false;
#endif
		}
	};

	// 获取配置实例
	static Config &getConfig()
	{
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
			if (readConfigFile_String(getConfig().conf_path.c_str(), "CAT", "logLevel", levelStr,
									  sizeof(levelStr)) == 0)
			{
				getConfig().level = levelStr;
			}
			if (readConfigFile_String(getConfig().conf_path.c_str(), "CAT", "logPath", logPath, sizeof(logPath)) == 0)
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

			// custom format
			m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [pid:%P] [thread:%t][%!] [%l]  %v(%s:%#)"); // with timestamp, thread_id, filename and line number

			const std::string &level = getConfig().level;
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
			// 创建 fallback stdout logger，避免后续 XLOG 调用空指针崩溃
			try
			{
				m_logger = spdlog::stdout_color_mt("fallback");
			}
			catch (...)
			{
				// 即使 fallback 也失败，无法恢复
			}
			std::cout << "Log initialization failed: " << ex.what() << std::endl;
		}
	}

	~XLogger()
	{
		m_destroyed = true;
		if (m_logger)
		{
			try
			{
				m_logger->flush();
			}
			catch (...)
			{
			}
		}
		m_logger.reset();
		spdlog::drop_all(); // 移除所有 logger
		spdlog::shutdown(); // 完整关闭 spdlog 系统
	}

	void *operator new(size_t) = delete;
	void *operator new[](size_t) = delete;

	XLogger(const XLogger &) = delete;
	XLogger &operator=(const XLogger &) = delete;

  private:
	std::shared_ptr<spdlog::logger> m_logger;
	bool m_destroyed = false;

  private:
	void err_handler_example()
	{
		// can be set globally or per logger(logger->set_error_handler(..))
		spdlog::set_error_handler([](const std::string &msg) { spdlog::get("console")->error("*** LOGGER ERROR ***: {}", msg); });
		spdlog::get("console")->info("some invalid message to trigger an error {}", 3);
	}

	// 从配置文件读取字符串类型数据
	int readConfigFile_String(const char *filename, const char *title, const char *key, char *value,
							  size_t valueCapacity)
	{
		int ret = -1;
		FILE *fp = NULL;
		char szLine[260] = {0};
		int rtnval = 0;
		int i = 0;
		int flag = 0;
		char *tmp = NULL;
		bool isFirst = true; // 是否为第一次扫描配置文件
		bool isEnd = false;	 // 是否扫描完配置文件
#ifdef _WIN32
		if (fopen_s(&fp, filename, "r") != 0 || fp == NULL)
#else
		if ((fp = fopen(filename, "r")) == NULL)
#endif
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
				if (i < (int)sizeof(szLine) - 1)
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
							const size_t length = strcspn(tmp + 1, "\r\n");
							if (value == NULL || valueCapacity == 0 || length >= valueCapacity)
							{
								ret = -1;
								goto end;
							}
							memcpy(value, tmp + 1, length);
							value[length] = '\0';
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
					const std::string section = std::string("[") + title + "]";
					if (strncmp(section.c_str(), szLine, section.size()) == 0)
					{
						// 找到title
						flag = 1;
					}
				}
				isFirst = false;
				if (i == 0 || (szLine[0] != '\0' && szLine[strlen(szLine) - 1] == '\n')) // 2019年11月22日19:45:13 沈雪冰 update ||i==0 上边在0a (\n) 时把 0a置为 00了导致不能走到这里，所以加了这个条件
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
		while (value != NULL && value[0] != '\0' &&
			   (value[strlen(value) - 1] == '\r' || value[strlen(value) - 1] == '\n')) // 去掉/r /n
		{
			value[strlen(value) - 1] = '\0';
		}
		return ret;
	}
	// 从配置文件读取整类型数据
	int readConfigFile_Int(const char *filename, const char *title, const char *key, int *value)
	{
		char value_string[260] = {0};
		if (readConfigFile_String(filename, title, key, value_string, sizeof(value_string)) == 0) // 成功
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
// 敏感字段脱敏输出：XLOG_SENSITIVE_FIELD(token) 打印 "token(sm4)=<SM4密文+随机因子>"
#define XLOG_SM4_EN(value) XLogger::encryptSensitiveValue(value)
#define XLOG_SENSITIVE_FIELD(value) XLOG_DEBUG("{}(sm4)={}", VNAME(value), XLOG_SM4_EN(value))
// XLOG 宏统一判空保护：logger 未就绪/已析构时静默跳过，防止空指针崩溃
#define XLOG_TRACE(...)                                                                                   \
	do                                                                                                    \
	{                                                                                                     \
		if (!XLogger::getInstance()->getLogger())                                                         \
			break;                                                                                        \
		SPDLOG_LOGGER_CALL(XLogger::getInstance()->getLogger().get(), spdlog::level::trace, __VA_ARGS__); \
	} while (0)
#define XLOG_DEBUG(...)                                                                                   \
	do                                                                                                    \
	{                                                                                                     \
		if (!XLogger::getInstance()->getLogger())                                                         \
			break;                                                                                        \
		SPDLOG_LOGGER_CALL(XLogger::getInstance()->getLogger().get(), spdlog::level::debug, __VA_ARGS__); \
	} while (0)
#define XLOG_INFO(...)                                                                                   \
	do                                                                                                   \
	{                                                                                                    \
		if (!XLogger::getInstance()->getLogger())                                                        \
			break;                                                                                       \
		SPDLOG_LOGGER_CALL(XLogger::getInstance()->getLogger().get(), spdlog::level::info, __VA_ARGS__); \
	} while (0)
#define XLOG_WARN(...)                                                                                   \
	do                                                                                                   \
	{                                                                                                    \
		if (!XLogger::getInstance()->getLogger())                                                        \
			break;                                                                                       \
		SPDLOG_LOGGER_CALL(XLogger::getInstance()->getLogger().get(), spdlog::level::warn, __VA_ARGS__); \
	} while (0)
#define XLOG_ERROR(...)                                                                                 \
	do                                                                                                  \
	{                                                                                                   \
		if (!XLogger::getInstance()->getLogger())                                                       \
			break;                                                                                      \
		SPDLOG_LOGGER_CALL(XLogger::getInstance()->getLogger().get(), spdlog::level::err, __VA_ARGS__); \
	} while (0)
#define XLOG_CRITICAL(...)                                                                                   \
	do                                                                                                       \
	{                                                                                                        \
		if (!XLogger::getInstance()->getLogger())                                                            \
			break;                                                                                           \
		SPDLOG_LOGGER_CALL(XLogger::getInstance()->getLogger().get(), spdlog::level::critical, __VA_ARGS__); \
	} while (0)
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
