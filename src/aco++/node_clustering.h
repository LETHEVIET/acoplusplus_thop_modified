extern int node_clustering_flag;
extern int n_cluster;
extern int cluster_size;
extern int n_sector;
extern double delta;

void create_cluster(void);

void update_cluter_total(void);

void node_clustering_move(ant_struct *a, long int phase);

void aaconc_update(void);

void evaporation_nc_list(void);

void compute_nc_list_total_information(void);