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
   return 1;
}


int getVertexArea(DataDependencyGraph& ddg,vertex_t v,mem_comp_paramJSON_format config){
   Instruction *I = ddg[v].inst;
   if(isa<LoadInst>(I) || isa<StoreInst>(I)){
        if(ddg[v].info == MRAM)
          return  config.memory_param.mram.area;
        else
          return  config.memory_param.sram.area;
   }
   if(isa<BinaryOperator>(I)){
        if(I->getOpcode() == Instruction::Add)
            return config.compute_param.funtional_unit.add.area;
        if(I->getOpcode() == Instruction::Mul)
            return config.compute_param.funtional_unit.mul.area;
   }
   return 1;
}

int getVertexDynamicPower(DataDependencyGraph& ddg,vertex_t v,mem_comp_paramJSON_format config){
   Instruction *I = ddg[v].inst;
   if(isa<LoadInst>(I)){
        if(ddg[v].info == MRAM)
          return  config.memory_param.mram.dynamic_read_power;
        else
          return  config.memory_param.sram.dynamic_read_power;
   }
   if(isa<StoreInst>(I)){
        if(ddg[v].info == MRAM)
           return config.memory_param.mram.dynamic_write_power;
        else
           return config.memory_param.sram.dynamic_write_power;
   }
   if(isa<BinaryOperator>(I)){
        if(I->getOpcode() == Instruction::Add)
            return config.compute_param.funtional_unit.add.dynamic_power;
        if(I->getOpcode() == Instruction::Mul)
            return config.compute_param.funtional_unit.mul.dynamic_power;
   }
   return 1;
}

int getVertexStaticPower(DataDependencyGraph& ddg,vertex_t v,mem_comp_paramJSON_format config){
   Instruction *I = ddg[v].inst;
   if(isa<LoadInst>(I) || isa<StoreInst>(I)){
        if(ddg[v].info == MRAM)
          return  config.memory_param.mram.static_power;
        else
          return  config.memory_param.sram.static_power;
   }
   if(isa<BinaryOperator>(I)){
        if(I->getOpcode() == Instruction::Add)
            return config.compute_param.funtional_unit.add.static_power;
        if(I->getOpcode() == Instruction::Mul)
            return config.compute_param.funtional_unit.mul.static_power;
   }
   return 1;
}

int FunctionalUnit::getArea(DataDependencyGraph& ddg,mem_comp_paramJSON_format config){
    vertex_t firstInstruction = front();
    int area =getVertexArea(ddg,firstInstruction,config); 
    return area;
}

int FunctionalUnit::getLatency(DataDependencyGraph& ddg,mem_comp_paramJSON_format config){
    vertex_t firstInstruction = front();
    int latency =getVertexLatency(ddg,firstInstruction,config); 
    return latency;
}

int FunctionalUnit::getDynamicPower(DataDependencyGraph& ddg,mem_comp_paramJSON_format config){
    vertex_t firstInstruction = front();
    int dyn_power =getVertexDynamicPower(ddg,firstInstruction,config); 
    return dyn_power;
}

int FunctionalUnit::getStaticPower(DataDependencyGraph& ddg,mem_comp_paramJSON_format config){
    vertex_t firstInstruction = front();
    int static_power =getVertexStaticPower(ddg,firstInstruction,config); 
    return static_power;
}

