Goal: You have a Main thread and a Worker thread. The Worker must wait for the Main thread to finish loading a 1GB configuration file into memory before it starts its work.

The Constraint: You cannot use sleep(), and you cannot use a while(loading == true) loop (busy-waiting).


1. Data Ownership & Sharing
- Private Data: 
- Shared Data: mutex, condition variable, bool loading done variable

2. Synchronization Strategy

Mechanism: (How do you put the worker to sleep and wake it up?) use a condition variable and use the main thread to signal the worker thread when it is finished, this seems like using broadcast would be more suitable, however due to there only being one worker thread, just a regular signal is fine.

The "Critical Section": > 
3. Hardware/Performance Consideration

Efficiency: Why is a Condition Variable better than a while loop for the CPU? becuase the condition variable puts the thread to sleep, whereas the while loop keeps the thread running and wastes cpu cycles 

4. The "Happy Path" Flow
(Step-by-step: Main thread does X... Worker thread does Y...)
main thread creates the worker thread, then loads the config file, the worker thread is waiting for the main thread to finish loading the config file (wrapped in a while loop to handle a spurious wake up), the main thread then signals that it is finished to the worker thread and sets loading_done variable to true to ensure the worker doesnt wait after its already finished loading, the worker thread performs its operations, the main thread waits for the worker thread to be finished
