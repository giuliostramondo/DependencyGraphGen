#ifndef ERROR_LOG_HPP
#define ERROR_LOG_HPP


#include <iostream>
#include <fstream>
#include "llvm/Support/ErrorHandling.h"

void report_error(std::string error_msg);
#endif
