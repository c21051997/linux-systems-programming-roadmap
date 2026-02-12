// data_layout.cpp
#include <iostream>
#include <chrono>
#include <vector>

// Array of Structures (AoS) - Traditional approach
struct AoS_Particle {
    float x, y, z;        // Position
    float vx, vy, vz;     // Velocity
    float mass;
    int id;
};

// Structure of Arrays (SoA) - Cache-friendly approach
struct SoA_Particles {
    std::vector<float> x, y, z;
    std::vector<float> vx, vy, vz;
    std::vector<float> mass;
    std::vector<int> id;
    
    void resize(size_t n) {
        x.resize(n); y.resize(n); z.resize(n);
        vx.resize(n); vy.resize(n); vz.resize(n);
        mass.resize(n);
        id.resize(n);
    }
};

void benchmark_data_layout() {
    const size_t N = 1000000;
    const int ITERATIONS = 100;
    
    std::cout << "=== Array of Structures vs Structure of Arrays ===" << std::endl;
    
    // AoS setup
    std::vector<AoS_Particle> aos_particles(N);
    for (size_t i = 0; i < N; i++) {
        aos_particles[i].x = i;
        aos_particles[i].vx = 1.0f;
    }
    
    // Task: Update only X position (common in physics engines)
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        for (size_t i = 0; i < N; i++) {
            aos_particles[i].x += aos_particles[i].vx;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto aos_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // SoA setup
    SoA_Particles soa_particles;
    soa_particles.resize(N);
    for (size_t i = 0; i < N; i++) {
        soa_particles.x[i] = i;
        soa_particles.vx[i] = 1.0f;
    }
    
    start = std::chrono::high_resolution_clock::now();
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        for (size_t i = 0; i < N; i++) {
            soa_particles.x[i] += soa_particles.vx[i];
        }
    }
    
    end = std::chrono::high_resolution_clock::now();
    auto soa_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "\nUpdating X position for " << N << " particles:" << std::endl;
    std::cout << "AoS time: " << aos_time.count() << " μs" << std::endl;
    std::cout << "SoA time: " << soa_time.count() << " μs" << std::endl;
    std::cout << "Speedup: " << (double)aos_time.count() / soa_time.count() << "x" << std::endl;
    
    std::cout << "\n=== Why SoA is Faster ===" << std::endl;
    std::cout << R"(
AoS Memory Layout (cache-unfriendly):
[x,y,z,vx,vy,vz,mass,id][x,y,z,vx,vy,vz,mass,id]...
 └─ Need x and vx     └─ Need x and vx
 
Accessing x: Load entire struct (32 bytes)
  → Wasted 24 bytes loaded into cache!

SoA Memory Layout (cache-friendly):
x array:  [x][x][x][x][x][x][x][x]...
vx array: [vx][vx][vx][vx][vx][vx]...

Accessing x: Sequential memory access
  → Every byte loaded is used!
  → Better cache utilization
  → CPU prefetcher works perfectly
)" << std::endl;
}

int main() {
    benchmark_data_layout();
    
    std::cout << "\n=== When to Use Each ===" << std::endl;
    std::cout << "Array of Structures (AoS):" << std::endl;
    std::cout << "  ✓ Random access patterns" << std::endl;
    std::cout << "  ✓ Need all fields together" << std::endl;
    std::cout << "  ✓ Object-oriented code" << std::endl;
    
    std::cout << "\nStructure of Arrays (SoA):" << std::endl;
    std::cout << "  ✓ Sequential access patterns" << std::endl;
    std::cout << "  ✓ Process one field at a time" << std::endl;
    std::cout << "  ✓ SIMD vectorization" << std::endl;
    std::cout << "  ✓ High-performance computing" << std::endl;
    
    return 0;
}