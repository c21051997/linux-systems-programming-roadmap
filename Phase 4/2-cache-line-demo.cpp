#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

#define CACHE_LINE_SIZE 64

struct alignas(CACHE_LINE_SIZE) AlignedCounter {
    int value;
    char padding[CACHE_LINE_SIZE - sizeof(int)];
};

// BAD: Two counters in same cache line
struct BadLayout {
    int counter1;  // Used by thread 1
    int counter2;  // Used by thread 2
    // Both in same 64-byte cache line!
};

// GOOD: Each counter in separate cache line
struct GoodLayout {
    alignas(CACHE_LINE_SIZE) int counter1;
    alignas(CACHE_LINE_SIZE) int counter2;
};

void demonstrate_cache_line_concept() {
    std::cout << "=== Cache Line Basics ===" << std::endl;
    std::cout << "\nMemory is organized in 64-byte cache lines:" << std::endl;
    std::cout << R"(
Address:  0x1000                    0x1040
          |<------- 64 bytes ------->|
          [                          ]  Cache Line 0
          [                          ]  Cache Line 1
          [                          ]  Cache Line 2
          
When you access address 0x1000:
  - CPU loads entire 64-byte line (0x1000-0x103F)
  - Subsequent accesses to 0x1001-0x103F are FREE!
)" << std::endl;
    
    std::cout << "Cache Line Size: " << CACHE_LINE_SIZE << " bytes" << std::endl;
    std::cout << "sizeof(int): " << sizeof(int) << " bytes" << std::endl;
    std::cout << "Ints per cache line: " << CACHE_LINE_SIZE / sizeof(int) << std::endl;
}

void benchmark_false_sharing() {
    const int ITERATIONS = 100000000;
    
    std::cout << "\n=== False Sharing Benchmark ===" << std::endl;
    
    // Test 1: False sharing (BAD)
    BadLayout bad;
    bad.counter1 = 0;
    bad.counter2 = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::thread t1([&]() {
        for (int i = 0; i < ITERATIONS; i++) {
            bad.counter1++;
        }
    });
    
    std::thread t2([&]() {
        for (int i = 0; i < ITERATIONS; i++) {
            bad.counter2++;
        }
    });
    
    t1.join();
    t2.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto bad_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Test 2: No false sharing (GOOD)
    GoodLayout good;
    good.counter1 = 0;
    good.counter2 = 0;
    
    start = std::chrono::high_resolution_clock::now();
    
    std::thread t3([&]() {
        for (int i = 0; i < ITERATIONS; i++) {
            good.counter1++;
        }
    });
    
    std::thread t4([&]() {
        for (int i = 0; i < ITERATIONS; i++) {
            good.counter2++;
        }
    });
    
    t3.join();
    t4.join();
    
    end = std::chrono::high_resolution_clock::now();
    auto good_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "False sharing (bad):  " << bad_time.count() << " ms" << std::endl;
    std::cout << "No false sharing (good): " << good_time.count() << " ms" << std::endl;
    std::cout << "Speedup: " << (double)bad_time.count() / good_time.count() << "x" << std::endl;
}

int main() {
    demonstrate_cache_line_concept();
    benchmark_false_sharing();
    
    return 0;
}