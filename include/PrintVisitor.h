#pragma once

#include <iostream>
#include <variant>
#include <vector>
#include <memory>
#include "Expr.h"

struct PrintVisitor {
    void operator()(double val) const {
        std::cout << val;
    }

    void operator()(const SymbolNode& node) const {
        std::cout << node.name;
    }

    void operator()(const std::shared_ptr<LetNode>& node) const {
        std::cout << "(let [";
        for (size_t i = 0; i < node->binds.size(); ++i) {
            std::visit(PrintVisitor{}, node->binds[i].value);
            if (i < node->binds.size() - 1) std::cout << " ";
        }
        std::cout << "] ";
        for (size_t i = 0; i < node->body.size(); ++i) {
            std::visit(PrintVisitor{}, node->body[i].value);
            if (i < node->body.size() - 1) std::cout << " ";
        }
        std::cout << ")";
    }

    void operator()(const std::shared_ptr<IfNode>& node) const {
        std::cout << "(if ";
        std::visit(PrintVisitor{}, node->test->value);
        std::cout << " ";
        std::visit(PrintVisitor{}, node->then_branch->value);
        std::cout << " ";
        std::visit(PrintVisitor{}, node->else_branch->value);
        std::cout << ")";
    }

    void operator()(const std::shared_ptr<FnNode>& node) const {
        std::cout << "(fn [";
        for (size_t i = 0; i < node->params.size(); ++i) {
            std::visit(PrintVisitor{}, node->params[i].value);
            if (i < node->params.size() - 1) std::cout << " ";
        }
        std::cout << "] ";
        for (size_t i = 0; i < node->body.size(); ++i) {
            std::visit(PrintVisitor{}, node->body[i].value);
            if (i < node->body.size() - 1) std::cout << " ";
        }
        std::cout << ")";
    }

    void operator()(const std::shared_ptr<SampleNode>& node) const {
        std::cout << "(sample ";
        std::visit(PrintVisitor{}, node->dist->value);
        std::cout << ")";
    }

    void operator()(const std::shared_ptr<ObserveNode>& node) const {
        std::cout << "(observe ";
        std::visit(PrintVisitor{}, node->dist->value);
        std::cout << " ";
        std::visit(PrintVisitor{}, node->value->value);
        std::cout << ")";
    }

    void operator()(const std::vector<Expr>& list) const {
        std::cout << "(";
        for (size_t i = 0; i < list.size(); ++i) {
            std::visit(PrintVisitor{}, list[i].value);
            if (i < list.size() - 1) std::cout << " ";
        }
        std::cout << ")";
    }
};