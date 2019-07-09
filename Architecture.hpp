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
        Architecture(DataDependencyGraph& g,int latency): ddg(g),maxLatency(latency) {}
    private:
        DataDependencyGraph& ddg;
        int maxLatency;
        void performALAPSchedule();
        void mergeFUs();
        std::map<std::string,std::list<std::list<Instruction*>>> units;
        
}; 
#endif

