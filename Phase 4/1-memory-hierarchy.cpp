#include <iostream>
#include <chrono>
#include <vector>
#include <random>

void benchmark_memory_levels() {
    std::cout << "=== Memory Hierarchy Benchmark ===" << std::endl;
    
    // L1 Cache size test (fits in L1)
    const size_t L1_SIZE = 16 * 1024;  // 16KB
    std::vector<int> l1_data(L1_SIZE / sizeof(int));
    
    auto start = std::chrono::high_resolution_clock::now();
    volatile int sum = 0;
    for (int iter = 0; iter < 10000; iter++) {
        for (size_t i = 0; i < l1_data.size(); i++) {
            sum += l1_data[i];
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto l1_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    
    // L2 Cache size test (fits in L2, not L1)
    const size_t L2_SIZE = 256 * 1024;  // 256KB
    std::vector<int> l2_data(L2_SIZE / sizeof(int));
    
    start = std::chrono::high_resolution_clock::now();
    sum = 0;
    for (int iter = 0; iter < 10000; iter++) {
        for (size_t i = 0; i < l2_data.size(); i++) {
            sum += l2_data[i];
        }
    }
    end = std::chrono::high_resolution_clock::now();
    auto l2_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    
    // L3 Cache size test (fits in L3, not L2)
    const size_t L3_SIZE = 8 * 1024 * 1024;  // 8MB
    std::vector<int> l3_data(L3_SIZE / sizeof(int));
    
    start = std::chrono::high_resolution_clock::now();
    sum = 0;
    for (int iter = 0; iter < 1000; iter++) {
        for (size_t i = 0; i < l3_data.size(); i += 16) {  // Stride to avoid prefetcher
            sum += l3_data[i];
        }
    }
    end = std::chrono::high_resolution_clock::now();
    auto l3_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    
    // RAM test (doesn't fit in cache)
    const size_t RAM_SIZE = 64 * 1024 * 1024;  // 64MB
    std::vector<int> ram_data(RAM_SIZE / sizeof(int));
    
    start = std::chrono::high_resolution_clock::now();
    sum = 0;
    for (int iter = 0; iter < 100; iter++) {
        for (size_t i = 0; i < ram_data.size(); i += 16) {
            sum += ram_data[i];
        }
    }
    end = std::chrono::high_resolution_clock::now();
    auto ram_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    
    std::cout << "\nAccess times (relative):" << std::endl;
    std::cout << "L1 Cache: " << l1_time / 10000 << " ns (baseline)" << std::endl;
    std::cout << "L2 Cache: " << l2_time / 10000 << " ns (" 
              << (l2_time / (double)l1_time) << "x slower)" << std::endl;
    std::cout << "L3 Cache: " << l3_time / 1000 << " ns (" 
              << (l3_time * 10 / (double)l1_time) << "x slower)" << std::endl;
    std::cout << "RAM:      " << ram_time / 100 << " ns (" 
              << (ram_time * 100 / (double)l1_time) << "x slower)" << std::endl;
}

void show_cache_info() {
    std::cout << "\n=== Typical Cache Configuration ===" << std::endl;
    std::cout << "\nModern CPU (e.g., Intel Core i7):" << std::endl;
    std::cout << "  L1 Data Cache: 32KB per core" << std::endl;
    std::cout << "  L1 Instruction Cache: 32KB per core" << std::endl;
    std::cout << "  L2 Cache: 256KB per core" << std::endl;
    std::cout << "  L3 Cache: 8-32MB shared" << std::endl;
    
    std::cout << "\nCache Line Size: 64 bytes (most important!)" << std::endl;
    std::cout << "  - Memory moves in 64-byte chunks" << std::endl;
    std::cout << "  - Accessing 1 byte loads entire 64-byte line" << std::endl;
    std::cout << "  - Critical for performance!" << std::endl;
}

int main() {
    show_cache_info();
    benchmark_memory_levels();
    
    return 0;
}