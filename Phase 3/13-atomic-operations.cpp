/*
Globals: atomic<int> (atomic_counter), int (regular_counter)

Funcs:
void* regular_increment(void* arg): input = iterations, loop iterations and increment regular counter
void* atomic_increment(void*arg): ^^, increment atomic counter

main: create 4 threads for regular, join them, 4 threads for atomic counter + join, do some common atomic operations on new variable
*/

// atomic_operations.cpp
#include <iostream>
#include <pthread.h>
#include <atomic>
#include <chrono>


std::atomic<int> atomic_counter(0);
int regular_counter = 0;

void* regular_increment(void* arg) {
    int increment = *(int*) arg;

    for (int i = 0; i < increment; ++i) {
        regular_counter++;
    }
    return nullptr;
}

void* atomic_increment(void* arg) {
    int increment = *(void*) arg;

    for (int i = 0; i < increment; ++i) {
        atomic_counter.fetch_add(1);
    }
    return nullptr;
}

int main() {
    std::cout << "=== Atomic Operations ===" << std::endl;
    
    int iterations = 1000000;
    pthread_t threads[4];
    
    // Test regular counter
    std::cout << "\nTesting regular counter (with race condition):" << std::endl;
    regular_counter = 0;
    
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], nullptr, regular_increment, &iterations);
    }
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], nullptr);
    }
    
    std::cout << "Expected: " << iterations * 4 << std::endl;
    std::cout << "Actual:   " << regular_counter << std::endl;
    std::cout << "Lost:     " << (iterations * 4 - regular_counter) << std::endl;
    
    // Test atomic counter
    std::cout << "\nTesting atomic counter (lock-free, safe):" << std::endl;
    atomic_counter = 0;
    
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], nullptr, atomic_increment, &iterations);
    }
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], nullptr);
    }
    
    std::cout << "Expected: " << iterations * 4 << std::endl;
    std::cout << "Actual:   " << atomic_counter.load() << std::endl;
    
    // Common atomic operations
    std::cout << "\n=== Atomic Operations Available ===" << std::endl;
    std::atomic<int> x(10);
    
    std::cout << "Initial value: " << x.load() << std::endl;
    
    x.store(20);
    std::cout << "After store(20): " << x.load() << std::endl;
    
    int old = x.fetch_add(5);
    std::cout << "fetch_add(5) returned: " << old << ", new value: " << x.load() << std::endl;
    
    old = x.fetch_sub(3);
    std::cout << "fetch_sub(3) returned: " << old << ", new value: " << x.load() << std::endl;
    
    int expected = 22;
    bool success = x.compare_exchange_strong(expected, 100);
    std::cout << "compare_exchange_strong(22, 100): " << (success ? "SUCCESS" : "FAILED") << std::endl;
    std::cout << "New value: " << x.load() << std::endl;
    
    return 0;
}