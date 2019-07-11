#ifndef ARCHITECTURE_HPP
#define ARCHITECTURE_HPP

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"
#include "llvm/Bitcode/LLVMBitCodes.h"
#include "Graph_Utils.hpp"
#include <utility> // for std::pair 
#include <unordered_map>

using namespace llvm;
class Architecture{


    class FunctionalUnit: public std::list<vertex_t>{
        public:
        unsigned opCode;
        unsigned extra_description;
        unsigned earliest_free_slot=0;
        std::string label;

    };
    public:
        Architecture(DataDependencyGraph& g,int latency,mem_comp_paramJSON_format config): 
            ddg(g), maxLatency(latency),config(config) {};
        void generateArchitecturalMapping();
        void generateSmallestArchitecturalMapping();
        void write_dot(std::string filename);
        void describe();
        void mergeFUs();
        void performALAPSchedule();
    private:
        DataDependencyGraph& ddg;
        int maxLatency;
        mem_comp_paramJSON_format config;
        //Map between OpCodes and list of FUs (implementing the opcode)
        //Each FU contains a list of vertices of the ddg that will execute
        //std::map<unsigned,std::list<std::list<vertex_t>>> units;
        std::map<unsigned,std::list<FunctionalUnit>> units;
        std::vector<std::list<vertex_t>> schedule_alap;
        
}; 
#endif

