#include "resource_database_interface.hpp"


//Note at the beginning DB is not initialized, being a static pointer
//this causes a problem during linking.
//This initializes it to Null and solves that problem.
sqlite3* resources_database::DB = NULL;

resources_database::resources_database(std::string db_file_name){
    sqlite3* DB_tmp;
    int exit = sqlite3_open("data/new_resource_utilization_data.db",&DB_tmp);
    if (exit != SQLITE_OK){
        std::cout<< "Error opening the database "<<db_file_name<<std::endl;
    }else{
        std::cout<<"Db is open"<<std::endl;
    }
    DB=DB_tmp;
}
int resources_database::getMultiplierLatency(int clockFrequency){
    std::string query = "select \"Latency\" from multiplier where \"Clock frequency\" =="+std::to_string(clockFrequency)+";";
    int latency=query_int(query);
    return latency;
}
double resources_database::getMultiplierIdleEnergy(int clockFrequency){
    std::string query = "select \"Idle Energy\" from multiplier where \"Clock frequency\" =="+std::to_string(clockFrequency)+";";
    double energy=query_double(query);
    return energy;
}
double resources_database::getMultiplierActiveEnergy(int clockFrequency){
    std::string query = "select \"Active Energy\" from multiplier where \"Clock frequency\" =="+std::to_string(clockFrequency)+";";
    double energy=query_double(query);
    return energy;
}
double resources_database::getMultiplierArea(int clockFrequency){
    std::string query = "select \"Total cell area\" from multiplier where \"Clock frequency\" =="+std::to_string(clockFrequency)+";";
    double area=query_double(query);
    return area;
}
int resources_database::getAdderLatency(int clockFrequency){
    std::string query = "select \"Latency\" from adder where \"Clock frequency\" =="+std::to_string(clockFrequency)+";";
    int latency=query_int(query);
    return latency;
}
double resources_database::getAdderIdleEnergy(int clockFrequency){
    std::string query = "select \"Idle Energy\" from adder where \"Clock frequency\" =="+std::to_string(clockFrequency)+";";
    double energy=query_double(query);
    return energy;
}
double resources_database::getAdderActiveEnergy(int clockFrequency){
    std::string query = "select \"Active Energy\" from adder where \"Clock frequency\" =="+std::to_string(clockFrequency)+";";
    double energy=query_double(query);
    return energy;
}
double resources_database::getAdderArea(int clockFrequency){
    std::string query = "select \"Total cell area\" from adder where \"Clock frequency\" =="+std::to_string(clockFrequency)+";";
    double area=query_double(query);
    return area;
}
std::string resources_database::getUnit(std::string ColumnName){
    std::string query = "select \"unit\" from units where \"Item\" == \""+ColumnName+"\"";
    std::string result=query_string(query);
    return result;
}
int resources_database::getRegisterFileLatency(int depth, int clockFrequency, int bitwidth){
    std::string query = "select \"Latency\" from register_file where \"Clock frequency\" =="+std::to_string(clockFrequency)+" and \"Depth\" == "+std::to_string(depth)+" and \"Bitwidth\"== "+std::to_string(bitwidth)+";";
    int latency=query_int(query);
    return latency;
}
double resources_database::getRegisterFileIdleEnergy(int depth, int clockFrequency, int bitwidth){
    std::string query = "select \"Idle Energy\" from register_file where \"Clock frequency\" =="+std::to_string(clockFrequency)+" and \"Depth\" == "+std::to_string(depth)+" and \"Bitwidth\"== "+std::to_string(bitwidth)+";";
    double energy=query_double(query);
    return energy;
}
double resources_database::getRegisterFileActiveEnergy(int depth, int clockFrequency, int bitwidth){
    std::string query = "select \"Active Energy\" from register_file where \"Clock frequency\" =="+std::to_string(clockFrequency)+" and \"Depth\" == "+std::to_string(depth)+" and \"Bitwidth\"== "+std::to_string(bitwidth)+";";
    double energy=query_double(query);
    return energy;
}
double resources_database::getRegisterFileArea(int depth, int clockFrequency, int bitwidth){
    std::string query = "select \"Total cell area\" from register_file where \"Clock frequency\" =="+std::to_string(clockFrequency)+" and \"Depth\" == "+std::to_string(depth)+" and \"Bitwidth\"== "+std::to_string(bitwidth)+";";
    double area=query_double(query);
    return area;
}
double resources_database::getRegisterFileDoubleBufferArea(int depth, int clockFrequency, int bitwidth){
    std::string query = "select \"Area with double buffer\" from register_file where \"Clock frequency\" =="+std::to_string(clockFrequency)+" and \"Depth\" == "+std::to_string(depth)+" and \"Bitwidth\"== "+std::to_string(bitwidth)+";";
    double area=query_double(query);
    return area;
}

