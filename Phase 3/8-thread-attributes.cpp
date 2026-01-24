// thread_attributes.cpp
#include <iostream>
#include <pthread.h>
#include <unistd.h>

void* detached_thread(void* arg) {
    std::cout << "Detached thread running..." << std::endl;
    sleep(2);
    std::cout << "Detached thread done!" << std::endl;
    return nullptr;
}

void* joinable_thread(void* arg) {
    std::cout << "Joinable thread running..." << std::endl;
    sleep(1);
    std::cout << "Joinable thread done!" << std::endl;
    return nullptr;
}

int main() {
    std::cout << "=== Thread Attributes ===" << std::endl;
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    
    // === Detached Thread ===
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // Don't need pthread_join for detached threads!
    pthread_t thread1;
    pthread_create(&thread1, &attr, detached_thread, nullptr);

    std::cout << "Detached thread created (no join needed)" << std::endl;
    
    // === Joinable Thread (default) ===
    pthread_t thread2;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    pthread_create(&thread2, &attr, joinable_thread, nullptr);
    
    std::cout << "Joinable thread created (will join)" << std::endl;
    pthread_join(thread2, nullptr);

    // === Stack Size ===
    size_t stacksize;
    pthread_attr_getstacksize(&attr, &stacksize);

    std::cout << "\nDefault stack size: " << stacksize / 1024 << " KB" << std::endl;
    
    pthread_attr_setstacksize(&attr, 2 * 1024 * 1024); // 2 MB
    pthread_attr_getstacksize(&attr, &stacksize);

    std::cout << "Modified stack size: " << stacksize / 1024 << " KB" << std::endl;
    
    pthread_attr_destroy(&attr); 
    
    sleep(3);  // Give detached thread time to finish
    
    return 0;
}