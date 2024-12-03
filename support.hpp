#ifndef SUPPORT_HPP
#define SUPPORT_HPP

#include <random>

using namespace std;

char generateRandom() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(33, 126);
    return static_cast<char>(dis(gen));
}

#endif