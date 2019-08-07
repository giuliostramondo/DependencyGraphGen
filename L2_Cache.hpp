#ifndef L2_CACHE_HPP
#define L2_CACHE_HPP

#include <list>
#include <string>
#include<fstream> // To write to file
#include "mem_comp_paramJSON.hpp"
#include "resource_database_interface.hpp"

enum Optype{
    READ_FROM_L2,
    WRITE_TO_L2,
    IDLE,
    SLEEP
};
struct L2_Operation{
    int clock_tick;
    Optype optype;
    int start_address;
    int last_address;
    int latency;
    double energy;
};
class L2_Cache{


    struct L2_Partition{
        std::string ID;
        int start_address;
        int size;
    };

    public:
        L2_Cache(mem_comp_paramJSON_format config_);
        L2_Cache(L2_Cache *toCopy);
        void add_L2_operation(int clock, Optype type, int start_address, int last_address,int latency);
        void add_L2_partition(std::string ID, int start_address, int size);
        void dumpMemoryPartitioning(std::string fileName);
        double getTotalEnergyConsumed();
        double getArea();
        void dumpMemoryOperations(std::string fileName);
        std::list<L2_Operation> memory_controller;
        int getNextAvail_L2_Cycle();
        std::map<std::string,int> L2_baseAddress;
    private:
        std::list<L2_Partition> memory_partitioning;
        double totalEnergyConsumed;
        mem_comp_paramJSON_format config;
};
#endif 
