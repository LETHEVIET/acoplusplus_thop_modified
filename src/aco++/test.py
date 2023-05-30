import optuna
import subprocess

def objective(trial):
    cluster_size = trial.suggest_int("cluster_size", 4, 36)
    sector = trial.suggest_int("sector", 4, 36)
    postfix = f"nc_bruce_clustersize_{cluster_size}_sector_{sector}"
    command = [
        "./acothop_nc_bruce",
        "--tries",
        "1",
        "--seed",
        "269070",
        "--inputfile",
        "../../instances/eil51-thop/eil51_10_usw_10_03.thop",
        "--outputfile",
        f"../../solutions/temp/aco++/eil51-thop/eil51_10_usw_10_03_{postfix}.thop.sol",
        "--ants",
        "100.0",
        "--alpha",
        "0.86",
        "--beta",
        "3.89",
        "--rho",
        "0.5",
        "--ptries",
        "1",
        "--localsearch",
        "1",
        "--log",
        "--q0",
        "0.0",
        "--mmas",
        "--adapt_evap",
        "--nodeclustering",
        "--sector",
        str(sector),
        "--clustersize",
        str(cluster_size),
        "--n_cluster",
        "4",
        "--logiter"]
    
    profit = 0
    try:
        result = subprocess.run(command, capture_output=True, check=True)
        stdout_log = result.stdout.decode()
        
        profit = int(stdout_log.split(': ')[1])
        print(profit)
    except:
        profit = -1
    return profit

study = optuna.create_study(direction='maximize')
study.optimize(objective, n_trials=100)

print(study.best_params)
