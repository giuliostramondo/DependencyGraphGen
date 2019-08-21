#include "ErrorLog.hpp"


void report_error(std::string error_msg){
    
    std::ofstream errorFile;
    errorFile.open("error.log",std::fstream::app);
    errorFile<<error_msg<<std::endl;
    llvm::report_fatal_error(error_msg+"\n"); 
}
