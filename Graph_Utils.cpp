#include "Graph_Utils.hpp"

int getVertexLatency(DataDependencyGraph& ddg,vertex_t v,mem_comp_paramJSON_format config){
   Instruction *I = ddg[v].inst;
   if(isa<LoadInst>(I)){
        if(ddg[v].info == MRAM)
          return  config.memory_param.mram.read_latency;
        else
          return  config.memory_param.sram.read_latency;
   }
   if(isa<StoreInst>(I)){
        if(ddg[v].info == MRAM)
           return config.memory_param.mram.write_latency;
        else
           return config.memory_param.sram.write_latency;
   }
   if(isa<BinaryOperator>(I)){
        if(I->getOpcode() == Instruction::Add)
            return config.compute_param.funtional_unit.add.latency;
        if(I->getOpcode() == Instruction::Mul)
            return config.compute_param.funtional_unit.mul.latency;
   }
   //errs()<<"Unknown instruction "<<*I<<"\n";
   //errs()<<"returning default latency (1)\n";
   return 1;
}


