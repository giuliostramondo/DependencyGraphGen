#include "FunctionalUnit.hpp"

void FunctionalUnit::operator=( const FunctionalUnit &F){
    opCode = F.opCode;
    label = F.label;
    ddg = F.ddg;
    extra_description = F.extra_description;
    earliest_free_slot = F.earliest_free_slot;
    config = F.config;
    ddg = F.ddg;
    registerFile=F.registerFile;
    //registerFileDepth = F.registerFileDepth;
    //instructionMemoryDepth = F.instructionMemoryDepth;
}

double FunctionalUnit::getRFArea(int rF_bitwidth,int rF_Depth){
        int ClockFreq=
               config.resource_database.clock_frequency;
        int technology = 
            config.resource_database.technology;
        double result=resources_database::getRegisterFileArea(
                   rF_Depth, ClockFreq, rF_bitwidth,technology);  
        if(result ==NO_RESULTS){
            result = getFromModelRegisterFileArea(rF_bitwidth,
                          ClockFreq, rF_Depth,technology); 
             if(result == 0){
                       report_error("Missing register file info in register file model:\nclock: "+std::to_string(ClockFreq)+"\ndepth: "+std::to_string(rF_Depth)+"\ntechnology: "+std::to_string(technology)+"nm"+"\nbitwidth:"+std::to_string(rF_bitwidth));
               }
        }
         return result;
}

double FunctionalUnit::getRFDynPow(int rF_bitwidth,int rF_Depth){
    int ClockFreq=
               config.resource_database.clock_frequency;
    int technology = 
        config.resource_database.technology;
    double result=resources_database::getRegisterFileActiveEnergy(
        rF_Depth, ClockFreq, rF_bitwidth, technology);
   if(result==NO_RESULTS){
        result=getFromModelRegisterFileActiveEnergy(rF_bitwidth,
                ClockFreq, rF_Depth, technology);
       if(result == 0){
           report_error("Missing register file info in register file model:\nclock: "+std::to_string(ClockFreq)+"\ndepth: "+std::to_string(rF_Depth)+"\ntechnology: "+std::to_string(technology)+"nm"+"\nbitwidth:"+std::to_string(rF_bitwidth));
           
       }
   }
   return result;
}
double FunctionalUnit::getRFStaticPow(int rF_bitwidth,int rF_Depth){
    int ClockFreq=
               config.resource_database.clock_frequency;
    int technology = 
        config.resource_database.technology;
    double result=resources_database::getRegisterFileIdleEnergy(
        rF_Depth, ClockFreq, rF_bitwidth, technology);
   if(result==NO_RESULTS){
        result=getFromModelRegisterFileIdleEnergy(rF_bitwidth,
                ClockFreq, rF_Depth, technology);
       if(result == 0){
           report_error("Missing register file info in register file model:\nclock: "+std::to_string(ClockFreq)+"\ndepth: "+std::to_string(rF_Depth)+"\ntechnology: "+std::to_string(technology)+"nm"+"\nbitwidth:"+std::to_string(rF_bitwidth));
           
       }
   }
   return result;
}
double FunctionalUnit::getArea(){
    vertex_t firstInstruction = front();
    Instruction* I = ddg[firstInstruction].inst;
    double area;
    if(isa<LoadInst>(I)||isa<StoreInst>(I)){
        //Register File component of the area
        area =getVertexArea(ddg,firstInstruction,config,registerFile.size()); 
        //Instruction Memory component of the area (#of instructions)
        int instructionMemoryBitwidth = 16;
        int instructionMemoryDepth=size();
        double result = getRFArea(instructionMemoryBitwidth,instructionMemoryDepth); 
         area += result;
    }else{
        if(isa<BinaryOperator>(I)){
            //BinaryOp area component
            area =getVertexArea(ddg,firstInstruction,config,0);
            //Register File component of the area
            if(registerFile.size()>0){
                int registerFileBitwidth = config.resource_database.bitwidth_register_file;
                int registerFileDepth=registerFile.size();
                double result = getRFArea(registerFileBitwidth,registerFileDepth); 
                 area += result;
            }
            //InstructionMemory component of area
                int registerFileBitwidth = 32;
                int registerFileDepth=size();
                double result = getRFArea(registerFileBitwidth,registerFileDepth); 
                area += result;
        }
    }
    return area;
}

int FunctionalUnit::getLatency(){
    vertex_t firstInstruction = front();
    int latency =getVertexLatency(ddg,firstInstruction,config); 
    return latency;
}

