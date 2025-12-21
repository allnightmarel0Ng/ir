#ifndef SEARCH_QUERY_PARSER_HPP
#define SEARCH_QUERY_PARSER_HPP

#include "containers/hash_map.hpp"
#include "containers/hash_set.hpp"
#include <vector>
#include <string>
#include <cstddef>

namespace search {

using DocID = std::string;
using InvertedIndex = containers::HashMap<std::wstring, containers::HashSet<DocID>>;

enum class TokenType {
    kTerm,
    kOperatorAnd,
    kOperatorOr,
    kOperatorNot,
    kLeftParen,
    kRightParen,
    kEnd
};

struct Token {
    TokenType type;
    std::wstring value;
    
    Token(TokenType t, const std::wstring& v = L"") : type(t), value(v) {}
};

class QueryParser {
public:
    QueryParser(const std::vector<std::wstring>& tokens);
    containers::HashSet<DocID> Parse(InvertedIndex& index);
    
private:
    std::vector<Token> tokens_;
    size_t current_pos_;
    
    void Tokenize(const std::vector<std::wstring>& input_tokens);
    containers::HashSet<DocID> ParseOrExpression(InvertedIndex& index);
    containers::HashSet<DocID> ParseAndExpression(InvertedIndex& index);
    containers::HashSet<DocID> ParseNotExpression(InvertedIndex& index);
    containers::HashSet<DocID> ParseTerm(InvertedIndex& index);
    Token CurrentToken() const;
    void Advance();
    bool Match(TokenType type);
};

} // namespace search

#endif // SEARCH_QUERY_PARSER_HPP

