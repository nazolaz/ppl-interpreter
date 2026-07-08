#ifndef INFERENCE_CONTROLLER_H
#define INFERENCE_CONTROLLER_H

#include "HOPPLParser.h"
#include "AnyRNG.h"
#include <string>

class InferenceController {
protected:
    AnyRNG rng;

public:
    InferenceController(AnyRNG generator);
    virtual ~InferenceController() = default;

    virtual void run(const std::string& filename, int iterations);
    virtual void run_ast(const Expr& ast, int iterations) = 0;
};

class LikelihoodWeightingController : public InferenceController {
public:
    LikelihoodWeightingController(AnyRNG generator);
    
    void run_ast(const Expr& ast, int iterations) override;
};

#endif