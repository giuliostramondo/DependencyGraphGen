#include "FunctionalUnit.hpp"

void FunctionalUnit::operator=( const FunctionalUnit &F){
    opCode = F.opCode;
    label = F.label;
    ddg = F.ddg;
    extra_description = F.extra_description;
    earliest_free_slot = F.earliest_free_slot;
    config = F.config;
    ddg = F.ddg;
    registerFileDepth = F.registerFileDepth;
    instructionMemoryDepth = F.instructionMemoryDepth;
}

double FunctionalUnit::getArea(int registerFileAVGDepth){
    vertex_t firstInstruction = front();
    double area =getVertexArea(ddg,firstInstruction,config,registerFileAVGDepth); 
    return area;
}

int FunctionalUnit::getLatency(){
    vertex_t firstInstruction = front();
    int latency =getVertexLatency(ddg,firstInstruction,config); 
    return latency;
}

double FunctionalUnit::getDynamicPower(int registerFileAVGDepth){
    vertex_t firstInstruction = front();
    double dyn_power =getVertexDynamicPower(ddg,firstInstruction,config,registerFileAVGDepth); 
    return dyn_power;
}

double FunctionalUnit::getStaticPower(int registerFileAVGDepth){
    vertex_t firstInstruction = front();
    double static_power =getVertexStaticPower(ddg,firstInstruction,config,registerFileAVGDepth); 
    return static_power;
}

void FunctionalUnit::computeRegisterFileAndInstructionMemorySize(){
    vertex_t firstInstruction = front();
    Instruction* I = ddg[firstInstruction].inst;
    //If this FU is a L1 bank
    //The size of its RF will be the number
    //of element for which the clock cycle at 
    //which they arrive from L2 (schedule[ASAP]) is different from 
    //the clock cycle at which it is read schedule[Architectural]
    if(isa<LoadInst>(I)){
       for(auto vert_it = begin();
             vert_it!=end(); vert_it++){
           if(ddg[*vert_it].schedules[ASAP] != 
                   ddg[*vert_it].schedules[ARCHITECTURAL]){
               registerFileDepth++;
           }
       }  
    }else{
        //IF this FU is a functional unit its register file size
        //will store all the incoming ( and resulting ) elements
        //that will be used in the computation at a later cycle.
        if(isa<BinaryOperator>(I)){
           //get list of incoming elements and their clock cycles
            for(auto vert_it = begin();
                 vert_it!=end(); vert_it++){          
                    
            }
        }else {

        }
    }
}
