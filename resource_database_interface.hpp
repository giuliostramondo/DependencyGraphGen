#ifndef RESOURCE_DATABASE_INTERFACE
#define RESOURCE_DATABASE_INTERFACE 

#include <sqlite3.h>
#include <iostream>
#include <string>

#define NO_RESULTS -1
#define ERROR 0
class resources_database{
    public: 
        resources_database(std::string db_file_name);
        static int getMultiplierLatency(int clockFrequency);
        static double getMultiplierIdleEnergy(int clockFrequency);
        static double getMultiplierActiveEnergy(int clockFrequency);
        static double getMultiplierArea(int clockFrequency);
        static int getAdderLatency(int clockFrequency);
        static double getAdderIdleEnergy(int clockFrequency);
        static double getAdderActiveEnergy(int clockFrequency);
        static double getAdderArea(int clockFrequency);
        static std::string getUnit(std::string ColumnName);
        static int getRegisterFileLatency(int depth, int clockFrequency, int bitwidth);
        static double getRegisterFileIdleEnergy(int depth, int clockFrequency, int bitwidth);
        static double getRegisterFileActiveEnergy(int depth, int clockFrequency, int bitwidth);
        static double getRegisterFileArea(int depth, int clockFrequency, int bitwidth);
        static double getRegisterFileDoubleBufferArea(int depth, int clockFrequency, int bitwidth);
        static int getL2Area(int depth, int clockFrequency, int bitwidth);
        static int getL2SetupLatency(int depth, int clockFrequency, int bitwidth);
        static double getL2IdleEnergy(int depth, int clockFrequency, int bitwidth);
        static double getL2ActiveReadEnergy(int depth, int clockFrequency, int bitwidth);
        static double getL2ActiveWriteEnergy(int depth, int clockFrequency, int bitwidth);
        static double getL2SleepEnergy(int depth, int clockFrequency, int bitwidth);
        ~resources_database();
    private:
        static sqlite3* DB;
        static double query_double(std::string query);
        static int query_int(std::string query);
        static std::string query_string(std::string query);
};
/*
CREATE TABLE sram_l2(
  "Clock frequency" INTEGER,
  "IO" INTEGER,
  "Depth" INTEGER,
  "Total cell area" REAL,
  "setup_latency" INTEGER,
  "Idle Energy" REAL,
  "Active Energy Read" REAL,
  "Active Energy Write" REAL,
  "Sleep Energy" REAL
);
*/
#endif
