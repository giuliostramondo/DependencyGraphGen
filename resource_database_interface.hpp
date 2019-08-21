#ifndef RESOURCE_DATABASE_INTERFACE
#define RESOURCE_DATABASE_INTERFACE 

#include <sqlite3.h>
#include <iostream>
#include <string>
#include "ErrorLog.hpp"


#define NO_RESULTS -1
#define ERROR 0
class resources_database{
    public: 
        resources_database(std::string db_file_name);
        static int getMultiplierLatency(int clockFrequency, int technology);
        static double getMultiplierIdleEnergy(int clockFrequency, int technology);
        static double getMultiplierActiveEnergy(int clockFrequency, int technology);
        static double getMultiplierArea(int clockFrequency, int technology);
        static int getAdderLatency(int clockFrequency,int technology);
        static double getAdderIdleEnergy(int clockFrequency, int technology);
        static double getAdderActiveEnergy(int clockFrequency,int technology);
        static double getAdderArea(int clockFrequency, int technology);
        static std::string getUnit(std::string ColumnName);
        static int getRegisterFileLatency(int depth, int clockFrequency, int bitwidth, int technology);
        static double getRegisterFileIdleEnergy(int depth, int clockFrequency, int bitwidth, int technology);
        static double getRegisterFileActiveEnergy(int depth, int clockFrequency, int bitwidth, int technology);
        static double getRegisterFileArea(int depth, int clockFrequency, int bitwidth, int technology);
        static double getRegisterFileDoubleBufferArea(int depth, int clockFrequency, int bitwidth, int technology);
        static double getL2Area(int depth, int clockFrequency, int bitwidth, std::string type, int technology);
        //static int getL2SetupLatency(int depth, int clockFrequency, int bitwidth);
        static double getL2IdleEnergy(int depth, int clockFrequency, int bitwidth, std::string type, int technology);
        static double getL2ActiveReadEnergy(int depth, int clockFrequency, int bitwidth, std::string type, int technology);
        static double getL2ActiveWriteEnergy(int depth, int clockFrequency, int bitwidth, std::string type, int technology);
        static double getL2SleepEnergy(int depth, int clockFrequency, int bitwidth, std::string type, int technology);
        static int getL2ReadLatency(int depth, int clockFrequency, int bitwidth, std::string type, int technology);
        static int getL2WriteLatency(int depth, int clockFrequency, int bitwidth, std::string type, int technology);

        ~resources_database();
    private:
        static sqlite3* DB;
        static double query_double(std::string query, bool canThrow=true);
        static int query_int(std::string query, bool canThrow=true);
        static std::string query_string(std::string query, bool canThrow=true);
};
#endif
