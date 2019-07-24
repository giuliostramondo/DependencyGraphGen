#ifndef ARCHITECTURE_HPP
#define ARCHITECTURE_HPP

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"
#include "llvm/Bitcode/LLVMBitCodes.h"
#include "Graph_Utils.hpp"
#include "resource_database_interface.hpp"
#include <utility> // for std::pair 
#include <unordered_map>
#include <set>
#include<fstream> // To write to file
#include<iostream>

using namespace llvm;
class Architecture{

    public:
        Architecture(DataDependencyGraph& g,int latency,mem_comp_paramJSON_format config): 
            ddg(g), maxLatency(latency),config(config) {};
        void generateArchitecturalMapping();
        void generateSmallestArchitecturalMapping();
        void write_dot(std::string filename);
        void write_architecture_dot(std::string filename);
        void describe();
        void mergeFUs();
        void performALAPSchedule();
        //MaxLatency is the maximum latency allowed to the scheduler
        int getMaxLatency();
        //ActualMaxLatency is the maximum latency obtained after scheduling
        int getActualMaxLatency();
        double getArea();
        double getStaticPower();
        double getDynamicPower();
        double getTotalPower();
        void appendArchInfoToCSV(std::string csvFileName);
        void dumpSchedule();

    private:
        //TODO Integrate with resource_database_interface
        DataDependencyGraph& ddg;
        int getAVGBankDepth();
        int maxLatency;
        int bankDepth = 0;
        int bankNumber = 0;
        mem_comp_paramJSON_format config;
        //Map between OpCodes and list of FUs (implementing the opcode)
        //Each FU contains a list of vertices of the ddg that will execute
        std::map<unsigned,std::list<FunctionalUnit>> units;
        std::vector<std::list<vertex_t>> schedule_alap;
        std::vector<std::list<vertex_t>> schedule_architectural;
        
}; 
#endif

