/* 
class Barrier: private: mutex, cond, int (count), int (waiting), const int (total)
public funcs: constructor (initialise mutex / cond as well), 
wait(): lock mutex, increment waiting, check if all threads have arrived (if they have then set waiting to 0, increment count, broadcast to all threads),
    else while current count == count, wait, outside if: unlock
Destructor: destory mutex + cd
*/

#include <iostream>
#include <pthread.h>
#include <unistd.h>

class Barrier {
private: 
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    int count;
    int waiting;
    const int total;
public:
    Barrier(int n) : total(n), count(0), waiting(0) {
        pthread_mutex_init(&mtx, nullptr);
        pthread_cond_init(&cond, nullptr);
    }

    void wait() {
        pthread_mutex_lock(&mtx);

        waiting++;

        if (waiting == total) {
            // Last thread to arrive
            waiting = 0;
            count++;
            pthread_cond_broadcast(&cond); // Wake everyone
        } else {
            int current_count = count;
            while (current_count = count) {
                pthread_cond_wait(&cond, &mtx);
            }
        }

        pthread_mutex_unlock(&mtx);
    }

    ~Barrier() {
        pthread_mutex_destroy(&mtx);
        pthread_cond_destroy(&cond);
    }
};

Barrier barrier(4);

void* worker_thread(void* arg) {
    int id = *(int*)arg;

    for (int phase = 1; phase <= 3; phase++) {
        std::cout << "Thread " << id << ": Starting phase " << phase << std::endl;
        
        // Simulate work (different durations)
        sleep(id);
        
        std::cout << "Thread " << id << ": Finished phase " << phase 
                  << ", waiting at barrier..." << std::endl;
        
        barrier.wait();  // Wait for all threads
        
        std::cout << "Thread " << id << ": All threads done with phase " 
                  << phase << ", continuing..." << std::endl;
    }
    
    std::cout << "Thread " << id << ": ALL PHASES COMPLETE" << std::endl;
    return nullptr;
}

int main() {
    std::cout << "=== Barrier Synchronization ===" << std::endl;
    std::cout << "4 threads, 3 phases each\n" << std::endl;
    
    pthread_t threads[4];
    int ids[] = {1, 2, 3, 4};
    
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], nullptr, worker_thread, &ids[i]);
    }
    
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], nullptr);
    }
    
    std::cout << "\n=== Barrier Use Cases ===" << std::endl;
    std::cout << "- Parallel algorithms with phases (e.g., iterative solvers)" << std::endl;
    std::cout << "- Ensuring consistent snapshots" << std::endl;
    std::cout << "- Coordinating parallel initialization" << std::endl;
    std::cout << "- Synchronizing simulation steps" << std::endl;
    
    return 0;
}