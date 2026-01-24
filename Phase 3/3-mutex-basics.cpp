// mutex_basic.cpp
#include <iostream>
#include <pthread.h>

int global_counter = 0;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void* safe_increment_thread(void* arg) {
    int iterations = *(int*)arg;
    
    for (int i = 0; i < iterations; i++) {
        pthread_mutex_lock(&counter_mutex);    // Lock
        global_counter++;                       // Critical section
        pthread_mutex_unlock(&counter_mutex);  // Unlock
    }
    
    return nullptr;
}

int main() {
    std::cout << "=== Mutex Protection ===" << std::endl;
    
    pthread_t thread1, thread2;
    int iterations = 1000000;
    
    std::cout << "Initial counter: " << global_counter << std::endl;
    std::cout << "Each thread will increment " << iterations << " times" << std::endl;
    std::cout << "Expected final value: " << iterations * 2 << std::endl;
    
    pthread_create(&thread1, nullptr, safe_increment_thread, &iterations);
    pthread_create(&thread2, nullptr, safe_increment_thread, &iterations);
    
    pthread_join(thread1, nullptr);
    pthread_join(thread2, nullptr);
    
    std::cout << "Actual final value: " << global_counter << std::endl;
    
    if (global_counter == iterations * 2) {
        std::cout << "✓ SUCCESS! No race condition!" << std::endl;
    }
    
    pthread_mutex_destroy(&counter_mutex);
    
    return 0;
}