/*

       AAAA    CCCC   OOOO   TTTTTT   SSSSS  PPPPP
      AA  AA  CC     OO  OO    TT    SS      PP  PP
      AAAAAA  CC     OO  OO    TT     SSSS   PPPPP
      AA  AA  CC     OO  OO    TT        SS  PP
      AA  AA   CCCC   OOOO     TT    SSSSS   PP

######################################################
##########    ACO algorithms for the TSP    ##########
######################################################

      Version: 1.0
      File:    ls.c
      Author:  Thomas Stuetzle
      Purpose: implementation of local search routines
      Check:   README and gpl.txt
      Copyright (C) 1999  Thomas Stuetzle
 */

/***************************************************************************

    Program's name: acotsp

    Ant Colony Optimization algorithms (AS, ACS, EAS, RAS, MMAS, BWAS) for the 
    symmetric TSP 

    Copyright (C) 2004  Thomas Stuetzle

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    email: stuetzle no@spam ulb.ac.be
    mail address: Universite libre de Bruxelles
                  IRIDIA, CP 194/6
                  Av. F. Roosevelt 50
                  B-1050 Brussels
                  Belgium

 ***************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "ls.h"
#include "inout.h"
#include "thop.h"
#include "ants.h"
#include "utilities.h"

long int compute_objective_value(long int *t, long int t_size, char*p){
    long int *profit_accumulated = (long int *) malloc( instance.n * sizeof(long int));
    long int *weight_accumulated = (long int *) malloc( instance.n * sizeof(long int));

    for (int i = 0 ; i < instance.n ; i++ ) {
        profit_accumulated[i] = weight_accumulated[i] = 0;
    }

    long int prev_city, curr_city;
    long int total_weight = 0;
    int violate_max_time;
    long int _total_time, _total_weight, _total_profit;

    const double v = ( instance.max_speed - instance.min_speed ) / instance.capacity_of_knapsack;
    
    for (int j= 0 ; j < instance.m ; j++ ) {
        if (p[j] == 0) continue;
        // printf("%d %d %d \n", j, instance.itemptr[j].profit, instance.itemptr[j].weight);
        if ( total_weight + instance.itemptr[j].weight > instance.capacity_of_knapsack ) continue;

        profit_accumulated[instance.itemptr[j].id_city] += instance.itemptr[j].profit;
        weight_accumulated[instance.itemptr[j].id_city] += instance.itemptr[j].weight; 
    }

    violate_max_time = FALSE;
    _total_time = _total_weight, _total_profit = 0;
    prev_city = 0;
    

    for (int i = 1 ; i < t_size; i++ ) {
        curr_city = t[i];

        if ( weight_accumulated[curr_city] == 0 && curr_city != instance.n - 2) continue;
        // printf("%d %d %d \n", i, profit_accumulated[curr_city], weight_accumulated[curr_city]);

        _total_time += instance.distance[prev_city][curr_city] / ( instance.max_speed - v * _total_weight );    
        
        if ( _total_time - EPSILON > instance.max_time ) {
            violate_max_time = TRUE; break;
        }                
        _total_weight += weight_accumulated[curr_city];
        _total_profit += profit_accumulated[curr_city];
        prev_city = curr_city;
    }

    if (violate_max_time == TRUE){
        _total_profit = -1;
    }

    free(profit_accumulated);
    free(weight_accumulated);
    // printf("%d %d \n", _total_profit, _total_weight);
    return _total_profit;
}

void first_improvement_insertion (long int *t, long int t_size, char*p, long int *fitness, long int *improvement_flag){
    long int z, z_o;
    long int *t_o = (long int *) calloc(instance.n + 1, sizeof(long int));
    long int *t_oo = (long int *) calloc(instance.n + 1, sizeof(long int));

    for (int i = t_size - 4; i > 0; i--){
        // t_oo = t;
        memcpy(t_oo, t, (instance.n + 1) * sizeof(long int));
        for (int j = 1; j < i; j++){
            // t_o = t;
            memcpy(t_o, t, (instance.n + 1) * sizeof(long int));

            t_o[j] = t[i];
            for (int k = j + 1; k <= i; k++){
                t_o[k] = t[k-1];
            }

            z_o = compute_objective_value(t_o, t_size, p);
            if ((*fitness ) < z_o){
                (*improvement_flag) = TRUE;
                (*fitness ) = z_o;
                printf("insertion improved!!\n");
                // t_oo = t_o;
                memcpy(t_oo, t_o, (instance.n + 1) * sizeof(long int));
                memcpy(t, t_oo, (instance.n + 1) * sizeof(long int));
                return;
            }

        }
        memcpy(t, t_oo, (instance.n + 1) * sizeof(long int));
    }
    // free(t_oo);
    // free(t_o);
}

void first_improvement_bit_flip (long int *t, long int t_size, char*p, long int *fitness, char*visited, long int *improvement_flag){
    char *t_p = (char *) calloc(instance.m, sizeof(char));
    long int z_hat = 0;
    long int z_o, z;
    z_o, z = compute_objective_value(t, t_size, p);
    
    for (int i = 0; i < instance.m; i++){
        if (visited[instance.itemptr[i].id_city] == FALSE) continue;
        // t_p = p;
        memcpy(t_p, p, instance.m* sizeof(char));

        if (t_p[i] == 1) t_p[i] = 0; else t_p[i] = 1;

        z_hat = compute_objective_value(t, t_size, t_p);
        // printf("%d \n", z_hat);
        if (z_hat > *fitness){
            (*improvement_flag) = TRUE;
            (*fitness) = z_hat;
            z = z_hat;
            // p = t_p;
            memcpy(p, t_p, instance.m* sizeof(char));
            printf("bitflip improved!!\n");
            free(t_p);
            return;
        }
    }

}

void first_improvement_exchange (long int *t, long int t_size, char*p, long int *fitness, char*visited, long int *improvement_flag){
    char *t_p = (char *) calloc(instance.m, sizeof(char));
    long int z_hat = 0;
    long int z_o, z;
    z_o, z = compute_objective_value(t, t_size, p);

    for (int i = 0; i < instance.m; i++){

        if (visited[instance.itemptr[i].id_city] == FALSE) continue;

        for (int j = 0; j < instance.m; j++){

            if (visited[instance.itemptr[j].id_city] == FALSE) continue;
            if (p[i] == p[j]) continue;

            // t_p = p;
            memcpy(t_p, p, instance.m* sizeof(char));
            if (t_p[i] == 1) t_p[i] = 0; else t_p[i] = 1;
            if (t_p[j] == 1) t_p[j] = 0; else t_p[j] = 1;

            z_hat = compute_objective_value(t, t_size, t_p);

            if (z_hat > z){
                (*improvement_flag) = TRUE;
                (*fitness ) = z_hat;
                z = z_hat;
                // p = t_p;
                memcpy(p, t_p, instance.m* sizeof(char));

                printf("exchange improved!!\n");
                free(t_p);
                return;
            }
        }
        
    }

    // free(t_p);
}

void first_improvement_two_opt(long int *tour, long int t_size, long int **distance, long int **nn_list, long int *improvement_flag){

    long int n = (long int) (t_size - 1);

    long int i, j, h, l, nn, local_nn_ls;

    long int *original_tour = (long int *) malloc(n * sizeof(long int));

    for ( i = 0 ; i < n ; i++ ) { original_tour[i] = tour[i]; tour[i] = i; }

    local_nn_ls = MIN(n - 1, nn_ls);

    long int c1, c2;             /* cities considered for an exchange */
    long int s_c1, s_c2;         /* successor cities of c1 and c2     */
    long int p_c1, p_c2;         /* predecessor cities of c1 and c2   */
    long int pos_c1, pos_c2;     /* positions of cities c1, c2        */
    long int help, n_exchanges=0;
    long int h1=0, h2=0, h3=0, h4=0;
    long int radius;             /* radius of nn-search */
    long int gain = 0;
    long int *random_vector;
    long int *pos;               /* positions of cities in tour */
    long int *dlb;               /* vector containing don't look bits */

    pos = (long int *) malloc(n * sizeof(long int));
    dlb = (long int *) malloc(n * sizeof(long int));
    for ( i = 0 ; i < n ; i++ ) {
        pos[tour[i]] = i;
        dlb[i] = FALSE;
    }

    random_vector = generate_random_permutation( n );

    for (l = 0 ; l < n; l++) {

        c1 = random_vector[l];
        DEBUG ( assert ( c1 < n && c1 >= 0); )
        if ( dlb_flag && dlb[c1] )
            continue;
        pos_c1 = pos[c1];
        s_c1 = tour[pos_c1+1];
        radius = distance[c1][s_c1];

        /* First search for c1's nearest neighbours, use successor of c1 */
        for ( h = 0 ; h < local_nn_ls ; h++ ) {
            c2 = nn_list[c1][h]; /* exchange partner, determine its position */
            if ( radius > distance[c1][c2] ) {
                s_c2 = tour[pos[c2]+1];
                gain =  - radius + distance[c1][c2] +
                        distance[s_c1][s_c2] - distance[c2][s_c2];
                if ( gain < 0 ) {
                    h1 = c1; h2 = s_c1; h3 = c2; h4 = s_c2;
                    goto exchange2opt;
                }
            }
            else
                break;
        }
        /* Search one for next c1's h-nearest neighbours, use predecessor c1 */
        if (pos_c1 > 0)
            p_c1 = tour[pos_c1-1];
        else
            p_c1 = tour[n-1];
        radius = distance[p_c1][c1];
        for ( h = 0 ; h < local_nn_ls ; h++ ) {
            c2 = nn_list[c1][h];  /* exchange partner, determine its position */
            if ( radius > distance[c1][c2] ) {
                pos_c2 = pos[c2];
                if (pos_c2 > 0)
                    p_c2 = tour[pos_c2-1];
                else
                    p_c2 = tour[n-1];
                if ( p_c2 == c1 )
                    continue;
                if ( p_c1 == c2 )
                    continue;
                gain =  - radius + distance[c1][c2] +
                        distance[p_c1][p_c2] - distance[p_c2][c2];
                if ( gain < 0 ) {
                    h1 = p_c1; h2 = c1; h3 = p_c2; h4 = c2;
                    goto exchange2opt;
                }
            }
            else
                break;
        }
        /* No exchange */
        dlb[c1] = TRUE;
        continue;

        exchange2opt:
        n_exchanges++;
        (*improvement_flag) = TRUE;
        dlb[h1] = FALSE; dlb[h2] = FALSE;
        dlb[h3] = FALSE; dlb[h4] = FALSE;
        /* Now perform move */
        if ( pos[h3] < pos[h1] ) {
            help = h1; h1 = h3; h3 = help;
            help = h2; h2 = h4; h4 = help;
        }
        if ( pos[h3] - pos[h2] < n / 2 + 1) {
            /* reverse inner part from pos[h2] to pos[h3] */
            i = pos[h2]; j = pos[h3];
            while (i < j) {
                c1 = tour[i];
                c2 = tour[j];
                tour[i] = c2;
                tour[j] = c1;
                pos[c1] = j;
                pos[c2] = i;
                i++; j--;
            }
        }
        else {
            /* reverse outer part from pos[h4] to pos[h1] */
            i = pos[h1]; j = pos[h4];
            if ( j > i )
                help = n - (j - i) + 1;
            else
                help = (i - j) + 1;
            help = help / 2;
            for ( h = 0 ; h < help ; h++ ) {
                c1 = tour[i];
                c2 = tour[j];
                tour[i] = c2;
                tour[j] = c1;
                pos[c1] = j;
                pos[c2] = i;
                i--; j++;
                if ( i < 0 )
                    i = n-1;
                if ( j >= n )
                    j = 0;
            }
            tour[n] = tour[0];
        }


    }

    for ( i = 0 ; i < n ; i++ ) tour[i] = original_tour[tour[i]];
    
    if ( tour[0] != 0 || tour[1] == instance.n-1) rotate_tour(tour, n);
    tour[n] = tour[0];

    free( dlb );
    free( pos );
    free( random_vector );
    free( original_tour);
}

