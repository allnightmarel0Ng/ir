#include "search/query_parser.hpp"
#include "text_processing/stemmer.hpp"
#include "search/set_operations.hpp"

namespace {

constexpr const wchar_t* kOpAnd = L"&&";
constexpr const wchar_t* kOpOr = L"||";
constexpr const wchar_t* kOpNot = L"!";
constexpr const wchar_t* kLeftParen = L"(";
constexpr const wchar_t* kRightParen = L")";

bool IsOperator(const std::wstring& token) {
    return token == kOpAnd || token == kOpOr || token == kOpNot;
}

search::TokenType GetOperatorType(const std::wstring& token) {
    if (token == kOpAnd) return search::TokenType::kOperatorAnd;
    if (token == kOpOr) return search::TokenType::kOperatorOr;
    if (token == kOpNot) return search::TokenType::kOperatorNot;
    return search::TokenType::kTerm;
}

} // anonymous namespace

namespace search {

QueryParser::QueryParser(const std::vector<std::wstring>& tokens)
    : current_pos_(0) {
    Tokenize(tokens);
}

void QueryParser::Tokenize(const std::vector<std::wstring>& input_tokens) {
    tokens_.clear();
    
    for (const auto& token : input_tokens) {
        if (token == kOpAnd) {
            tokens_.emplace_back(TokenType::kOperatorAnd);
        } else if (token == kOpOr) {
            tokens_.emplace_back(TokenType::kOperatorOr);
        } else if (token == kOpNot) {
            tokens_.emplace_back(TokenType::kOperatorNot);
        } else if (token == kLeftParen) {
            tokens_.emplace_back(TokenType::kLeftParen);
        } else if (token == kRightParen) {
            tokens_.emplace_back(TokenType::kRightParen);
        } else if (!token.empty()) {
            tokens_.emplace_back(TokenType::kTerm, token);
        }
    }
    
    tokens_.emplace_back(TokenType::kEnd);
}

containers::HashSet<DocID> QueryParser::Parse(InvertedIndex& index) {
    current_pos_ = 0;
    auto result = ParseOrExpression(index);
    
    if (CurrentToken().type != TokenType::kEnd) {
        // Unexpected token, return empty result
        return containers::HashSet<DocID>();
    }
    
    return result;
}

containers::HashSet<DocID> QueryParser::ParseOrExpression(InvertedIndex& index) {
    auto result = ParseAndExpression(index);
    
    while (Match(TokenType::kOperatorOr)) {
        auto right = ParseAndExpression(index);
        result = SetOr(result, right);
    }
    
    return result;
}

containers::HashSet<DocID> QueryParser::ParseAndExpression(InvertedIndex& index) {
    auto result = ParseNotExpression(index);
    
    while (Match(TokenType::kOperatorAnd)) {
        auto right = ParseNotExpression(index);
        result = SetAnd(result, right);
    }
    
    return result;
}

containers::HashSet<DocID> QueryParser::ParseNotExpression(InvertedIndex& index) {
    if (Match(TokenType::kOperatorNot)) {
        auto result = ParseNotExpression(index);
        containers::HashSet<DocID> all_docs;
        for (const auto& node : index) {
            for (const auto& doc_id : node.value) {
                all_docs.Insert(doc_id);
            }
        }

        for (const auto& doc_id : result) {
            all_docs.Erase(doc_id);
        }
        
        return all_docs;
    }
    
    return ParseTerm(index);
}

containers::HashSet<DocID> QueryParser::ParseTerm(InvertedIndex& index) {
    if (Match(TokenType::kLeftParen)) {
        auto result = ParseOrExpression(index);
        if (!Match(TokenType::kRightParen)) {
            // Mismatched parentheses
            return containers::HashSet<DocID>();
        }
        return result;
    }
    
    if (CurrentToken().type == TokenType::kTerm) {
        auto token = CurrentToken();
        Advance();
        
        auto stem = text_processing::StemRu(token.value);
        // Use operator[] which creates empty set if not found
        return index[stem];
    }
    
    // Unexpected token
    return containers::HashSet<DocID>();
}

Token QueryParser::CurrentToken() const {
    if (current_pos_ >= tokens_.size()) {
        return Token(TokenType::kEnd);
    }
    return tokens_[current_pos_];
}

void QueryParser::Advance() {
    if (current_pos_ < tokens_.size()) {
        current_pos_++;
    }
}

bool QueryParser::Match(TokenType type) {
    if (CurrentToken().type == type) {
        Advance();
        return true;
    }
    return false;
}

} // namespace search

