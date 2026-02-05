/*
TP:

Test 1: mutex
initialise mutex + mutex_counter

mutex_test - loop through iterations and increment counter

Test 2: spinlock
init spinlock (atomic flag) + spin_counter

spinlock_test - loop through, then while spinlock test and set (memory order acquire), increment counter, spinglock clear (memoru prder release)

Test 3: atomic - fetch add with memory order relaxed

Test 4: sempahore

*/
// sync_benchmark.cpp
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <atomic>
#include <chrono>

#define ITERATIONS 1000000

// Test 1: Mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int mutex_counter = 0;

void* mutex_test(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&mutex);
        mutex_counter++;
        pthread_mutex_unlock(&mutex);
    }
    return nullptr;
}

// Test 2: Spinlock
std::atomic_flag spinlock = ATOMIC_FLAG_INIT;
int spin_counter = 0;

void* spinlock_test(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        while (spinlock.test_and_set(std::memory_order_acquire)) {}
        spin_counter++;
        spinlock.clear(std::memory_order_release);
    }
    return nullptr;
}

// Test 3: Atomic
std::atomic<int> atomic_counter(0);

void* atomic_test(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        atomic_counter.fetch_add(1, std::memory_order_relaxed);
    }
    return nullptr;
}

// Test 4: Semaphore
sem_t semaphore;
int sem_counter = 0;

void* semaphore_test(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        sem_wait(&semaphore);
        sem_counter++;
        sem_post(&semaphore);
    }
    return nullptr;
}

void benchmark(const char* name, void* (*func)(void*), int& counter) {
    counter = 0;
    pthread_t threads[4];
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], nullptr, func, nullptr);
    }
    
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], nullptr);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << name << ":" << std::endl;
    std::cout << "  Time: " << duration.count() << " ms" << std::endl;
    std::cout << "  Throughput: " << (ITERATIONS * 4 / duration.count()) << " ops/ms" << std::endl;
    std::cout << "  Final value: " << counter << std::endl;
    std::cout << std::endl;
}

int main() {
    std::cout << "=== Synchronization Primitive Benchmark ===" << std::endl;
    std::cout << "4 threads, " << ITERATIONS << " increments each\n" << std::endl;
    
    sem_init(&semaphore, 0, 1);
    
    benchmark("Mutex     ", mutex_test, mutex_counter);
    benchmark("Spinlock  ", spinlock_test, spin_counter);
    
    int atomic_val = atomic_counter.load();
    benchmark("Atomic    ", atomic_test, atomic_val);
    atomic_val = atomic_counter.load();
    
    benchmark("Semaphore ", semaphore_test, sem_counter);
    
    sem_destroy(&semaphore);
    
    std::cout << "=== Performance Summary ===" << std::endl;
    std::cout << "Fastest to Slowest (typically):" << std::endl;
    std::cout << "1. Atomic operations (no synchronization overhead)" << std::endl;
    std::cout << "2. Spinlock (short critical sections)" << std::endl;
    std::cout << "3. Mutex (general purpose)" << std::endl;
    std::cout << "4. Semaphore (counting + overhead)" << std::endl;
    
    return 0;
}