#pragma once

#include <string>

namespace liveutils
{

#if defined(_WIN32)
// Convert Windows UTF-16 and UTF-8 without depending on the active code page.
std::wstring utf8ToWide(const std::string& text);
std::string wideToUtf8(const std::wstring& text);
#endif

// Windows 错误码转换为 UTF-8 文本；其他平台保留数字错误码。
std::string lastErrorMessage(unsigned long error_code);

// 把 HRESULT 翻成 "0x887A0001 (消息)"，用于日志
std::string hResultToText(long hresult);

} // namespace liveutils
