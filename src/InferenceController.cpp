#include <string>
#include <random>

using ExprPtr = std::string; 

class HOPPLParser {
public:
    ExprPtr parse_file(const std::string& filename) { 
        return "parsed_ast_mock"; 
    }
};

template <typename RNGType = std::mt19937>
class InferenceController {
protected:
    RNGType rng;

public:
    InferenceController(RNGType generator = RNGType(std::random_device{}())) : rng(generator) {}
    virtual ~InferenceController() = default;

    virtual void run(const std::string& filename, int iterations) {
        HOPPLParser parser;
        ExprPtr ast = parser.parse_file(filename);

        this->run_ast(ast, iterations);
    }

    virtual void run_ast(ExprPtr ast, int iterations) = 0;
};

template <typename RNGType = std::mt19937>
class LikelihoodWeightingController : public InferenceController<RNGType> {
public:
    LikelihoodWeightingController(RNGType generator = RNGType(std::random_device{}())) 
        : InferenceController<RNGType>(generator) {}
    
    void run_ast(ExprPtr ast, int iterations) override {
    }
};