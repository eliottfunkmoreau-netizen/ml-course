#pragma once
// Append-only, hash-chained event log. Records are tab-separated:
//   seq  prev_hash  type  payload_json  hash
// Editing, removing or reordering any record makes verification fail closed.
#include <fcntl.h>
#include <unistd.h>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "sha256.hpp"

namespace quant {

struct LedgerEvent { long seq; std::string prev, type, payload, hash; };

inline std::string read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open " + path);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

class Ledger {
public:
    explicit Ledger(std::string path) : path_(std::move(path)) { load(); }

    const std::vector<LedgerEvent>& events() const { return events_; }

    const LedgerEvent* last_of(const std::string& type) const {
        for (auto it = events_.rbegin(); it != events_.rend(); ++it) if (it->type == type) return &*it;
        return nullptr;
    }

    void append(const std::string& type, const std::string& payload) {
        if (type.find('\t') != std::string::npos || payload.find('\t') != std::string::npos ||
            payload.find('\n') != std::string::npos)
            throw std::runtime_error("ledger: control characters in record");
        LedgerEvent e{static_cast<long>(events_.size()), events_.empty() ? genesis() : events_.back().hash, type, payload, ""};
        e.hash = digest(e);
        const std::string line = std::to_string(e.seq) + "\t" + e.prev + "\t" + e.type + "\t" + e.payload + "\t" + e.hash + "\n";
        int fd = ::open(path_.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd < 0) throw std::runtime_error("ledger: cannot open for append");
        bool ok = ::write(fd, line.data(), line.size()) == static_cast<ssize_t>(line.size()) && ::fsync(fd) == 0;
        ::close(fd);
        if (!ok) throw std::runtime_error("ledger: durable write failed");
        events_.push_back(std::move(e));
    }

private:
    static std::string genesis() { return std::string(64, '0'); }
    static std::string digest(const LedgerEvent& e) {
        return sha256(std::to_string(e.seq) + "|" + e.prev + "|" + e.type + "|" + e.payload);
    }

    void load() {
        std::ifstream f(path_);
        if (!f) return;
        std::string line, prev = genesis();
        long expect = 0;
        while (std::getline(f, line)) {
            std::vector<std::string> p;
            std::stringstream ss(line);
            std::string cell;
            while (std::getline(ss, cell, '\t')) p.push_back(cell);
            if (p.size() != 5) throw std::runtime_error("ledger: malformed record " + std::to_string(expect));
            LedgerEvent e{std::stol(p[0]), p[1], p[2], p[3], p[4]};
            if (e.seq != expect || e.prev != prev || digest(e) != e.hash)
                throw std::runtime_error("ledger: chain broken at record " + std::to_string(expect));
            prev = e.hash;
            ++expect;
            events_.push_back(std::move(e));
        }
    }

    std::string path_;
    std::vector<LedgerEvent> events_;
};

// Atomically claims a one-use capability: durable before any protected data is read.
inline bool claim_receipt(const std::string& path, const std::string& body) {
    int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0444);
    if (fd < 0) return false;
    bool ok = ::write(fd, body.data(), body.size()) == static_cast<ssize_t>(body.size()) && ::fsync(fd) == 0;
    ::close(fd);
    if (!ok) throw std::runtime_error("receipt: durable write failed");
    return true;
}

}  // namespace quant
