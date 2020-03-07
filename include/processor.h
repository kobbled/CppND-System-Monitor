#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <vector>
#include "linux_parser.h"

class Processor {
 public:
  float Utilization();  // TODO: See src/processor.cpp
  void getUsage(LinuxParser::procData& cpu, LinuxParser::procData& prevCpu);

  // TODO: Declare any necessary private members
 private:
    LinuxParser::usage usage_;
    LinuxParser::usage prevUsage_;
};

#endif