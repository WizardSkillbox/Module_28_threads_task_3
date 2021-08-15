#include <cassert>
#include <random_int_generator.h>

using namespace std;

IntRandom::IntRandom(int vector) {
    mt_rand = mt19937(vector);
}

int IntRandom::generateRandom(uint32_t min, uint32_t max) {
    assert(max > min);
    return (int) ((float) min + ((float) mt_rand() / (float) mt19937::max()) * ((float) (max - min) + 0.999f));
}

