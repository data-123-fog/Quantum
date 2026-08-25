#ifndef CPP_TOKENIZER_HPP
#define CPP_TOKENIZER_HPP

#include <string>
#include <vector>
#include <regex>
#include <cctype>

namespace Tokenizer {

enum class TokenType {
    KEYWORD,       // int, float, class, if, else, for, while, return, etc.
    IDENTIFIER,    // variable names, function names
    OPERATOR,      // +, -, *, /, =, ==, !=, <, >, <=, >=, &&, ||, !, ++, --, etc.
    DELIMITER,     // ;, ,, ., ->, ::
    BRACKET_OPEN,  // (, [, {
    BRACKET_CLOSE, // ), ], }
    NUMBER,        // 123, 3.14, 0xFF
    STRING,        // "hello", 'c'
    COMMENT,       // //, /* */
    PREPROCESSOR,  // #include, #define, etc.
    WHITESPACE,    // spaces, tabs, newlines
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
    size_t line;
    size_t column;

    Token(TokenType t, const std::string& v, size_t l, size_t c)
        : type(t), value(v), line(l), column(c) {}
};

class CppTokenizer {
public:
    std::vector<Token> tokenize(const std::string& source) {
        std::vector<Token> tokens;
        size_t i = 0;
        size_t line = 1;
        size_t column = 1;

        while (i < source.length()) {

            // Skip whitespace but track position
            if (std::isspace(static_cast<unsigned char>(source[i]))) {
                if (source[i] == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
                i++;
                continue;
            }

            // Comments
            if (source[i] == '/' && i + 1 < source.length()) {
                if (source[i + 1] == '/') {
                    // Single line comment
                    size_t start = i;
                    size_t startLine = line;
                    size_t startColumn = column; // ФИКС: запоминаем стартовую позицию
                    while (i < source.length() && source[i] != '\n') {
                        i++;
                        column++;
                    }
                    tokens.emplace_back(TokenType::COMMENT, source.substr(start, i - start), startLine, startColumn);
                    continue;
                }
                if (source[i + 1] == '*') {
                    // Multi-line comment
                    size_t start = i;
                    size_t startLine = line;
                    size_t startColumn = column; // ФИКС
                    i += 2;
                    column += 2;
                    while (i + 1 < source.length() && !(source[i] == '*' && source[i + 1] == '/')) {
                        if (source[i] == '\n') {
                            line++;
                            column = 1;
                        } else {
                            column++;
                        }
                        i++;
                    }
                    i += 2;
                    column += 2;
                    tokens.emplace_back(TokenType::COMMENT, source.substr(start, i - start), startLine, startColumn);
                    continue;
                }
            }

            // Preprocessor
            if (source[i] == '#') {
                size_t start = i;
                size_t startColumn = column; // ФИКС
                while (i < source.length() && source[i] != '\n') {
                    i++;
                    column++;
                }
                tokens.emplace_back(TokenType::PREPROCESSOR, source.substr(start, i - start), line, startColumn);
                continue;
            }

            // Strings
            if (source[i] == '"' || source[i] == '\'') {
                char quote = source[i];
                size_t start = i;
                size_t startColumn = column; // ФИКС
                i++;
                column++;
                while (i < source.length() && source[i] != quote) {
                    if (source[i] == '\\' && i + 1 < source.length()) {
                        i++;
                        column++;
                    }
                    i++;
                    column++;
                }
                if (i < source.length()) {
                    i++; // closing quote
                    column++;
                }
                tokens.emplace_back(TokenType::STRING, source.substr(start, i - start), line, startColumn);
                continue;
            }

            // Numbers (включая hex: 0xFF, 0X1A3)
            if (std::isdigit(static_cast<unsigned char>(source[i])) ||
                (source[i] == '.' && i + 1 < source.length() && std::isdigit(static_cast<unsigned char>(source[i + 1])))) {
                size_t start = i;
                size_t startColumn = column; // ФИКС
                while (i < source.length() && (
                           std::isdigit(static_cast<unsigned char>(source[i])) ||
                           source[i] == '.' ||
                           source[i] == 'e' || source[i] == 'E' ||
                           source[i] == 'x' || source[i] == 'X' ||
                           (source[i] >= 'a' && source[i] <= 'f') || // ФИКС: hex-цифры a-f
                           (source[i] >= 'A' && source[i] <= 'F')    // ФИКС: hex-цифры A-F
                       )) {
                    i++;
                    column++;
                }
                tokens.emplace_back(TokenType::NUMBER, source.substr(start, i - start), line, startColumn);
                continue;
            }

            // Operators (check multi-char first)
            static const std::vector<std::string> multiOps = {
                "->", "::", "==", "!=", "<=", ">=", "&&", "||", "++", "--",
                "+=", "-=", "*=", "/=", "%=", "<<", ">>", "<<=", ">>="
            };
            bool foundMulti = false;
            for (const auto& op : multiOps) {
                if (i + op.length() <= source.length() && source.substr(i, op.length()) == op) {
                    tokens.emplace_back(TokenType::OPERATOR, op, line, column);
                    i += op.length();
                    column += op.length();
                    foundMulti = true;
                    break;
                }
            }
            if (foundMulti) continue;

            // Single char operators
            static const std::string singleOps = "+-*/%=!&|^~<>";
            if (singleOps.find(source[i]) != std::string::npos) {
                tokens.emplace_back(TokenType::OPERATOR, std::string(1, source[i]), line, column);
                i++;
                column++;
                continue;
            }

            // Brackets
            if (source[i] == '(' || source[i] == '[' || source[i] == '{') {
                tokens.emplace_back(TokenType::BRACKET_OPEN, std::string(1, source[i]), line, column);
                i++;
                column++;
                continue;
            }
            if (source[i] == ')' || source[i] == ']' || source[i] == '}') {
                tokens.emplace_back(TokenType::BRACKET_CLOSE, std::string(1, source[i]), line, column);
                i++;
                column++;
                continue;
            }

            // Delimiters
            if (source[i] == ';' || source[i] == ',' || source[i] == ':' || source[i] == '.') {
                tokens.emplace_back(TokenType::DELIMITER, std::string(1, source[i]), line, column);
                i++;
                column++;
                continue;
            }

            // Identifiers and keywords
            if (std::isalpha(static_cast<unsigned char>(source[i])) || source[i] == '_') {
                size_t start = i;
                size_t startColumn = column; // ФИКС
                while (i < source.length() && (std::isalnum(static_cast<unsigned char>(source[i])) || source[i] == '_')) {
                    i++;
                    column++;
                }
                std::string word = source.substr(start, i - start);
                TokenType type = isKeyword(word) ? TokenType::KEYWORD : TokenType::IDENTIFIER;
                tokens.emplace_back(type, word, line, startColumn);
                continue;
            }

            // Unknown
            tokens.emplace_back(TokenType::UNKNOWN, std::string(1, source[i]), line, column);
            i++;
            column++;
        }

        return tokens;
    }

private:
    bool isKeyword(const std::string& word) {
        static const std::vector<std::string> keywords = {
            "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor",
            "bool", "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t",
            "class", "compl", "concept", "const", "consteval", "constexpr", "constinit",
            "const_cast", "continue", "co_await", "co_return", "co_yield", "decltype",
            "default", "delete", "do", "double", "dynamic_cast", "else", "enum",
            "explicit", "export", "extern", "false", "float", "for", "friend", "goto",
            "if", "inline", "int", "long", "mutable", "namespace", "new", "noexcept",
            "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private",
            "protected", "public", "register", "reinterpret_cast", "requires", "return",
            "short", "signed", "sizeof", "static", "static_assert", "static_cast",
            "struct", "switch", "template", "this", "thread_local", "throw", "true",
            "try", "typedef", "typeid", "typename", "union", "unsigned", "using",
            "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
        };
        for (const auto& kw : keywords) {
            if (kw == word) return true;
        }
        return false;
    }
};

} // namespace Tokenizer

#endif
