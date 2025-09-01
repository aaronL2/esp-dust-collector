#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <cstdio>
#include <cstdarg>

class String : public std::string {
public:
  using std::string::string;
  String(const std::string& other) : std::string(other) {}
  bool isEmpty() const { return empty(); }
};

class HardwareSerial {
public:
  void printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
  }
  void println(const char* s="") { printf("%s\n", s); }
};

extern HardwareSerial Serial;

using uint8_t = std::uint8_t;
using size_t = std::size_t;

unsigned long millis();
void digitalWrite(int, int);

constexpr int HIGH = 1;
constexpr int LOW = 0;
