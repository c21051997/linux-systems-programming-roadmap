/*
Define: queue_depth 32, buffer size 4096


func read_file_with_io_uring: const char* filename:
- init io_uring + check for errors
- open file using open(), readonly, error check and exit ring
- init iovec
- init io_uring_sqe + error check
- prepare read operation
- attached user data
- submit operation

- init cqe + wait command
- if ret ^ < 0; error
- if not: if res of cqe < 0: error, if not then output no. of bytes + data;
    - mark as seen
- close fd, delete buffer, exit ring return 0
*/

// io_uring_demo.cpp
#include <iostream>
#include <liburing.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

#define QUEUE_DEPTH 32
#define BUFFER_SIZE 4096

void demonstrate_io_uring_concepts() {
    std::cout << "=== io_uring Architecture ===" << std::endl;
    std::cout << R"(
Traditional:
  User Space          Kernel Space
      │                   │
      ├─ syscall() ──────►│ (context switch)
      │◄─────────────── result
      │                   │
  Every operation = 2 context switches!

io_uring:
  User Space          Kernel Space
      │                   │
   ┌──▼───┐          ┌───▼───┐
   │ SQ   │─────────►│ Polls │
   │(Submit│          │  SQ   │
   │Queue) │          └───┬───┘
   └──────┘              │
                         │ Process
   ┌──────┐              │
   │ CQ   │◄─────────────┘
   │(Complete│
   │Queue) │
   └──────┘
   
  Batch operations, fewer syscalls!
  Can even run without ANY syscalls (polling mode)
)" << std::endl;
}

// Simple io_uring file read example
int read_file_with_io_uring(const char* filename) {
    struct io_uring ring;
    
    // Initialize io_uring
    if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
        std::cerr << "io_uring_queue_init failed" << std::endl;
        return -1;
    }
    
    std::cout << "io_uring initialized with queue depth " << QUEUE_DEPTH << std::endl;
    
    // Open file
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        std::cerr << "open failed" << std::endl;
        io_uring_queue_exit(&ring);
        return -1;
    }
    
    // Allocate buffer
    char* buffer = new char[BUFFER_SIZE];
    struct iovec iov;
    iov.iov_base = buffer;
    iov.iov_len = BUFFER_SIZE;
    
    // Get submission queue entry
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    if (!sqe) {
        std::cerr << "io_uring_get_sqe failed" << std::endl;
        close(fd);
        delete[] buffer;
        io_uring_queue_exit(&ring);
        return -1;
    }
    
    // Prepare read operation
    io_uring_prep_readv(sqe, fd, &iov, 1, 0);  // Read from offset 0
    io_uring_sqe_set_data(sqe, buffer);  // Attach user data
    
    // Submit operation
    io_uring_submit(&ring);
    std::cout << "Read operation submitted" << std::endl;
    
    // Wait for completion
    struct io_uring_cqe* cqe;
    int ret = io_uring_wait_cqe(&ring, &cqe);
    
    if (ret < 0) {
        std::cerr << "io_uring_wait_cqe failed" << std::endl;
    } else {
        if (cqe->res < 0) {
            std::cerr << "Read failed: " << strerror(-cqe->res) << std::endl;
        } else {
            std::cout << "Read " << cqe->res << " bytes" << std::endl;
            std::cout << "Content: " << std::string(buffer, std::min(100, cqe->res)) 
                      << "..." << std::endl;
        }
        
        io_uring_cqe_seen(&ring, cqe);
    }
    
    // Cleanup
    close(fd);
    delete[] buffer;
    io_uring_queue_exit(&ring);
    
    return 0;
}

int main() {
    demonstrate_io_uring_concepts();
    
    std::cout << "\n=== io_uring File Read Example ===" << std::endl;
    
    // Create test file
    const char* test_file = "io_uring_test.txt";
    int fd = open(test_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    const char* content = "Hello from io_uring! This is asynchronous I/O at its finest.";
    write(fd, content, strlen(content));
    close(fd);
    
    read_file_with_io_uring(test_file);
    
    std::cout << "\n=== io_uring Benefits ===" << std::endl;
    std::cout << "✓ Fewer system calls (batch operations)" << std::endl;
    std::cout << "✓ Zero-copy possible" << std::endl;
    std::cout << "✓ Polling mode (no syscalls at all!)" << std::endl;
    std::cout << "✓ Unified interface (files, sockets, etc.)" << std::endl;
    std::cout << "✓ Better performance than epoll" << std::endl;
    
    unlink(test_file);
    
    return 0;
}