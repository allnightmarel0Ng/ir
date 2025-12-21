#include "text_processing/utf8_converter.hpp"

namespace {

std::wstring_convert<std::codecvt_utf8<wchar_t>>& GetConverter() {
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter;
}

} // anonymous namespace

namespace text_processing {

std::wstring Utf8ToWstring(const std::string& str) {
    auto& conv = GetConverter();
    return conv.from_bytes(str);
}

std::string WstringToUtf8(const std::wstring& wstr) {
    auto& conv = GetConverter();
    return conv.to_bytes(wstr);
}

} // namespace text_processing

