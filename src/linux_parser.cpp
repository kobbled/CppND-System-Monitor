#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <thread>
#include <experimental/filesystem>

#include "linux_parser.h"

namespace fs = std::experimental::filesystem;

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

//find all pids of processes
vector<int> LinuxParser::Pids() {
  vector<int> pids;

  //taken from https://stackoverflow.com/questions/4654636/how-to-determine-if-a-string-is-a-number-with-c
  auto is_number = [](const std::string& s){
      std::string::const_iterator it = s.begin();
      while (it != s.end() && std::isdigit(*it)) ++it;
      return !s.empty() && it == s.end();
  };

  //(ref) https://www.bfilipek.com/2019/04/dir-iterate.html
  for (const auto& entry : fs::directory_iterator(kProcDirectory.c_str())) {
    const auto filename = entry.path().filename().string();
    if (fs::is_directory(entry.status())) {
      if (is_number(filename)) {
        int pid = std::stoi(filename);
        pids.push_back(pid);
      }
    }
  }

  return pids;
}

// Read and return the system memory utilization
std::unordered_map<std::string, int> LinuxParser::MemoryUtilization() {
  string line;
  //regex out the key value pairs
  std::smatch matches;
  //regex for parsing /proc/meminfo
  std::regex reg("((\\w+):\\s*(.+?)\\s)");
  //start stream for /proc/meminfo file
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  //store parsed file in a dictionary
  std::unordered_map<std::string, int> meminfo;
  //we do not need to store all of the members, and regex
  //will error out if full file is read.
  if (stream.is_open()) {
    for(int i = 0; i < 32; i++) {
      std::getline(stream, line);
      while(std::regex_search(line, matches, reg)){
        if(memSet.find(matches[2].str()) != memSet.end()){
          //store in dictionary for memory utilization 
           meminfo[matches[2].str()] = std::stoi(matches[3].str());
        }
        line = matches.suffix().str();
      }
    }
    stream.close();
  }

  //check if dictionary is the same size as memSet, if not throw
  //error
  if (meminfo.size() != memSet.size()){
    throw std::length_error("MemoryUtilization() did not find all"
                            " the specified keys. stream file might"
                            " be wrong.");
  }

  return meminfo;

}

// Read and return the system uptime
long LinuxParser::UpTime() {
  string suptime, sidle;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> suptime >> sidle;
  }

  return std::stol(suptime); 
}

// Read /proc/stat and return total processes, running processes, and cpu utilization
LinuxParser::procData LinuxParser::Processor() {
  string line;
  //start stream for /proc/stat file
  std::ifstream stream(kProcDirectory + kStatFilename);
  //regex for total processes
  std::regex re("((?:^|\\W)(?:processes)(?:$|\\W)(?:\\s*)(\\d+$))");
  //regex for running processes
  std::regex re2("((?:^|\\W)(?:procs_running)(?:$|\\W)(?:\\s*)(\\d+$))");
  //regex for cpu
  std::regex re3("(?:cpu)([\\d]?)\\s*(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)\\s(\\d+)");

  //create structure to store parsed data
  LinuxParser::procData proc_stat;

  if (stream.is_open()) {
    while (std::getline(stream, line)) {

      //total processes
      std::smatch match;
      if (std::regex_search(line, match, re) && match.size() > 1) {
          proc_stat.tot_proc = std::stoi(match[2].str());
      }
      //running processes
      std::smatch match2;
      if (std::regex_search(line, match2, re2) && match2.size() > 1) {
          proc_stat.run_proc = std::stoi(match2[2].str());
      }
      //cpu
      std::smatch match3;
      if (std::regex_search(line, match3, re3) && match3.size() > 1) {
          //cpu
          if(match3[1].str().empty()){
            vector<int> v;
            for(int i = 2; i < 12; i++){
              v.push_back(std::stoi(match3[i].str()));
            }

            // store aggegated cpu with key '-1'
            proc_stat.cpu[-1] = v;
          //processor
          } else {
            vector<int> v;
            for(int i = 2; i < 12; i++){
              v.push_back(std::stoi(match3[i].str()));
            }

            proc_stat.cpu[std::stoi(match3[1].str())] = v;
          }
      }

    }
  }

  return proc_stat;

}


