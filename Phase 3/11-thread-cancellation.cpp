// thread_cancellation.cpp
#include <iostream>
#include <pthread.h>
#include <unistd.h>

void* cancellable_thread(void* arg) {
    // Set cancellation state
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, nullptr);
    
    for (int i = 1; i <= 10; i++) {
        std::cout << "Thread working... " << i << std::endl;
        sleep(1);
        
        // Cancellation point (can be cancelled here)
        pthread_testcancel();
    }
    
    std::cout << "Thread finished normally" << std::endl;
    return nullptr;
}

void cleanup_handler(void* arg) {
    std::cout << "Cleanup handler called!" << std::endl;
    // Free resources here
}

void* thread_with_cleanup(void* arg) {
    pthread_cleanup_push(cleanup_handler, nullptr);
    
    for (int i = 1; i <= 10; i++) {
        std::cout << "Thread with cleanup working... " << i << std::endl;
        sleep(1);
    }
    
    pthread_cleanup_pop(1);  // Execute cleanup
    return nullptr;
}

int main() {
    std::cout << "=== Thread Cancellation ===" << std::endl;
    
    pthread_t thread;
    pthread_create(&thread, nullptr, cancellable_thread, nullptr);
    
    sleep(3);  // Let it run for 3 seconds
    
    std::cout << "\nMain: Cancelling thread..." << std::endl;
    pthread_cancel(thread);
    
    pthread_join(thread, nullptr);
    std::cout << "Thread cancelled\n" << std::endl;
    
    // Thread with cleanup
    std::cout << "=== Thread with Cleanup ===" << std::endl;
    pthread_create(&thread, nullptr, thread_with_cleanup, nullptr);
    
    sleep(2);
    pthread_cancel(thread);
    pthread_join(thread, nullptr);
    
    return 0;
}