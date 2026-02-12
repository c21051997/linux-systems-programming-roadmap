// branch_prediction.cpp
#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <algorithm>

void predictable_branches() {
    const size_t N = 10000000;
    std::vector<int> data(N);
    
    for (size_t i = 0; i < N; i++) {
        data[i] = i % 100;
    }
    
    std::cout << "=== Predictable Branches ===" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    int sum = 0;
    for (size_t i = 0; i < N; i++) {
        if (data[i] < 50) {  // Predictable pattern (alternating)
            sum += data[i];
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto predictable = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Time: " << predictable.count() << " ms" << std::endl;
    std::cout << "Branch predictor learns the pattern!" << std::endl;
}

void unpredictable_branches() {
    const size_t N = 10000000;
    std::vector<int> data(N);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 99);
    
    for (size_t i = 0; i < N; i++) {
        data[i] = dis(gen);
    }
    
    std::cout << "\n=== Unpredictable Branches ===" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    int sum = 0;
    for (size_t i = 0; i < N; i++) {
        if (data[i] < 50) {  // Random, unpredictable!
            sum += data[i];
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto unpredictable = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Time: " << unpredictable.count() << " ms" << std::endl;
    std::cout << "Branch mispredictions cause pipeline stalls!" << std::endl;
}

void branchless_code() {
    const size_t N = 10000000;
    std::vector<int> data(N);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 99);
    
    for (size_t i = 0; i < N; i++) {
        data[i] = dis(gen);
    }
    
    std::cout << "\n=== Branchless Code (No Prediction Needed) ===" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    int sum = 0;
    for (size_t i = 0; i < N; i++) {
        // Branchless: Use arithmetic instead of if
        int mask = (data[i] < 50) ? -1 : 0;  // Compiler converts to conditional move
        sum += data[i] & mask;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto branchless = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Time: " << branchless.count() << " ms" << std::endl;
    std::cout << "No branches = no mispredictions!" << std::endl;
}

void demonstrate_likely_unlikely() {
    std::cout << "\n=== Compiler Hints: likely/unlikely ===" << std::endl;
    std::cout << R"(
C++20 attributes:
    if [[likely]] (condition) {
        // Hot path
    }
    
    if [[unlikely]] (condition) {
        // Error handling
    }

GCC built-ins:
    if (__builtin_expect(condition, 1)) {  // Likely true
        // Hot path
    }
    
    if (__builtin_expect(error, 0)) {      // Unlikely true
        // Error path
    }

Effect:
  - Compiler optimizes code layout
  - Hot path stays in instruction cache
  - Cold path moved out of the way
)" << std::endl;
}

int main() {
    predictable_branches();
    unpredictable_branches();
    branchless_code();
    demonstrate_likely_unlikely();
    
    std::cout << "\n=== Branch Prediction Best Practices ===" << std::endl;
    std::cout << "1. Sort data before processing (makes branches predictable)" << std::endl;
    std::cout << "2. Use branchless code for unpredictable conditions" << std::endl;
    std::cout << "3. Use [[likely]] / [[unlikely]] hints" << std::endl;
    std::cout << "4. Keep hot paths straightforward" << std::endl;
    std::cout << "5. Measure with: perf stat -e branch-misses" << std::endl;
    
    return 0;
}