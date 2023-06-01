#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <time.h>
#include <float.h>

#include <iostream>
#include <vector>
#include <cmath>

#include "inout.h"
#include "thop.h"
#include "ants.h"
#include "ls.h"
#include "utilities.h"
#include "timer.h"

std::vector<std::vector<std::vector<int>>> cluster_chunk;
std::vector<std::vector<int>> clusters;
std::vector<int> cluster;

std::vector<std::vector<double>> total_cluster;
std::vector<double> t_cluster;

int node_clustering_flag;
int n_cluster;
int cluster_size;
int n_sector;
double delta = 1;

void aaconc_update(void){

    std::vector<bool> in_best_route(instance.n+1, false);
    long int iteration_best_ant = find_best();
    for (int i = 0; i < ant[iteration_best_ant].tour_size; i++){
        in_best_route[ant[iteration_best_ant].tour[i]] = true;
    }
    
    for (int k = 0; k < n_ants; k++){
        for (int i = 0; i < ant[k].tour_size - 1; i++){
            long int j = ant[k].tour[i];
            long int h = ant[k].tour[i + 1];

            if (in_best_route[j] && in_best_route[h]){
                pheromone[j][h] += delta * best_so_far_ant->fitness / ant[k].fitness;
                pheromone[h][j] = pheromone[j][h];

                total[h][j] = pow(pheromone[h][j], alpha) * pow(HEURISTIC(h, j), beta);
                total[j][h] = total[h][j];
            }
        }
    }
}

void evaporation_nc_list(void){

    TRACE(printf("pheromone evaporation nn_list\n"););

    for (int i = 0; i < instance.n; i++)
    {
        for (int cur_cluster = 0; cur_cluster < cluster_chunk[i].size(); cur_cluster++)
        {
            for (int j = 0; j < cluster_chunk[i][cur_cluster].size(); j++){
                int help_city = cluster_chunk[i][cur_cluster][j];
                pheromone[i][help_city] = (1.0 - rho) * pheromone[i][help_city];
            }
        }
    }
}

void compute_nc_list_total_information(void)
/*
      FUNCTION: calculates heuristic info times pheromone for arcs in nn_list
      INPUT:    none
      OUTPUT:   none
 */
{
    long int i, j, h;

    TRACE(printf("compute total information nn_list\n"););

     for (int i = 0; i < instance.n; i++)
    {
        for (int cur_cluster = 0; cur_cluster < cluster_chunk[i].size(); cur_cluster++)
        {
            for (int j = 0; j < cluster_chunk[i][cur_cluster].size(); j++){
                h = cluster_chunk[i][cur_cluster][j];
                if (pheromone[i][h] < pheromone[h][i])
                    pheromone[h][i] = pheromone[i][h];
                total[i][h] = pow(pheromone[i][h], alpha) * pow(HEURISTIC(i, h), beta);
                total[h][i] = total[i][h];
            }
        }
    }
}

int find_closest_node(int pivot_node, const std::vector<bool> &visited)
{
    int closest_node = -1;
    double min_distance = FLT_MAX;
    for (int node = 0; node < instance.n - 1; node++)
        if (!visited[node])
        {
            if (instance.distance[pivot_node][node] < min_distance)
            {
                closest_node = node;
                min_distance = instance.distance[pivot_node][node];
            }
        }

    return closest_node;
}

bool check_in_sector(point a, point b, int n_sector, int sector_index)
{
    double angle_1 = (2 * M_PI / n_sector) * (sector_index);
    double angle_2 = (2 * M_PI / n_sector) * (sector_index + 1);

    double x1 = 1;
    double y1 = 0;
    double x2 = b.x - a.x;
    double y2 = b.y - a.y;

    double angle = atan2(x1 * x2 + y1 * y2, x1 * y2 - y1 * x2);

    if (angle < 0)
        angle = 2 * M_PI + angle;

    return angle_1 <= angle && angle < angle_2;
}

int find_closest_node_in_sector(int pivot_node, int n_sector, int sector_index)
{
    int closest_node = -1;
    double min_distance = FLT_MAX;
    for (int node = 0; node < instance.n - 1; node++)
        if (node != pivot_node)
        {
            bool in_sector = check_in_sector(instance.nodeptr[pivot_node], instance.nodeptr[node], n_sector, sector_index);
            if (in_sector && instance.distance[pivot_node][node] < min_distance)
            {
                closest_node = node;
                min_distance = instance.distance[pivot_node][node];
            }
        }

    return closest_node;
}

