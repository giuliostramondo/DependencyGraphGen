#include "Graph_Utils.hpp"


//Note: To perform accesses to the database
//an istance of resources_database needs to be instantiated in the main 
int getVertexLatency(DataDependencyGraph& ddg,vertex_t v,mem_comp_paramJSON_format config){
    Instruction *I = ddg[v].inst;
    //if true, value for latency/area/power are directly taken from 
    //the configuration file
    //if false these are ignored and values from the database will be used
    if(config.overwrite_resouce_database){
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
    }else{
        int ClockFreq=
               config.resource_database.clock_frequency;

       if(isa<LoadInst>(I)){
           // if(ddg[v].info == MRAM)
           //   return  config.memory_param.mram.read_latency;
           // else
           //   return  config.memory_param.sram.read_latency;
            int registerFileBitwidth= 
               config.resource_database.bitwidth_register_file;
           return resources_database::getRegisterFileLatency(
                   2, ClockFreq, registerFileBitwidth);
       }
       if(isa<StoreInst>(I)){
            //if(ddg[v].info == MRAM)
            //   return config.memory_param.mram.write_latency;
            //else
            //   return config.memory_param.sram.write_latency;
            //
            //NOTE: as read/write ops are similar for register file
            //the implementation below is the same as the load
            //Differentiate in the future among layers (L1 L2 and type (MRAM SRAM).
            int registerFileBitwidth= 
               config.resource_database.bitwidth_register_file;
           return resources_database::getRegisterFileLatency(
                   2, ClockFreq, registerFileBitwidth);     
       }
       if(isa<BinaryOperator>(I)){
            if(I->getOpcode() == Instruction::Add){
                return resources_database::getAdderLatency(ClockFreq);
               
            }
            if(I->getOpcode() == Instruction::Mul){
                return resources_database::getMultiplierLatency(ClockFreq);
            }
       }
    }
       return 1;
}


double getVertexArea(DataDependencyGraph& ddg,vertex_t v,mem_comp_paramJSON_format config, int registerFileAVGDepth){
    Instruction *I = ddg[v].inst;
   if(config.overwrite_resouce_database){
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
   }else{
    int ClockFreq=
               config.resource_database.clock_frequency;
        if(isa<LoadInst>(I) || isa<StoreInst>(I)){
            //if(ddg[v].info == MRAM)
              //return  config.memory_param.mram.area;
            //else
              //return  config.memory_param.sram.area;
            int registerFileBitwidth= 
               config.resource_database.bitwidth_register_file;
            double result=resources_database::getRegisterFileArea(
                   registerFileAVGDepth, ClockFreq, registerFileBitwidth);   
            //if no results where found in the db use model
            if(result == NO_RESULTS){
               result = getFromModelRegisterFileArea(registerFileBitwidth,
                      ClockFreq, registerFileAVGDepth); 
            }
            return result;
       }
       if(isa<BinaryOperator>(I)){
            if(I->getOpcode() == Instruction::Add){
                return resources_database::getAdderArea(ClockFreq);
            }
            if(I->getOpcode() == Instruction::Mul){
                return resources_database::getMultiplierArea(ClockFreq);
            }
       }
    }
   return 1;
}

double getVertexDynamicPower(DataDependencyGraph& ddg,vertex_t v,mem_comp_paramJSON_format config, int registerFileAVGDepth){
   Instruction *I = ddg[v].inst;
   if(config.overwrite_resouce_database){
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
   }else{
    int ClockFreq=
               config.resource_database.clock_frequency;
       if(isa<LoadInst>(I)){
           // if(ddg[v].info == MRAM)
           //   return  config.memory_param.mram.dynamic_read_power;
           // else
           //   return  config.memory_param.sram.dynamic_read_power;
            int registerFileBitwidth= 
               config.resource_database.bitwidth_register_file;
           double result=resources_database::getRegisterFileActiveEnergy(
                   registerFileAVGDepth, ClockFreq, registerFileBitwidth);  
           if(result==NO_RESULTS){
                result=getFromModelRegisterFileActiveEnergy(registerFileBitwidth,
                        ClockFreq, registerFileAVGDepth);
           }
           return result;
       }
       if(isa<StoreInst>(I)){
            //if(ddg[v].info == MRAM)
            //   return config.memory_param.mram.dynamic_write_power;
            //else
            //   return config.memory_param.sram.dynamic_write_power;
            int registerFileBitwidth= 
               config.resource_database.bitwidth_register_file;
           double result= resources_database::getRegisterFileActiveEnergy(
                   registerFileAVGDepth, ClockFreq, registerFileBitwidth);  
           if(result==NO_RESULTS){
                result=getFromModelRegisterFileActiveEnergy(registerFileBitwidth,
                        ClockFreq, registerFileAVGDepth);
           }
           return result;

       }
       if(isa<BinaryOperator>(I)){
            if(I->getOpcode() == Instruction::Add){
                return resources_database::getAdderActiveEnergy(ClockFreq);

            }
            if(I->getOpcode() == Instruction::Mul){
                return resources_database::getMultiplierActiveEnergy(ClockFreq);
            }
       }
   }
   return 1;
}

double getVertexStaticPower(DataDependencyGraph& ddg,vertex_t v,mem_comp_paramJSON_format config,
        int registerFileAVGDepth){
   Instruction *I = ddg[v].inst;
   if(config.overwrite_resouce_database){

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
   }else{
    int ClockFreq=
               config.resource_database.clock_frequency;
       if(isa<LoadInst>(I) || isa<StoreInst>(I)){
           // if(ddg[v].info == MRAM)
           //   return  config.memory_param.mram.static_power;
           // else
           //   return  config.memory_param.sram.static_power;
            int registerFileBitwidth= 
               config.resource_database.bitwidth_register_file;
           double result= resources_database::getRegisterFileIdleEnergy(
                   registerFileAVGDepth, ClockFreq, registerFileBitwidth);      
           if (result == NO_RESULTS){
                result = getFromModelRegisterFileIdleEnergy(
                        registerFileBitwidth,ClockFreq,
                        registerFileAVGDepth);
           }
           return result;
       }
       if(isa<BinaryOperator>(I)){
            if(I->getOpcode() == Instruction::Add){
                return resources_database::getAdderIdleEnergy(ClockFreq);

            }
            if(I->getOpcode() == Instruction::Mul){
                return resources_database::getMultiplierIdleEnergy(ClockFreq);
            }
       }

   }
   return 1;
}

double FunctionalUnit::getArea(DataDependencyGraph& ddg,mem_comp_paramJSON_format config,
        int registerFileAVGDepth){
    vertex_t firstInstruction = front();
    double area =getVertexArea(ddg,firstInstruction,config,registerFileAVGDepth); 
    return area;
}

int FunctionalUnit::getLatency(DataDependencyGraph& ddg,mem_comp_paramJSON_format config){
    vertex_t firstInstruction = front();
    int latency =getVertexLatency(ddg,firstInstruction,config); 
    return latency;
}

double FunctionalUnit::getDynamicPower(DataDependencyGraph& ddg,mem_comp_paramJSON_format config,
        int registerFileAVGDepth){
    vertex_t firstInstruction = front();
    double dyn_power =getVertexDynamicPower(ddg,firstInstruction,config,registerFileAVGDepth); 
    return dyn_power;
}

double FunctionalUnit::getStaticPower(DataDependencyGraph& ddg,mem_comp_paramJSON_format config,
        int registerFileAVGDepth){
    vertex_t firstInstruction = front();
    double static_power =getVertexStaticPower(ddg,firstInstruction,config,registerFileAVGDepth); 
    return static_power;
}