int FunctionalUnit::getTotalAccessesToRF(){
    int readAccesses=0;
    vertex_t firstInstruction = front();
    Instruction* I = ddg[firstInstruction].inst;
    for(unsigned i=0;i<registerFile.size();i++){
        readAccesses+=registerFile[i].size();
    }

    if(isa<LoadInst>(I)){
        //If this is a load FU the number of write might be
        //less than the number of reads (reused data)
        int writeAccesses=0;
        std::set<std::pair<std::string,int>> uniqueWrites;
        for(unsigned i=0;i<registerFile.size();i++){
            for(auto vertex_it = registerFile[i].begin();
                    vertex_it!= registerFile[i].end();
                    vertex_it++){
               std::string arrayName=ddg[*vertex_it].arrayName;
               int arrayOffset=ddg[*vertex_it].arrayOffset;
               uniqueWrites.insert(std::make_pair(arrayName,arrayOffset)); 
            }
        }
        writeAccesses =uniqueWrites.size();
        return readAccesses+writeAccesses;
    }
    //If it is not a load FU, the number of accesses to each 
    //register are 1 write and 1 read per instruction
    return 2*readAccesses;
}

double FunctionalUnit::getDynamicPower(){
    vertex_t firstInstruction = front();
    Instruction* I = ddg[firstInstruction].inst;
    int totalAccessesToRF=getTotalAccessesToRF();
    double dyn_power=0;
    if(isa<LoadInst>(I)||isa<StoreInst>(I)){
        if(registerFile.size()>0){
            //registerFileComponent of the Dyn Pow 
            int registerFileBitwidth = config.resource_database.bitwidth_register_file;
            int registerFileDepth=registerFile.size();
            dyn_power= totalAccessesToRF*getRFDynPow(registerFileBitwidth,registerFileDepth); 
        }
        //instruction memory component of Dyn Pow
        int instructionMemoryBitwidth = 16;
        int instructionMemoryDepth=size();
        //Each instruction in the instruction memory is accessed in read only once
        dyn_power+=
            instructionMemoryDepth*getRFDynPow(instructionMemoryBitwidth,instructionMemoryDepth);
    }else{
        if(isa<BinaryOperator>(I)){
            //BinaryOpComponent
            dyn_power =size()*getVertexDynamicPower(ddg,firstInstruction,config,0); 
            if(registerFile.size()>0){
                //registerFileComponent of the Dyn Pow 
                int registerFileBitwidth = config.resource_database.bitwidth_register_file;
                int registerFileDepth=registerFile.size();
                dyn_power+= totalAccessesToRF*getRFDynPow(registerFileBitwidth,registerFileDepth); 
            }
            //instruction memory component
            int instructionMemoryBitwidth = 32;
            int instructionMemoryDepth=size();
            dyn_power+=
            instructionMemoryDepth*getRFDynPow(instructionMemoryBitwidth,instructionMemoryDepth); 
        }
    }
   return dyn_power; 
}

double FunctionalUnit::getStaticPower(){
    vertex_t firstInstruction = front();
    double static_power =0; 
    Instruction* I = ddg[firstInstruction].inst;
    if(isa<LoadInst>(I)||isa<StoreInst>(I)){
        if(registerFile.size()>0){
            //registerFileComponent of the Static Pow 
            int registerFileBitwidth = config.resource_database.bitwidth_register_file;
            int registerFileDepth=registerFile.size();
            static_power= getRFStaticPow(registerFileBitwidth,registerFileDepth); 
        }
        //instruction memory component
            int instructionMemoryBitwidth = 16;
            int instructionMemoryDepth=size();
            static_power+=getRFStaticPow(instructionMemoryBitwidth,instructionMemoryDepth); 
    }else{
        if(isa<BinaryOperator>(I)){
            //BinaryOpComponent
            static_power = getVertexStaticPower(ddg,firstInstruction,config,0); 
            if(registerFile.size()>0){
                //registerFileComponent of the Dyn Pow 
                int registerFileBitwidth = config.resource_database.bitwidth_register_file;
                int registerFileDepth=registerFile.size();
                static_power+= getRFStaticPow(registerFileBitwidth,registerFileDepth); 
            }
            //instruction memory component
            int instructionMemoryBitwidth = 32;
            int instructionMemoryDepth=size();
            static_power+=getRFDynPow(instructionMemoryBitwidth,instructionMemoryDepth); 
        }
    }
    return static_power;
}

