#ifndef FUNCTIONAL_UNIT_HPP
#define FUNCTIONAL_UNIT_HPP

#include "Graph_Utils.hpp"

class FunctionalUnit: public std::list<vertex_t>{
    public:
    FunctionalUnit(DataDependencyGraph& ddg_, mem_comp_paramJSON_format config_,std::map<vertex_t,unsigned>& vertexToClock,
            std::map<vertex_t,FunctionalUnit>& vertexToFU):
    config(config_), ddg(ddg_),registerFile(),vertexToClock(vertexToClock),
    vertexToFU(vertexToFU){
    };
    FunctionalUnit(const FunctionalUnit &FU2):std::list<vertex_t>(FU2),config(FU2.config),ddg(FU2.ddg),vertexToClock(FU2.vertexToClock),vertexToFU(FU2.vertexToFU){
        registerFile=FU2.registerFile;
        opCode= FU2.opCode;
        earliest_free_slot=FU2.earliest_free_slot;
        extra_description=FU2.extra_description;
        label=FU2.label;
    }
    unsigned opCode;
    unsigned extra_description;
    unsigned earliest_free_slot=0;
    std::string label;
    int getLatency();
    double getArea();
    double getStaticPower();
            
    double getDynamicPower();
    void computeRegisterFileAndInstructionMemorySize();
    void DumpRegisterFileAllocation(std::string fileBaseName);
    void operator=( const FunctionalUnit &F);
    int getTotalAccessesToRF();
    void appendFUInfo(std::string fileName);
    private:
        double getRFDynPow(int rF_bitwidth,int rF_Depth);
        double getRFStaticPow(int rF_bitwidth,int rF_Depth);
        double getRFArea(int rF_bitwidth,int rF_Depth);
        mem_comp_paramJSON_format config;
        DataDependencyGraph& ddg;
        std::vector<std::list<vertex_t>> registerFile;
        std::map<vertex_t,unsigned>& vertexToClock;
        std::map<vertex_t,FunctionalUnit>& vertexToFU;
};

#endif 
