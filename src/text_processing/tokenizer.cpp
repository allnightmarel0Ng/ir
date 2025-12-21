#include "text_processing/tokenizer.hpp"
#include "text_processing/utf8_converter.hpp"
#include <cwctype>

namespace {

constexpr wchar_t kRussianLowerA = L'а';
constexpr wchar_t kRussianLowerYa = L'я';
constexpr wchar_t kRussianUpperA = L'А';
constexpr wchar_t kRussianUpperYa = L'Я';
constexpr wchar_t kRussianLowerYo = L'ё';
constexpr wchar_t kRussianUpperYo = L'Ё';

} // anonymous namespace

namespace text_processing {

bool IsRussianLetter(wchar_t c) {
    return (c >= kRussianLowerA && c <= kRussianLowerYa) ||
           (c >= kRussianUpperA && c <= kRussianUpperYa) ||
           c == kRussianLowerYo || c == kRussianUpperYo;
}

std::vector<std::wstring> TokenizeRu(const std::string& text) {
    std::wstring wtext = Utf8ToWstring(text);

    for (auto& c : wtext)
        c = towlower(c);

    std::vector<std::wstring> tokens;
    std::wstring current;

    for (wchar_t c : wtext) {
        if (IsRussianLetter(c)) {
            current += c;
        } else if (!current.empty()) {
            tokens.push_back(current);
            current.clear();
        }
    }

    if (!current.empty())
        tokens.push_back(current);

    return tokens;
}

} // namespace text_processing

