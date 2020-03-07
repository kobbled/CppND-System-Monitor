#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <thread>

#include "linux_parser.h"

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

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
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

// TODO: Read and return the system uptime
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

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return 0; }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { return 0; }

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return 0; }

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { return {}; }

// Read /proc/stat and return total processes, running processes, and cpu utilization
LinuxParser::procData LinuxParser::Processes() {
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


// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { return 0; }

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid[[maybe_unused]]) { return string(); }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid[[maybe_unused]]) { return 0; }