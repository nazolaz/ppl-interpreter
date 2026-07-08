#ifndef ANY_RNG_H
#define ANY_RNG_H

#include <cstdint>
#include <functional>
#include <random>
#include <utility>

class AnyRNG {
private:
    std::function<uint32_t()> generator;

public:
    template<typename T>
    AnyRNG(T engine) {
        std::independent_bits_engine<T, 32, uint32_t> adapter(std::move(engine));
        generator = [adapter]() mutable { 
            return adapter(); 
        };
    }

    AnyRNG(const AnyRNG&) = delete;
    AnyRNG& operator=(const AnyRNG&) = delete;

    AnyRNG(AnyRNG&&) noexcept = default;
    AnyRNG& operator=(AnyRNG&&) noexcept = default;

    uint32_t operator()() {
        return generator();
    }

    static constexpr uint32_t min() {
        return 0;
    }

    static constexpr uint32_t max() {
        return 0xFFFFFFFF;
    }
};

#endif