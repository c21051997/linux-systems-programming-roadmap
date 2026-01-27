// shared_memory_threads.cpp
#include <iostream>
#include <pthread.h>
#include <unistd.h>

// Global variable (shared by all threads!)
int global_counter = 0;

void* increment_thread(void* arg) {
    int iterations = *(int*)arg;
    
    for (int i = 0; i < iterations; i++) {
        global_counter++;  // DANGER! Race condition!
    }
    
    return nullptr;
}

int main() {
    std::cout << "=== Demonstrating Shared Memory ===" << std::endl;
    
    pthread_t thread1, thread2;
    int iterations = 1000000;
    
    std::cout << "Initial counter: " << global_counter << std::endl;
    std::cout << "Each thread will increment " << iterations << " times" << std::endl;
    std::cout << "Expected final value: " << iterations * 2 << std::endl;
    
    pthread_create(&thread1, nullptr, increment_thread, &iterations);
    pthread_create(&thread2, nullptr, increment_thread, &iterations);
    
    pthread_join(thread1, nullptr);
    pthread_join(thread2, nullptr);
    
    std::cout << "Actual final value: " << global_counter << std::endl;
    std::cout << "Lost updates: " << (iterations * 2 - global_counter) << std::endl;
    
    if (global_counter != iterations * 2) {
        std::cout << "\n⚠️  RACE CONDITION! Results are incorrect!" << std::endl;
    }
    
    return 0;
}