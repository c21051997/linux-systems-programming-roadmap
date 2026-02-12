// cache_measurement.cpp
#include <iostream>
#include <vector>

void cache_friendly_code() {
    const size_t N = 10000000;
    std::vector<int> data(N);
    
    // Sequential access - cache friendly
    for (size_t i = 0; i < N; i++) {
        data[i] = i * 2;
    }
    
    long long sum = 0;
    for (size_t i = 0; i < N; i++) {
        sum += data[i];
    }
    
    std::cout << "Cache-friendly sum: " << sum << std::endl;
}

void cache_unfriendly_code() {
    const size_t N = 10000000;
    std::vector<int> data(N);
    
    // Random stride access - cache unfriendly
    for (size_t i = 0; i < N; i++) {
        data[i] = i;
    }
    
    long long sum = 0;
    for (size_t i = 0; i < N; i += 16) {  // Large stride
        sum += data[i];
    }
    
    std::cout << "Cache-unfriendly sum: " << sum << std::endl;
}

int main() {
    std::cout << "=== Cache Performance Measurement ===" << std::endl;
    std::cout << "\nRun this program with perf to see cache statistics:" << std::endl;
    std::cout << "  perf stat -e cache-references,cache-misses,L1-dcache-loads,L1-dcache-load-misses ./cache_measurement" << std::endl;
    
    cache_friendly_code();
    cache_unfriendly_code();
    
    return 0;
}


/*
Perf Commands:
bash# General cache stats
perf stat -e cache-references,cache-misses ./program

# Detailed cache events
perf stat -e L1-dcache-loads,L1-dcache-load-misses,L1-dcache-stores ./program

# All cache events
perf stat -e cache-references,cache-misses,LLC-loads,LLC-load-misses ./program

# Record and analyze
perf record -e cache-misses ./program
perf report
*/