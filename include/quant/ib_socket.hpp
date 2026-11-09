#pragma once
// Localhost-only socket helpers shared by the read-only IBKR tools.
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdexcept>
#include <string>

#include "ib_wire.hpp"

namespace quant::ib {

inline int connect_local(int port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throw std::runtime_error("socket failed");
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_port = htons(static_cast<uint16_t>(port));
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (::connect(fd, reinterpret_cast<sockaddr*>(&a), sizeof a) != 0)
        throw std::runtime_error("cannot reach Gateway on localhost:" + std::to_string(port));
    int one = 1;
    ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
    return fd;
}

inline void send_all(int fd, const std::string& m) {
    size_t off = 0;
    while (off < m.size()) {
        ssize_t n = ::send(fd, m.data() + off, m.size() - off, 0);
        if (n <= 0) throw std::runtime_error("send failed");
        off += static_cast<size_t>(n);
    }
}

// Completes the version handshake and starts the API session.
inline int open_session(int fd, Framer& framer, int client_id) {
    send_all(fd, handshake());
    char buf[65536];
    std::string body;
    while (true) {
        ssize_t n = ::recv(fd, buf, sizeof buf, 0);
        if (n <= 0) throw std::runtime_error("Gateway closed the connection during handshake");
        framer.feed(buf, static_cast<size_t>(n));
        if (framer.next(body)) break;
    }
    const int server = to_int(split_fields(body).at(0));
    if (server != kServerVersion) throw std::runtime_error("unexpected server version " + std::to_string(server));
    send_all(fd, start_api(client_id));
    return server;
}

}  // namespace quant::ib
