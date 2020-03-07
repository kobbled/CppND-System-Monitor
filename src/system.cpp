#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>

#include "process.h"
#include "processor.h"
#include "system.h"

#include "linux_parser.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

// TODO: Return the system's CPU
Processor& System::Cpu() { return cpu_; }

// TODO: Return a container composed of the system's processes
vector<Process>& System::Processes() { return processes_; }

// Return the system's kernel identifier (string)
std::string System::Kernel() { return LinuxParser::Kernel(); }

// get system's memory utilization
void System::parseMemoryUtilization(){
    meminfo_ = LinuxParser::MemoryUtilization();
}

// calculate memory utilization
float System::MemoryUtilization() {
    float freeMem = meminfo_["MemFree"]/(1024*1024);
    float totMem = meminfo_["MemTotal"]/(1024*1024);
    return (totMem-freeMem)/totMem;
}

// Return the operating system name
std::string System::OperatingSystem() { return LinuxParser::OperatingSystem(); }

//get processor utilization and stash previous utilization
//for comparison
void System::parseProcessorData(){
    old_proc_stat_ = proc_stat_;
    proc_stat_ = LinuxParser::Processes();
}

// Return the number of processes actively running on the system
int System::RunningProcesses() { return proc_stat_.run_proc; }

// Return the total number of processes on the system
int System::TotalProcesses() { return proc_stat_.tot_proc; }

// TODO: Return the number of seconds since the system started running
long int System::UpTime() { return 0; }