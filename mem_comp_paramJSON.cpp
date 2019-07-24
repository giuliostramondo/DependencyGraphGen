#include "mem_comp_paramJSON.hpp"

mem_comp_paramJSON_format parse_mem_comp_paramJSON(const char *filename){
	std::ifstream confFileStream(filename);
	if(!confFileStream.good()){
		std::cout<<"Could not open file:"<< filename<<"\n";
	}
	std::stringstream buffer;
	buffer<< confFileStream.rdbuf();
	mem_comp_paramJSON_format param_out = initConf();
	Expected<json::Value> param = json::parse(buffer.str());
	if(param){
		json::Object* O = param->getAsObject();
			if(Optional<int64_t> data_width = O->getInteger("data_width")){
				if(data_width.hasValue()){ 
					param_out.data_width = data_width.getValue();
				}else{ 
					param_out.data_width = 32;
				}
			}
			if(Optional<int64_t> overwrite_resouce_database = O->getInteger("overwrite_resouce_database")){
				if(overwrite_resouce_database.hasValue()){ 
					param_out.overwrite_resouce_database = overwrite_resouce_database.getValue();
				}else{ 
					param_out.overwrite_resouce_database = 0;
				}
			}
			if (json::Object* o_compute_param = O->getObject("compute_param")){
				if (json::Object* o_funtional_unit = o_compute_param->getObject("funtional_unit")){
					if (json::Object* o_add = o_funtional_unit->getObject("add")){
						if(Optional<int64_t> add_latency = o_add->getInteger("latency")){
							if(add_latency.hasValue()){ 
								param_out.compute_param.funtional_unit.add.latency = add_latency.getValue();
							}else{ 
								param_out.compute_param.funtional_unit.add.latency = 1;
							}
						}
						if(Optional<int64_t> add_area = o_add->getInteger("area")){
							if(add_area.hasValue()){ 
								param_out.compute_param.funtional_unit.add.area = add_area.getValue();
							}else{ 
								param_out.compute_param.funtional_unit.add.area = 2;
							}
						}
						if(Optional<int64_t> add_static_power = o_add->getInteger("static_power")){
							if(add_static_power.hasValue()){ 
								param_out.compute_param.funtional_unit.add.static_power = add_static_power.getValue();
							}else{ 
								param_out.compute_param.funtional_unit.add.static_power = 1;
							}
						}
						if(Optional<int64_t> add_dynamic_power = o_add->getInteger("dynamic_power")){
							if(add_dynamic_power.hasValue()){ 
								param_out.compute_param.funtional_unit.add.dynamic_power = add_dynamic_power.getValue();
							}else{ 
								param_out.compute_param.funtional_unit.add.dynamic_power = 2;
							}
						}
					}
					if (json::Object* o_mul = o_funtional_unit->getObject("mul")){
						if(Optional<int64_t> mul_latency = o_mul->getInteger("latency")){
							if(mul_latency.hasValue()){ 
								param_out.compute_param.funtional_unit.mul.latency = mul_latency.getValue();
							}else{ 
								param_out.compute_param.funtional_unit.mul.latency = 2;
							}
						}
						if(Optional<int64_t> mul_area = o_mul->getInteger("area")){
							if(mul_area.hasValue()){ 
								param_out.compute_param.funtional_unit.mul.area = mul_area.getValue();
							}else{ 
								param_out.compute_param.funtional_unit.mul.area = 4;
							}
						}
						if(Optional<int64_t> mul_static_power = o_mul->getInteger("static_power")){
							if(mul_static_power.hasValue()){ 
								param_out.compute_param.funtional_unit.mul.static_power = mul_static_power.getValue();
							}else{ 
								param_out.compute_param.funtional_unit.mul.static_power = 3;
							}
						}
						if(Optional<int64_t> mul_dynamic_power = o_mul->getInteger("dynamic_power")){
							if(mul_dynamic_power.hasValue()){ 
								param_out.compute_param.funtional_unit.mul.dynamic_power = mul_dynamic_power.getValue();
							}else{ 
								param_out.compute_param.funtional_unit.mul.dynamic_power = 5;
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
			if (json::Object* o_resource_database = O->getObject("resource_database")){
				if(Optional<int64_t> resource_database_clock_frequency = o_resource_database->getInteger("clock_frequency")){
					if(resource_database_clock_frequency.hasValue()){ 
						param_out.resource_database.clock_frequency = resource_database_clock_frequency.getValue();
					}else{ 
						param_out.resource_database.clock_frequency = 1000;
					}
				}
				if(Optional<int64_t> resource_database_bitwidth_adder = o_resource_database->getInteger("bitwidth_adder")){
					if(resource_database_bitwidth_adder.hasValue()){ 
						param_out.resource_database.bitwidth_adder = resource_database_bitwidth_adder.getValue();
					}else{ 
						param_out.resource_database.bitwidth_adder = 64;
					}
				}
				if(Optional<int64_t> resource_database_bitwidth_multiplier = o_resource_database->getInteger("bitwidth_multiplier")){
					if(resource_database_bitwidth_multiplier.hasValue()){ 
						param_out.resource_database.bitwidth_multiplier = resource_database_bitwidth_multiplier.getValue();
					}else{ 
						param_out.resource_database.bitwidth_multiplier = 32;
					}
				}
				if(Optional<int64_t> resource_database_bitwidth_register_file = o_resource_database->getInteger("bitwidth_register_file")){
					if(resource_database_bitwidth_register_file.hasValue()){ 
						param_out.resource_database.bitwidth_register_file = resource_database_bitwidth_register_file.getValue();
					}else{ 
						param_out.resource_database.bitwidth_register_file = 64;
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

mem_comp_paramJSON_format initConf(){
	mem_comp_paramJSON_format defaultConf; 
	defaultConf.data_width = 32;
	defaultConf.overwrite_resouce_database = 0;
	defaultConf.compute_param.funtional_unit.add.latency = 1;
	defaultConf.compute_param.funtional_unit.add.area = 2;
	defaultConf.compute_param.funtional_unit.add.static_power = 1;
	defaultConf.compute_param.funtional_unit.add.dynamic_power = 2;
	defaultConf.compute_param.funtional_unit.mul.latency = 2;
	defaultConf.compute_param.funtional_unit.mul.area = 4;
	defaultConf.compute_param.funtional_unit.mul.static_power = 3;
	defaultConf.compute_param.funtional_unit.mul.dynamic_power = 5;
	defaultConf.memory_param.mram.read_latency = 2;
	defaultConf.memory_param.mram.write_latency = 5;
	defaultConf.memory_param.mram.area = 3;
	defaultConf.memory_param.mram.static_power = 0;
	defaultConf.memory_param.mram.dynamic_read_power = 2;
	defaultConf.memory_param.mram.dynamic_write_power = 5;
	defaultConf.memory_param.sram.read_latency = 1;
	defaultConf.memory_param.sram.write_latency = 1;
	defaultConf.memory_param.sram.area = 6;
	defaultConf.memory_param.sram.static_power = 4;
	defaultConf.memory_param.sram.dynamic_read_power = 2;
	defaultConf.memory_param.sram.dynamic_write_power = 2;
	defaultConf.resource_database.clock_frequency = 1000;
	defaultConf.resource_database.bitwidth_adder = 64;
	defaultConf.resource_database.bitwidth_multiplier = 32;
	defaultConf.resource_database.bitwidth_register_file = 64;
	return defaultConf;
}
