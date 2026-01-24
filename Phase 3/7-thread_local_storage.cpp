// thread_local_storage.cpp
#include <iostream>
#include <pthread.h>

// Each thread gets its own copy
__thread int thread_local_counter = 0;

// Global (shared)
int global_counter = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* worker_thread(void* arg) {
    int id = *(int*)arg;
    
    for (int i = 0; i < 5; i++) {
        // Thread-local: no synchronization needed!
        thread_local_counter++;
        
        // Global: needs mutex
        pthread_mutex_lock(&mutex);
        global_counter++;
        pthread_mutex_unlock(&mutex);
        
        std::cout << "Thread " << id 
                  << ": local=" << thread_local_counter 
                  << ", global=" << global_counter << std::endl;
    }
    
    return nullptr;
}

int main() {
    std::cout << "=== Thread-Local Storage ===" << std::endl;
    
    pthread_t threads[3];
    int ids[] = {1, 2, 3};
    
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], nullptr, worker_thread, &ids[i]);
    }
    
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], nullptr);
    }
    
    std::cout << "\nMain thread local counter: " << thread_local_counter 
              << std::endl;
    std::cout << "Global counter: " << global_counter << std::endl;
    
    return 0;
}