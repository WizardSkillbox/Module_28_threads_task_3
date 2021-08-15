#ifndef GENERATE_RANDOM_H
#define GENERATE_RANDOM_H

#include <chrono>
#include <random>

class IntRandom {
    std::mt19937 mt_rand;
public:
    IntRandom(int vector);
    int generateRandom(uint32_t min, uint32_t max);
};

#endif //GENERATE_RANDOM_H
