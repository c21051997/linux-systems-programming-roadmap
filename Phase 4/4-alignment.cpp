// alignment.cpp
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <cstring>

void demonstrate_alignment() {
    std::cout << "=== Memory Alignment ===" << std::endl;
    
    std::cout << "\nNatural alignment requirements:" << std::endl;
    std::cout << "char:   " << alignof(char) << " byte(s)" << std::endl;
    std::cout << "short:  " << alignof(short) << " byte(s)" << std::endl;
    std::cout << "int:    " << alignof(int) << " byte(s)" << std::endl;
    std::cout << "long:   " << alignof(long) << " byte(s)" << std::endl;
    std::cout << "float:  " << alignof(float) << " byte(s)" << std::endl;
    std::cout << "double: " << alignof(double) << " byte(s)" << std::endl;
    std::cout << "pointer:" << alignof(void*) << " byte(s)" << std::endl;
    
    // Struct packing
    struct Unaligned {
        char a;     // 1 byte
        int b;      // 4 bytes
        char c;     // 1 byte
        double d;   // 8 bytes
    };
    
    struct Aligned {
        double d;   // 8 bytes (largest first)
        int b;      // 4 bytes
        char a;     // 1 byte
        char c;     // 1 byte
        // Compiler adds 2 bytes padding
    };
    
    struct __attribute__((packed)) Packed {
        char a;
        int b;
        char c;
        double d;
    };
    
    std::cout << "\n=== Struct Sizes ===" << std::endl;
    std::cout << "Unaligned struct: " << sizeof(Unaligned) << " bytes" << std::endl;
    std::cout << "Aligned struct:   " << sizeof(Aligned) << " bytes" << std::endl;
    std::cout << "Packed struct:    " << sizeof(Packed) << " bytes" << std::endl;
    
    std::cout << "\nMemory layout of Unaligned struct:" << std::endl;
    std::cout << R"(
Offset: 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
        [a][--padding--][b b b b][c][--padding--][d d d d d d d d]
         1      3        4         1      3        8
        
Total: 16 bytes (7 bytes wasted on padding!)
)" << std::endl;
    
    std::cout << "Memory layout of Aligned struct:" << std::endl;
    std::cout << R"(
Offset: 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
        [d d d d d d d d][b b b b][a][c][--padding--]
         8                4         1  1      2
        
Total: 16 bytes (only 2 bytes wasted - better!)
)" << std::endl;
}

void benchmark_alignment() {
    const size_t N = 10000000;
    
    std::cout << "\n=== Alignment Performance Impact ===" << std::endl;
    
    // Aligned allocation
    int* aligned = (int*)aligned_alloc(64, N * sizeof(int));
    
    // Misaligned allocation
    char* buffer = new char[N * sizeof(int) + 1];
    int* misaligned = (int*)(buffer + 1);  // Offset by 1 byte!
    
    // Initialize
    for (size_t i = 0; i < N; i++) {
        aligned[i] = i;
        misaligned[i] = i;
    }
    
    // Benchmark aligned access
    auto start = std::chrono::high_resolution_clock::now();
    long long sum = 0;
    for (size_t i = 0; i < N; i++) {
        sum += aligned[i];
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto aligned_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Benchmark misaligned access
    start = std::chrono::high_resolution_clock::now();
    sum = 0;
    for (size_t i = 0; i < N; i++) {
        sum += misaligned[i];
    }
    end = std::chrono::high_resolution_clock::now();
    auto misaligned_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Aligned access:    " << aligned_time.count() << " μs" << std::endl;
    std::cout << "Misaligned access: " << misaligned_time.count() << " μs" << std::endl;
    std::cout << "Penalty: " << (double)misaligned_time.count() / aligned_time.count() 
              << "x slower" << std::endl;
    
    free(aligned);
    delete[] buffer;
}

int main() {
    demonstrate_alignment();
    benchmark_alignment();
    
    std::cout << "\n=== Alignment Best Practices ===" << std::endl;
    std::cout << "1. Order struct members by size (largest first)" << std::endl;
    std::cout << "2. Use alignas() for cache line alignment" << std::endl;
    std::cout << "3. Use aligned_alloc() for dynamic allocation" << std::endl;
    std::cout << "4. Avoid packed structs in hot paths" << std::endl;
    
    return 0;
}