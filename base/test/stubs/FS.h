#pragma once
class File {
public:
  bool operator!() const { return true; }
  void close() {}
};
