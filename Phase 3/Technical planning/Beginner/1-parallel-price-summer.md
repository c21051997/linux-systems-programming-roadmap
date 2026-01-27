Goal: You have a massive array of 1,000,000 stock prices. 
You want to calculate the total sum using 4 threads simultaneously to save time.

The Problem: If all 4 threads try to add to the same total_sum variable at the same time, 
you get a Race Condition.

The Planning Goal: Decide how to partition the data and where to place the Mutex to ensure 
the result is correct without making the code slower than a single-threaded version.



1. Data Ownership & Sharing

Private Data: (What variables does only ONE thread touch?) each thread gets 1/4 of the array to handle, thus they need variables for defining the start and end index

Shared Data: (What variables do ALL threads touch?) the array of variables

2. Synchronization Strategy

Mechanism: (Mutex? Condition Variable? Atomic? Thread-Local?) nothing as the threads will be individually handling a separate part of the array and will not be updating the same variable, they instead will return the value of their block in the array

The "Critical Section": (Describe exactly which lines of logic need to be protected by a lock). none

3. Hardware/Performance Consideration

Contention Risk: (Will threads be waiting for each other? How do we minimize that?) yes threads may finish at different times, but becuase they are ll handling the same sized amount of data it will be mimimal, also there's no locks involved so no waiting there either

Cache Line Alignment: (If threads write to variables sitting right next to each other, will they cause "False Sharing"?) doesnt apply to this

4. The "Happy Path" Flow

(Step-by-step: Thread A does X, Thread B waits for Y, Thread A signals Z...) 
Thread A calculates sum of their quarter, returns the result or with a 'exit()' command same with the other threads, the result is then added at the end


Answer with help:
1. Data Ownership & Sharing

Private: local_sum (a variable on the thread's stack) and start_index/end_index.

Shared: The const double* prices array (Read-Only) and a struct Results { alignas(64) double value; } final_sums[4].

2. Synchronization Strategy

Mechanism: pthread_join. No mutexes are needed because we use "Join" as a synchronization barrier.

Critical Section: None. We avoid the critical section by giving each thread a unique memory location for its result that is padded to avoid false sharing.


3. Hardware/Performance Consideration

False Sharing: We use alignas(64) or padding between the result variables so each thread's write-buffer stays in its own L1 cache.

Thread Affinity: We should pin each thread to a specific physical core so the OS doesn't move them around during the calculation.

4. The "Happy Path" Flow

Main thread allocates prices and a padded results[4] array.

Main spawns 4 threads, passing each a pointer to their slice.

Each thread loops through its slice, storing the total in a local stack variable (fastest).

At the very last instruction, the thread writes its local total to results[i].

Main thread calls join() on all 4, then sums the 4 values in results.