#pragma once

#include <string>
#include <vector>
#include <utility>
#include "Expr.h"

enum class TokenType {
    Normal,
    String
};

struct Token {
    TokenType type;
    std::string value;
};

class HOPPLParser {
public:
    static std::vector<Token> tokenize(const std::string& text);
    static Expr parse_atom(const Token& token);
    static Expr transform_list_to_node(const std::vector<Expr>& raw_list);
    static std::pair<Expr, size_t> read_form(const std::vector<Token>& tokens, size_t pos);
    static std::vector<Expr> parse(const std::string& text);
    static Expr parse_one(const std::string& text);
    static Expr parse_file(const std::string& filename);
};