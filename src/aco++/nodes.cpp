#include <math.h>
#include <assert.h>

#include "utilities.h"
#include "nodes.h"

Node::Node(Node *left_ptr, Node *right_ptr, const bool &is_root)
    : Node_Base<Node>(left_ptr, right_ptr, is_root)
{
    const bool is_leaf = (left_ptr == nullptr) and (right_ptr == nullptr);

    _local_evap_times = 0;
    _local_restart_times = 0;
    if (!is_leaf)
        _heuristic = (left_ptr->_heuristic + right_ptr->_heuristic) / 2.0;
}

Leaf::Leaf(
    const std::size_t &city_index,
    const double &heuristic)
    : Node()
{
    _heuristic = heuristic;
    this->_city_index = city_index;
}

std::size_t Leaf::city_index() { return _city_index; };

void Node::_restart_if_needed(const std::size_t &global_restart_times, const double &past_trail_restart)
{
    if (_local_restart_times < global_restart_times)
    {
        _local_restart_times = global_restart_times;
        _local_evap_times = 0;
        _past_pheromone = past_trail_restart;
    }
}

double Node::get_pheromone(const double &one_minus_rho,
                           const double &past_trail_restart,
                           const double &past_trail_min,
                           const std::size_t &global_restart_times, const std::size_t &global_evap_times)
{
    _restart_if_needed(global_restart_times, past_trail_restart);
    double pheromone = _past_pheromone * pow(one_minus_rho, global_evap_times - _local_evap_times);
    if (pheromone < past_trail_min)
        pheromone = past_trail_min;

    return pheromone;
}

void Node::_pay_evaporation_debt(const double &one_minus_rho,
                                 const double &past_trail_restart,
                                 const double &past_trail_min,
                                 const std::size_t &global_restart_times, const std::size_t &global_evap_times)
{

    _past_pheromone = get_pheromone(one_minus_rho, past_trail_restart, past_trail_min, global_restart_times, global_evap_times);
    _local_evap_times = global_evap_times;
}

void Node::reinforce(const double &invert_fitness, const double &one_minus_rho,
                     const double &past_trail_restart,
                     const double &past_trail_min,
                     const double &trail_max,
                     const std::size_t &global_restart_times, const std::size_t &global_evap_times)
{
    _pay_evaporation_debt(one_minus_rho, past_trail_restart, past_trail_min, global_restart_times, global_evap_times);
    _past_pheromone += invert_fitness;
    if (_past_pheromone > trail_max)
        _past_pheromone = trail_max;
}

double Node::_prob_weight(const double &alpha, const double &beta, const double &one_minus_rho,
                          const double &past_trail_restart,
                          const double &past_trail_min,
                          const std::size_t &global_restart_times, const std::size_t &global_evap_times)
{
    const double pheromone = this->get_pheromone(one_minus_rho, past_trail_restart, past_trail_min, global_restart_times, global_evap_times);
    return pow(pheromone, alpha) * pow(_heuristic, beta);
}

std::size_t Node::choose_child_with_prob(const double &one_minus_q_0,
                                         const double &alpha, const double &beta, const double &one_minus_rho,
                                         const double &past_trail_restart,
                                         const double &past_trail_min,
                                         const std::size_t &global_restart_times,
                                         const std::size_t &global_evap_times)
{
    // IMPORTANCE NOTE: Remember to check won't visit before go to this function

    std::array<double, 2> weights;
    std::size_t index_of_min;
    double prob_of_min;

    weights[0] = child_ptrs[0]->_prob_weight(alpha, beta, one_minus_rho, past_trail_restart, past_trail_min, global_restart_times, global_evap_times);
    weights[1] = child_ptrs[1]->_prob_weight(alpha, beta, one_minus_rho, past_trail_restart, past_trail_min, global_restart_times, global_evap_times);
    if (weights[0] < weights[1])
        index_of_min = 0;
    else
        index_of_min = 1;

    prob_of_min = weights[index_of_min] / (weights[0] + weights[1]);
    prob_of_min *= one_minus_q_0;
    if (new_rand01() < prob_of_min)
        return index_of_min;
    else
        return 1 - index_of_min;
}

void Wont_Visit_Node::_restart_if_needed(const std::size_t &global_wont_visit_restart_times)
{
    if (_local_wont_visit_restart_times < global_wont_visit_restart_times)
    {
        _wont_visit = false;
        _local_wont_visit_restart_times = global_wont_visit_restart_times;
    }
}

void Wont_Visit_Node::set_wont_visit(const std::size_t &global_wont_visit_restart_times)
{
    _wont_visit = true;
    _local_wont_visit_restart_times = global_wont_visit_restart_times;
    if (parent_ptr != nullptr)
        parent_ptr->_check_wont_visit(global_wont_visit_restart_times);
}

void Wont_Visit_Node::_check_wont_visit(const std::size_t &global_wont_visit_restart_times)
{
    if (child_ptrs[0]->get_wont_visit(global_wont_visit_restart_times) and child_ptrs[1]->get_wont_visit(global_wont_visit_restart_times))
        set_wont_visit(global_wont_visit_restart_times);
}

bool Wont_Visit_Node::get_wont_visit(const std::size_t &global_wont_visit_restart_times)
{
    _restart_if_needed(global_wont_visit_restart_times);
    return _wont_visit;
}