/**
 * Calculate total and idle usage of aggregate processor
 */
LinuxParser::usage LinuxParser::CpuUsage(LinuxParser::procData& data) {
    auto cpu = data.cpu;
    LinuxParser::usage usage = {};

    //if on fist cycle handle previous cpu usage
    if (!data.cpu.empty()){
      //(ref) https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
      long idle = cpu[-1][CPU_IDLE] + cpu[-1][CPU_IOWAIT];
      long nonidle = cpu[-1][CPU_USER] + cpu[-1][CPU_NICE] + 
                  cpu[-1][CPU_SYSTEM] + cpu[-1][CPU_IRQ] + 
                  cpu[-1][CPU_SOFTIRQ] + cpu[-1][CPU_STEAL];
      
      usage.total = idle + nonidle;
      usage.idle = idle;
    } else {
      usage.total = 0;
      usage.idle = 0;
    }

    return(usage);
}

/**
 * Calculate total and idle usage of each processor
 */
vector<LinuxParser::usage> LinuxParser::ProcessorUsage(LinuxParser::procData& data) {

    auto cpu = data.cpu;
    //if on fist cycle handle previous cpu usage
    vector<LinuxParser::usage> processor_usage;
    if (!data.cpu.empty()){
      for ( auto it = cpu.begin(); it != cpu.end(); ++it ) {
          if(it->first != -1){

              LinuxParser::usage usage = {};
              //(ref) https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
              long idle = it->second[CPU_IDLE] + it->second[CPU_IOWAIT];
              long nonidle = it->second[CPU_USER] + it->second[CPU_NICE] + 
                      it->second[CPU_SYSTEM] + it->second[CPU_IRQ] + 
                      it->second[CPU_SOFTIRQ] + it->second[CPU_STEAL];
              usage.total = idle + nonidle;
              usage.idle = idle;

              processor_usage.emplace_back(usage);
          }
      }
    } else {
      //get total number of cpus
      unsigned numCpu = std::thread::hardware_concurrency();

      for(unsigned i=0; i < numCpu; i++){
        LinuxParser::usage usage = {};
        usage.total = 0;
        usage.idle = 0;

        processor_usage.emplace_back(usage);
      }

    }

    return processor_usage;

}


LinuxParser::processStat LinuxParser::Process(int pid){
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);

  //struct to store data
  LinuxParser::processStat stat;

  string line;
  //regex
  std::smatch match;
  std::regex re("(?:\\(([^)]+)\\)\\s*[A-Z]\\s*)(?:\\-?\\d+\\s*){10}(\\d+)\\s*(\\d+)\\s*(\\d+)\\s*(\\d+)\\s*(?:\\-?\\d+\\s*){4}(\\d+)");

  if (stream.is_open()) {
    string fl;
    while (std::getline(stream, line)) {
        fl += line;
    }
    stream.close();

    //got match
    if (std::regex_search(fl, match, re) && match.size() > 1) {
      stat.name = match[1].str();
      stat.utime = std::stoi(match[2].str());
      stat.stime = std::stoi(match[3].str());
      stat.cutime = std::stoi(match[4].str());
      stat.cstime = std::stoi(match[5].str());
      stat.starttime = std::stoi(match[6].str());
    }

  }

  return stat;
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  // /proc/[pid]/stat  
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  string line;
  string fl;
  string uid;

  std::smatch match;
  std::regex re("((?:Uid:[\\s\\t]*)(\\d+))");

  if (stream.is_open()) {
    for(int i = 0; i < 20; i++) {
      std::getline(stream, line);
      fl += line;
    }
    stream.close();

    if (std::regex_search(fl, match, re) && match.size() > 1) {
      uid = match[2].str();
    }
  }

  return uid; 
  }

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string search_uid = LinuxParser::Uid(pid);
  string user, password, uid, line;

  //open stream /etc/passwd
  std::ifstream filestream(kPasswordPath);
  //if process is not running, dump it.
  if (!search_uid.empty() && filestream.is_open()){
    while (std::getline(filestream, line)){
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> user >> password >> uid) {
        if(search_uid == uid){
          std::replace(user.begin(), user.end(), '_', ' ');
          return user;
        }
      }
    }
  }

  return user; 
  }