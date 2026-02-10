/*
Globals: define max_events, func for setting non blockin

func main - check for errors when necessary EVERYWHERE:
- create socket, set sockopt to reuseaddr (opt 1)
- create sockaddr_in struct (INADDR_ANY)
- bind socket + address
- listen (SOMAXCONN)
- call set non blocking func

- create epoll notebook
- create epoll_event struct (EPOLLIN | EPOLLET for events)
- set epoll notebook to control (EPOLL_CTL_ADD) the event struct
- create epoll_event struct array with MAX_EVENTS
- create client count int, set to 0

while true loop:
- wait for the events
- loop through the events (indexes)
- check if event fd == server fd

if true:
- while true loop:
- accept connections
- check for no more connections (errno == EAGAIN or EWOULDBLOCK) + errors
- set client to non blocking
- add client to epoll (use old ev struct) using epoll control
- increment client count
- write welcome message to client

else:
- get client fd
- while true loop:
- read from client fd
- if read bytes == -1:
- check for no more data + break if so, if not then delete fd from epoll and close client fd + break
- else if read bytes == 0: (client disconnected)
- delete client fd from epoll, close fd, break
- else: echo data
- write buffer back to client
*/

// epoll_server.cpp
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>

#define MAX_EVENTS 64

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    // Create listening socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
    
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, SOMAXCONN);
    
    set_nonblocking(server_fd);
    
    // Create epoll instance
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        std::cerr << "epoll_create1 failed" << std::endl;
        return 1;
    }
    
    // Add server socket to epoll
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;  // Edge-triggered
    ev.data.fd = server_fd;
    
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        std::cerr << "epoll_ctl failed" << std::endl;
        return 1;
    }
    
    std::cout << "=== epoll() Server on port 8080 ===" << std::endl;
    std::cout << "Edge-triggered, non-blocking I/O\n" << std::endl;
    
    struct epoll_event events[MAX_EVENTS];
    int client_count = 0;
    
    while (true) {
        std::cout << "Waiting for events... (clients: " << client_count << ")" << std::endl;
        
        // Wait for events (only returns READY fds!)
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        
        std::cout << "Got " << nfds << " events" << std::endl;
        
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server_fd) {
                // New connection(s)
                while (true) {
                    int client_fd = accept(server_fd, nullptr, nullptr);
                    
                    if (client_fd == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // No more connections
                            break;
                        } else {
                            std::cerr << "accept error" << std::endl;
                            break;
                        }
                    }
                    
                    std::cout << "New client: fd " << client_fd << std::endl;
                    
                    set_nonblocking(client_fd);
                    
                    // Add client to epoll
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = client_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                    
                    client_count++;
                    
                    const char* welcome = "Welcome to epoll() server!\n";
                    write(client_fd, welcome, strlen(welcome));
                }
            }
            else {
                // Existing client has data
                int client_fd = events[i].data.fd;
                
                while (true) {
                    char buffer[1024];
                    ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
                    
                    if (n == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // No more data
                            break;
                        } else {
                            std::cerr << "read error" << std::endl;
                            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
                            close(client_fd);
                            client_count--;
                            break;
                        }
                    }
                    else if (n == 0) {
                        // Client disconnected
                        std::cout << "Client disconnected: fd " << client_fd << std::endl;
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
                        close(client_fd);
                        client_count--;
                        break;
                    }
                    else {
                        // Echo data
                        buffer[n] = '\0';
                        std::cout << "Received: " << buffer;
                        write(client_fd, buffer, n);
                    }
                }
            }
        }
    }
    
    close(epoll_fd);
    close(server_fd);
    return 0;
}