void first_improvement_hill_climbing(ant_struct *ant) 
/*    
      FUNCTION:       2-opt a tour 
      INPUT:          pointer to the tour that undergoes local optimization
      OUTPUT:         none
      (SIDE)EFFECTS:  tour is 2-opt
      COMMENTS:       the neighbourhood is scanned in random order (this need 
                      not be the best possible choice). Concerning the speed-ups used 
                      here consult, for example, Chapter 8 of
                      Holger H. Hoos and Thomas Stuetzle, 
                      Stochastic Local Search---Foundations and Applications, 
                      Morgan Kaufmann Publishers, 2004.
                      or some of the papers online available from David S. Johnson.
 */
{

    long int t_size = ant->tour_size;
    long int *tour = (long int *) calloc(instance.n + 1, sizeof(long int));
    memcpy(tour, ant->tour, (instance.n + 1) * sizeof(long int));

    long int fitness = compute_fitness(ant->tour, ant->visited, ant->tour_size, ant->packing_plan);  
    
    char *packing_plan = (char *) calloc(instance.m, sizeof(char));
    memcpy(packing_plan, ant->packing_plan, instance.m* sizeof(char));

    
    long int n = (long int) (t_size - 1);
    long int  **distance = compute_local_distances(tour, n);
    long int  **nn_list = compute_local_nn_lists(distance, n);

    char *visited = (char *) calloc(instance.n + 1, sizeof(char));

    for (int i = 0; i < instance.n; i++){
        visited[i] = FALSE;
    }

    for (int i = 0; i < n; i++) visited[tour[i]] = TRUE;
    
    long int improvement_flag = TRUE, n_improves = 0;

    while ( improvement_flag ) {

        improvement_flag = FALSE;
        // first_improvement_two_opt(tour, t_size, distance, nn_list, &improvement_flag);
        two_opt_first(tour, t_size);
        // printf("passed\n");

        first_improvement_insertion(tour, t_size, packing_plan, &fitness, &improvement_flag);
        // printf("passed\n");

        first_improvement_bit_flip(tour, t_size, packing_plan, &fitness, visited, &improvement_flag);
        // printf("passed\n");

        first_improvement_exchange(tour, t_size, packing_plan, &fitness, visited, &improvement_flag);
        // printf("passed\n");

        if ( improvement_flag ) {
            n_improves++;
            printf("fitness: %d \n", fitness);
        }

        // exit(1);
    }

    
    // ant->tour = tour;
    // ant->packing_plan = packing_plan;

    memcpy(ant->packing_plan, packing_plan, instance.m* sizeof(char));
    memcpy(ant->tour, tour, (instance.n + 1)* sizeof(long int));
    ant->fitness = fitness;

    free( distance );
    free( nn_list );
}
