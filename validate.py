import subprocess
import csv
import os

TESTS = [
    {
        "name": "The Flip Model (Stochastic Control Flow)",
        "file": "ppl_programs/flip_model.txt",
        "algo": "lw",
        "expected_mean": 0.414, 
        "tolerance": 0.05      
    },
    {
        "name": "The 8-Bit Model (Combinatorial Latents)",
        "file": "ppl_programs/8bit_model.txt",
        "algo": "mh",
        "expected_mean": 6.00,  
        "tolerance": 0.20       
    },
    {
        "name": "3-Step Random Walk (Sequential HMM)",
        "file": "ppl_programs/random_walk_model.txt",
        "algo": "smc",
        "expected_mean": 1.75,  
        "tolerance": 0.10
    },
    {
        "name": "Bayesian Linear Regression (Pyro Data)",
        "file": "ppl_programs/linreg.txt",
        "algo": "lw",            
        "expected_mean": 0.612,  
        "tolerance": 0.05
    }
]

ENGINE_PATH = "./build/ppl_engine"
TEMP_CSV = "temp_results.csv"

def calculate_mean_from_csv(csv_path, is_lw):
    mean = 0.0
    count = 0
    with open(csv_path, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            val = float(row['value'])
            if is_lw:
                weight = float(row['weight'])
                mean += val * weight
            else:
                mean += val
                count += 1
                
    if not is_lw and count > 0:
        mean /= count
        
    return mean

def run_tests():
    print("--- STARTING PPL HEURISTIC VALIDATION ---")
    
    all_passed = True
    
    for test in TESTS:
        print(f"\nRunning test: {test['name']} [{test['algo'].upper()}]")
        
        cmd = [
            ENGINE_PATH, 
            test['file'], 
            "--algo", test['algo'], 
            "--iter", "50000", 
            "--seed", "42",    
            "--out", TEMP_CSV
        ]
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"[FAIL] C++ Engine crashed:\n{result.stderr}")
            all_passed = False
            continue
            
        is_lw = (test['algo'] == 'lw')
        actual_mean = calculate_mean_from_csv(TEMP_CSV, is_lw)
        
        diff = abs(actual_mean - test['expected_mean'])
        if diff <= test['tolerance']:
            print(f"[PASS] Expected: {test['expected_mean']} | Actual: {actual_mean:.4f} | Diff: {diff:.4f}")
        else:
            print(f"[FAIL] Expected: {test['expected_mean']} | Actual: {actual_mean:.4f} | Diff: {diff:.4f} (exceeds {test['tolerance']})")
            all_passed = False

    if os.path.exists(TEMP_CSV):
        os.remove(TEMP_CSV)
        
    print("\n--- VALIDATION COMPLETE ---")
    if all_passed:
        print("ALL TESTS PASSED! The C++ engine matches the reference notebooks.")
    else:
        print("SOME TESTS FAILED")

if __name__ == "__main__":
    run_tests()