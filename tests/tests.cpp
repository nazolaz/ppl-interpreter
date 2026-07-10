#include "../include/HOPPLParser.h"
#include "../include/PrintVisitor.h"
#include "../include/Primitives.h"
#include "../include/Distribution.h"
#include "../include/AnyRNG.h"
#include "../include/Machine.h"
#include "../include/Message.h"
#include <cassert>
#include <variant>
#include <vector>
#include <sstream>
#include <iostream>
#include <cmath>

// PARSER TESTS 

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

// PrintVisitor TESTS 

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

// PRIMITIVES AND DISTRIBUTIONS TEST

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

// MACHINE SYMBOL AND LITERAL EVALUATION TESTS

void test_machine_1_literal_evaluation() {
    Machine m;
    m.C.push_back(EvInstr{Expr(42.0), Env{}, Address{}});
    
    Message msg = m.resume();

    assert(std::holds_alternative<DoneMsg>(msg));
    auto done_msg = std::get<DoneMsg>(msg);
    
    assert(std::holds_alternative<double>(done_msg.value));
    assert(std::get<double>(done_msg.value) == 42.0);
    
    assert(m.V.size() == 1);
    assert(std::get<double>(m.V.back()) == 42.0);
}

void test_machine_2_symbol_evaluation() {
    Machine m;
    
    Env env;
    env["mu"] = 3.14;

    m.C.push_back(EvInstr{Expr("mu"), env, Address{}});
    
    Message msg = m.resume();

    assert(std::holds_alternative<DoneMsg>(msg));
    auto done_msg = std::get<DoneMsg>(msg);
    
    assert(std::holds_alternative<double>(done_msg.value));
    assert(std::get<double>(done_msg.value) == 3.14);
    assert(m.V.size() == 1);
    assert(std::get<double>(m.V.back()) == 3.14);
}

void test_machine_3_symbol_not_found() {
    Machine m;
    
    m.C.push_back(EvInstr{Expr("x"), Env{}, Address{}});
    
    bool threw_exception = false;
    try {
        m.resume();
    } catch (const std::runtime_error& e) {
        std::string err_msg = e.what();
        assert(err_msg.find("NameError: x") != std::string::npos);
        threw_exception = true;
    }
    
    assert(threw_exception);
}

// MACHINE FUNCTION EVALUATION TESTS

Value run_file_through_machine(const std::string& filepath, Env env = Env{}) {
    Expr ast = HOPPLParser::parse_file(filepath);

    Machine m;
    m.C.push_back(EvInstr{ast, env, Address{}});
    Message msg = m.resume();

    assert(std::holds_alternative<DoneMsg>(msg));
    return std::get<DoneMsg>(msg).value;
}

void test_machine_integration_1_basic_math() {
    Value res = run_file_through_machine("ppl_programs/test_machine_eval_1.txt");
    assert(std::get<double>(res) == 20.0);
}

void test_machine_integration_2_nested_math() {
    Value res = run_file_through_machine("ppl_programs/test_machine_eval_2.txt");
    assert(std::get<double>(res) == 15.0);
}

void test_machine_integration_3_comparison() {
    Value res = run_file_through_machine("ppl_programs/test_machine_eval_3.txt");
    assert(std::get<double>(res) == 1.0);
}

void test_machine_integration_4_env_vars() {
    Env env;
    env["x"] = 10.0;
    env["y"] = 3.0;
    
    Value res = run_file_through_machine("ppl_programs/test_machine_eval_4.txt", env);
    assert(std::get<double>(res) == 16.0);
}

void test_machine_integration_5_complex_calc() {
    Value res = run_file_through_machine("ppl_programs/test_machine_eval_5.txt");
    assert(std::get<double>(res) == 125.0);
}

// LET IMPLEMENTATION TESTS

void test_machine_let_1_simple() {
    Value res = run_file_through_machine("ppl_programs/test_machine_let_1.txt");
    assert(std::get<double>(res) == 15.0);
}

void test_machine_let_2_dependent() {
    Value res = run_file_through_machine("ppl_programs/test_machine_let_2.txt");
    assert(std::get<double>(res) == 25.0);
}

void test_machine_let_3_body() {
    Value res = run_file_through_machine("ppl_programs/test_machine_let_3.txt");
    assert(std::get<double>(res) == 12.0);
}

// HIGHER ORDER AND FUNCTION TESTS

void test_machine_fn_1_basic() {
    Value res = run_file_through_machine("ppl_programs/test_machine_fn_1.txt");
    assert(std::get<double>(res) == 15.0);
}

void test_machine_fn_2_closure() {
    Value res = run_file_through_machine("ppl_programs/test_machine_fn_2.txt");
    assert(std::get<double>(res) == 15.0);
}

void test_machine_fn_3_higher_order() {
    Value res = run_file_through_machine("ppl_programs/test_machine_fn_3.txt");
    assert(std::get<double>(res) == 20.0);
}

// IF IMPLEMENTATION TESTS

void test_machine_if_1_true() {
    Value res = run_file_through_machine("ppl_programs/test_machine_if_1.txt");
    assert(std::get<double>(res) == 42.0);
}

void test_machine_if_2_false() {
    Value res = run_file_through_machine("ppl_programs/test_machine_if_2.txt");
    assert(std::get<double>(res) == 99.0);
}

void test_machine_if_3_nested() {
    Value res = run_file_through_machine("ppl_programs/test_machine_if_3.txt");
    assert(std::get<double>(res) == 10.0);
}


// TEST RUNNERS

void run_machine_tests() {
    test_machine_1_literal_evaluation();
    test_machine_2_symbol_evaluation();
    test_machine_3_symbol_not_found();
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

void run_machine_function_eval_tests() {
    test_machine_integration_1_basic_math();
    test_machine_integration_2_nested_math();
    test_machine_integration_3_comparison();
    test_machine_integration_4_env_vars();
    test_machine_integration_5_complex_calc();
}


void run_machine_let_tests() {
    test_machine_let_1_simple();
    test_machine_let_2_dependent();
    test_machine_let_3_body();
}

void run_machine_fn_tests() {
    test_machine_fn_1_basic();
    test_machine_fn_2_closure();
    test_machine_fn_3_higher_order();
}

void run_machine_if_tests() {
    test_machine_if_1_true();
    test_machine_if_2_false();
    test_machine_if_3_nested();
}

int main() {
    run_parser_tests();
    run_printVisitor_tests();
    test_primitives_1_basic_math();
    run_distributions_tests();
    run_machine_tests();
    run_machine_function_eval_tests();
    run_machine_let_tests();
    run_machine_fn_tests();
    run_machine_if_tests();
    std::cout << "All tests passed successfully.\n";
    return 0;
}