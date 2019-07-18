#include "Architecture.hpp"


// Below function is deprecated use generateSmallestArchitecturalMapping instead
void Architecture::generateArchitecturalMapping(){
    errs()<<"Generating architectural mappings\n";
    vertex_it_t vi,vi_end,next;
    boost::tie(vi,vi_end) = vertices(ddg);
    //iterate over all vertices 
    for(next=vi; vi !=vi_end;vi=next){
        ++next;
        //create FU
        //std::list<vertex_t> newFUnit;
        FunctionalUnit newFUnit;
        // Add Instruction to new FU
        newFUnit.push_back(*vi); 
        Instruction *I=ddg[*vi].inst; 
        errs()<<"Vertex: "<<ddg[*vi].name<<"\n";
        unsigned opCode = I->getOpcode();
        newFUnit.opCode=opCode;
        if(units.find(opCode) != units.end()){
            //std::list<std::list<vertex_t>> fuList = units.find(opCode)->second;
            std::list<FunctionalUnit> fuList = units.find(opCode)->second;
            std::string fulabel=std::string(Instruction::getOpcodeName(opCode));
            fulabel+="_"+std::to_string(fuList.size());
            newFUnit.label=fulabel;
            fuList.push_back(newFUnit);
            units[opCode]=fuList; 
            for(auto it=fuList.begin(); it!=fuList.end();it++){
                for(auto it_FUnit = it->begin(); it_FUnit != it->end();it_FUnit++){
                    errs()<<ddg[*it_FUnit].name;
                }
            }errs()<<"\n";
        }else{
            std::list<FunctionalUnit> newFuList;
            std::string FULabel=std::string(Instruction::getOpcodeName(opCode));
            FULabel+="_"+std::to_string(0);
            newFUnit.label=FULabel;
            newFuList.push_back(newFUnit);
            units.insert(std::pair<unsigned,std::list<FunctionalUnit>>(opCode,newFuList));
        }
    }
}

