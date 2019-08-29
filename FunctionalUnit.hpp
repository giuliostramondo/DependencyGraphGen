#ifndef FUNCTIONAL_UNIT_HPP
#define FUNCTIONAL_UNIT_HPP

#include "Graph_Utils.hpp"

class FunctionalUnit: public std::list<vertex_t>{
    public:
    FunctionalUnit(DataDependencyGraph& ddg_, mem_comp_paramJSON_format config_):
    config(config_), ddg(ddg_){
        registerFileDepth=0;
        instructionMemoryDepth=0;
    };
    unsigned opCode;
    unsigned extra_description;
    unsigned earliest_free_slot=0;
    std::string label;
    int getLatency();
    double getArea(int registerFileAVGDepth = 5);
    double getStaticPower(int registerFileAVGDepth = 5);
            
    double getDynamicPower(int registerFileAVGDepth = 5);
    void computeRegisterFileAndInstructionMemorySize();
    void operator=( const FunctionalUnit &F);
    private:
        mem_comp_paramJSON_format config;
        DataDependencyGraph& ddg;
        int registerFileDepth;
        int instructionMemoryDepth;
};

#endif 
