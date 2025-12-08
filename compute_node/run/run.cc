// Author: Ming Zhang
// Copyright (c) 2023

#include "handler/handler.h"

// Entrance to run threads that spawn coroutines as coordinators to run distributed transactions
int main(int argc, char* argv[]) {
  if (argc != 5) {
    std::cerr << "./run <benchmark_name> <thread_num> <coroutine_num> <isolation_level>" << std::endl;
    return 0;
  }
system("sudo ip link set ens1f0 up");
system("sudo ifconfig ens1f0 10.10.1.1/24");
sleep(10);
  Handler* handler = new Handler();

  handler->ConfigureComputeNode(argc, argv);

  handler->GenThreads(std::string(argv[1]));

  handler->OutputResult(std::string(argv[1]), "Motor");
}
