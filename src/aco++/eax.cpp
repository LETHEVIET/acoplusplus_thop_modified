#include "thop.h"
#include "ants.h"

#include "eax/cross.h"
#include "eax/evaluator.h"
#include "eax/indi.h"
#include "eax/rand.h"
#include "eax/sort.h"
#include "acothop.hpp"
#include "eax.hpp"

#include <algorithm>


void tour_crossover(void)
{
    long int k;
    offspring_struct* local_best_offspring_ptr;
    std::vector<offspring_struct> offsprings;
    std::vector<long int> tour_key;
    bool swapped;

    for (k = 0; k < n_ants; k++)
    {
        tour_key.resize(ant[k].tour_size);
        std::copy(ant[k].tour, ant[k].tour + ant[k].tour_size, tour_key.begin());
        sort(tour_key.begin(), tour_key.end());
        if dict.find(key) == dict.end()
        {
            auto& new_tour = best_offspring_record[tour_key].tour;
            auto& new_packing_plan = best_offspring_record[tour_key].packing_plan;
            auto& new_visited = best_offspring_record[tour_key].visited;
            auto& new_fitness = best_offspring_record[tour_key].fitness;

            std::copy(ant[k].tour, ant[k].tour + ant[k].tour_size, new_tour.begin());
            std::copy(ant[k].packing_plan, ant[k].packing_plan + new_packing_plan.size(), new_packing_plan.begin());
            std::copy(ant[k].visited, ant[k].visited + new_visited.size(), new_visited.begin());
            new_fitness = compute_fitness(ant[k].tour);

            return;
        }

        const auto& global_best_offspring = best_offspring_record[tour_key]
        offsprings = eax(global_best_offspring, ant[k].tour, ant[k].tour_size - 2);

        swapped = false;
        local_best_offspring_ptr->fitness = compute_fitness(ant[k].tour);???
        for (const auto& offspr : offsprings)
        {
            if (offspr.fitness <= local_best_offspring_ptr->fitness)
            {
                local_best_offspring_ptr = &offspr;
                swapped = true;
            }
        }
        if (global_best_offspring.fitness <= local_best_offspring_ptr->fitness)
        {
            best_offspring_record[tour_key] = *local_best_offspring_ptr;
        }

        if swapped {
            const auto& new_tour = local_best_offspring_ptr->tour;
            const auto& new_packing_plan = local_best_offspring_ptr->packing_plan;
            const auto& new_visited = local_best_offspring_ptr->visited;

            std::copy(new_tour.begin(), new_tour.begin() + ant[k].tour_size, ant[k].tour);
            std::copy(new_packing_plan.begin(), new_packing_plan.end(), ant[k].packing_plan);
            std::copy(new_visited.begin(), new_visited.end(), ant[k].visited);
        }
    }
}

vector<offspring_struct> eax(const long int*& tour, const long int& global_best_offspring)
{
    vector<offspring_struct *> offsprings(1);

    assert(offsprings.size() <= MAX_NUM_OFFSPRING);
    return offsprings;
}
