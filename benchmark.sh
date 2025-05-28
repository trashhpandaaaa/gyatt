#!/bin/bash

# Performance benchmark tests for Gyatt
echo "=========================================="
echo "GYATT PERFORMANCE BENCHMARK"
echo "=========================================="

BENCH_DIR="/tmp/gyatt_benchmark_$(date +%s)"
mkdir -p "$BENCH_DIR"
cd "$BENCH_DIR"

# Initialize repository
echo "Initializing repository..."
time "$OLDPWD/gyatt" init

# Create test files
echo "Creating test files..."
for i in {1..100}; do
    echo "This is test file $i with some content to make it realistic" > "test_file_$i.txt"
    echo "Line 2 of file $i" >> "test_file_$i.txt"
    echo "Line 3 of file $i" >> "test_file_$i.txt"
done

# Benchmark add operations
echo "Benchmarking add operations..."
start_time=$(date +%s.%N)
for i in {1..100}; do
    "$OLDPWD/gyatt" add "test_file_$i.txt"
done
end_time=$(date +%s.%N)
add_time=$(printf "%.3f" $(echo "$end_time - $start_time" | awk '{print $1}'))
echo "Time to add 100 files: ${add_time}s"

# Benchmark commit operation
echo "Benchmarking commit operation..."
start_time=$(date +%s.%N)
"$OLDPWD/gyatt" commit -m "Performance test commit with 100 files"
end_time=$(date +%s.%N)
commit_time=$(printf "%.3f" $(echo "$end_time - $start_time" | awk '{print $1}'))
echo "Time to commit 100 files: ${commit_time}s"

# Benchmark status operation
echo "Benchmarking status operation..."
start_time=$(date +%s.%N)
"$OLDPWD/gyatt" status >/dev/null
end_time=$(date +%s.%N)
status_time=$(printf "%.3f" $(echo "$end_time - $start_time" | awk '{print $1}'))
echo "Time to get status: ${status_time}s"

# Benchmark log operation
echo "Benchmarking log operation..."
start_time=$(date +%s.%N)
"$OLDPWD/gyatt" log >/dev/null
end_time=$(date +%s.%N)
log_time=$(printf "%.3f" $(echo "$end_time - $start_time" | awk '{print $1}'))
echo "Time to get log: ${log_time}s"

# Check repository size
echo "Repository statistics:"
echo "Number of objects: $(find .gyatt/objects -type f | wc -l)"
echo "Repository size: $(du -sh .gyatt | cut -f1)"
echo "Working directory size: $(du -sh . --exclude=.gyatt | cut -f1)"

# Clean up
cd "$OLDPWD"
rm -rf "$BENCH_DIR"

echo "Benchmark completed!"
