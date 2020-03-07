#include "processor.h"


//compose from System the current and previous CPU utilization
void Processor::getUsage(LinuxParser::procData& cpu, LinuxParser::procData& prevCpu){
    this->usage_ = LinuxParser::CpuUsage(cpu);
    this->prevUsage_ = LinuxParser::CpuUsage(prevCpu);
}

// Return the aggregate CPU utilization
float Processor::Utilization() {
    long totald = this->usage_.total - this->prevUsage_.total;
    long idled = this->usage_.idle - this->prevUsage_.idle;

    return ((float)(totald-idled)/totald); 
}