//Greedy Interval Partitioning
void Architecture::generateSmallestArchitecturalMapping(){
    errs()<<"Generating architectural mappings\n";
    typedef std::list<vertex_t> InstructionOrder;
    InstructionOrder instruction_order;
    boost::topological_sort(ddg, std::front_inserter(instruction_order));
    for (InstructionOrder::iterator i = instruction_order.begin();
            i != instruction_order.end(); ++i){
        unsigned architecturalASAP=ddg[*i].schedules[ASAP];
        if (boost::in_degree (*i, ddg) > 0) {
            in_edge_it_t j, j_end;
            unsigned maxdist = 0;
            for(boost::tie(j,j_end) = in_edges(*i,ddg); j!= j_end;++j){
                int v_source_id = source(*j,ddg);
                unsigned sourceArchitecturalASAP=ddg[v_source_id].schedules[ARCHITECTURAL];
                maxdist = std::max(sourceArchitecturalASAP+getVertexLatency(ddg,v_source_id,config)-1,maxdist);
            }
            architecturalASAP=maxdist+1;
        }
        Instruction *I=ddg[*i].inst; 
        unsigned opCode = I->getOpcode();
        if(units.find(opCode) != units.end()){
            std::list<FunctionalUnit> fuList = units.find(opCode)->second;
            bool allocated=false;
            std::list<FunctionalUnit>::iterator it,next;
            //Check if instruction is compatible with existing FUs
            for(it=fuList.begin(); it!=fuList.end();it=next){
                next=it;
                next++;
                if(ddg[*i].schedules[ALAP]>=it->earliest_free_slot){
                    unsigned allocated_clock=it->earliest_free_slot;
                    if(architecturalASAP>allocated_clock){
                        allocated_clock=architecturalASAP;
                    }
                    ddg[*i].schedules[ARCHITECTURAL]=allocated_clock;
                    if(schedule_architectural.size() > it->earliest_free_slot){
                        schedule_architectural[it->earliest_free_slot].push_back(*i);
                    }else{
                        while(schedule_architectural.size() < architecturalASAP + 1){
                            std::list<vertex_t> current_cycle;
                            schedule_architectural.push_back(current_cycle);
                        }
                        schedule_architectural[architecturalASAP].push_back(*i);
                    }
                    it->earliest_free_slot=
                        ddg[*i].schedules[ARCHITECTURAL]+getVertexLatency(ddg,*i,config); 
                    it->push_back(*i);
                    ddg[*i].FU=it->label;
                    units[opCode]=fuList;
                    allocated=true;
                    break;
                }

            }
            if(!allocated){
                //Add new functional unit and allocate
                FunctionalUnit newFUnit;
                // Add Instruction to new FU
                newFUnit.push_back(*i); 
                ddg[*i].schedules[ARCHITECTURAL]=architecturalASAP;
                if(schedule_architectural.size() > architecturalASAP){
                    schedule_architectural[architecturalASAP].push_back(*i);
                }else{
                    while(schedule_architectural.size() < architecturalASAP +1){
                        std::list<vertex_t> current_cycle;
                        schedule_architectural.push_back(current_cycle);
                    }
                    schedule_architectural[architecturalASAP].push_back(*i);
                }
                newFUnit.earliest_free_slot=architecturalASAP+getVertexLatency(ddg,*i,config);
                newFUnit.opCode=opCode;
                std::string fulabel=std::string(Instruction::getOpcodeName(opCode));
                fulabel+="_"+std::to_string(fuList.size());
                newFUnit.label=fulabel;
                fuList.push_back(newFUnit);
                ddg[*i].FU=fulabel;
                units[opCode]=fuList;
            }
        }else{
            //Add opcode to map
            FunctionalUnit newFUnit;
            newFUnit.push_back(*i); 
            ddg[*i].schedules[ARCHITECTURAL]=architecturalASAP;
            if(schedule_architectural.size() > architecturalASAP){
                schedule_architectural[architecturalASAP].push_back(*i);
            }else{
                while(schedule_architectural.size() < architecturalASAP +1){
                    std::list<vertex_t> current_cycle;
                    schedule_architectural.push_back(current_cycle);
                }
                schedule_architectural[architecturalASAP].push_back(*i);
            }
            newFUnit.earliest_free_slot=architecturalASAP+getVertexLatency(ddg,*i,config);
            std::list<FunctionalUnit> newFuList;
            std::string FULabel=std::string(Instruction::getOpcodeName(opCode));
            FULabel+="_"+std::to_string(0);
            newFUnit.label=FULabel;
            newFuList.push_back(newFUnit);
            ddg[*i].FU=FULabel;
            units.insert(std::pair<unsigned,std::list<FunctionalUnit>>(opCode,newFuList));
        }


    }
}

void Architecture::describe(){
    std::map<unsigned,std::list<FunctionalUnit>>::iterator units_it;
    for(units_it = units.begin();units_it != units.end();units_it++){
        unsigned opCode = units_it->first;
        const char*  opCodeName = Instruction::getOpcodeName(opCode);
        std::list<FunctionalUnit> FUList = units_it->second; 
        size_t count = FUList.size();
        errs()<<count<<"x "<<opCodeName<<":\n";
        for(auto it=FUList.begin(); it!=FUList.end();it++){
            for(auto it_FUnit = it->begin(); it_FUnit != it->end();it_FUnit++){
                errs()<<ddg[*it_FUnit].name<<", ";    
            } 
            errs()<<"\n";
        }
    }
}

