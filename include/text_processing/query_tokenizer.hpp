#ifndef TEXT_PROCESSING_QUERY_TOKENIZER_HPP
#define TEXT_PROCESSING_QUERY_TOKENIZER_HPP

#include <string>
#include <vector>

namespace text_processing {

std::vector<std::wstring> TokenizeQuery(const std::string& query);

} // namespace text_processing

#endif // TEXT_PROCESSING_QUERY_TOKENIZER_HPP

