/*
Goal: build one process, one thread that:
> accepts many TCP connections
> reads HTTP requests
> serves files
> never blocks
> scales to 100+ clients

globals: server_fd, epoll_fd, MAX EVENTS, map<int, Connection> connections

connection struct: 
> int fd 
> string read_buffer (accumulates bytes from read())
> bool request_complete (true when \r\n\r\n seen)
> string method, string path
> string write_buffer, size_t write_offset
> enum state: reading, writing, closed


funcs:
setup_server_socket() - create socket, set SO_REUSUE ADDR, bind, listen, set non blocking, returns server_fd

add_fd_to_epoll() - add / modify fd

accept_new_connection - called when epoll signals server_fd readable
> loop accept until eagain
> set client socket non blocking
> create connection
> add client fd to epoll (EPOLLIN)

handle_client_read(Connection&) - called when epoll signals EPOLLIN
> read until EAGAIN
> append to read buffer
> detect \r\n\r\n
> if request compelte: parse HTTP request, prepare response, switch state to writing, update epoll to EPOLLOUT

parse_http_request(Connection&): extract GET method and path, normalise path

build_http_response(Connection&):
> open requested file
> if exists: 200 ok
> else: 404 not found
> build headers
> load file contents
> fill write buffer

handle_client_write(connection&): called when epoll signals EPOLLOUT
> write remaining bytes
>update write_offset
if fully written: close connection

close_connection(connection&): remove fd from epoll, close socket, remove from connection loop



Success criteria:
✅ curl localhost:8080/index.html works
✅ Multiple terminals can connect at once
✅ ab -c 100 doesn’t hang
✅ No threads
✅ No blocking calls
*/

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

#define MAX_EVENTS 100
#define BUFFER_SIZE 4096

enum State {
    READING,
    WRITING,
    CLOSED
};

struct Connection {
    int fd;
    std::string read_buffer;
    bool request_complete;
    std::string method;
    std::string path;
    std::string write_buffer;
    size_t write_offset;
    State state;
};

std::map<int, Connection*> connections;

void set_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int setup_server_socket() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, SOMAXCONN);

    set_non_blocking(server_fd);

    return server_fd;
}

void add_fd_to_epoll(int fd, int epoll_fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_ctl add failed")
        close(fd);
    }
}

void accept_new_connections(int server_fd, int epoll_fd) {
    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);

        if (client_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                perror("accept failed");
                break;
            }
        } 

        set_non_blocking(client_fd);
        Connection* conn = new Connection;
        conn->fd = client_fd;
        conn->request_complete = false;
        conn->state = State::READING;
        conn->write_offset = 0;
        
        add_fd_to_epoll(client_fd, epoll_fd, EPOLLIN | EPOLLET);

        connections[client_fd] = conn;
    }
}

void handle_client_read(Connection* conn, int epoll_fd) {
    while (true) {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(conn->fd, buffer, sizeof(buffer));

        if (bytes_read == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                perror("Read failed");
                close_connection(conn, epoll_fd);
                return;
            }
        } else if (bytes_read == 0) {
            close_connection(conn, epoll_fd);
            return;
        }

        conn->read_buffer.append(buffer, bytes_read);
    }
    
    // Now that we have data, parse the HTTP request if not done yet
    if (!conn->request_complete) {
        parse_http_request(conn); // Fill method, path, and mark request_complete
    }

    // If request is complete, prepare response and switch to writing
    if (conn->request_complete && conn->state == READING) {
        prepare_http_response(conn); // Build write_buffer

        // Update epoll to listen for write events
        struct epoll_event ev;
        ev.events = EPOLLOUT | EPOLLET;
        ev.data.fd = conn->fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, conn->fd, &ev);
    }
}

void handle_client_write(Connection* conn, int epoll_fd) {
    while (!conn->write_buffer.empty()) {

        ssize_t bytes_written = write(conn->fd,
                                      conn->write_buffer.data() + conn->write_offset, 
                                      conn->write_buffer.size() - conn->write_offset);
        
        if (bytes_written == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            } else {
                perror("write failed");
                break;
            }
        } 

        conn->write_offset += bytes_written;
    }

    if (conn->write_offset == conn->write_buffer.size()) {
        conn->write_buffer.clear();
        conn->write_offset = 0;
        conn->state = State::READING;

        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = conn->fd;

        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, conn->fd, &ev);
    }
}

void parse_http_request(Connection* conn) {
    // Check if request is complete (look for double CRLF)
    size_t pos = conn->read_buffer.find("\r\n\r\n");
    if (pos == std::string::npos) return; // not complete yet

    // Extract the first line: "GET /path HTTP/1.1"
    std::string request_line = conn->read_buffer.substr(0, conn->read_buffer.find("\r\n"));
    std::istringstream iss(request_line);
    iss >> conn->method >> conn->path;

    // Default to /index.html if root is requested
    if (conn->path == "/") conn->path = "/index.html";

    // Mark request as complete
    conn->request_complete = true;
}

void prepare_http_response(Connection* conn, const std::string& root_dir = "./www") {
    std::string file_path = root_dir + conn->path;
    std::ifstream file(file_path, std::ios::binary);

    std::string body;
    std::string status;

    if (file) {
        // File exists
        status = "200 OK";
        std::ostringstream oss;
        oss << file.rdbuf();
        body = oss.str();
    } else {
        // File not found
        status = "404 Not Found";
        body = "<html><body><h1>404 Not Found</h1></body></html>";
    }

    // Build HTTP response headers
    std::ostringstream response;
    response << "HTTP/1.1 " << status << "\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Content-Type: text/html\r\n"
             << "Connection: close\r\n"
             << "\r\n"
             << body;

    conn->write_buffer = response.str();
    conn->write_offset = 0;
    conn->state = WRITING;
}


void close_connection(Connection* conn, int epoll_fd) {
    if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn->fd, nullptr) == -1) {
        perror("epoll del failed")
    }

    close(conn->fd);
    connections.erase(conn);
    delete conn;
}