void Architecture::write_architecture_dot(std::string filename){
    std::ofstream output_dot_file;
    output_dot_file.open(filename);
    //Obtain Edges between the functional units
    typedef std::pair<std::string, std::string> edgeFU; 
    std::set<edgeFU> edgeFUset;
    std::map<unsigned,std::list<FunctionalUnit>>::iterator units_it;
    for(units_it = units.begin();units_it != units.end();units_it++){
        std::list<FunctionalUnit> FUList = units_it->second;
        for(auto it=FUList.begin(); it!=FUList.end();it++){
            std::string source_label=it->label;
            errs()<<"Checking Connections of: "<<source_label<<"\n";
            for(auto it_FUnit = it->begin(); it_FUnit != it->end();it_FUnit++){
                out_edge_it_t out_edge_it,out_edge_end;
                for(boost::tie(out_edge_it,out_edge_end) = boost::out_edges(*it_FUnit,ddg);
                        out_edge_it != out_edge_end; ++out_edge_it){
                    vertex_t target = boost::target(*out_edge_it,ddg);
                    std::string target_label= ddg[target].FU;
                    edgeFUset.insert(edgeFU(source_label,target_label));
                    errs()<<"adding edge: "<<source_label<<" -> "<<target_label<<"\n";
                }
            }
        }
    }
    output_dot_file << "digraph G{\n";         
    errs()<<"out of edge loop";
    for(units_it = units.begin();units_it != units.end();units_it++){
        std::list<FunctionalUnit> FUList = units_it->second;
        for(auto it=FUList.begin(); it!=FUList.end();it++){
            output_dot_file<<"\t"<<it->label<<"[label=\""<<it->label<<"\";shape=rectangle];\n";
        }

    }
    for(auto edge: edgeFUset){
        std::string source = edge.first;
        std::string target = edge.second;
        output_dot_file<<source<<"->"<<target<<";\n";
    }
    output_dot_file << "}\n";         
    output_dot_file.close();

}

void Architecture::write_dot(std::string filename){
    std::ofstream output_dot_file;
    output_dot_file.open(filename);
    output_dot_file<<"digraph G{\n\n";
    std::map<unsigned,std::list<FunctionalUnit>>::iterator units_it;
    unsigned cluster_number=0; 
    for(units_it = units.begin();units_it != units.end();units_it++){
        std::list<FunctionalUnit> FUList = units_it->second;
        for(auto it=FUList.begin(); it!=FUList.end();it++){
            output_dot_file << "subgraph cluster_"<<std::to_string(cluster_number++)  <<" {\n";
            output_dot_file << "\tnode [style=filled];\n";
            for(auto it_FUnit = it->begin(); it_FUnit != it->end();it_FUnit++){
                output_dot_file<<"\t"<<std::to_string(*it_FUnit)<<";\n";    
            }
            output_dot_file << "\tcolor=blue;\n";
            output_dot_file << "\tlabel = \" "<<it->label<<"\";\n";
            output_dot_file << "}\n"; 
        }
    }
    vertex_it_t vi,vi_end,next;
    boost::tie(vi,vi_end) = vertices(ddg);
    vertex_writer vert_writer = vertex_writer(ddg,std::map<vertex_t,std::string>(),ARCHITECTURAL);
    for(next=vi; vi !=vi_end;vi=next){
        ++next;
        output_dot_file << std::to_string(*vi);
        vert_writer(output_dot_file,*vi);
        output_dot_file <<";\n";
    }

    edges_writer ed_writer(ddg,std::map<edge_t,std::string>());
    edge_it_t edge_it,edge_end;
    for(boost::tie(edge_it,edge_end) = boost::edges(ddg);
            edge_it != edge_end; ++edge_it){
        vertex_t source = boost::source(*edge_it,ddg);
        vertex_t target= boost::target(*edge_it,ddg);
        output_dot_file <<std::to_string(source)<<"->"<<std::to_string(target);
        ed_writer(output_dot_file,*edge_it);
        output_dot_file << ";\n";

    }
    output_dot_file<<"}\n";
    output_dot_file.close();
}

