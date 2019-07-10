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

    public:
        Architecture(DataDependencyGraph& g,int latency): ddg(g), maxLatency(latency) {};
        void generateArchitecturalMapping();
        void write_dot(std::string filename);
        void describe();
    private:
        DataDependencyGraph& ddg;
        int maxLatency;
        void performALAPSchedule();
        void mergeFUs();
        //Map between OpCodes and list of FUs (implementing the opcode)
        //Each FU contains a list of vertices of the ddg that will execute
        std::map<unsigned,std::list<std::list<vertex_t>>> units;
        
}; 
#endif

