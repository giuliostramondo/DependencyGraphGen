#include "L2_Cache.hpp"

L2_Cache::L2_Cache(mem_comp_paramJSON_format config_){
    config = config_;
    totalEnergyConsumed = 0;
}
L2_Cache::L2_Cache(L2_Cache *toCopy){
   config = toCopy->config; 
   totalEnergyConsumed = toCopy->totalEnergyConsumed;
   memory_controller = std::list<L2_Operation>(toCopy->memory_controller);
   memory_partitioning = std::list<L2_Partition>(toCopy->memory_partitioning);
   L2_baseAddress = std::map<std::string,int>(toCopy->L2_baseAddress);
}

int L2_Cache::getNextAvail_L2_Cycle(){
      L2_Operation last_l2_op = memory_controller.back();
    int nextAvail_L2Cycle = last_l2_op.clock_tick+ last_l2_op.latency;
    return nextAvail_L2Cycle;
  
}

double L2_Cache::getArea(){
    int clock_l2 = config.resource_database.clock_l2;
    int depth_l2 = config.resource_database.depth_l2;
    int bitwidth_l2 = config.resource_database.bitwidth_l2;
    int technology = config.resource_database.technology_l2;
    std::string type = config.resource_database.type_l2;
    return resources_database::getL2Area(depth_l2,clock_l2,bitwidth_l2,type,technology);
}
void L2_Cache::add_L2_operation(int clock, Optype type, int start_addr, int last_addr,int latency){
    L2_Operation l2_op;
    l2_op.clock_tick=clock;
    l2_op.optype=type;    
    l2_op.start_address=start_addr;
    l2_op.last_address=last_addr;
    int clock_l2 = config.resource_database.clock_l2;
    int depth_l2 = config.resource_database.depth_l2;
    int bitwidth_l2 = config.resource_database.bitwidth_l2;
    int startupLatencyL2_write = config.resource_database.startup_write_latency_l2;
    int startupLatencyL2_read = config.resource_database.startup_read_latency_l2;
    int technology = config.resource_database.technology_l2;
    std::string type_l2 = config.resource_database.type_l2;
    double idleEnergy=resources_database::getL2IdleEnergy(depth_l2,clock_l2,bitwidth_l2,type_l2,technology);

    double elementTransfered_perClock = ((double)bitwidth_l2)/config.resource_database.bitwidth_register_file;
    int totalCycles = (last_addr-start_addr)/elementTransfered_perClock;
    switch(type){
        case READ_FROM_L2:{
                double readEnergy=resources_database::getL2ActiveReadEnergy(depth_l2,clock_l2,bitwidth_l2,type_l2,technology);
                l2_op.energy= totalCycles * readEnergy;
                l2_op.energy+= startupLatencyL2_read * idleEnergy;
                l2_op.latency=totalCycles+startupLatencyL2_read;
                break;
                          }
        case WRITE_TO_L2:{
                double writeEnergy=resources_database::getL2ActiveWriteEnergy(depth_l2,clock_l2,bitwidth_l2,type_l2,technology);
                l2_op.energy= totalCycles * writeEnergy;
                l2_op.energy+= startupLatencyL2_write * idleEnergy;
                l2_op.latency=totalCycles+startupLatencyL2_write;
                break;
                         }
        case IDLE:{
                double idleEnergy=resources_database::getL2IdleEnergy(depth_l2,clock_l2,bitwidth_l2,type_l2,technology);
                l2_op.energy = latency * idleEnergy;
                l2_op.latency=latency;
                break;
                  }
        case SLEEP:{
                double sleepEnergy=resources_database::getL2SleepEnergy(depth_l2,clock_l2,bitwidth_l2,type_l2,technology);
                l2_op.energy = latency*sleepEnergy;
                l2_op.latency=latency;
                break;
                   }
    }
    totalEnergyConsumed += l2_op.energy;
    memory_controller.push_back(l2_op);
    return;
}

void L2_Cache::add_L2_partition(std::string ID, int start_address, int size){
    L2_Partition partition;
    partition.ID= ID;
    partition.start_address= start_address;
    partition.size=size;
    memory_partitioning.push_back(partition);    
    return;
}

void L2_Cache::dumpMemoryPartitioning(std::string fileName){
    std::ofstream file;
    file.open(fileName);
    file<<"ID,start_address,size\n";
    for(auto part_it = memory_partitioning.begin();
           part_it != memory_partitioning.end();
          part_it++){
       file<<part_it->ID+","; 
       file<<std::to_string(part_it->start_address)+","; 
       file<<std::to_string(part_it->size)+""; 
       file<<"\n";
    } 
    file.close();
    return;
}

void L2_Cache::dumpMemoryOperations(std::string fileName){
    std::ofstream file;
    file.open(fileName);
    file<<"Clock,OpType,StartAddress,LastAddress,Latency,Energy\n";
    for(auto op_it=memory_controller.begin();
            op_it!=memory_controller.end();
            op_it++){
        file<<std::to_string(op_it->clock_tick)+",";
        switch(op_it->optype){
        case READ_FROM_L2:{
                file<<"read,";
                break;
                          }
        case WRITE_TO_L2:{
                file<<"write,";
                break;
                         }
        case IDLE:{
                file<<"idle,";
                break;
                  }
        case SLEEP:{
                file<<"sleep,";
                break;
                   }

        }
        file<<std::to_string(op_it->start_address)+",";
        file<<std::to_string(op_it->last_address)+",";
        file<<std::to_string(op_it->latency)+",";
        file<<std::to_string(op_it->energy)+"\n";

    }
}
double L2_Cache::getTotalEnergyConsumed(){
    return totalEnergyConsumed;
}
