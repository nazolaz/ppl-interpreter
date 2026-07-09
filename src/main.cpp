#include "../include/HOPPLParser.h"
#include "../include/PrintVisitor.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file_to_parse>\n";
        return 1;
    }

    try {
        Expr ast = HOPPLParser::parse_file(argv[1]);
        
        std::cout << "Success! Here is the parsed AST:\n";
        std::visit(PrintVisitor{}, ast.value);
        std::cout << "\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Parser crashed with error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}