int resources_database::getL2Area(int depth, int clockFrequency, int bitwidth){
    std::string query = "select \"Total cell area\" from sram_l2 where \"Clock frequency\" =="+std::to_string(clockFrequency)+" and \"Depth\" == "+std::to_string(depth)+" and \"IO\"== "+std::to_string(bitwidth)+";";
    double area=query_double(query);
    return area;
}
//int resources_database::getL2SetupLatency(int depth, int clockFrequency, int bitwidth){
//    std::string query = "select \"setup_latency\" from sram_l2 where \"Clock frequency\" =="+std::to_string(clockFrequency)+" and \"Depth\" == "+std::to_string(depth)+" and \"IO\"== "+std::to_string(bitwidth)+";";
//    int latency=query_int(query);
//    return latency;
//}
double resources_database::getL2IdleEnergy(int depth, int clockFrequency, int bitwidth){
    std::string query = "select \"Idle Energy\" from sram_l2 where \"Clock frequency\" =="+std::to_string(clockFrequency)+" and \"Depth\" == "+std::to_string(depth)+" and \"IO\"== "+std::to_string(bitwidth)+";";
    double energy=query_double(query);
    return energy;
}
double resources_database::getL2ActiveReadEnergy(int depth, int clockFrequency, int bitwidth){
    std::string query = "select \"Active Energy Read\" from sram_l2 where \"Clock frequency\" =="+std::to_string(clockFrequency)+" and \"Depth\" == "+std::to_string(depth)+" and \"IO\"== "+std::to_string(bitwidth)+";";
    double energy=query_double(query);
    return energy;
}
double resources_database::getL2ActiveWriteEnergy(int depth, int clockFrequency, int bitwidth){
    std::string query = "select \"Active Energy Write\" from sram_l2 where \"Clock frequency\" =="+std::to_string(clockFrequency)+" and \"Depth\" == "+std::to_string(depth)+" and \"IO\"== "+std::to_string(bitwidth)+";";
    double energy=query_double(query);
    return energy;
}
double resources_database::getL2SleepEnergy(int depth, int clockFrequency, int bitwidth){
    std::string query = "select \"Sleep Energy\" from sram_l2 where \"Clock frequency\" =="+std::to_string(clockFrequency)+" and \"Depth\" == "+std::to_string(depth)+" and \"IO\"== "+std::to_string(bitwidth)+";";
    double energy=query_double(query);
    return energy;
}



resources_database::~resources_database(){
    sqlite3_close(DB);
}
double resources_database::query_double(std::string query){
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(DB, query.c_str(),-1,&stmt,NULL);
    if(rc != SQLITE_OK){
        std::cout<<"error1: "<<sqlite3_errmsg(DB)<<std::endl;
        return 0;
    }
    double id=NO_RESULTS;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW){
        id = sqlite3_column_double(stmt,0);
    }
    if (rc != SQLITE_DONE){
        std::cout <<"error2: "<<sqlite3_errmsg(DB)<<std::endl;
        return 0;
    }
    sqlite3_finalize(stmt);
    return id; 
}
int resources_database::query_int(std::string query){
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(DB, query.c_str(),-1,&stmt,NULL);
    if(rc != SQLITE_OK){
        std::cout<<"error: "<<sqlite3_errmsg(DB)<<std::endl;
        return 0;
    }
    int id = NO_RESULTS;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW){
        id = sqlite3_column_int(stmt,0);
    }
    if (rc != SQLITE_DONE){
        std::cout <<"error: "<<sqlite3_errmsg(DB)<<std::endl;
        return 0;
    }
    sqlite3_finalize(stmt);
    return id; 
}

std::string resources_database::query_string(std::string query){
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(DB, query.c_str(),-1,&stmt,NULL);
    if(rc != SQLITE_OK){
        std::cout<<"error: "<<sqlite3_errmsg(DB)<<std::endl;
        return 0;
    }
    std::string result="NO_RESULTS";
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW){
        result = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
    }
    if (rc != SQLITE_DONE){
        std::cout <<"error: "<<sqlite3_errmsg(DB)<<std::endl;
        return 0;
    }
    sqlite3_finalize(stmt);
    return result; 
}