void FunctionalUnit::computeRegisterFileAndInstructionMemorySize(){
    vertex_t firstInstruction = front();
    Instruction* I = ddg[firstInstruction].inst;
    //If this FU is a L1 bank
    //The size of its RF will be the number
    //of element for which the clock cycle at 
    //which they arrive from L2 (schedule[ASAP]) is different from 
    //the clock cycle at which it is read schedule[Architectural]
    std::vector<unsigned> earliest_free_slot_RF=std::vector<unsigned>();
    if(isa<LoadInst>(I)){
        //sort instruction by accessed elements (so that different 
        //accesses from different instructions to the same elements are
        //close)
        sort([this](vertex_t a, vertex_t b){
               return ddg[a].schedules[ASAP]< ddg[b].schedules[ASAP];
               });
       for(auto vert_it = begin();
             vert_it!=end(); vert_it++){
           if(ddg[*vert_it].schedules[ASAP] != 
                   vertexToClock[*vert_it]){
               bool allocated=false;
               //verify if same element is already assigned to a register
               
               for(unsigned i=0;i<registerFile.size();i++){
                   std::list<vertex_t> reg_vtx_list = registerFile[i];
                   for(auto vert_it_j = reg_vtx_list.begin();
                           vert_it_j != reg_vtx_list.end();
                           vert_it_j++){
                       if(ddg[*vert_it].arrayName == ddg[*vert_it_j].arrayName &&
                               ddg[*vert_it].arrayOffset == ddg[*vert_it_j].arrayOffset){
                           if(earliest_free_slot_RF[i]<vertexToClock[*vert_it])
                               earliest_free_slot_RF[i] = vertexToClock[*vert_it];
                            registerFile[i].push_back(*vert_it);
                            allocated=true;
                       }
                   }
               }
               if(allocated)
                   continue;
               for(unsigned i=0;i<registerFile.size();i++){
                    if(ddg[*vert_it].schedules[ASAP]>=
                            earliest_free_slot_RF[i]){
                        earliest_free_slot_RF[i]=vertexToClock[*vert_it];
                        registerFile[i].push_back(*vert_it);
                        allocated=true;
                    }
               }
               if(!allocated){
                    std::list<vertex_t> newRF;
                    newRF.push_back(*vert_it);
                    registerFile.push_back(newRF);
                    earliest_free_slot_RF.push_back(vertexToClock[*vert_it]);

               }
           }
       }  
    }else{
        //IF this FU is a functional unit its register file size
        //will store all the incoming ( and resulting ) elements
        //that will be used in the computation at a later cycle.
        if(isa<BinaryOperator>(I)){
           //get list of incoming (from outside or reuse) elements
            //Tuple < VertexID, ArrivalClock, UsageClock>
            std::list<std::tuple<vertex_t,unsigned,unsigned>> maybeToStore;
            for(auto vert_it = begin();
                 vert_it!=end(); vert_it++){   
                //A FU needs to store only self edges
                //i.e. instructions it computes whose result
                //needs to be reused.
                out_edge_it_t out,out_end;       
                for(boost::tie(out,out_end) = out_edges(*vert_it,ddg); out!= out_end;++out){
                    int v_target_id = target(*out,ddg);
                    //if it is a self edge
                    FunctionalUnit& targetFU=vertexToFU.find(v_target_id)->second;
                    if(targetFU.label == label){
                        //if result is not immediatly used
                        unsigned arrivalClock= vertexToClock[*vert_it]+
                                    getVertexLatency(ddg,*vert_it,config);
                        unsigned usageClock=vertexToClock[v_target_id];
                        if(usageClock > arrivalClock){
                            maybeToStore.push_back(
                                    std::make_tuple(*vert_it,arrivalClock,usageClock)
                                        );
                        }
                    }
                }
                //A FU needs to store incoming data which are not
                //directly used for computation
                in_edge_it_t j, j_end;
                for(boost::tie(j,j_end) = in_edges(*vert_it,ddg); j!= j_end;++j){
                    int v_source_id = source(*j,ddg);
                    //if element comes from another FU
                    FunctionalUnit& sourceFU = vertexToFU.find(v_source_id)->second;
                    if(sourceFU.label != label){
                        //if element is not immediately used
                        unsigned arrivalClock= vertexToClock[v_source_id]+
                                    getVertexLatency(ddg,v_source_id,config);
                        unsigned usageClock=vertexToClock[*vert_it];
                        if(usageClock>arrivalClock)
                        {
                            maybeToStore.push_back(
                                    std::make_tuple(v_source_id,arrivalClock,usageClock)
                                        );
                        }
                    }

                }
                
            }
            maybeToStore.sort([](std::tuple<vertex_t, unsigned, unsigned> a, 
                                std::tuple<vertex_t, unsigned, unsigned> b){
                                unsigned clockArrival_a=std::get<1>(a);
                                unsigned clockArrival_b = std::get<1>(b);
                                return clockArrival_a< clockArrival_b;
                            });
           // std::sort(maybeToStore.begin(),
           //             maybeToStore.end(),
           //                 [](std::tuple<vertex_t, unsigned, unsigned> a, 
           //                     std::tuple<vertex_t, unsigned, unsigned> b){
           //                     unsigned clockArrival_a=std::get<1>(a);
           //                     unsigned clockArrival_b = std::get<1>(b);
           //                     return clockArrival_a< clockArrival_b;
           //                 });
            for(auto element = maybeToStore.begin();
                    element!= maybeToStore.end(); element++){
                //look for compatible register
                bool allocated = false;
               for(unsigned i=0;i<registerFile.size();i++){
                    if(std::get<1>(*element) >
                            earliest_free_slot_RF[i]){
                        registerFile[i].push_back(std::get<0>(*element)); 
                        earliest_free_slot_RF[i]=std::get<2>(*element);
                        allocated=true;
                        break;
                    }
               }
               if(!allocated){
                    std::list<vertex_t> newRF;
                    newRF.push_back(std::get<0>(*element));
                    registerFile.push_back(newRF);
                    earliest_free_slot_RF.push_back(std::get<2>(*element)); 
                    
               }
                
            }

        }else {
            if(isa<StoreInst>(I)){
                //Every element needs to be stored in its own register
                //to collect all the results
                for(auto vert_it = begin();
                    vert_it!=end(); vert_it++){
                    std::list<vertex_t> newReg;
                    newReg.push_back(*vert_it);   
                    registerFile.push_back(newReg);
                }
            }

        }
    }
}

