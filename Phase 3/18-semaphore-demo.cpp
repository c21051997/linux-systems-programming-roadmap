/*
Tehnical planning:
class ParkingLot: private: sem_t(spaces), int (total_spaces)
public: constructor, enter func (int car_id), leave (car_id), destructor

enter: get space, leave: free up space

Global: parking lot (lot)
car_thread func: (arg = id) enter, sleep, leave

main func: init global parking log to 3 spaces, creae 10 car threads, array of 10 ids,
    create threads, usleep(200000) in loop but after create, join threads, DELETE lot
*/
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

class ParkingLot {
private:
    sem_t spaces;
    int total_spaces;
    
public:
    ParkingLot(int capacity) : total_spaces(capacity) {
        sem_init(&spaces, 0, capacity);
    }
    
    void enter(int car_id) {
        std::cout << "Car " << car_id << ": Trying to enter..." << std::endl;
        
        sem_wait(&spaces);  // Wait for available space
        
        int available;
        sem_getvalue(&spaces, &available);
        
        std::cout << "Car " << car_id << ": ENTERED! " 
                  << available << " spaces remaining" << std::endl;
    }
    
    void leave(int car_id) {
        std::cout << "Car " << car_id << ": LEAVING!" << std::endl;
        
        sem_post(&spaces);  // Release space
        
        int available;
        sem_getvalue(&spaces, &available);
        
        std::cout << "Car " << car_id << ": Left. " 
                  << available << " spaces available" << std::endl;
    }
    
    ~ParkingLot() {
        sem_destroy(&spaces);
    }
};

ParkingLot* lot;

void* car_thread(void* arg) {
    int id = *(int*)arg;
    
    lot->enter(id);
    
    // Park for a while
    sleep(2 + (id % 3));
    
    lot->leave(id);
    
    return nullptr;
}

int main() {
    std::cout << "=== Semaphore: Parking Lot with 3 Spaces ===" << std::endl;
    
    lot = new ParkingLot(3);  // Only 3 spaces
    
    pthread_t cars[10];
    int ids[10];
    
    std::cout << "\n10 cars trying to park in 3-space lot:\n" << std::endl;
    
    for (int i = 0; i < 10; i++) {
        ids[i] = i + 1;
        pthread_create(&cars[i], nullptr, car_thread, &ids[i]);
        usleep(200000);  // Stagger arrivals
    }
    
    for (int i = 0; i < 10; i++) {
        pthread_join(cars[i], nullptr);
    }
    
    delete lot;
    
    std::cout << "\n=== Semaphore Types ===" << std::endl;
    std::cout << "Binary semaphore (0 or 1): Like a mutex" << std::endl;
    std::cout << "Counting semaphore (0 to N): Resource pool" << std::endl;
    
    std::cout << "\n=== Operations ===" << std::endl;
    std::cout << "sem_wait():  Decrement (wait if 0)" << std::endl;
    std::cout << "sem_post():  Increment (signal)" << std::endl;
    std::cout << "sem_trywait(): Non-blocking wait" << std::endl;
    
    return 0;
}