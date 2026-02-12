// aba_problem_detailed.cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

// Problem demonstration
struct Node {
    int data;
    Node* next;
    Node(int d) : data(d), next(nullptr) {}
};

std::atomic<Node*> head{nullptr};

void demonstrate_aba_problem() {
    std::cout << "=== ABA Problem Scenario ===" << std::endl;
    std::cout << R"(
Initial state: head → A → B → C

Thread 1:
  1. Read head (A)
  2. Read A->next (B)
  3. Gets interrupted...

Thread 2 (while Thread 1 is paused):
  4. Pop A (head = B)
  5. Pop B (head = C)
  6. Push A back (head = A)  ← Same pointer value!

Thread 1 resumes:
  7. CAS(head, A, B) succeeds! ← Thinks nothing changed
  8. head = B
  
PROBLEM: B was already freed! Use-after-free!

Result: head → B (freed!) → ???
)" << std::endl;
}

// Solution 1: Tagged Pointers (Version Counter)
struct TaggedPointer {
    Node* ptr;
    uintptr_t tag;
    
    TaggedPointer(Node* p = nullptr, uintptr_t t = 0) : ptr(p), tag(t) {}
};

class TaggedStack {
private:
    std::atomic<TaggedPointer> head;
    
    // Helper for atomic operations on tagged pointers
    static bool compare_exchange(std::atomic<TaggedPointer>& atomic_tp,
                                 TaggedPointer& expected,
                                 TaggedPointer desired) {
        // On x86-64, use CMPXCHG16B (128-bit CAS)
        // Simplified for demonstration
        TaggedPointer current = atomic_tp.load();
        if (current.ptr == expected.ptr && current.tag == expected.tag) {
            atomic_tp.store(desired);
            return true;
        }
        expected = current;
        return false;
    }
    
public:
    TaggedStack() {
        head.store(TaggedPointer(nullptr, 0));
    }
    
    void push(int value) {
        Node* new_node = new Node(value);
        TaggedPointer old_head = head.load();
        TaggedPointer new_head;
        
        do {
            new_node->next = old_head.ptr;
            new_head.ptr = new_node;
            new_head.tag = old_head.tag + 1;  // Increment version!
        } while (!compare_exchange(head, old_head, new_head));
    }
    
    bool pop(int& result) {
        TaggedPointer old_head = head.load();
        TaggedPointer new_head;
        
        while (old_head.ptr != nullptr) {
            new_head.ptr = old_head.ptr->next;
            new_head.tag = old_head.tag + 1;  // Increment version!
            
            if (compare_exchange(head, old_head, new_head)) {
                result = old_head.ptr->data;
                delete old_head.ptr;
                return true;
            }
        }
        return false;
    }
};

// Solution 2: Hazard Pointers
class HazardPointer {
private:
    static constexpr int MAX_THREADS = 128;
    std::atomic<Node*> hazard_ptrs[MAX_THREADS];
    
public:
    HazardPointer() {
        for (int i = 0; i < MAX_THREADS; i++) {
            hazard_ptrs[i].store(nullptr);
        }
    }
    
    void set_hazard(int thread_id, Node* ptr) {
        hazard_ptrs[thread_id].store(ptr);
    }
    
    void clear_hazard(int thread_id) {
        hazard_ptrs[thread_id].store(nullptr);
    }
    
    bool is_hazardous(Node* ptr) {
        for (int i = 0; i < MAX_THREADS; i++) {
            if (hazard_ptrs[i].load() == ptr) {
                return true;
            }
        }
        return false;
    }
};

class HazardPointerStack {
private:
    std::atomic<Node*> head;
    HazardPointer hp;
    std::vector<Node*> retired_nodes;
    
public:
    HazardPointerStack() : head(nullptr) {}
    
    void push(int value) {
        Node* new_node = new Node(value);
        Node* old_head = head.load();
        
        do {
            new_node->next = old_head;
        } while (!head.compare_exchange_weak(old_head, new_node));
    }
    
    bool pop(int& result, int thread_id) {
        Node* old_head;
        
        while (true) {
            old_head = head.load();
            if (old_head == nullptr) {
                return false;
            }
            
            // Mark as hazardous before accessing
            hp.set_hazard(thread_id, old_head);
            
            // Verify pointer is still valid
            if (old_head != head.load()) {
                continue;  // Changed, retry
            }
            
            Node* next = old_head->next;
            
            if (head.compare_exchange_weak(old_head, next)) {
                result = old_head->data;
                
                hp.clear_hazard(thread_id);
                
                // Can't delete immediately - might be hazardous to other threads
                retired_nodes.push_back(old_head);
                
                // Periodically scan and delete safe nodes
                scan_and_delete();
                
                return true;
            }
        }
    }
    
private:
    void scan_and_delete() {
        std::vector<Node*> still_retired;
        
        for (Node* node : retired_nodes) {
            if (!hp.is_hazardous(node)) {
                delete node;  // Safe to delete
            } else {
                still_retired.push_back(node);  // Keep for later
            }
        }
        
        retired_nodes = still_retired;
    }
};

void explain_solutions() {
    std::cout << "\n=== ABA Problem Solutions ===" << std::endl;
    
    std::cout << "\n1. TAGGED POINTERS (Version Counter):" << std::endl;
    std::cout << "   - Store pointer + version counter together" << std::endl;
    std::cout << "   - Increment version on each modification" << std::endl;
    std::cout << "   - Requires 128-bit CAS (CMPXCHG16B on x86-64)" << std::endl;
    std::cout << "   - Pro: Simple, efficient" << std::endl;
    std::cout << "   - Con: Version can wrap (rare but possible)" << std::endl;
    
    std::cout << "\n2. HAZARD POINTERS:" << std::endl;
    std::cout << "   - Mark pointers before accessing" << std::endl;
    std::cout << "   - Don't delete if any thread has it marked" << std::endl;
    std::cout << "   - Defer deletion until safe" << std::endl;
    std::cout << "   - Pro: No ABA problem, works on 32-bit" << std::endl;
    std::cout << "   - Con: More complex, overhead for scanning" << std::endl;
    
    std::cout << "\n3. EPOCH-BASED RECLAMATION:" << std::endl;
    std::cout << "   - Divide time into epochs" << std::endl;
    std::cout << "   - Threads announce current epoch" << std::endl;
    std::cout << "   - Delete only when all threads past that epoch" << std::endl;
    std::cout << "   - Pro: Lower overhead than hazard pointers" << std::endl;
    std::cout << "   - Con: Delayed reclamation" << std::endl;
    
    std::cout << "\n4. REFERENCE COUNTING:" << std::endl;
    std::cout << "   - Track how many threads reference each node" << std::endl;
    std::cout << "   - Delete when count reaches zero" << std::endl;
    std::cout << "   - Pro: Deterministic reclamation" << std::endl;
    std::cout << "   - Con: Atomic increment/decrement overhead" << std::endl;
}

int main() {
    demonstrate_aba_problem();
    explain_solutions();
    
    return 0;
}