void FunctionalUnit::DumpRegisterFileAllocation(std::string fileBaseName){
    if(registerFile.size()==0)
        return;
    std::string fileName=fileBaseName+"_"+label+"_rf_allocation.csv";
    std::ofstream outFile;
    outFile.open(fileName);
    outFile<<"Reg_Address,Instruction\n";
    //to check and complete 
    for(unsigned i=0; i< registerFile.size();i++){
        for(auto vert= registerFile[i].begin();
                vert != registerFile[i].end(); vert++){
            outFile<<std::to_string(i)+","+"("+std::to_string(*vert)+
                ") "+ ddg[*vert].name+"\n";
        }
    }
    outFile.close();
}

void FunctionalUnit::appendFUInfo(std::string fileName){
    vertex_t firstInstruction = front();
    Instruction* I = ddg[firstInstruction].inst;
    std::ofstream file;
    file.open(fileName,std::fstream::app);
    //Label,OpArea, OpStaticEnergy, OpDynEnergy,InstMemBitwidth, InstMemDepth,InstMemArea, InstMemStaticEn
    //InstMemDynEn,InternalRegFileBitwidth,InternalRegFileDepth,InternalRegFileAccesses,
    //InternalRegFileArea, InternalRegFileStaticEn, InternalRegFileDynEn,TotalFUArea,TotalFUStaticEn,TotalFUDynEn
    file<<label<<",";
    int instructionMemBitwidth=0; 
    if(isa<LoadInst>(I)||isa<StoreInst>(I)){
        file<<"0,0,0,";
        instructionMemBitwidth=16;
    }else{
        file<<std::to_string(getVertexArea(ddg,firstInstruction,config,0))<<",";
        file<<std::to_string(getVertexStaticPower(ddg,firstInstruction,config,0))<<",";
        file<<std::to_string(getVertexDynamicPower(ddg,firstInstruction,config,0))<<",";
        instructionMemBitwidth=32;
    }//InstMEm
        file<<std::to_string(instructionMemBitwidth)<<",";
        file<<std::to_string(size())<<",";
        file<<std::to_string(getRFArea(instructionMemBitwidth,size()))<<","; 
        file<<std::to_string(getRFStaticPow(instructionMemBitwidth,size()))<<","; 
        file<<std::to_string(getRFDynPow(instructionMemBitwidth,size()))<<",";
       //internalRegisterFile 
        if(registerFile.size()>0){
            //registerFileComponent of the Dyn Pow 
            int registerFileBitwidth = config.resource_database.bitwidth_register_file;
            file<<std::to_string(registerFileBitwidth)<<",";
            int registerFileDepth=registerFile.size();
            file<<std::to_string(registerFileDepth)<<",";
            int totalAccessesToRF=getTotalAccessesToRF();
            file<<std::to_string(totalAccessesToRF)<<",";
            file<<std::to_string(getRFArea(registerFileBitwidth,registerFileDepth))<<","; 
            file<<std::to_string(getRFStaticPow(registerFileBitwidth,registerFileDepth))<<","; 
            file<<std::to_string(totalAccessesToRF*getRFDynPow(registerFileBitwidth,registerFileDepth))<<","; 
        }else{
            file<<"0,0,0,0,0,0,";
        }
            file<<std::to_string(getArea())<<","; 
            file<<std::to_string(getStaticPower())<<","; 
            file<<std::to_string(getDynamicPower()); 
    file<<"\n"; 
    file.close(); 
}
