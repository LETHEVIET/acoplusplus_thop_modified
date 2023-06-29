#include "utilities.h"
#include "thop.h"
#include "inout.h"
#include "tree_map.h"

Tree_Map::Tree_Map(const std::size_t &num_city, long int **&distance_matrix)
{
    size_t i;

    this->_num_city = num_city;
    _tree_edge_ptrs.resize(num_city - 1);
    for (i = 0; i < num_city - 1; i++) // Do not go from the end city
        _tree_edge_ptrs[i] = new Tree_Edge(num_city, i, distance_matrix);

    _wont_visit_tree_ptr = new Wont_Visit_Tree(num_city);

    _global_wont_visit_restart_times = 0;
    _global_restart_times = 0;
    _global_evap_times = 0;
}

Tree_Map::~Tree_Map()
{
    delete _wont_visit_tree_ptr;
    for (auto &_ptr : _tree_edge_ptrs)
        delete _ptr;
}

void Tree_Map::_set_wont_visit(const std::size_t &city_index)
{
    _wont_visit_tree_ptr->set_wont_visit(city_index, _num_city, _global_wont_visit_restart_times);
}

void Tree_Map::reinforce(
    const ant_struct &an_ant,
    const double &rho,
    const double &trail_max)
{
    std::size_t i;
    const double invert_fitness = 1.0 / an_ant.fitness;
    const double one_minus_rho = 1 - rho;

    for (i = 0; i <= an_ant.tour_size - 4; i++)
    {
        _tree_edge_ptrs[an_ant.tour[i]]->reinforce(
            an_ant.tour[i + 1],
            invert_fitness,
            one_minus_rho,
            _past_trail_restart,
            _past_trail_min,
            trail_max,
            _global_restart_times,
            _global_evap_times);
    }
}

void Tree_Map::_reset_wont_visit()
{
    _global_wont_visit_restart_times += 1;
}

void Tree_Map::evaporate(const double &past_trail_min)
{
    _global_evap_times += 1;
    _past_trail_min = trail_min;
}

void Tree_Map::restart_pheromone(const double &past_trail_restart)
{
    _global_restart_times += 1;
    _past_trail_restart = past_trail_restart;
}

void Tree_Map::choose_route(
    ant_struct &an_ant,
    const double &q_0,
    const double &alpha,
    const double &beta,
    const double &rho,
    long int &n_tours)
{
    const double &one_minus_q_0 = 1 - q_0;
    const double &one_minus_rho = 1 - rho;
    size_t current_city, i;

    _reset_wont_visit();
    ant_empty_memory(&an_ant);

    an_ant.tour_size = 1;
    an_ant.tour[0] = 0;
    an_ant.visited[0] = TRUE;
    an_ant.visited[_num_city] = TRUE; // virtual city

    while (true)
    {
        current_city = an_ant.tour[an_ant.tour_size - 1];
        if (current_city == _num_city + 1 - 2)
            break;

        current_city = _tree_edge_ptrs[current_city]->choose_next_city(
            _wont_visit_tree_ptr->get_root_ptr(),
            one_minus_q_0,
            alpha,
            beta,
            one_minus_rho,
            _past_trail_restart,
            _past_trail_min,
            _global_restart_times,
            _global_evap_times,
            _global_wont_visit_restart_times);
        an_ant.tour[an_ant.tour_size] = current_city;
        an_ant.visited[current_city] = TRUE;
        an_ant.tour_size++;

        _set_wont_visit(current_city);
    }

    an_ant.tour[an_ant.tour_size++] = _num_city;      // virtual city
    an_ant.tour[an_ant.tour_size++] = an_ant.tour[0]; // to form a tour

    for (i = an_ant.tour_size; i < _num_city + 1; i++)
        an_ant.tour[i] = 0;
    an_ant.fitness = compute_fitness(an_ant.tour, an_ant.visited, an_ant.tour_size, an_ant.packing_plan);

    n_tours += 1;
}

double Tree_Map::leaf_pheromone(
    const std::size_t &i,
    const std::size_t &j,
    const double &rho)
{
    const double one_minus_rho = 1 - rho;
    return _tree_edge_ptrs[i]->leaf_pheromone(
        j,
        one_minus_rho,
        _past_trail_restart,
        _past_trail_min,
        _global_restart_times,
        _global_evap_times);
}

double Tree_Map::node_branching(const double &lambda)
{
    size_t i, j, sum_num_branches = 0;
    double min_pheromone, max_pheromone, cutoff, pheromone;
    const size_t num_city = instance.n - 1;

    for (i = 0; i < num_city - 1; i++)
    {
        min_pheromone = 2;
        max_pheromone = -1;
        for (j = 1; j < num_city; j++)
        {
            if (i == j)
                continue;
            pheromone = tree_map->leaf_pheromone(i, j, rho);
            if (pheromone > max_pheromone)
                max_pheromone = pheromone;
            if (pheromone < min_pheromone)
                min_pheromone = pheromone;
        }

        cutoff = min_pheromone + lambda * (max_pheromone - min_pheromone);
        for (j = 1; j < num_city; j++)
        {
            if (tree_map->leaf_pheromone(i, j, rho) > cutoff)
                sum_num_branches += 1;
        }
    }

    return (sum_num_branches / (double)(num_city * 2));
}

// hyperparameters
bool tree_map_flag = true;

Tree_Map *tree_map;

void tree_map_init()
{
    tree_map = new Tree_Map(instance.n - 1, instance.distance);
}
void tree_map_force_set_parameters()
{
    mmas_flag = true;
}
void tree_map_deallocate()
{
    delete tree_map;
}

void tree_map_construct_solutions()
{
    std::size_t i;
    for (i = 0; i < n_ants; i++)
        tree_map->choose_route(ant[i], instance.n - 1, alpha, beta, rho, n_tours);
}