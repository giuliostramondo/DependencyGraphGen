#ifndef MEM_COMP_PARAMJSON_HPP
#define MEM_COMP_PARAMJSON_HPP

#include "llvm/Support/JSON.h" //to parse raw JSON
#include <fstream> //to write/read files
#include <iostream> //to use cout
#include <sstream> //to use stringstream

using namespace llvm;

typedef struct mem_comp_paramJSON_format_t{
	struct compute_param_t{
		int data_width;
		struct funtional_unit_t{
			struct add_32_t{
				int latency;
				int area;
				int static_power;
				int dynamic_power;
			}add_32;
			struct mul_32_t{
				int latency;
				int area;
				int static_power;
				int dynamic_power;
			}mul_32;
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

mem_comp_paramJSON_format parse_json_file(const char *filename){
	std::ifstream confFileStream(filename);
	if(!confFileStream.good()){
		std::cout<<"Could not open file:"<< filename<<"\n";
	}
	std::stringstream buffer;
	buffer<< confFileStream.rdbuf();
	mem_comp_paramJSON_format param_out;
	Expected<json::Value> param = json::parse(buffer.str());
	if(param){
		json::Object* O = param->getAsObject();
			if (json::Object* o_compute_param = O->getObject("compute_param")){
				if(Optional<int64_t> compute_param_data_width = o_compute_param->getInteger("data_width")){
					if(compute_param_data_width.hasValue()){ 
						param_out.compute_param.data_width = compute_param_data_width.getValue();
					}else{ 
						param_out.compute_param.data_width = 32;
					}
				}
				if (json::Object* o_funtional_unit = o_compute_param->getObject("funtional_unit")){
					if (json::Object* o_add_32 = o_funtional_unit->getObject("add_32")){
						if(Optional<int64_t> add_32_latency = o_add_32->getInteger("latency")){
							if(add_32_latency.hasValue()){ 
								param_out.compute_param.funtional_unit.add_32.latency = add_32_latency.getValue();
							}else{ 
								param_out.compute_param.funtional_unit.add_32.latency = 1;
							}
						}
						if(Optional<int64_t> add_32_area = o_add_32->getInteger("area")){
							if(add_32_area.hasValue()){ 
								param_out.compute_param.funtional_unit.add_32.area = add_32_area.getValue();
							}else{ 
								param_out.compute_param.funtional_unit.add_32.area = 2;
							}
						}
						if(Optional<int64_t> add_32_static_power = o_add_32->getInteger("static_power")){
							if(add_32_static_power.hasValue()){ 
								param_out.compute_param.funtional_unit.add_32.static_power = add_32_static_power.getValue();
							}else{ 
								param_out.compute_param.funtional_unit.add_32.static_power = 1;
							}
						}
						if(Optional<int64_t> add_32_dynamic_power = o_add_32->getInteger("dynamic_power")){
							if(add_32_dynamic_power.hasValue()){ 
								param_out.compute_param.funtional_unit.add_32.dynamic_power = add_32_dynamic_power.getValue();
							}else{ 
								param_out.compute_param.funtional_unit.add_32.dynamic_power = 2;
							}
						}
					}
					if (json::Object* o_mul_32 = o_funtional_unit->getObject("mul_32")){
						if(Optional<int64_t> mul_32_latency = o_mul_32->getInteger("latency")){
							if(mul_32_latency.hasValue()){ 
								param_out.compute_param.funtional_unit.mul_32.latency = mul_32_latency.getValue();
							}else{ 
								param_out.compute_param.funtional_unit.mul_32.latency = 2;
							}
						}
						if(Optional<int64_t> mul_32_area = o_mul_32->getInteger("area")){
							if(mul_32_area.hasValue()){ 
								param_out.compute_param.funtional_unit.mul_32.area = mul_32_area.getValue();
							}else{ 
								param_out.compute_param.funtional_unit.mul_32.area = 4;
							}
						}
						if(Optional<int64_t> mul_32_static_power = o_mul_32->getInteger("static_power")){
							if(mul_32_static_power.hasValue()){ 
								param_out.compute_param.funtional_unit.mul_32.static_power = mul_32_static_power.getValue();
							}else{ 
								param_out.compute_param.funtional_unit.mul_32.static_power = 3;
							}
						}
						if(Optional<int64_t> mul_32_dynamic_power = o_mul_32->getInteger("dynamic_power")){
							if(mul_32_dynamic_power.hasValue()){ 
								param_out.compute_param.funtional_unit.mul_32.dynamic_power = mul_32_dynamic_power.getValue();
							}else{ 
								param_out.compute_param.funtional_unit.mul_32.dynamic_power = 5;
							}
						}
					}
				}
			}
			if (json::Object* o_memory_param = O->getObject("memory_param")){
				if (json::Object* o_mram = o_memory_param->getObject("mram")){
					if(Optional<int64_t> mram_read_latency = o_mram->getInteger("read_latency")){
						if(mram_read_latency.hasValue()){ 
							param_out.memory_param.mram.read_latency = mram_read_latency.getValue();
						}else{ 
							param_out.memory_param.mram.read_latency = 2;
						}
					}
					if(Optional<int64_t> mram_write_latency = o_mram->getInteger("write_latency")){
						if(mram_write_latency.hasValue()){ 
							param_out.memory_param.mram.write_latency = mram_write_latency.getValue();
						}else{ 
							param_out.memory_param.mram.write_latency = 5;
						}
					}
					if(Optional<int64_t> mram_area = o_mram->getInteger("area")){
						if(mram_area.hasValue()){ 
							param_out.memory_param.mram.area = mram_area.getValue();
						}else{ 
							param_out.memory_param.mram.area = 3;
						}
					}
					if(Optional<int64_t> mram_static_power = o_mram->getInteger("static_power")){
						if(mram_static_power.hasValue()){ 
							param_out.memory_param.mram.static_power = mram_static_power.getValue();
						}else{ 
							param_out.memory_param.mram.static_power = 0;
						}
					}
					if(Optional<int64_t> mram_dynamic_read_power = o_mram->getInteger("dynamic_read_power")){
						if(mram_dynamic_read_power.hasValue()){ 
							param_out.memory_param.mram.dynamic_read_power = mram_dynamic_read_power.getValue();
						}else{ 
							param_out.memory_param.mram.dynamic_read_power = 2;
						}
					}
					if(Optional<int64_t> mram_dynamic_write_power = o_mram->getInteger("dynamic_write_power")){
						if(mram_dynamic_write_power.hasValue()){ 
							param_out.memory_param.mram.dynamic_write_power = mram_dynamic_write_power.getValue();
						}else{ 
							param_out.memory_param.mram.dynamic_write_power = 5;
						}
					}
				}
				if (json::Object* o_sram = o_memory_param->getObject("sram")){
					if(Optional<int64_t> sram_read_latency = o_sram->getInteger("read_latency")){
						if(sram_read_latency.hasValue()){ 
							param_out.memory_param.sram.read_latency = sram_read_latency.getValue();
						}else{ 
							param_out.memory_param.sram.read_latency = 1;
						}
					}
					if(Optional<int64_t> sram_write_latency = o_sram->getInteger("write_latency")){
						if(sram_write_latency.hasValue()){ 
							param_out.memory_param.sram.write_latency = sram_write_latency.getValue();
						}else{ 
							param_out.memory_param.sram.write_latency = 1;
						}
					}
					if(Optional<int64_t> sram_area = o_sram->getInteger("area")){
						if(sram_area.hasValue()){ 
							param_out.memory_param.sram.area = sram_area.getValue();
						}else{ 
							param_out.memory_param.sram.area = 6;
						}
					}
					if(Optional<int64_t> sram_static_power = o_sram->getInteger("static_power")){
						if(sram_static_power.hasValue()){ 
							param_out.memory_param.sram.static_power = sram_static_power.getValue();
						}else{ 
							param_out.memory_param.sram.static_power = 4;
						}
					}
					if(Optional<int64_t> sram_dynamic_read_power = o_sram->getInteger("dynamic_read_power")){
						if(sram_dynamic_read_power.hasValue()){ 
							param_out.memory_param.sram.dynamic_read_power = sram_dynamic_read_power.getValue();
						}else{ 
							param_out.memory_param.sram.dynamic_read_power = 2;
						}
					}
					if(Optional<int64_t> sram_dynamic_write_power = o_sram->getInteger("dynamic_write_power")){
						if(sram_dynamic_write_power.hasValue()){ 
							param_out.memory_param.sram.dynamic_write_power = sram_dynamic_write_power.getValue();
						}else{ 
							param_out.memory_param.sram.dynamic_write_power = 2;
						}
					}
				}
			}
			}
			else{
				if(auto Err =handleErrors(param.takeError(),[](const json::ParseError &PE){
					errs()<< "Couldn't parse the parameter file\n";
					}) )
				errs()<<"Unexpected Error\n";
			}
return param_out;
}

#endif
