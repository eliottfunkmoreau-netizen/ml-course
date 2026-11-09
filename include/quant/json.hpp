#pragma once
// Strict JSON: rejects duplicate keys, trailing content and excessive nesting.
#include <cmath>
#include <cstdio>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace quant {

struct Json {
    enum Kind { Null, Bool, Num, Str, Arr, Obj } kind = Null;
    bool b = false;
    double n = 0;
    std::string s;
    std::vector<Json> a;
    std::map<std::string, Json> o;

    const Json& at(const std::string& k) const {
        auto it = o.find(k);
        if (kind != Obj || it == o.end()) throw std::runtime_error("missing field: " + k);
        return it->second;
    }
    bool has(const std::string& k) const { return kind == Obj && o.count(k); }
    double num() const { if (kind != Num) throw std::runtime_error("expected number"); return n; }
    const std::string& str() const { if (kind != Str) throw std::runtime_error("expected string"); return s; }

    std::string dump() const {
        switch (kind) {
            case Null: return "null";
            case Bool: return b ? "true" : "false";
            case Num: {
                char buf[40];
                std::snprintf(buf, sizeof buf, "%.12g", n);
                return buf;
            }
            case Str: return quote(s);
            case Arr: {
                std::string r = "[";
                for (size_t i = 0; i < a.size(); ++i) r += (i ? "," : "") + a[i].dump();
                return r + "]";
            }
            case Obj: {
                std::string r = "{";
                bool first = true;
                for (auto& [k, v] : o) {
                    r += (first ? "" : ",") + quote(k) + ":" + v.dump();
                    first = false;
                }
                return r + "}";
            }
        }
        return "";
    }

    static std::string quote(const std::string& v) {
        std::string r = "\"";
        for (char c : v) {
            if (c == '"' || c == '\\') { r += '\\'; r += c; }
            else if (c == '\n') r += "\\n";
            else if (c == '\t') r += "\\t";
            else if (static_cast<unsigned char>(c) < 0x20) r += ' ';
            else r += c;
        }
        return r + "\"";
    }
};

class JsonParser {
public:
    explicit JsonParser(const std::string& t) : t_(t) {}
    Json parse() {
        Json v = value(0);
        ws();
        if (p_ != t_.size()) fail("trailing content");
        return v;
    }

private:
    const std::string& t_;
    size_t p_ = 0;
    [[noreturn]] void fail(const std::string& m) { throw std::runtime_error("json: " + m); }
    void ws() { while (p_ < t_.size() && (t_[p_]==' '||t_[p_]=='\n'||t_[p_]=='\t'||t_[p_]=='\r')) ++p_; }
    char peek() { ws(); if (p_ >= t_.size()) fail("eof"); return t_[p_]; }
    void lit(const char* w) { for (; *w; ++w, ++p_) if (p_ >= t_.size() || t_[p_] != *w) fail("bad literal"); }
    std::string str() {
        if (t_[p_] != '"') fail("expected string");
        ++p_;
        std::string r;
        while (true) {
            if (p_ >= t_.size()) fail("unterminated string");
            char c = t_[p_++];
            if (c == '"') return r;
            if (c == '\\') {
                if (p_ >= t_.size()) fail("bad escape");
                char e = t_[p_++];
                if (e == 'n') r += '\n'; else if (e == 't') r += '\t';
                else if (e == '"' || e == '\\' || e == '/') r += e;
                else fail("unsupported escape");
            } else r += c;
        }
    }
    Json value(int depth) {
        if (depth > 8) fail("too deep");
        Json v;
        char c = peek();
        if (c == '{') {
            v.kind = Json::Obj; ++p_;
            if (peek() == '}') { ++p_; return v; }
            while (true) {
                peek();
                std::string k = str();
                if (v.o.count(k)) fail("duplicate key " + k);
                if (peek() != ':') fail("expected :");
                ++p_;
                v.o[k] = value(depth + 1);
                char d = peek(); ++p_;
                if (d == '}') return v;
                if (d != ',') fail("expected ,");
            }
        }
        if (c == '[') {
            v.kind = Json::Arr; ++p_;
            if (peek() == ']') { ++p_; return v; }
            while (true) {
                v.a.push_back(value(depth + 1));
                char d = peek(); ++p_;
                if (d == ']') return v;
                if (d != ',') fail("expected ,");
            }
        }
        if (c == '"') { v.kind = Json::Str; v.s = str(); return v; }
        if (c == 't') { lit("true"); v.kind = Json::Bool; v.b = true; return v; }
        if (c == 'f') { lit("false"); v.kind = Json::Bool; return v; }
        if (c == 'n') { lit("null"); return v; }
        size_t used = 0;
        try { v.n = std::stod(t_.substr(p_, 32), &used); } catch (...) { fail("bad number"); }
        if (!std::isfinite(v.n)) fail("non-finite number");
        v.kind = Json::Num; p_ += used;
        return v;
    }
};

inline Json parse_json(const std::string& t) { return JsonParser(t).parse(); }

}  // namespace quant
