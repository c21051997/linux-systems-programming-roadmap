/*
Globals: define max events (1024) and buffer size 4096
- Client struct: fd, vector char read & write buffers
- map of int, Client* clients

set_nonblocking func

handle_new_connection func: server_fd and epoll_fd as inputs
- while true loop
- accept connection + store client address
- if client fd == -1:
    - Check if non blocking + break, if not then error + break
- set client fd to non blocking
- add to epoll
- add to client map

handle_client_read: Client* client and epoll_fd as parameters
- while true loop
- read from client
- if bytes read == -1: non blocking check + error check
- else if bytes read == 0: delete client from epoll, map, close client, delete memory
- 'else' (not needed in an else block) add to write buffer
- outside while loop:
- if we have data to write (write buffer != empty), modify for EPOLLOUT

handle_client_write: Client* client and epoll_fd as para
- while write buffer not empty:
- write the data (.data())
- check if bytes wrote == -1, non blocking + error
- .erase written data up to bytes wrote
- outside while loop:
- if write buffer empty, then stop monitoring EPOLLOUT
*/


// high_performance_server.cpp
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <map>
#include <vector>

#define MAX_EVENTS 1024
#define BUFFER_SIZE 4096

struct Client {
    int fd;
    std::vector<char> read_buffer;
    std::vector<char> write_buffer;
};

std::map<int, Client*> clients;

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void handle_new_connection(int server_fd, int epoll_fd) {
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;  // No more connections
            } else {
                std::cerr << "accept error" << std::endl;
                break;
            }
        }
        
        set_nonblocking(client_fd);
        
        // Add to epoll
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;  // Edge-triggered
        ev.data.fd = client_fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
        
        // Track client
        Client* client = new Client;
        client->fd = client_fd;
        clients[client_fd] = client;
        
        std::cout << "New client: fd " << client_fd 
                  << " (total: " << clients.size() << ")" << std::endl;
    }
}

void handle_client_read(Client* client, int epoll_fd) {
    while (true) {
        char buffer[BUFFER_SIZE];
        ssize_t n = read(client->fd, buffer, sizeof(buffer));
        
        if (n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No more data
                break;
            } else {
                std::cerr << "read error" << std::endl;
                return;
            }
        }
        else if (n == 0) {
            // Client disconnected
            std::cout << "Client disconnected: fd " << client->fd << std::endl;
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client->fd, nullptr);
            close(client->fd);
            clients.erase(client->fd);
            delete client;
            return;
        }
        
        // Echo: add to write buffer
        client->write_buffer.insert(client->write_buffer.end(), buffer, buffer + n);
    }
    
    // If we have data to write, register for EPOLLOUT
    if (!client->write_buffer.empty()) {
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
        ev.data.fd = client->fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client->fd, &ev);
    }
}

void handle_client_write(Client* client, int epoll_fd) {
    while (!client->write_buffer.empty()) {
        ssize_t n = write(client->fd, client->write_buffer.data(), 
                         client->write_buffer.size());
        
        if (n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Socket buffer full, try later
                break;
            } else {
                std::cerr << "write error" << std::endl;
                return;
            }
        }
        
        // Remove written data
        client->write_buffer.erase(client->write_buffer.begin(), 
                                   client->write_buffer.begin() + n);
    }
    
    // If write buffer empty, stop monitoring EPOLLOUT
    if (client->write_buffer.empty()) {
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = client->fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client->fd, &ev);
    }
}

int main() {
    // Create server socket
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
    
    // Create epoll
    int epoll_fd = epoll_create1(0);
    
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);
    
    std::cout << "=== High-Performance Echo Server ===" << std::endl;
    std::cout << "Listening on port 8080" << std::endl;
    std::cout << "Edge-triggered, non-blocking I/O\n" << std::endl;
    
    struct epoll_event events[MAX_EVENTS];
    
    while (true) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server_fd) {
                // New connections
                handle_new_connection(server_fd, epoll_fd);
            }
            else {
                // Existing client
                auto it = clients.find(events[i].data.fd);
                if (it == clients.end()) continue;
                
                Client* client = it->second;
                
                if (events[i].events & EPOLLIN) {
                    handle_client_read(client, epoll_fd);
                }
                
                if (events[i].events & EPOLLOUT) {
                    handle_client_write(client, epoll_fd);
                }
                
                if (events[i].events & (EPOLLERR | EPOLLHUP)) {
                    std::cout << "Error on client: fd " << client->fd << std::endl;
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client->fd, nullptr);
                    close(client->fd);
                    clients.erase(client->fd);
                    delete client;
                }
            }
        }
    }
    
    return 0;
}