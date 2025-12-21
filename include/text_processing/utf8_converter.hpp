#ifndef TEXT_PROCESSING_UTF8_CONVERTER_HPP
#define TEXT_PROCESSING_UTF8_CONVERTER_HPP

#include <string>
#include <codecvt>
#include <locale>

namespace text_processing {

std::wstring Utf8ToWstring(const std::string& str);
std::string WstringToUtf8(const std::wstring& wstr);

} // namespace text_processing

#endif // TEXT_PROCESSING_UTF8_CONVERTER_HPP

