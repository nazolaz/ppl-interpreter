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