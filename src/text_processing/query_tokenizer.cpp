#include "text_processing/query_tokenizer.hpp"
#include "text_processing/utf8_converter.hpp"
#include "text_processing/tokenizer.hpp"
#include <cwctype>

namespace {

constexpr wchar_t kOpAnd1 = L'&';
constexpr wchar_t kOpOr1 = L'|';
constexpr wchar_t kOpNot = L'!';
constexpr wchar_t kLeftParen = L'(';
constexpr wchar_t kRightParen = L')';
constexpr wchar_t kSpace = L' ';

bool IsOperatorChar(wchar_t c) {
    return c == kOpAnd1 || c == kOpOr1 || c == kOpNot || 
           c == kLeftParen || c == kRightParen;
}

} // anonymous namespace

namespace text_processing {

std::vector<std::wstring> TokenizeQuery(const std::string& query) {
    std::wstring wquery = Utf8ToWstring(query);
    
    // Convert to lowercase for text parts
    for (auto& c : wquery) {
        if (!IsOperatorChar(c) && c != kSpace) {
            c = towlower(c);
        }
    }
    
    std::vector<std::wstring> tokens;
    std::wstring current;
    
    for (size_t i = 0; i < wquery.length(); ++i) {
        wchar_t c = wquery[i];
        
        if (c == kSpace) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }
        
        if (IsOperatorChar(c)) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            
            // Handle multi-character operators
            if (c == kOpAnd1 && i + 1 < wquery.length() && wquery[i + 1] == kOpAnd1) {
                tokens.push_back(L"&&");
                i++; // Skip next character
            } else if (c == kOpOr1 && i + 1 < wquery.length() && wquery[i + 1] == kOpOr1) {
                tokens.push_back(L"||");
                i++; // Skip next character
            } else if (c == kOpNot) {
                tokens.push_back(L"!");
            } else if (c == kLeftParen) {
                tokens.push_back(L"(");
            } else if (c == kRightParen) {
                tokens.push_back(L")");
            }
        } else if (IsRussianLetter(c)) {
            current += c;
        } else {
            // Other characters - treat as separators
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        }
    }
    
    if (!current.empty()) {
        tokens.push_back(current);
    }
    
    return tokens;
}

} // namespace text_processing

