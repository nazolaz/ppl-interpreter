#pragma once

#include <string>
#include <vector>
#include <variant>
#include <memory>

struct Expr;

struct SymbolNode {
    std::string name;
};

struct LetNode {
    std::vector<Expr> binds;
    std::vector<Expr> body;

    bool hasBinding(int pair_index) const;
    std::string getBindingName(int pair_index) const;
    Expr getBindingValue(int pair_index) const;
};

struct IfNode {
    std::shared_ptr<Expr> test;
    std::shared_ptr<Expr> then_branch;
    std::shared_ptr<Expr> else_branch;
};

struct FnNode {
    std::vector<Expr> params;
    std::vector<Expr> body;
};

struct SampleNode {
    std::shared_ptr<Expr> dist;
};

struct ObserveNode {
    std::shared_ptr<Expr> dist;
    std::shared_ptr<Expr> value;
};

struct Expr {
    std::variant<
        double,
        SymbolNode,
        std::shared_ptr<LetNode>,
        std::shared_ptr<IfNode>,
        std::shared_ptr<FnNode>,
        std::shared_ptr<SampleNode>,
        std::shared_ptr<ObserveNode>,
        std::vector<Expr>
    > value;    

    Expr() : value(std::vector<Expr>{}) {}
    Expr(double d) : value(d) {}
    Expr(const std::string& s) : value(SymbolNode{s}) {}
    Expr(const char* s) : value(SymbolNode{std::string(s)}) {}
    Expr(const std::vector<Expr>& l) : value(l) {}
    Expr(std::shared_ptr<LetNode> n) : value(n) {}
    Expr(std::shared_ptr<IfNode> n) : value(n) {}
    Expr(std::shared_ptr<FnNode> n) : value(n) {}
    Expr(std::shared_ptr<SampleNode> n) : value(n) {}
    Expr(std::shared_ptr<ObserveNode> n) : value(n) {}
};

inline bool LetNode::hasBinding(int pair_index) const { 
    return (2 * pair_index) < binds.size(); 
}

inline std::string LetNode::getBindingName(int pair_index) const { 
    return std::get<SymbolNode>(binds[2 * pair_index].value).name; 
}

inline Expr LetNode::getBindingValue(int pair_index) const { 
    return binds[(2 * pair_index) + 1]; 
}
