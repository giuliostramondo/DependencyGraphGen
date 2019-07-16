#ifndef MEM_COMP_PARAMJSON_HPP
#define MEM_COMP_PARAMJSON_HPP

#include "llvm/Support/JSON.h" //to parse raw JSON
#include <fstream> //to write/read files
#include <iostream> //to use cout
#include <sstream> //to use stringstream

using namespace llvm;

typedef struct mem_comp_paramJSON_format_t{
	int data_width;
	struct compute_param_t{
		struct funtional_unit_t{
			struct add_t{
				int latency;
				int area;
				int static_power;
				int dynamic_power;
			}add;
			struct mul_t{
				int latency;
				int area;
				int static_power;
				int dynamic_power;
			}mul;
		}funtional_unit;
	}compute_param;
	struct memory_param_t{
		struct mram_t{
			int read_latency;
			int write_latency;
			int area;
			int static_power;
			int dynamic_read_power;
			int dynamic_write_power;
		}mram;
		struct sram_t{
			int read_latency;
			int write_latency;
			int area;
			int static_power;
			int dynamic_read_power;
			int dynamic_write_power;
		}sram;
	}memory_param;
} mem_comp_paramJSON_format;

mem_comp_paramJSON_format parse_mem_comp_paramJSON(const char *filename);
mem_comp_paramJSON_format initConf();

#endif
