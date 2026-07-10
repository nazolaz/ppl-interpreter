#include "Distribution.h"
#include <cmath>
#include <random>
#include <stdexcept>
#include <numeric>

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

Exponential::Exponential(double rate) : rate(rate) {
    if (rate <= 0) throw std::invalid_argument("exponential: rate must be > 0");
}
double Exponential::sample(AnyRNG& rng) {
    std::exponential_distribution<double> dist(rate);
    return dist(rng);
}
double Exponential::log_prob(double x) {
    if (x < 0.0) return -INFINITY;
    return std::log(rate) - rate * x;
}

Uniform::Uniform(double a, double b) : a(a), b(b) {
    if (b <= a) throw std::invalid_argument("uniform: requires b > a");
}
double Uniform::sample(AnyRNG& rng) {
    std::uniform_real_distribution<double> dist(a, b);
    return dist(rng);
}
double Uniform::log_prob(double x) {
    if (x >= a && x <= b) return -std::log(b - a);
    return -INFINITY;
}

Poisson::Poisson(double lam) : lam(lam) {
    if (lam <= 0) throw std::invalid_argument("poisson: rate must be > 0");
}
double Poisson::sample(AnyRNG& rng) {
    std::poisson_distribution<int> dist(lam);
    return static_cast<double>(dist(rng));
}
double Poisson::log_prob(double x) {
    int k = static_cast<int>(x);
    if (k < 0) return -INFINITY;
    return k * std::log(lam) - lam - std::lgamma(k + 1.0);
}

BetaDist::BetaDist(double alpha, double beta_param) : alpha(alpha), beta_param(beta_param) {
    if (alpha <= 0 || beta_param <= 0) throw std::invalid_argument("beta: parameters must be > 0");
}
double BetaDist::sample(AnyRNG& rng) {
   std::gamma_distribution<double> dist_a(alpha, 1.0);
   std::gamma_distribution<double> dist_b(beta_param, 1.0);
   double x = dist_a(rng);
   double y = dist_b(rng);
   return x / (x + y);
}
double BetaDist::log_prob(double x) {
    if (x <= 0.0 || x >= 1.0) return -INFINITY;
    double logB = std::lgamma(alpha) + std::lgamma(beta_param) - std::lgamma(alpha + beta_param);
    return (alpha - 1.0) * std::log(x) + (beta_param - 1.0) * std::log1p(-x) - logB;
}

GammaDist::GammaDist(double shape, double rate) : shape(shape), rate(rate) {
     if (shape <= 0 || rate <= 0) throw std::invalid_argument("gamma: parameters must be > 0");
}
double GammaDist::sample(AnyRNG& rng) {
    std::gamma_distribution<double> dist(shape, 1.0 / rate);
    return dist(rng);
}
double GammaDist::log_prob(double x) {
     if (x <= 0.0) return -INFINITY;
     return shape * std::log(rate) - std::lgamma(shape) + (shape - 1.0) * std::log(x) - rate * x;
}

Discrete::Discrete(const std::vector<double>& probs) : probs(probs) {
    double sum = std::accumulate(probs.begin(), probs.end(), 0.0);
    if (sum <= 0) throw std::invalid_argument("discrete: invalid probability vector");
    for(double& p : this->probs) {
        if(p < 0) throw std::invalid_argument("discrete: invalid probability vector");
        p /= sum;
    }
}
double Discrete::sample(AnyRNG& rng) {
    std::discrete_distribution<int> dist(probs.begin(), probs.end());
    return static_cast<double>(dist(rng));
}
double Discrete::log_prob(double x) {
     int k = static_cast<int>(x);
     if (k >= 0 && k < probs.size() && probs[k] > 0.0) {
         return std::log(probs[k]);
     }
     return -INFINITY;
}