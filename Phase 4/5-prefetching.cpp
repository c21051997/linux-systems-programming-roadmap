// prefetching.cpp
#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <algorithm>

void sequential_access_benchmark() {
    const size_t N = 10000000;
    std::vector<int> data(N);
    
    for (size_t i = 0; i < N; i++) {
        data[i] = i;
    }
    
    std::cout << "=== Sequential Access (Prefetcher Friendly) ===" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    long long sum = 0;
    for (size_t i = 0; i < N; i++) {
        sum += data[i];
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto seq_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Sequential access time: " << seq_time.count() << " ms" << std::endl;
    std::cout << "CPU prefetcher can predict and load next cache lines!" << std::endl;
}

void random_access_benchmark() {
    const size_t N = 10000000;
    std::vector<int> data(N);
    std::vector<size_t> indices(N);
    
    for (size_t i = 0; i < N; i++) {
        data[i] = i;
        indices[i] = i;
    }
    
    // Shuffle indices for random access
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);
    
    std::cout << "\n=== Random Access (Prefetcher Defeated) ===" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    long long sum = 0;
    for (size_t i = 0; i < N; i++) {
        sum += data[indices[i]];
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto rand_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Random access time: " << rand_time.count() << " ms" << std::endl;
    std::cout << "CPU prefetcher cannot predict - cache misses!" << std::endl;
}

void manual_prefetch_demo() {
    const size_t N = 1000000;
    std::vector<int> data(N);
    std::vector<size_t> indices(N);
    
    for (size_t i = 0; i < N; i++) {
        data[i] = i;
        indices[i] = (i * 7919) % N;  // Pseudo-random pattern
    }
    
    std::cout << "\n=== Manual Prefetching ===" << std::endl;
    
    // Without prefetch
    auto start = std::chrono::high_resolution_clock::now();
    long long sum = 0;
    for (size_t i = 0; i < N; i++) {
        sum += data[indices[i]];
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto no_prefetch = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // With manual prefetch
    start = std::chrono::high_resolution_clock::now();
    sum = 0;
    for (size_t i = 0; i < N; i++) {
        // Prefetch next iteration's data
        if (i + 8 < N) {
            __builtin_prefetch(&data[indices[i + 8]], 0, 3);
        }
        sum += data[indices[i]];
    }
    end = std::chrono::high_resolution_clock::now();
    auto with_prefetch = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Without prefetch: " << no_prefetch.count() << " μs" << std::endl;
    std::cout << "With prefetch:    " << with_prefetch.count() << " μs" << std::endl;
    std::cout << "Improvement: " << (double)no_prefetch.count() / with_prefetch.count() 
              << "x faster" << std::endl;
}

int main() {
    sequential_access_benchmark();
    random_access_benchmark();
    manual_prefetch_demo();
    
    std::cout << "\n=== Prefetching Explanation ===" << std::endl;
    std::cout << R"(
Hardware Prefetcher:
  - Detects sequential/strided access patterns
  - Automatically loads next cache lines
  - Works great for: arrays, linked lists (if sequential)
  - Fails for: random access, pointer chasing

Manual Prefetching:
  __builtin_prefetch(addr, rw, locality)
    addr: Address to prefetch
    rw: 0 = read, 1 = write
    locality: 0-3 (0=no locality, 3=high locality)

Example:
  for (int i = 0; i < n; i++) {
      __builtin_prefetch(&array[i + 8]);  // Prefetch ahead
      process(array[i]);
  }
)" << std::endl;
    
    return 0;
}