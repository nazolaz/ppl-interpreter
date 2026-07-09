#include "../include/HOPPLParser.h"
#include <stdexcept>
#include <fstream>
#include <sstream>

std::vector<Token> HOPPLParser::tokenize(const std::string& text) {
    std::vector<Token> tokens;
    size_t i = 0;
    size_t n = text.length();

    while (i < n) {
        char c = text[i];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',') {
            i++;
        } else if (c == ';') {
            while (i < n && text[i] != '\n') {
                i++;
            } 
        } else if (c == '(' || c == ')' || c == '[' || c == ']') {
            tokens.push_back({TokenType::Normal, (c == '(' || c == '[') ? "(" : ")"});
            i++;
        } else if (c == '"') {
            size_t j = i + 1;
            std::string buf;
            while (j < n && text[j] != '"') {
                if (text[j] == '\\' && j + 1 < n) {
                    j++;
                }
                buf += text[j];
                j++;
            }
            if (j >= n) {
                throw std::invalid_argument("unterminated string literal");
            }
            tokens.push_back({TokenType::String, buf});
            i = j + 1;
        } else {
            size_t j = i;
            while (j < n && text[j] != ' ' && text[j] != '\t' && text[j] != '\n' &&
                   text[j] != '\r' && text[j] != ',' && text[j] != '(' &&
                   text[j] != ')' && text[j] != '[' && text[j] != ']' &&
                   text[j] != ';' && text[j] != '"') {
                j++;
            }
            tokens.push_back({TokenType::Normal, text.substr(i, j - i)});
            i = j;
        }
    }
    return tokens;
}

Expr HOPPLParser::parse_atom(const Token& token) {
    if (token.type == TokenType::String) {
        return Expr{token.value};
    }

    try {
        size_t pos;
        int i_val = std::stoi(token.value, &pos);
        if (pos == token.value.length()) return Expr{static_cast<double>(i_val)};
    } catch (...) {}

    try {
        size_t pos;
        double d_val = std::stod(token.value, &pos);
        if (pos == token.value.length()) return Expr{d_val};
    } catch (...) {}

    return Expr{token.value};
}

Expr HOPPLParser::transform_list_to_node(const std::vector<Expr>& raw_list) {
    if (raw_list.empty()) {
        return Expr{raw_list};
    }

    if (std::holds_alternative<SymbolNode>(raw_list[0].value)) {
        std::string head = std::get<SymbolNode>(raw_list[0].value).name;

        if (head == "let") {
            auto let_node = std::make_shared<LetNode>();
            let_node->binds = std::get<std::vector<Expr>>(raw_list[1].value);
            for (size_t i = 2; i < raw_list.size(); ++i) {
                let_node->body.push_back(raw_list[i]);
            }
            return Expr{let_node};
        }

        if (head == "if") {
            auto if_node = std::make_shared<IfNode>();
            if_node->test = std::make_shared<Expr>(raw_list[1]);
            if_node->then_branch = std::make_shared<Expr>(raw_list[2]);
            if_node->else_branch = std::make_shared<Expr>(raw_list[3]);
            return Expr{if_node};
        }

        if (head == "fn") {
            auto fn_node = std::make_shared<FnNode>();
            fn_node->params = std::get<std::vector<Expr>>(raw_list[1].value);
            for (size_t i = 2; i < raw_list.size(); ++i) {
                fn_node->body.push_back(raw_list[i]);
            }
            return Expr{fn_node};
        }

        if (head == "sample") {
            auto sample_node = std::make_shared<SampleNode>();
            sample_node->dist = std::make_shared<Expr>(raw_list[1]);
            return Expr{sample_node};
        }

        if (head == "observe") {
            auto observe_node = std::make_shared<ObserveNode>();
            observe_node->dist = std::make_shared<Expr>(raw_list[1]);
            observe_node->value = std::make_shared<Expr>(raw_list[2]);
            return Expr{observe_node};
        }
    }

    return Expr{raw_list};
}

std::pair<Expr, size_t> HOPPLParser::read_form(const std::vector<Token>& tokens, size_t pos) {
    if (pos >= tokens.size()) {
        throw std::invalid_argument("unexpected end of input");
    }
    Token tok = tokens[pos];
    if (tok.type == TokenType::Normal && tok.value == "(") {
        std::vector<Expr> form;
        pos++;
        while (true) {
            if (pos >= tokens.size()) {
                throw std::invalid_argument("missing closing parenthesis");
            }
            if (tokens[pos].type == TokenType::Normal && tokens[pos].value == ")") {
                return {transform_list_to_node(form), pos + 1};
            }
            auto [sub_expr, next_pos] = read_form(tokens, pos);
            form.push_back(sub_expr);
            pos = next_pos;
        }
    }
    if (tok.type == TokenType::Normal && tok.value == ")") {
        throw std::invalid_argument("unexpected )");
    }
    return {parse_atom(tok), pos + 1};
}

std::vector<Expr> HOPPLParser::parse(const std::string& text) {
    std::vector<Token> tokens = tokenize(text);
    std::vector<Expr> forms;
    size_t pos = 0;
    while (pos < tokens.size()) {
        auto [form, next_pos] = read_form(tokens, pos);
        forms.push_back(form);
        pos = next_pos;
    }
    return forms;
}

Expr HOPPLParser::parse_one(const std::string& text) {
    std::vector<Expr> forms = parse(text);
    if (forms.size() != 1) {
        throw std::invalid_argument("expected exactly one form");
    }
    return forms[0];
}

Expr HOPPLParser::parse_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    return parse_one(buffer.str());
}