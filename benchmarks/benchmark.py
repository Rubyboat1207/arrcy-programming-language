import subprocess
import time
import signal
import sys
import csv
import datetime

# Global variable to store results
results = []

def run_benchmark(command, iterations):
    times = []
    
    for i in range(iterations):
        start_time = time.time()  # Record the start time
        
        # Run the executable or script
        result = subprocess.run(command, capture_output=True)
        
        if result.returncode != 0:
            print(f"Error running {' '.join(command)}: {result.stderr.decode()}")
            return None
        
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
    
    return times

def save_results():
    if not results:
        return
    # Get current timestamp
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"benchmark_results_{timestamp}.csv"
    # Write results to CSV file
    with open(filename, mode='w', newline='') as csvfile:
        fieldnames = ['Program', 'Iteration', 'Time (s)']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        for result in results:
            program = result['Program']
            times = result['Times']
            for i, time_taken in enumerate(times):
                writer.writerow({'Program': program, 'Iteration': i+1, 'Time (s)': time_taken})
    print(f"\nResults saved to {filename}")

def handle_exit(signum, frame):
    print("\nExiting and saving results...")
    save_results()
    sys.exit(0)

# Register signal handlers
signal.signal(signal.SIGINT, handle_exit)
signal.signal(signal.SIGTERM, handle_exit)

if __name__ == "__main__":
    test = input('Input the test name: ')
    # List of program commands for each type
    programs = {
        # "python": ["python", f"./{test}.py"],  # Path to Python script
        # "cpp": [f"./{test}_cpp.exe"],  # Path to C++ executable
        "cs": [f"./{test}_cs.exe"],  # Path to C# executable
        "rcy": [f"./{test}_rcy.exe"],  # Path to rcy executable
        # "nodejs": ["node", f"./{test}.js"],  # Path to Node.js script
        # "deno": ["deno", f"./{test}.js"]
    }

    iterations = int(input("Enter the number of iterations: "))

    # Run benchmarks for each program type
    for program_type, command in programs.items():
        print(f"\nRunning benchmark for {program_type} program...")
        times = run_benchmark(command, iterations)
        
        # Save results if benchmark ran successfully
        if times is not None:
            min_time = min(times)
            max_time = max(times)
            average_time = sum(times) / len(times)
            results.append({
                "Program": program_type,
                "Times": times,
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

    # Save results upon normal completion
    save_results()
