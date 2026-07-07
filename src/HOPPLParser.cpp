#include <string>
#include <vector>
#include <variant>
#include <stdexcept>

struct Symbol {
    std::string value;
    bool operator==(const Symbol& other) const { return value == other.value; }
};

struct Nil {
    bool operator==(const Nil&) const { return true; }
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
    static std::vector<Token> tokenize(const std::string& text) {
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

    static Expr parse_atom(const Token& token) {
        if (token.type == TokenType::String) {
            return Expr{token.value};
        }
        if (token.value == "true") return Expr{true};
        if (token.value == "false") return Expr{false};
        if (token.value == "nil") return Expr{Nil{}};

        try {
            size_t pos;
            int i_val = std::stoi(token.value, &pos);
            if (pos == token.value.length()) return Expr{i_val};
        } catch (...) {}

        try {
            size_t pos;
            double d_val = std::stod(token.value, &pos);
            if (pos == token.value.length()) return Expr{d_val};
        } catch (...) {}

        return Expr{Symbol{token.value}};
    }

    static std::pair<Expr, size_t> read_form(const std::vector<Token>& tokens, size_t pos) {
        if (pos >= tokens.size()) {
            throw std::invalid_argument("unexpected end of input");
        }
        Token tok = tokens[pos];
        if (tok.type == TokenType::Normal && tok.value == "(") {
            ExprList form;
            pos++;
            while (true) {
                if (pos >= tokens.size()) {
                    throw std::invalid_argument("missing closing parenthesis");
                }
                if (tokens[pos].type == TokenType::Normal && tokens[pos].value == ")") {
                    return {Expr{form}, pos + 1};
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

    static ExprList parse(const std::string& text) {
        std::vector<Token> tokens = tokenize(text);
        ExprList forms;
        size_t pos = 0;
        while (pos < tokens.size()) {
            auto [form, next_pos] = read_form(tokens, pos);
            forms.push_back(form);
            pos = next_pos;
        }
        return forms;
    }

    static Expr parse_one(const std::string& text) {
        ExprList forms = parse(text);
        if (forms.size() != 1) {
            throw std::invalid_argument("expected exactly one form");
        }
        return forms[0];
    }

    static std::string to_string(const Expr& expr) {
        if (std::holds_alternative<Nil>(expr.data)) return "nil";
        if (std::holds_alternative<bool>(expr.data)) return std::get<bool>(expr.data) ? "true" : "false";
        if (std::holds_alternative<int>(expr.data)) return std::to_string(std::get<int>(expr.data));
        if (std::holds_alternative<double>(expr.data)) {
            std::string s = std::to_string(std::get<double>(expr.data));
            s.erase(s.find_last_not_of('0') + 1, std::string::npos);
            if (s.back() == '.') s += '0';
            return s;
        }
        if (std::holds_alternative<Symbol>(expr.data)) return std::get<Symbol>(expr.data).value;
        if (std::holds_alternative<std::string>(expr.data)) return "\"" + std::get<std::string>(expr.data) + "\"";
        if (std::holds_alternative<ExprList>(expr.data)) {
            const auto& list = std::get<ExprList>(expr.data);
            std::string res = "(";
            for (size_t i = 0; i < list.size(); ++i) {
                res += to_string(list[i]);
                if (i < list.size() - 1) res += " ";
            }
            res += ")";
            return res;
        }
        return "";
    }
};