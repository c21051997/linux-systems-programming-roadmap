// thread_basic.cpp
#include <iostream>
#include <pthread.h>
#include <unistd.h>

// Thread function signature: void* function(void* arg)
void* thread_function(void* arg) {
    int thread_id = *(int*)arg;
    
    std::cout << "Thread " << thread_id << " started!" << std::endl;
    std::cout << "  Thread ID (pthread_t): " << pthread_self() << std::endl;
    
    // Do some work
    for (int i = 0; i < 5; i++) {
        std::cout << "Thread " << thread_id << " working... " << i << std::endl;
        sleep(1);
    }
    
    std::cout << "Thread " << thread_id << " finished!" << std::endl;
    
    // Return value (can be retrieved by pthread_join)
    int* result = new int(thread_id * 100);
    return (void*)result;
}

int main() {
    std::cout << "=== Basic Threading ===" << std::endl;
    std::cout << "Main thread ID: " << pthread_self() << std::endl;
    
    pthread_t thread1, thread2;
    int id1 = 1, id2 = 2;
    
    // Create threads
    std::cout << "\nCreating threads..." << std::endl;
    
    int ret1 = pthread_create(&thread1, nullptr, thread_function, &id1);
    int ret2 = pthread_create(&thread2, nullptr, thread_function, &id2);
    
    if (ret1 != 0 || ret2 != 0) {
        std::cerr << "pthread_create failed!" << std::endl;
        return 1;
    }
    
    std::cout << "Threads created!" << std::endl;
    
    // Main thread continues...
    std::cout << "Main thread doing other work..." << std::endl;
    sleep(2);
    
    // Wait for threads to finish
    std::cout << "\nWaiting for threads to finish..." << std::endl;
    
    void* result1;
    void* result2;
    
    pthread_join(thread1, &result1);
    pthread_join(thread2, &result2);
    
    std::cout << "\nThread 1 returned: " << *(int*)result1 << std::endl;
    std::cout << "Thread 2 returned: " << *(int*)result2 << std::endl;
    
    delete (int*)result1;
    delete (int*)result2;
    
    std::cout << "\nAll threads finished. Main exiting." << std::endl;
    
    return 0;
}