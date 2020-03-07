#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <vector>
#include <unordered_map>

#include "process.h"
#include "processor.h"
#include "linux_parser.h"


class System {
 public:
  Processor& Cpu();                   // TODO: See src/system.cpp
  std::vector<Process>& Processes();  // TODO: See src/system.cpp
  float MemoryUtilization();          // TODO: See src/system.cpp
  long UpTime();                      // TODO: See src/system.cpp
  int TotalProcesses();               // TODO: See src/system.cpp
  int RunningProcesses();             // TODO: See src/system.cpp
  std::string Kernel();               // TODO: See src/system.cpp
  std::string OperatingSystem();      // TODO: See src/system.cpp
  void parseProcessorData();
  void parseMemoryUtilization();

  // TODO: Define any necessary private members
 private:
  Processor cpu_ = {};
  std::vector<Process> processes_ = {};
  //store linux_parser
  std::unordered_map<std::string, int> meminfo_{};
  LinuxParser::procData proc_stat_;
  LinuxParser::procData old_proc_stat_;

};

#endif