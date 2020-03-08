#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// Return this process's ID
int Process::Pid() { return this->pid_; }

//set pid
void Process::Pid(int pid) { this->pid_ = pid; }

//parse stat cpu usage anc calculate percentage
void Process::Usage(){
    //parse stat file store as a processStat struct
    //(ref:) https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
    LinuxParser::processStat cpu = LinuxParser::Process(pid_);

    long total_time = cpu.utime + cpu.stime + 
                      cpu.cutime + cpu.cstime;
    
    //store startime as member
    starttime_ = cpu.starttime;

    long seconds = LinuxParser::UpTime() - cpu.starttime/sysconf(_SC_CLK_TCK);

    // store usage for comparison
    usage_ = (float)(total_time/sysconf(_SC_CLK_TCK))/seconds ;
}

// Return this process's CPU utilization
float Process::CpuUtilization() { return (usage_); }

// TODO: Return the command that generated this process
string Process::Command() { return string(); }

// TODO: Return this process's memory utilization
string Process::Ram() { return string(); }

// TODO: Return the user (name) that generated this process
string Process::User() { return string(); }

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() { return 0; }

// Overload the "less than" comparison operator for Process objects
bool Process::operator>(Process const& a) const { return this->usage_ > a.usage_ ; }