void Architecture::performALAPSchedule(){
    //ALAP
    int latency= maxLatency; 
    std::vector<int> clocks_alap(num_vertices(ddg),0);
    typedef std::list<vertex_t> InstructionOrder;
    InstructionOrder instruction_order;
    boost::topological_sort(ddg, std::front_inserter(instruction_order));
    for (std::list<vertex_t>::reverse_iterator r_i = instruction_order.rbegin();
            r_i != instruction_order.rend(); ++r_i){
        if (boost::out_degree (*r_i, ddg) > 0) {
            out_edge_it_t out_j, out_j_end;
            int maxdist = 0;
            for(boost::tie(out_j,out_j_end) = out_edges(*r_i,ddg);out_j!= out_j_end;++out_j){
                int v_target_id = target(*out_j,ddg);
                errs()<<"v_target_id:"<<v_target_id<<", clocks_alap[v_target_id]:"<<clocks_alap[v_target_id]<<"\n";
                maxdist = std::max(clocks_alap[v_target_id]+(getVertexLatency(ddg,*r_i,config)-1),maxdist);
            }
            clocks_alap[*r_i]=maxdist+1;
            ddg[*r_i].schedules[ALAP] =latency-(maxdist+1);
            if(schedule_alap.size() >= (unsigned) maxdist+1){
                std::list<vertex_t> current_cycle = schedule_alap[maxdist];
                current_cycle.push_back(*r_i);
            }
            else{
                std::list<vertex_t> current_cycle;
                current_cycle.push_back(*r_i);
                schedule_alap.push_back(current_cycle);
            }
        }else{
            clocks_alap[*r_i]=0;
            ddg[*r_i].schedules[ALAP]=latency;
            if(schedule_alap.size() >= 1){
                std::list<vertex_t> current_cycle = schedule_alap[0];
                current_cycle.push_back(*r_i);
            }else{
                std::list<vertex_t> current_cycle;
                current_cycle.push_back(*r_i);
                schedule_alap.push_back(current_cycle);

            }
        }
    }
    std::reverse(schedule_alap.begin(),schedule_alap.end());
}

int Architecture::getMaxLatency(){
   return maxLatency; 
}

int Architecture::getActualMaxLatency(){
    return schedule_architectural.size();
}

int Architecture::getArea(){    
    int totalArea = 0;
    std::map<unsigned,std::list<FunctionalUnit>>::iterator units_it;
    for(units_it = units.begin();units_it != units.end();units_it++){
        std::list<FunctionalUnit> FUList = units_it->second;
        FunctionalUnit firstFU = FUList.front();
        int instanceArea = firstFU.getArea(ddg,config);
        int instancesArea = instanceArea * FUList.size();
        totalArea+= instancesArea;
    }
    return totalArea;
}

int Architecture::getStaticPower(){
    int totalStaticPower = 0;
    std::map<unsigned,std::list<FunctionalUnit>>::iterator units_it;
    for(units_it = units.begin();units_it != units.end();units_it++){
        std::list<FunctionalUnit> FUList = units_it->second;
        FunctionalUnit firstFU = FUList.front();
        int instanceStaticPower = firstFU.getArea(ddg,config);
        int instancesStaticPower = instanceStaticPower * FUList.size();
        totalStaticPower+= instancesStaticPower;
    }
    return totalStaticPower* getActualMaxLatency();
}

int Architecture::getDynamicPower(){
    int totalDynamicPower = 0;
    std::map<unsigned,std::list<FunctionalUnit>>::iterator units_it;
    for(units_it = units.begin();units_it != units.end();units_it++){
        std::list<FunctionalUnit> FUList = units_it->second;
        FunctionalUnit firstFU = FUList.front();
        int instanceDynamicPower = firstFU.getArea(ddg,config);

        int instancesDynamicPower = 0;
        for(auto it=FUList.begin(); it!=FUList.end();it++){
           instancesDynamicPower += instanceDynamicPower * it->size(); 
        }
        totalDynamicPower+= instancesDynamicPower;
    }
    return totalDynamicPower;
}
int Architecture::getTotalPower(){
    int dyn_pow = getDynamicPower();
    int stc_pow = getStaticPower();
    return dyn_pow + stc_pow;
}

void Architecture::appendArchInfoToCSV(std::string csvFileName){
    std::ofstream csvFile;
    csvFile.open(csvFileName,std::fstream::app);
    csvFile<<getMaxLatency()<<",";
    csvFile<<getActualMaxLatency()<<",";
    csvFile<<getArea()<<",";
    csvFile<<getStaticPower()<<",";
    csvFile<<getDynamicPower()<<",";
    csvFile<<getTotalPower()<<"\n";
    csvFile.close();
}

void Architecture::dumpSchedule(){
    for(unsigned i=0; i<schedule_architectural.size();i++){
        std::cout<<"Cycle "<<i<<" :";
        for(auto inst_it = schedule_architectural[i].begin();
               inst_it != schedule_architectural[i].end();
                ++inst_it){
            std::cout<<ddg[*inst_it].name<<", ";
        } 
        std::cout<<"\n";
    }
}
