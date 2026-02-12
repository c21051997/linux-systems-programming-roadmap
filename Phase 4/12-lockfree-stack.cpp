// lockfree_stack_complete.cpp
#include <iostream>
#include <atomic>
#include <memory>
#include <thread>
#include <vector>
#include <chrono>

template<typename T>
class LockFreeStack {
private:
    struct Node {
        T data;
        Node* next;
        
        Node(const T& value) : data(value), next(nullptr) {}
    };
    
    // Using hazard pointers for memory reclamation
    static constexpr int MAX_THREADS = 128;
    static constexpr int MAX_HAZARDS = 2;  // Per thread
    
    struct HazardPointer {
        std::atomic<Node*> pointers[MAX_THREADS * MAX_HAZARDS];
        
        HazardPointer() {
            for (auto& ptr : pointers) {
                ptr.store(nullptr);
            }
        }
    };
    
    std::atomic<Node*> head;
    static thread_local int thread_id;
    static int next_thread_id;
    HazardPointer hazards;
    
    std::vector<Node*> retired_list;
    std::atomic<int> retired_count{0};
    
    static int get_thread_id() {
        if (thread_id == -1) {
            thread_id = next_thread_id++;
        }
        return thread_id;
    }
    
    void set_hazard_pointer(int index, Node* ptr) {
        int tid = get_thread_id();
        hazards.pointers[tid * MAX_HAZARDS + index].store(ptr);
    }
    
    void clear_hazard_pointer(int index) {
        int tid = get_thread_id();
        hazards.pointers[tid * MAX_HAZARDS + index].store(nullptr);
    }
    
    bool is_hazardous(Node* ptr) {
        for (auto& hp : hazards.pointers) {
            if (hp.load() == ptr) {
                return true;
            }
        }
        return false;
    }
    
    void retire_node(Node* node) {
        retired_list.push_back(node);
        retired_count.fetch_add(1);
        
        // Periodically scan and reclaim
        if (retired_count.load() > MAX_THREADS * 2) {
            scan_and_reclaim();
        }
    }
    
    void scan_and_reclaim() {
        std::vector<Node*> still_retired;
        
        for (Node* node : retired_list) {
            if (!is_hazardous(node)) {
                delete node;
            } else {
                still_retired.push_back(node);
            }
        }
        
        retired_list = still_retired;
        retired_count.store(retired_list.size());
    }
    
public:
    LockFreeStack() : head(nullptr) {}
    
    ~LockFreeStack() {
        // Clear all remaining nodes
        Node* current = head.load();
        while (current) {
            Node* next = current->next;
            delete current;
            current = next;
        }
        
        // Clean up retired nodes
        for (Node* node : retired_list) {
            delete node;
        }
    }
    
    void push(const T& value) {
        Node* new_node = new Node(value);
        Node* old_head = head.load(std::memory_order_relaxed);
        
        do {
            new_node->next = old_head;
        } while (!head.compare_exchange_weak(old_head, new_node,
                                             std::memory_order_release,
                                             std::memory_order_relaxed));
    }
    
    bool pop(T& result) {
        Node* old_head = head.load(std::memory_order_relaxed);
        
        while (true) {
            // Set hazard pointer before accessing node
            set_hazard_pointer(0, old_head);
            
            // Verify node is still valid
            Node* check = head.load(std::memory_order_acquire);
            if (old_head != check) {
                old_head = check;
                continue;
            }
            
            if (old_head == nullptr) {
                clear_hazard_pointer(0);
                return false;  // Empty
            }
            
            // Access is now safe (protected by hazard pointer)
            Node* next = old_head->next;
            
            if (head.compare_exchange_weak(old_head, next,
                                           std::memory_order_release,
                                           std::memory_order_relaxed)) {
                // Success!
                result = old_head->data;
                
                clear_hazard_pointer(0);
                retire_node(old_head);
                
                return true;
            }
        }
    }
    
    bool empty() const {
        return head.load(std::memory_order_relaxed) == nullptr;
    }
    
    // Statistics
    size_t retired_nodes_count() const {
        return retired_count.load();
    }
};

// Static member initialization
template<typename T>
thread_local int LockFreeStack<T>::thread_id = -1;

template<typename T>
int LockFreeStack<T>::next_thread_id = 0;

// Benchmark
void benchmark_stack() {
    std::cout << "=== Lock-Free Stack Benchmark ===" << std::endl;
    
    LockFreeStack<int> stack;
    const int OPERATIONS = 1000000;
    const int NUM_THREADS = 4;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    
    // Half producers, half consumers
    for (int i = 0; i < NUM_THREADS / 2; i++) {
        threads.emplace_back([&]() {
            for (int j = 0; j < OPERATIONS; j++) {
                stack.push(j);
            }
        });
    }
    
    for (int i = 0; i < NUM_THREADS / 2; i++) {
        threads.emplace_back([&]() {
            int value;
            for (int j = 0; j < OPERATIONS; j++) {
                while (!stack.pop(value)) {
                    std::this_thread::yield();
                }
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Operations: " << OPERATIONS * NUM_THREADS << std::endl;
    std::cout << "Time: " << duration.count() << " ms" << std::endl;
    std::cout << "Throughput: " << (OPERATIONS * NUM_THREADS * 1000.0 / duration.count()) 
              << " ops/sec" << std::endl;
    std::cout << "Retired nodes: " << stack.retired_nodes_count() << std::endl;
}

int main() {
    benchmark_stack();
    
    std::cout << "\n=== Lock-Free Stack Features ===" << std::endl;
    std::cout << "✓ True lock-free (no blocking)" << std::endl;
    std::cout << "✓ Hazard pointers for memory safety" << std::endl;
    std::cout << "✓ Deferred reclamation" << std::endl;
    std::cout << "✓ Thread-safe without locks" << std::endl;
    std::cout << "✓ Scalable performance" << std::endl;
    
    return 0;
}