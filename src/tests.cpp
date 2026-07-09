#include "../include/HOPPLParser.h"
#include "../include/PrintVisitor.h"
#include "../include/Primitives.h"
#include "../include/Distribution.h"
#include "../include/AnyRNG.h"
#include <cassert>
#include <variant>
#include <vector>
#include <sstream>
#include <iostream>
#include <cmath>

void test_parser_1_basic_list_and_atoms() {
    Expr ast = HOPPLParser::parse_one("(+ 1 2.5)");
    
    assert(std::holds_alternative<std::vector<Expr>>(ast.value));
    auto list = std::get<std::vector<Expr>>(ast.value);
    
    assert(list.size() == 3);
    assert(std::holds_alternative<SymbolNode>(list[0].value));
    assert(std::get<SymbolNode>(list[0].value).name == "+");
    
    assert(std::holds_alternative<double>(list[1].value));
    assert(std::get<double>(list[1].value) == 1.0);
    
    assert(std::holds_alternative<double>(list[2].value));
    assert(std::get<double>(list[2].value) == 2.5);
}

void test_parser_2_if_node() {
    Expr ast = HOPPLParser::parse_one("(if x y 0)");
    
    assert(std::holds_alternative<std::shared_ptr<IfNode>>(ast.value));
    auto if_node = std::get<std::shared_ptr<IfNode>>(ast.value);
    
    assert(std::holds_alternative<SymbolNode>(if_node->test->value));
    assert(std::get<SymbolNode>(if_node->test->value).name == "x");
    
    assert(std::holds_alternative<SymbolNode>(if_node->then_branch->value));
    assert(std::get<SymbolNode>(if_node->then_branch->value).name == "y");
    
    assert(std::holds_alternative<double>(if_node->else_branch->value));
    assert(std::get<double>(if_node->else_branch->value) == 0.0);
}

void test_parser_3_let_and_fn_nodes() {
    Expr ast = HOPPLParser::parse_one("(let (x (fn (y) y)) x)");
    
    assert(std::holds_alternative<std::shared_ptr<LetNode>>(ast.value));
    auto let_node = std::get<std::shared_ptr<LetNode>>(ast.value);
    
    assert(let_node->binds.size() == 2);
    assert(std::get<SymbolNode>(let_node->binds[0].value).name == "x");
    
    auto fn_ast = let_node->binds[1];
    assert(std::holds_alternative<std::shared_ptr<FnNode>>(fn_ast.value));
    auto fn_node = std::get<std::shared_ptr<FnNode>>(fn_ast.value);
    
    assert(fn_node->params.size() == 1);
    assert(std::get<SymbolNode>(fn_node->params[0].value).name == "y");
    assert(fn_node->body.size() == 1);
    assert(std::get<SymbolNode>(fn_node->body[0].value).name == "y");
    
    assert(let_node->body.size() == 1);
    assert(std::get<SymbolNode>(let_node->body[0].value).name == "x");
}

void test_parser_4_sample_and_observe_nodes() {
    Expr ast1 = HOPPLParser::parse_one("(sample (normal 0 1))");
    
    assert(std::holds_alternative<std::shared_ptr<SampleNode>>(ast1.value));
    auto sample_node = std::get<std::shared_ptr<SampleNode>>(ast1.value);
    
    assert(std::holds_alternative<std::vector<Expr>>(sample_node->dist->value));
    auto dist = std::get<std::vector<Expr>>(sample_node->dist->value);
    assert(dist.size() == 3);
    assert(std::get<SymbolNode>(dist[0].value).name == "normal");

    Expr ast2 = HOPPLParser::parse_one("(observe dist 2.5)");
    
    assert(std::holds_alternative<std::shared_ptr<ObserveNode>>(ast2.value));
    auto obs_node = std::get<std::shared_ptr<ObserveNode>>(ast2.value);
    
    assert(std::holds_alternative<SymbolNode>(obs_node->dist->value));
    assert(std::get<SymbolNode>(obs_node->dist->value).name == "dist");
    
    assert(std::holds_alternative<double>(obs_node->value->value));
    assert(std::get<double>(obs_node->value->value) == 2.5);
}

void test_print_1_basic_operation() {
    Expr ast = HOPPLParser::parse_one("(+ 10.5 20)");

    std::stringstream buffer;
    std::streambuf* old_stdout = std::cout.rdbuf(buffer.rdbuf());

    std::visit(PrintVisitor{}, ast.value);

    std::cout.rdbuf(old_stdout);

    std::string expected = "(+ 10.5 20)";
    assert(buffer.str() == expected);
}

void test_print_2_regular_code() {
    Expr ast = HOPPLParser::parse_one("(let (x 1) (if x (sample (normal 0 1)) (observe dist 2.5)))");

    std::stringstream buffer;
    std::streambuf* old_stdout = std::cout.rdbuf(buffer.rdbuf());

    std::visit(PrintVisitor{}, ast.value);

    std::cout.rdbuf(old_stdout);

    std::string expected = "(let [x 1] (if x (sample (normal 0 1)) (observe dist 2.5)))";
    assert(buffer.str() == expected);
}

void test_print_3_deep_nested_expresion() {
    Expr ast = HOPPLParser::parse_one("(fn (x y) (let (z (+ x y)) (if true (sample (normal z 1)) (observe (normal 0 1) 2.5))))");

    std::stringstream buffer;
    std::streambuf* old_stdout = std::cout.rdbuf(buffer.rdbuf());

    std::visit(PrintVisitor{}, ast.value);

    std::cout.rdbuf(old_stdout);

    std::string expected = "(fn [x y] (let [z (+ x y)] (if true (sample (normal z 1)) (observe (normal 0 1) 2.5))))";
    assert(buffer.str() == expected);
}

void test_primitives_1_basic_math() {
    std::vector<Value> args1 = {2.0, 3.5};
    Value res1 = PRIMITIVES["+"](args1);
    assert(std::get<double>(res1) == 5.5);

    std::vector<Value> args2 = {10.0, 2.0};
    Value res2 = PRIMITIVES["/"](args2);
    assert(std::get<double>(res2) == 5.0);

    std::vector<Value> args3 = {5.0, 8.0};
    Value res3 = PRIMITIVES["<"](args3);
    assert(std::get<double>(res3) == 1.0);
}

void test_distributions_1_normal() {
    Normal norm(0.0, 1.0);
    double lp_norm = norm.log_prob(0.0);
    assert(lp_norm > -1.0 && lp_norm < -0.9);
}

void test_distributions_2_bernoulli() {
    Bernoulli bern(0.7);
    double lp_bern = bern.log_prob(1.0);
    assert(std::abs(lp_bern - std::log(0.7)) < 1e-5);
}

void run_parser_tests() {
    test_parser_1_basic_list_and_atoms();
    test_parser_2_if_node();
    test_parser_3_let_and_fn_nodes();
    test_parser_4_sample_and_observe_nodes();
}

void run_printVisitor_tests(){
    test_print_1_basic_operation();
    test_print_2_regular_code();
    test_print_3_deep_nested_expresion();
}

void run_distributions_tests() {
    test_distributions_1_normal();
    test_distributions_2_bernoulli();
}

int main() {
    run_parser_tests();
    run_printVisitor_tests();
    test_primitives_1_basic_math();
    run_distributions_tests();
    std::cout << "All tests passed successfully.\n";
    return 0;
}