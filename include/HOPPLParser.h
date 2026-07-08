#ifndef HOPPL_PARSER_H
#define HOPPL_PARSER_H

#include <string>
#include <vector>
#include <variant>

struct Symbol {
    std::string value;
    bool operator==(const Symbol& other) const;
};

struct Nil {
    bool operator==(const Nil&) const;
};

struct Expr;
using ExprList = std::vector<Expr>;

struct Expr {
    std::variant<Nil, bool, int, double, std::string, Symbol, ExprList> data;
};

enum class TokenType {
    String,
    Normal
};

struct Token {
    TokenType type;
    std::string value;
};

class HOPPLParser {
public:
    static std::vector<Token> tokenize(const std::string& text);
    static Expr parse_atom(const Token& token);
    static std::pair<Expr, size_t> read_form(const std::vector<Token>& tokens, size_t pos);
    static ExprList parse(const std::string& text);
    static Expr parse_one(const std::string& text);
    static Expr parse_file(const std::string& filename);
    static std::string to_string(const Expr& expr);
};

#endif