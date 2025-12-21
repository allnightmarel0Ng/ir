#ifndef TEXT_PROCESSING_TOKENIZER_HPP
#define TEXT_PROCESSING_TOKENIZER_HPP

#include <string>
#include <vector>

namespace text_processing {

bool IsRussianLetter(wchar_t c);
std::vector<std::wstring> TokenizeRu(const std::string& text);

} // namespace text_processing

#endif // TEXT_PROCESSING_TOKENIZER_HPP

