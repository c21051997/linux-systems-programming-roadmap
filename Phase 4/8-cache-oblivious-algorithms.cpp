// cache_oblivious.cpp
#include <iostream>
#include <chrono>
#include <vector>

// Traditional matrix multiplication (cache-unfriendly)
void matrix_mult_naive(const std::vector<std::vector<int>>& A,
                       const std::vector<std::vector<int>>& B,
                       std::vector<std::vector<int>>& C,
                       int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = 0;
            for (int k = 0; k < n; k++) {
                C[i][j] += A[i][k] * B[k][j];
                // B[k][j] accesses are strided (cache-unfriendly!)
            }
        }
    }
}

// Blocked matrix multiplication (cache-aware)
void matrix_mult_blocked(const std::vector<std::vector<int>>& A,
                         const std::vector<std::vector<int>>& B,
                         std::vector<std::vector<int>>& C,
                         int n, int block_size) {
    for (int ii = 0; ii < n; ii += block_size) {
        for (int jj = 0; jj < n; jj += block_size) {
            for (int kk = 0; kk < n; kk += block_size) {
                // Work on a block at a time (fits in cache)
                for (int i = ii; i < std::min(ii + block_size, n); i++) {
                    for (int j = jj; j < std::min(jj + block_size, n); j++) {
                        int sum = 0;
                        for (int k = kk; k < std::min(kk + block_size, n); k++) {
                            sum += A[i][k] * B[k][j];
                        }
                        C[i][j] += sum;
                    }
                }
            }
        }
    }
}

void benchmark_matrix_multiply() {
    const int N = 512;  // Matrix size
    
    std::vector<std::vector<int>> A(N, std::vector<int>(N, 1));
    std::vector<std::vector<int>> B(N, std::vector<int>(N, 1));
    std::vector<std::vector<int>> C(N, std::vector<int>(N, 0));
    
    std::cout << "=== Matrix Multiplication (" << N << "x" << N << ") ===" << std::endl;
    
    // Naive version
    auto start = std::chrono::high_resolution_clock::now();
    matrix_mult_naive(A, B, C, N);
    auto end = std::chrono::high_resolution_clock::now();
    auto naive_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Blocked version
    C = std::vector<std::vector<int>>(N, std::vector<int>(N, 0));
    start = std::chrono::high_resolution_clock::now();
    matrix_mult_blocked(A, B, C, N, 64);  // 64x64 blocks
    end = std::chrono::high_resolution_clock::now();
    auto blocked_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Naive:   " << naive_time.count() << " ms" << std::endl;
    std::cout << "Blocked: " << blocked_time.count() << " ms" << std::endl;
    std::cout << "Speedup: " << (double)naive_time.count() / blocked_time.count() 
              << "x" << std::endl;
}

void explain_cache_blocking() {
    std::cout << "\n=== Cache Blocking Explanation ===" << std::endl;
    std::cout << R"(
Naive Matrix Multiplication:
  C[i][j] = Σ A[i][k] * B[k][j]
  
  Problem: B[k][j] accesses are strided
    - B[0][j], B[1][j], B[2][j], ...
    - Each access might be a cache miss!
    - For 512x512 matrix: 512³ = 134M operations
    - Many cache misses = SLOW

Blocked Matrix Multiplication:
  1. Divide matrices into blocks (e.g., 64x64)
  2. Multiply blocks
  3. Each block fits in cache!
  
  Benefits:
    - A's block: reused for entire row of C blocks
    - B's block: reused for entire column of C blocks
    - C's block: stays in cache during updates
    
  Result: ~10x fewer cache misses!

Visualization:
    A (NxN)     B (NxN)     C (NxN)
    [  ][  ]    [  ][  ]    [  ][  ]
    [  ][  ] ×  [  ][  ] =  [  ][  ]
    
    Process one block at a time:
    [XX][  ]    [XX][  ]    [XX][  ]
    [  ][  ] ×  [  ][  ] =  [  ][  ]
)" << std::endl;
}

int main() {
    explain_cache_blocking();
    benchmark_matrix_multiply();
    
    std::cout << "\n=== Cache-Oblivious Algorithms ===" << std::endl;
    std::cout << "Algorithms that work well regardless of cache size:" << std::endl;
    std::cout << "- Divide-and-conquer approaches" << std::endl;
    std::cout << "- Recursive algorithms" << std::endl;
    std::cout << "- Examples: merge sort, FFT, matrix operations" << std::endl;
    
    return 0;
}