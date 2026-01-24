// deadlock_demo.cpp
#include <iostream>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void* thread1_function(void* arg) {
    std::cout << "Thread 1: Trying to lock mutex1..." << std::endl;
    pthread_mutex_lock(&mutex1);
    std::cout << "Thread 1: Locked mutex1" << std::endl;
    
    sleep(1);  // Give thread 2 time to lock mutex2
    
    std::cout << "Thread 1: Trying to lock mutex2..." << std::endl;
    pthread_mutex_lock(&mutex2);  // DEADLOCK HERE!
    std::cout << "Thread 1: Locked mutex2" << std::endl;
    
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    
    return nullptr;
}

void* thread2_function(void* arg) {
    std::cout << "Thread 2: Trying to lock mutex2..." << std::endl;
    pthread_mutex_lock(&mutex2);
    std::cout << "Thread 2: Locked mutex2" << std::endl;
    
    sleep(1);  // Give thread 1 time to lock mutex1
    
    std::cout << "Thread 2: Trying to lock mutex1..." << std::endl;
    pthread_mutex_lock(&mutex1);  // DEADLOCK HERE!
    std::cout << "Thread 2: Locked mutex1" << std::endl;
    
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    
    return nullptr;
}

int main() {
    std::cout << "=== Deadlock Demonstration ===" << std::endl;
    std::cout << "This program will HANG!" << std::endl;
    std::cout << "Press Ctrl+C to kill it\n" << std::endl;
    
    pthread_t thread1, thread2;
    
    pthread_create(&thread1, nullptr, thread1_function, nullptr);
    pthread_create(&thread2, nullptr, thread2_function, nullptr);
    
    pthread_join(thread1, nullptr);
    pthread_join(thread2, nullptr);
    
    std::cout << "Done (you won't see this!)" << std::endl;
    
    return 0;
}


// **Deadlock Visualization:**
// ```
// Thread 1                 Thread 2
//    │                        │
//    ├─ Lock mutex1           │
//    │                        ├─ Lock mutex2
//    │                        │
//    ├─ Wait for mutex2 ─────►│
//    │      (blocked)         │
//    │                        ├─ Wait for mutex1
//    │◄────────────────────── │     (blocked)
//    │                        │
//    DEADLOCK! Both waiting forever



// Lock Ordering (most common solution):

// // ALWAYS lock in the same order!
// void* safe_thread(void* arg) {
//     pthread_mutex_lock(&mutex1);  // Always lock 1 first
//     pthread_mutex_lock(&mutex2);  // Then lock 2
    
//     // Do work...
    
//     pthread_mutex_unlock(&mutex2);
//     pthread_mutex_unlock(&mutex1);
    
//     return nullptr;
// }


// Try-lock with timeout:

// if (pthread_mutex_trylock(&mutex2) != 0) {
//     // Couldn't get lock, release first lock
//     pthread_mutex_unlock(&mutex1);
//     // Retry later
// }