#ifndef DEPENDENCY_GRAPH_HPP
#define DEPENDENCY_GRAPH_HPP

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"
#include "llvm/Bitcode/LLVMBitCodes.h"
#include "llvm/Support/CommandLine.h"
#include<fstream> // To write to file
//Boost graph includes 

#include <tuple>

#include "Architecture.hpp"
#include "Graph_Utils.hpp"
#include "mem_comp_paramJSON.hpp"
#include "L2_Cache.hpp"

using namespace llvm;

struct ArrayReference{
    StringRef arrayName;
    APInt offset;
};


std::string replaceAll(StringRef inString, char toReplace, char replacement);

class DependencyGraph {
    public:
     DependencyGraph(mem_comp_paramJSON_format _conf, BasicBlock* BB);    
    int inst_count=0;
    void write_dot(std::string fileName, Schedule schedule = NONE);
    void supernode_opt();
    void computeSchedules();
    void performArchitecturalDSE(std::string ParameterFilename,
                                               int OptSearchLimit );
    //TODO 
    void merge_loads();
    bool asap_scheduled=false;
    bool alap_scheduled=false;
    void computeL2_L1_transfertimes();
    void max_par_schedule();
    void sequential_schedule();
    void regenerateBasicBlock(BasicBlock *BB);
    void dumpBasicBlockIR(std::string fileName,BasicBlock* bb);
    /** Gives back the latency of a given instruction.*/
    int getLatency(vertex_t v); 
    DataDependencyGraph ddg;
    mem_comp_paramJSON_format config;
    std::vector<std::list<vertex_t>> schedule;
    std::vector<std::list<vertex_t>> schedule_sequential;
    L2_Cache l2_model;
    private:
    void populateGraph(BasicBlock *BB);
    std::unordered_map<std::string, Instruction*> nodeNameToInstructionMap;
    std::unordered_map<Instruction*,vertex_t> InstructionToVertexMap;
    std::list<vertex_t> write_nodes;
    std::list<vertex_t> read_nodes;
    std::map<vertex_t,std::string> vertices_to_highlight;
    std::map<edge_t, std::string> edges_to_highlight;
    ArrayReference solveElementPtr(BasicBlock *BB, StringRef elementPtrID);
    void supernode_opt_inner(unsigned opCode);
    void replaceAndErase(Instruction *I);
    Instruction* getElementPtr(BasicBlock *BB, StringRef elementPtrID);
    void clearHighlights();
    std::vector<std::list<vertex_t>> schedule_alap;
    //std::list<Architecture> architectures;
};
#endif 
