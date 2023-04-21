#ifndef EAX_HPP
#define EAX_HPP

#include <vector>
#include <array>
#include <map>
#include <functional>

#include "thop.h"

namespace std
{
    template<typename T>
    struct hash<vector<T>>
    {
        size_t operator()(const vector<T>& v) const
        {
            hash<T> hasher;
            size_t seed = 0;
            for (auto& elem : v)
            {
                seed ^= hasher(elem) + 0x9e3779b9 + (seed<<6) + (seed>>2);
            }
            return seed;
        }
    };
}

typedef struct
{
    std::array<long int, instance.n + 1> tour;
    std::array<char, instance.m> packing_plan;
    std::array<char, instance.n> visited;
    long int fitness;
} offspring_struct;

void tour_crossover(void);
std::vector<offspring_struct> eax(const std::vector<long int>& best_tour, const long int*& tour, const long int& tour_size);

std::map<std::vector<long int>, offspring_struct> best_tour_record;
extern constexpr int MAX_NUM_OFFSPRING = 3;

#endif