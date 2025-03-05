#pragma once
#include <cstdio>
#include <iostream>
#include <string>

// code == false is error
struct ResStat {
  bool code;
  std::string msg;

  bool isFailure() const {
    if (code == false) {
      std::cout << msg << "\n";
    }
    return !code;
  }
};
