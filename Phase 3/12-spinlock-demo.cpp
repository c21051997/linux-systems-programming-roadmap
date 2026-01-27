/*
Technical planning:

Class Name = Spinlock
Member variables = std::atmoic_flag (lock_Flag)

globals = pthread_mutex_t (mutex), Spinlock (spinlock), int (shared_counter)

API:
Spinlock - lock() = attempt to acquire the flag, pause if on 86 bit system for better performance (compile time)
Spinlock - unlock = release the flag

spinlock_thread() = input void* arg (no. of iterations), output void* (nullptr), loop and lock and unlock spinlock, increase the shared counter between
mutex_thread = same ^
benchmark_lock = inputs: const char* (name), void* (*thread_func)(void*), int (iterations)
    Logic: set thread count to 0, create 4 threads, start clock, create all threads (loop), then join all threads (loop), stop clock, calc + print duration
*/

// spinlock_demo.cpp
#include <iostream>
#include <pthread.h>
#include <chrono>
#include <atomic>

class Spinlock {
private:
    std::atomic_flag lock_flag = ATOMIC_FLAG_INIT;
public:
    void lock(){

        while (lock_flag.test_and_set(std::memory_order_acquire)) {
            // Spin (busy wait)
            // Optional: Can add a pause instruction for better performance
            // This is a compile time check for x86 bit CPU
            #if defined(__x86_64__) || defined(__i386__)
            __builtin_ia32_pause(); // Tell it to pause to avoid hammering CPU as much as possible
            #endif
        }
    }

    void unlock(){
        lock_flag.clear(std::memory_order_release);
    }
};

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
Spinlock spinlock;
int shared_counter = 0;

void* spinlock_thread(void* arg) {
    int iterations = *(int*) arg;

    for (int i = 0; i < iterations; ++i) {
        spinlock.lock();
        shared_counter++;
        spinlock.unlock();
    }

    return nullptr;
}

void* mutex_thread(void* arg) {
    int iterations = *(int*) arg;

    for (int i = 0; i < iterations; ++i) {
        pthread_mutex_lock(&mtx);
        shared_counter++;
        pthread_mutex_unlock(&mtx);
    }

    return nullptr;
}

void benchmark_lock(const char* name, void* (*thread_func)(void*), int iterations) {
    shared_counter = 0;
    pthread_t threads[4];

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 4; ++i) {
        pthread_create(&threads[i], nullptr, thread_func, &iterations);
    }

    for (int i = 0; i < 4; ++i) {
        pthread_join(threads[i], nullptr);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << name << ": " << duration.count() << " μs" << std::endl;
    std::cout << "  Final counter: " << shared_counter << std::endl;
}

int main() {
    std::cout << "=== Spinlock vs Mutex Performance ===" << std::endl;
    
    int iterations = 100000;
    
    std::cout << "\nShort critical section (" << iterations << " iterations):" << std::endl;
    benchmark_lock("Spinlock", spinlock_thread, iterations);
    benchmark_lock("Mutex   ", mutex_thread, iterations);
    
    std::cout << "\n=== When to Use Each ===" << std::endl;
    std::cout << "Spinlock:" << std::endl;
    std::cout << "  ✓ Very short critical sections (<100ns)" << std::endl;
    std::cout << "  ✓ Low contention" << std::endl;
    std::cout << "  ✓ Real-time systems (no scheduling delays)" << std::endl;
    std::cout << "  ✗ Long critical sections (wastes CPU)" << std::endl;
    std::cout << "  ✗ More threads than CPU cores" << std::endl;
    
    std::cout << "\nMutex:" << std::endl;
    std::cout << "  ✓ Longer critical sections" << std::endl;
    std::cout << "  ✓ High contention" << std::endl;
    std::cout << "  ✓ More threads than CPU cores" << std::endl;
    std::cout << "  ✗ Real-time requirements (scheduling delays)" << std::endl;
    
    return 0;
}