void update_cluter_total(void)
{
    total_cluster.clear();
    for (int i = 0; i < instance.n; i++)
    {
        total_cluster.push_back(t_cluster);
        for (int c = 0; c < cluster_chunk[i].size(); c++)
        {
            double p = 0;
            double h = 0;
            for (int v = 0; v < cluster_chunk[i][c].size(); v++)
            {
                p += pheromone[i][cluster_chunk[i][c][v]];
                h += pheromone[i][cluster_chunk[i][c][v]];//HEURISTIC(i, cluster_chunk[i][c][v]);
            }

            p /= cluster_chunk[i][c].size();
            h *= cluster_chunk[i][c].size();

            total_cluster[i].push_back(pow(p, alpha) * pow(h, beta));
        }
    }
}

void create_cluster(void)
{

    cluster_chunk.clear();

    for (int i = 0; i < instance.n; i++)
    {
        cluster_chunk.push_back(clusters);
        // Node curr_node = nodes[i];
        std::vector<bool> visited(instance.n, false);
        int nb_visited = 0;
        visited[i] = true;
        nb_visited++;

        int cluster_index = 0;
        cluster_chunk[i].push_back(cluster);

        for (int j = 0; j < n_sector; j++)
        {
            int node = find_closest_node_in_sector(i, n_sector, j);
            if (node != -1)
            {
                if (cluster_chunk[i][cluster_index].size() >= cluster_size) {
                    cluster_index++;   
                    cluster_chunk[i].push_back(cluster);
                }
                cluster_chunk[i][cluster_index].push_back(node);
                nb_visited++;
                visited[node] = true;
            }
        }

        while (nb_visited < instance.n && cluster_chunk[i].size() < n_cluster)
        {
            while (cluster_chunk[i][cluster_index].size() < cluster_size)
            {
                int node = find_closest_node(i, visited);
                if (node != -1)
                {
                    cluster_chunk[i][cluster_index].push_back(node);
                    nb_visited++;
                    visited[node] = true;
                }
                if (nb_visited == instance.n - 1)
                    break;
            }
            if (nb_visited < instance.n - 1)
            {
                cluster_index++;
                cluster_chunk[i].push_back(cluster);
            }
            else
            {
                break;
            }
        }
    }

    // update_cluter_total();
}

void node_clustering_move(ant_struct *a, long int phase)
{

    if ((q_0 > 0.0) && (ran01(&seed) < q_0))
    {
        choose_best_next(a, phase);
        return;
    }

    int first = 1;
    int i = 0;
    double rnd;

    long int current_city = a->tour[phase - 1];
    int curr_cluster_size = total_cluster[current_city].size();

    std::vector<double> prob_clusters;
    double sum_prob_cluster;
    double partial_cluster_sum;
    int selected_cluster;
    
    std::vector<long int> candidates;
    std::vector<double> prob_cities;
    double sum_prob_cities ;
    int selected_city;
    double partial_cities_sum;
    
    // select cluster
    sum_prob_cluster = 0.0f;
    for (int j = 0; j < curr_cluster_size; j++){
        prob_clusters.push_back(total_cluster[current_city][j]);
        sum_prob_cluster += prob_clusters[j];
    }

    if (sum_prob_cluster == 0.0){
        std::cout<<iteration<<" " << phase << std::endl;
    }

    selected_cluster = 0;
    rnd = ran01(&seed);
    rnd *= sum_prob_cluster;
    partial_cluster_sum = prob_clusters[selected_cluster];
    while (partial_cluster_sum <= rnd){
        selected_cluster++;
        partial_cluster_sum += prob_clusters[selected_cluster];
    }

    if (selected_cluster == curr_cluster_size){
        neighbour_choose_best_next(a, phase);
        return;
    }

    while (true)
    {
        for (i = 0; i < cluster_chunk[current_city][selected_cluster].size(); i++)
        {
            long int city = cluster_chunk[current_city][selected_cluster][i];
            if (a->visited[city])
                continue;

            candidates.push_back(city);
        }

        if (candidates.size() != 0)
            break;

        if (first == 1 && selected_cluster != 0)
        {
            selected_cluster = 0;
            first = 0;
        }
        else if (selected_cluster == cluster_chunk[current_city].size() - 1){
            neighbour_choose_best_next(a, phase);
            return;
        }else{
            selected_cluster++;
        }
    }

    // select next city
    sum_prob_cities = 0;
    for (i = 0; i < candidates.size(); i++)
    {
        prob_cities.push_back(total[current_city][candidates[i]]);
        sum_prob_cities += prob_cities[i];
    }

    selected_city = 0;
    rnd = ran01(&seed);
    rnd *= sum_prob_cities;
    partial_cities_sum = prob_cities[selected_city];
    while (partial_cities_sum <= rnd){
        selected_city++;
        partial_cities_sum += prob_cities[selected_city];
    }

    if (selected_city == candidates.size()){
        neighbour_choose_best_next(a, phase);
        return;
    }

    a->tour[phase] = candidates[selected_city];
    a->visited[candidates[selected_city]] = TRUE;
}
