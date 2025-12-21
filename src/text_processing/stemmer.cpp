#include "text_processing/stemmer.hpp"
#include <vector>

namespace {

constexpr size_t kMinWordLength = 3;
constexpr size_t kMinStemLength = 2;

const std::vector<std::wstring> kEndings = {
    L"иями", L"ями", L"ами",
    L"ией", L"ий", L"ый", L"ой",
    L"ия", L"ья", L"ие", L"ье",
    L"ых", L"ую", L"юю",
    L"ая", L"яя",
    L"ом", L"ем",
    L"ах", L"ях",
    L"ы", L"и", L"а", L"я", L"о", L"е", L"у", L"ю"
};

} // anonymous namespace

namespace text_processing {

std::wstring StemRu(const std::wstring& word) {
    if (word.length() <= kMinWordLength)
        return word;

    for (const auto& end : kEndings) {
        if (word.length() > end.length() + kMinStemLength &&
            word.compare(word.length() - end.length(), end.length(), end) == 0) {
            return word.substr(0, word.length() - end.length());
        }
    }
    return word;
}

} // namespace text_processing

