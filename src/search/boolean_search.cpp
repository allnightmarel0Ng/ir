#include "search/boolean_search.hpp"
#include "search/query_parser.hpp"
#include "text_processing/query_tokenizer.hpp"

namespace search {

containers::HashSet<DocID> BooleanSearchRu(const std::string& query, InvertedIndex& index) {
    // Tokenize the query (handles operators &&, ||, ! and parentheses)
    auto tokens = text_processing::TokenizeQuery(query);
    
    if (tokens.empty()) {
        return containers::HashSet<DocID>();
    }
    
    // Parse using recursive descent parser with proper operator precedence
    QueryParser parser(tokens);
    return parser.Parse(index);
}

} // namespace search

