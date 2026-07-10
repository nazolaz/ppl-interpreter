#pragma once

#include <vector>
#include <string>
#include <memory>
#include "AnyRNG.h"

struct Distribution {
    virtual ~Distribution() = default;
    virtual double sample(AnyRNG& rng) = 0;
    virtual double log_prob(double x) = 0;
};

struct Normal : public Distribution {
    double mu;
    double sigma;

    Normal(double mu, double sigma);
    double sample(AnyRNG& rng) override;
    double log_prob(double x) override;
};

struct Bernoulli : public Distribution {
    double p;

    Bernoulli(double p);
    double sample(AnyRNG& rng) override;
    double log_prob(double x) override;
};

struct Exponential : public Distribution {
    double rate;

    Exponential(double rate);
    double sample(AnyRNG& rng) override;
    double log_prob(double x) override;
};

struct Uniform : public Distribution {
    double a;
    double b;

    Uniform(double a, double b);
    double sample(AnyRNG& rng) override;
    double log_prob(double x) override;
};

struct Poisson : public Distribution {
    double lam;

    Poisson(double lam);
    double sample(AnyRNG& rng) override;
    double log_prob(double x) override;
};

struct BetaDist : public Distribution { 
    double alpha;
    double beta_param;

    BetaDist(double alpha, double beta_param);
    double sample(AnyRNG& rng) override;
    double log_prob(double x) override;
};

struct GammaDist : public Distribution {
    double shape;
    double rate;

    GammaDist(double shape, double rate);
    double sample(AnyRNG& rng) override;
    double log_prob(double x) override;
};

struct Discrete : public Distribution {
    std::vector<double> probs;

    Discrete(const std::vector<double>& probs);
    double sample(AnyRNG& rng) override;
    double log_prob(double x) override;
};