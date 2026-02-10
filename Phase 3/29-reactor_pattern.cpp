// reactor_pattern.cpp
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <functional>
#include <map>
#include <vector>

#define MAX_EVENTS 64

class Reactor {
private:
    int epoll_fd;
    std::map<int, std::function<void()>> read_handlers;
    std::map<int, std::function<void()>> write_handlers;
    bool running;
    
public:
    Reactor() : running(false) {
        epoll_fd = epoll_create1(0);
        if (epoll_fd == -1) {
            throw std::runtime_error("epoll_create1 failed");
        }
    }
    
    ~Reactor() {
        close(epoll_fd);
    }
    
    void register_read_handler(int fd, std::function<void()> handler) {
        read_handlers[fd] = handler;
        update_epoll(fd);
    }
    
    void register_write_handler(int fd, std::function<void()> handler) {
        write_handlers[fd] = handler;
        update_epoll(fd);
    }
    
    void unregister(int fd) {
        read_handlers.erase(fd);
        write_handlers.erase(fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    }
    
    void run() {
        running = true;
        struct epoll_event events[MAX_EVENTS];
        
        std::cout << "Reactor: Event loop started" << std::endl;
        
        while (running) {
            int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
            
            for (int i = 0; i < nfds; i++) {
                int fd = events[i].data.fd;
                
                if (events[i].events & EPOLLIN) {
                    auto it = read_handlers.find(fd);
                    if (it != read_handlers.end()) {
                        it->second();  // Call handler
                    }
                }
                
                if (events[i].events & EPOLLOUT) {
                    auto it = write_handlers.find(fd);
                    if (it != write_handlers.end()) {
                        it->second();  // Call handler
                    }
                }
            }
        }
        
        std::cout << "Reactor: Event loop stopped" << std::endl;
    }
    
    void stop() {
        running = false;
    }
    
private:
    void update_epoll(int fd) {
        struct epoll_event ev;
        ev.data.fd = fd;
        ev.events = 0;
        
        if (read_handlers.count(fd)) {
            ev.events |= EPOLLIN;
        }
        if (write_handlers.count(fd)) {
            ev.events |= EPOLLOUT;
        }
        
        // Try to modify first
        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1) {
            // Doesn't exist, add it
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
        }
    }
};

void demonstrate_reactor_pattern() {
    std::cout << "=== Reactor Pattern ===" << std::endl;
    std::cout << R"(
Structure:
    ┌─────────────────────────┐
    │   Reactor (epoll)       │
    └───────────┬─────────────┘
                │
        ┌───────┴───────┐
        │               │
    ┌───▼───┐       ┌───▼───┐
    │Handler│       │Handler│  Register callbacks
    │  1    │       │  2    │  for events
    └───────┘       └───────┘

Flow:
  1. Register FD with callback
  2. Reactor waits for events
  3. Event occurs → Reactor calls callback
  4. Callback handles event
  5. Repeat

Example use:
  reactor.register_read_handler(socket_fd, []{
      // Handle incoming data
      char buf[1024];
      read(socket_fd, buf, sizeof(buf));
      process(buf);
  });
)" << std::endl;
}

int main() {
    demonstrate_reactor_pattern();
    
    std::cout << "\n=== Reactor Pattern Benefits ===" << std::endl;
    std::cout << "✓ Separation of concerns (I/O vs business logic)" << std::endl;
    std::cout << "✓ Easy to add new event sources" << std::endl;
    std::cout << "✓ Flexible callback system" << std::endl;
    std::cout << "✓ Foundation for async frameworks" << std::endl;
    
    std::cout << "\n=== Real-World Examples ===" << std::endl;
    std::cout << "- Node.js event loop" << std::endl;
    std::cout << "- Nginx HTTP server" << std::endl;
    std::cout << "- Redis database" << std::endl;
    std::cout << "- HAProxy load balancer" << std::endl;
    
    return 0;
}