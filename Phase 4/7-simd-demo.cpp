// simd_demo.cpp
#include <iostream>
#include <chrono>
#include <vector>
#include <immintrin.h>  // AVX/SSE intrinsics

void scalar_add() {
    const size_t N = 10000000;
    std::vector<float> a(N, 1.0f);
    std::vector<float> b(N, 2.0f);
    std::vector<float> c(N);
    
    std::cout << "=== Scalar Addition ===" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < N; i++) {
        c[i] = a[i] + b[i];  // One operation at a time
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto scalar_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Time: " << scalar_time.count() << " ms" << std::endl;
}

void simd_add() {
    const size_t N = 10000000;
    std::vector<float> a(N, 1.0f);
    std::vector<float> b(N, 2.0f);
    std::vector<float> c(N);
    
    std::cout << "\n=== SIMD Addition (AVX - 8 floats at once) ===" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < N; i += 8) {
        // Load 8 floats from a and b
        __m256 va = _mm256_load_ps(&a[i]);
        __m256 vb = _mm256_load_ps(&b[i]);
        
        // Add 8 floats in parallel
        __m256 vc = _mm256_add_ps(va, vb);
        
        // Store 8 floats to c
        _mm256_store_ps(&c[i], vc);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto simd_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Time: " << simd_time.count() << " ms" << std::endl;
}

void demonstrate_simd_concepts() {
    std::cout << "\n=== SIMD Architecture ===" << std::endl;
    std::cout << R"(
Scalar (Traditional):
    Add operation processes ONE float at a time:
    [a1] + [b1] = [c1]  (1 cycle)
    [a2] + [b2] = [c2]  (1 cycle)
    [a3] + [b3] = [c3]  (1 cycle)
    ...
    8 operations = 8 cycles

SIMD (AVX):
    Add operation processes EIGHT floats at once:
    [a1|a2|a3|a4|a5|a6|a7|a8] +
    [b1|b2|b3|b4|b5|b6|b7|b8] =
    [c1|c2|c3|c4|c5|c6|c7|c8]  (1 cycle!)
    
    8 operations = 1 cycle
    8x speedup!

SIMD Instruction Sets:
  SSE  (x86):     128-bit (4 floats)
  AVX  (x86):     256-bit (8 floats)
  AVX-512 (x86):  512-bit (16 floats)
  NEON (ARM):     128-bit (4 floats)
)" << std::endl;
}

void auto_vectorization_demo() {
    std::cout << "\n=== Auto-Vectorization ===" << std::endl;
    std::cout << R"(
The compiler can often vectorize your code automatically!

Example:
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }

Compile with: g++ -O3 -march=native -ftree-vectorize

Compiler transforms to:
    for (int i = 0; i < n; i += 8) {
        // SIMD instructions
    }

Check vectorization:
    g++ -O3 -fopt-info-vec-optimized
)" << std::endl;
}

int main() {
    demonstrate_simd_concepts();
    scalar_add();
    simd_add();
    auto_vectorization_demo();
    
    std::cout << "\n=== SIMD Best Practices ===" << std::endl;
    std::cout << "1. Use -march=native -O3 for auto-vectorization" << std::endl;
    std::cout << "2. Align data to 16/32 bytes (use alignas or posix_memalign)" << std::endl;
    std::cout << "3. Avoid pointer aliasing (use __restrict)" << std::endl;
    std::cout << "4. Keep loops simple (no complex control flow)" << std::endl;
    std::cout << "5. Use SoA instead of AoS" << std::endl;
    
    return 0;
}