#include "Distribution.h"
#include <cmath>
#include <random>
#include <stdexcept>

const double LOG2PI = std::log(2.0 * M_PI);

Normal::Normal(double mu, double sigma) : mu(mu), sigma(sigma) {
    if (sigma <= 0.0) {
        throw std::invalid_argument("normal: sigma must be > 0");
    }
}

double Normal::sample(AnyRNG& rng) {
    std::normal_distribution<double> dist(mu, sigma);
    return dist(rng);
}

double Normal::log_prob(double x) {
    double z = (x - mu) / sigma;
    return -0.5 * (LOG2PI + z * z) - std::log(sigma);
}

Bernoulli::Bernoulli(double p) : p(p) {
    if (p < 0.0 || p > 1.0) {
        throw std::invalid_argument("flip: p must be in [0,1]");
    }
}

double Bernoulli::sample(AnyRNG& rng) {
    std::bernoulli_distribution dist(p);
    return dist(rng) ? 1.0 : 0.0;
}

double Bernoulli::log_prob(double x) {
    bool b = (x != 0.0);
    if (b) {
        return (p > 0.0) ? std::log(p) : -INFINITY;
    } else {
        return (p < 1.0) ? std::log1p(-p) : -INFINITY;
    }
}