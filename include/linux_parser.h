#ifndef SYSTEM_PARSER_H
#define SYSTEM_PARSER_H

#include <fstream>
#include <regex>
#include <string>
#include <set>
#include <unordered_map>
#include <vector>


// CPU field list names
#define CPU_USER 0
#define CPU_NICE 1
#define CPU_SYSTEM 2
#define CPU_IDLE 3
#define CPU_IOWAIT 4
#define CPU_IRQ 5
#define CPU_SOFTIRQ 6
#define CPU_STEAL 7
#define CPU_GUEST 8
#define CPU_GUEST_NICE 9


namespace LinuxParser {
// Paths
const std::string kProcDirectory{"/proc/"};
const std::string kCmdlineFilename{"/cmdline"};
const std::string kCpuinfoFilename{"/cpuinfo"};
const std::string kStatusFilename{"/status"};
const std::string kStatFilename{"/stat"};
const std::string kUptimeFilename{"/uptime"};
const std::string kMeminfoFilename{"/meminfo"};
const std::string kVersionFilename{"/version"};
const std::string kOSPath{"/etc/os-release"};
const std::string kPasswordPath{"/etc/passwd"};

//Struct to store parsed data from proc_stat
struct procData {
  int tot_proc;
  int run_proc;
  std::unordered_map<int, std::vector<int>> cpu;
};

//store cpu usage struct
struct usage {
  long total;
  long idle;
};

struct processStat {
  std::string name;
  long utime;
  long stime;
  long cutime;
  long cstime;
  long starttime;
};


// System
std::unordered_map<std::string, int> MemoryUtilization();
long UpTime();
std::vector<int> Pids();
procData Processor();
std::string OperatingSystem();
std::string Kernel();

usage CpuUsage(procData& data);
std::vector<usage> ProcessorUsage(procData& data);



// CPU
enum CPUStates {
  kUser_ = 0,
  kNice_,
  kSystem_,
  kIdle_,
  kIOwait_,
  kIRQ_,
  kSoftIRQ_,
  kSteal_,
  kGuest_,
  kGuestNice_
};

//set of key names for parsing memory utilization
const std::set<std::string> memSet {"MemTotal","MemFree",
                              "MemAvailable", "Buffers",
                              "Cached", "SwapTotal",
                              "SwapFree", "Shmem",
                              "SReclaimable"};

// Processes
processStat Process(int pid);
std::string Command(int pid);
std::string Ram(int pid);
std::string Uid(int pid);
std::string User(int pid);
};  // namespace LinuxParser

#endif //SYSTEM_PARSER_H