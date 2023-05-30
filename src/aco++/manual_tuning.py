import itertools
import multiprocessing
import subprocess
from tqdm import tqdm
import argparse

def launcher(arg):
    global aaco_nc_flag, number_of_runs, sol_dir, debug_log
    instance_name, cluster_size, sector = arg
    postfix = f"nc_bruce_clustersize_{cluster_size}_sector_{sector}"
    # command = [
    #     "python3", 
    #     "run.py", 
    #     "--instance_name",
    #     instance_name, 
    #     "--aaco_nc",
    #     "--q0",
    #     "0.0", 
    #     "--n_cluster",
    #     "4", 
    #     "--cluster_size",
    #     str(cluster_size), 
    #     "--sector",
    #     str(sector), 
    #     "--rho",
    #     "0.5", 
    #     "--postfix",
    #     postfix, 
    #     "--debug",
    #     "--silent",
    #     "1", 
    #     "--log_iter",
    # ]

    command = [
        "./acothop_nc_bruce",
        "--tries",
        "1",
        "--seed",
        "269070",
        "--time",
        "120.0",
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
        "--logiter",
        "--silent",
        "1"]
    
    result = subprocess.run(command, capture_output=True, check=True)
    stdout_log = result.stdout.decode()

    return (cluster_size, sector, stdout_log)

def imap_unordered_bar(func, args, total, n_processes=2):
    p = multiprocessing.Pool(n_processes)

    with tqdm(total=total, desc="outer") as pbar:
        for i, result in tqdm(
            enumerate(p.imap_unordered(func, args)), desc="iner", disable=True
        ):
            cluster_size, sector, stdout_log = result
            pbar.update(1)
            pbar.write("cluster_size ", cluster_size, "\tsector ",sector,"\t profit ", stdout_log)
    pbar.close()
    p.close()
    p.join()

def build():
    command = ["python3", "run.py", "--build_only", "--postfix", "nc_bruce"]
    print(f"$ {' '.join(command)}")
    result = subprocess.run(command, capture_output=True, check=True)
    print(result.stdout.decode())

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--instance_name", type=str)
    args = parser.parse_args()
    instance_name = args.instance_name

    clustersize_ls = [4,8,16,24,32,64]
    sector_ls = [4,8,16,24,32,64]

    build()

    best = {
        "sector": -1,
        "clustersize": -1,
        "score": -1,
    }

    n_processes = max(1, multiprocessing.cpu_count() // 2)

    total = len(clustersize_ls) * len(sector_ls)

    args = []
    for _product in itertools.product(
        clustersize_ls,
        sector_ls,
    ):
        clustersize, sector = _product
        args.append((instance_name, clustersize, sector))

    imap_unordered_bar(launcher, args, total, n_processes)