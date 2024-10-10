import subprocess
import time

def run_benchmark(command, iterations):
    times = []
    
    for i in range(iterations):
        start_time = time.time()  # Record the start time
        
        # Run the executable or script
        result = subprocess.run(command, capture_output=True)
        
        if result.returncode != 0:
            print(f"Error running {' '.join(command)}: {result.stderr.decode()}")
            return None, None, None
        
        end_time = time.time()  # Record the end time
        elapsed_time = end_time - start_time
        
        print(f"Iteration {i+1} ({' '.join(command)}): {elapsed_time:.6f} seconds")
        times.append(elapsed_time)
    
    # Calculate min, max, and average time
    min_time = min(times)
    max_time = max(times)
    average_time = sum(times) / len(times)
    
    print(f"\nResults for {' '.join(command)}:")
    print(f"  Average time: {average_time:.6f} seconds")
    print(f"  Min time: {min_time:.6f} seconds")
    print(f"  Max time: {max_time:.6f} seconds\n")
    
    return average_time, min_time, max_time

if __name__ == "__main__":
    test = input('input the test name: ')
    # List of program commands for each type
    programs = {
        "python": ["python", f"./{test}.py"],  # Path to Python script
        "cpp": [f"./{test}_cpp.exe"],  # Path to C++ executable
        "cs": [f"./{test}_cs.exe"],  # Path to C++ executable
        "rcy": [f"./{test}_rcy.exe"],  # Path to rcy executable
        "node": ["node", f"./{test}.js"]  # Path to Node.js script
    }

    iterations = int(input("Enter the number of iterations: "))

    # Store results for the summary table
    results = []

    # Run benchmarks for each program type
    for program_type, command in programs.items():
        print(f"\nRunning benchmark for {program_type} program...")
        average_time, min_time, max_time = run_benchmark(command, iterations)
        
        # Save results for summary if benchmark ran successfully
        if average_time is not None:
            results.append({
                "Program": program_type,
                "Average Time (s)": average_time,
                "Min Time (s)": min_time,
                "Max Time (s)": max_time
            })
    
    # Display a summary table of all results
    print("\n--- Benchmark Summary ---")
    print(f"{'Program':<10} | {'Average Time (s)':<20} | {'Min Time (s)':<15} | {'Max Time (s)':<15}")
    print("-" * 65)
    for result in results:
        print(f"{result['Program']:<10} | {result['Average Time (s)']:<20.6f} | {result['Min Time (s)']:<15.6f} | {result['Max Time (s)']:<15.6f}")
    print("-" * 65)
