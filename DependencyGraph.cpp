#include "DependencyGraph.hpp"

std::unordered_set<Instruction*> removedInstructions;

DependencyGraph::DependencyGraph(mem_comp_paramJSON_format _conf, BasicBlock *BB): ddg(0), config(_conf),l2_model(_conf){
            errs()<<"DependencyGraph constructor\n";
            errs()<<"Latency of MRAM read :"+std::to_string(config.memory_param.mram.read_latency)<<"\n";    
    populateGraph(BB);
} 
void DependencyGraph::replaceAndErase(Instruction *I){
    if(I==NULL)return;
    if(I->getParent() == NULL)return;
    if(removedInstructions.find(I) != removedInstructions.end())return;
    Value::user_iterator_impl<User> u_it,next;
    for (u_it = I->users().begin(); u_it != I->users().end(); u_it=next) {
        next=u_it;
        next++;
        Instruction* it_inner =(Instruction*) *u_it;
        replaceAndErase(it_inner);
    }
    I->replaceAllUsesWith(UndefValue::get(I->getType()));
    I->eraseFromParent();
    removedInstructions.insert(I);
}

std::string replaceAll(StringRef inString, char toReplace, char replacement){
    std::string str_data = inString.str();
    std::replace(str_data.begin(), str_data.end(),toReplace,replacement);
    return str_data;
}

void DependencyGraph::performArchitecturalDSE(std::string ParameterFilename,
                                               int OptSearchLimit ){
    
                bool firstArchitecture=true;
                int totalLatencyFromL2= l2_model.getNextAvail_L2_Cycle();
                for(unsigned i=schedule.size();i<=schedule_sequential.size()+totalLatencyFromL2;i++){
                    Architecture a(ddg,i, config,l2_model);
                    a.performALAPSchedule();
                    Architecture *curr_a;
                    if(OptSearchLimit!=0){
                        errs()<<"Selected the BETTER THAN GREEDY algorithm\n";
                        errs()<<"Optimization limit set to "<<OptSearchLimit<<"\n";
                        curr_a=a.generateSmallestArchitecturalMapping_Opt(OptSearchLimit);
                    }else{
                        errs()<<"Selected the GREEDY algorithm\n";
                        a.generateSmallestArchitecturalMapping_Heu();
                        curr_a=&a;
                    
                    }
                    if (firstArchitecture){
                        std::ofstream csvFile;
                        csvFile.open(std::string(ParameterFilename.c_str())+".arch_info.csv");
                        csvFile<<"MaxLatency,ActualMaxLatency,Area,StaticPower,DynamicPower,TotalEnergy,";
                        csvFile<<curr_a->getCSVResourceHeader()<<"\n";
                        csvFile.close();
                        firstArchitecture=false;
                    }

                    //a.describe();
                    curr_a->computeIdleAndWriteBack_L2_Ops();
                    curr_a->computeRegisterFileAndInstructionMemorySize();
                    curr_a->dumpSchedule();
                    std::string baseFileName = std::string("Architecture_latency_");
                    baseFileName += std::to_string(i);
                    std::string arcFileName=baseFileName+".dot";
                    curr_a->write_dot(arcFileName);
                    std::string arc_schemeFilename=baseFileName+"_schematic.dot";
                    curr_a->write_architecture_dot(arc_schemeFilename);
                    curr_a->appendArchInfoToCSV(std::string(ParameterFilename.c_str())+".arch_info.csv");
                    std::string arc_l2ControllerFilename= baseFileName+ "_l2_memory_controller.csv";
                    curr_a->l2_model.dumpMemoryOperations(arc_l2ControllerFilename);
                    std::string architectureErrLog=baseFileName+"_error.log";
                    curr_a->DumpRegisterFileAllocation(baseFileName);
                    curr_a->respect_dependencies(architectureErrLog);
                    curr_a->respect_FU_execution(architectureErrLog);
                    curr_a->L1_sends_doesnt_send_data_before_arrival_fromL2(architectureErrLog);
                    curr_a->DumpFUInfo(baseFileName);
                    if(curr_a->isMinimal())
                        break;
                }
}


void DependencyGraph::computeSchedules(){
    write_dot("DependencyGraph_original_DBG1.dot");
    supernode_opt();
    write_dot("DependencyGraph_after_supernode_opt_DBG1.dot");
    max_par_schedule();
    write_dot("DependencyGraph_ASAP_ALAP_schedule_DBG1.dot",ASAP_ALAP);
    sequential_schedule();
    write_dot("DependencyGraph_SEQUENTIAL_schedule_DBG1.dot",SEQUENTIAL);

    l2_model.dumpMemoryPartitioning("l2_memory_partitioning.csv");

}
void DependencyGraph::populateGraph(BasicBlock *BB){
    errs() << "populating ddg ... ";
    for(BasicBlock::iterator inst = BB->begin(),inst_e = BB->end(); inst != inst_e; ++inst){
        Instruction *I = &*inst;
        if( !(isa<GetElementPtrInst>(I) || isa<ReturnInst>(I))){
            if(isa<LoadInst>(I) || isa<StoreInst>(I)){
                Value *op;
                std::string nodeName;
                vertex_options additional_info = MRAM;
                if(isa<LoadInst>(I)){
                    op=I->getOperand(0); 
                }
                else{
                    op=I->getOperand(1);
                }
                ArrayReference a;
                Instruction* elementPtrInst=NULL;
                if( op->hasName() && op->getName().contains(StringRef("arrayidx"))){
                    a=solveElementPtr(BB,op->getName());
                    elementPtrInst=getElementPtr(BB,op->getName());
                }else{
                    StringRef ArrayID = op->getName();
                    a.arrayName = ArrayID;
                    a.offset = 0;
                }
                if(isa<LoadInst>(I)){
                    nodeName = I->getName();
                }
                else{
                    nodeName =(a.arrayName+"_"+a.offset.toString(10,false)).str();
                }
                nodeNameToInstructionMap[nodeName]=I;
                vertex_t inst_vertex = boost::add_vertex(ddg);
                ddg[inst_vertex].inst = I;
                std::string arrayOffset = a.offset.toString(10,false);
                std::string vertexName = (a.arrayName+"["+arrayOffset).str()+"]";
                ddg[inst_vertex].name =vertexName;
                ddg[inst_vertex].elementPtrInst=elementPtrInst;
                ddg[inst_vertex].info = additional_info;
                ddg[inst_vertex].arrayName = a.arrayName;
                ddg[inst_vertex].arrayOffset = std::stoi(arrayOffset);
                InstructionToVertexMap[I]=inst_vertex;
                if(isa<StoreInst>(I)){
                    Value *source = I->getOperand(0);
                    std::string operandNodeName=source->getName();
                    operandNodeName=replaceAll(operandNodeName,'.','_');
                    Instruction *operandInstruction;
                    operandInstruction = nodeNameToInstructionMap[operandNodeName];
                    vertex_t operandVertex = 
                        InstructionToVertexMap[operandInstruction];
                    add_edge(operandVertex,inst_vertex,ddg);
                    write_nodes.push_back(inst_vertex);
                }
                else{
                    read_nodes.push_back(inst_vertex);
                }
            }else{
                std::string nodeName;
                nodeName = I->getName();
                nodeName = replaceAll(nodeName,'.','_');
                nodeNameToInstructionMap[nodeName]=I;
                vertex_t inst_vertex = boost::add_vertex(ddg);
                ddg[inst_vertex].inst = I;
                ddg[inst_vertex].name = I->getName();
                InstructionToVertexMap[I] = inst_vertex;
                for(unsigned i=0;i<I->getNumOperands();i++){
                    Value *op = I->getOperand(i);
                    std::string operandNodeName = op->getName();
                    operandNodeName = replaceAll(operandNodeName,'.','_');
                    Instruction *openrandInstruction = 
                        nodeNameToInstructionMap[operandNodeName];
                    vertex_t operandVertex= InstructionToVertexMap[openrandInstruction];
                    vertex_t instructionVertex = InstructionToVertexMap[I];
                    add_edge(operandVertex,instructionVertex,ddg);
                }
            } 
        }
        inst_count++;
    }
}

void DependencyGraph::write_dot(std::string fileName, Schedule schedule){
    std::ofstream output_dot_file;
    output_dot_file.open(fileName);
    vertex_writer vtx_writer=vertex_writer(ddg,vertices_to_highlight,schedule);
    edges_writer edg_writer= edges_writer(ddg,edges_to_highlight);
    boost::write_graphviz(output_dot_file,ddg,vtx_writer,edg_writer);
    output_dot_file.close();
}

void DependencyGraph::dumpBasicBlockIR(std::string fileName,BasicBlock* bb){
    std::ofstream output_dot_file;
    output_dot_file.open(fileName);
    for (BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; ++i) {
        Instruction* ii = &*i;
        std::string str;
        llvm::raw_string_ostream ss(str);
        ss<< *ii;
        output_dot_file << ss.str()<< "\n";
    }  

}

//TODO filter out unused instruction by current code
void DependencyGraph::supernode_opt(){
    std::list<unsigned> commutativeInstructions;
    commutativeInstructions.push_back(Instruction::Add);
    commutativeInstructions.push_back(Instruction::FAdd);
    commutativeInstructions.push_back(Instruction::And);
    commutativeInstructions.push_back(Instruction::Mul);
    commutativeInstructions.push_back(Instruction::FMul);
    commutativeInstructions.push_back(Instruction::Or);
    commutativeInstructions.push_back(Instruction::Xor);
    std::list<unsigned>::iterator inst;
    for(inst=commutativeInstructions.begin();
            inst!=commutativeInstructions.end();
            inst++){
        supernode_opt_inner(*inst);
    }
}
void DependencyGraph::supernode_opt_inner(unsigned opCode){
    errs()<<"*****####@@@@ Running supernode opt for inst "<<Instruction::getOpcodeName(opCode)<<" @@@@####*****\n";
    std::list<vertex_t>::iterator it;
    std::list<edge_t>::iterator e_it;
    std::list<vertex_t> vertex_to_check;
    in_edge_it_t in_edge_it,in_edge_end;
    out_edge_it_t out_edge_it,out_edge_end;
    std::list<std::tuple<std::list<vertex_t>,std::list<edge_t>,std::list<edge_t>,std::list<edge_t>>> supernode_list; 


    for (it=write_nodes.begin(); it!= write_nodes.end(); ++it){
        vertex_to_check.push_back(*it);
    }
    while(!vertex_to_check.empty()){
        vertex_t curr_vertex = vertex_to_check.front();
        vertex_to_check.pop_front();
        Instruction *currVertexInstruction=ddg[curr_vertex].inst;

        errs()<<"Checking node "<<currVertexInstruction->getName()<<", Opcode: "<<currVertexInstruction->getOpcode()<<"\n";
        if( isa<BinaryOperator>(currVertexInstruction) &&
                currVertexInstruction->getOpcode() == opCode){
            errs()<<"Found add instruction node\n";

            std::list<vertex_t> supernode;
            std::list<edge_t> in_edges_supernode;
            std::list<edge_t> inner_edges_supernode;
            std::list<edge_t> out_edge_supernode;

            std::list<vertex_t> supernode_frontier;
            supernode.push_back(curr_vertex);
            supernode_frontier.push_back(curr_vertex);
            for(boost::tie(out_edge_it,out_edge_end) = boost::out_edges(curr_vertex,ddg);
                    out_edge_it != out_edge_end; ++out_edge_it){
                out_edge_supernode.push_back(*out_edge_it);
            }
            while(!supernode_frontier.empty()){
                vertex_t curr_supernode_frontier = supernode_frontier.front();
                supernode_frontier.pop_front();
                errs()<<"Created new supernode\n";

                for(boost::tie(in_edge_it,in_edge_end) = 
                        boost::in_edges(curr_supernode_frontier,ddg);
                        in_edge_it != in_edge_end; ++in_edge_it){
                    vertex_t currFrontier = source(*in_edge_it,ddg);
                    Instruction *currFrontierInstruction=ddg[currFrontier].inst; 
                    if( isa<BinaryOperator>(currFrontierInstruction) &&
                            currFrontierInstruction->getOpcode() == opCode){
                        errs()<<"Expanding supernode with new add instruction node\n";
                        supernode_frontier.push_back(currFrontier);
                        supernode.push_back(currFrontier);
                        inner_edges_supernode.push_back(*in_edge_it);
                    }else{
                        in_edges_supernode.push_back(*in_edge_it);
                        vertex_to_check.push_back(currFrontier);
                    }
                }
            }
            if(supernode.size() > 1){
                errs()<<"Adding supernode to supernode list\n";
                supernode_list.push_back(std::make_tuple(supernode,in_edges_supernode,inner_edges_supernode,out_edge_supernode));
            }
        }else{
            for(boost::tie(in_edge_it,in_edge_end) = boost::in_edges(curr_vertex,ddg);
                    in_edge_it != in_edge_end; ++in_edge_it){
                vertex_to_check.push_back(source(*in_edge_it,ddg));
            }
        }
    }

    errs()<< "Found "<<supernode_list.size()<<" supernodes.";
    for (it=write_nodes.begin(); it!= write_nodes.end(); ++it){
        for(boost::tie(in_edge_it,in_edge_end) = boost::in_edges(*it,ddg);
                in_edge_it != in_edge_end; ++in_edge_it){

        }
    }
    std::list<std::tuple<std::list<vertex_t>,std::list<edge_t>,std::list<edge_t>,std::list<edge_t>>>::iterator supernode_it; 
    for(supernode_it=supernode_list.begin();
            supernode_it!=supernode_list.end(); ++supernode_it){

        //Color supernodes nodes and inner edges red
        //in-edges to the suepernodes in green
        //out-edges to the supernodes in blue
        for(it=std::get<0>(*supernode_it).begin();
                it!=std::get<0>(*supernode_it).end();++it){
            errs()<<"Supernode nodes ID: "<<*it<<" "<<ddg[*it].name <<"\n";
            vertices_to_highlight[*it]="red";
        }
        for(e_it=std::get<1>(*supernode_it).begin();
                e_it!=std::get<1>(*supernode_it).end();++e_it){
            edges_to_highlight[*e_it]="green";
        }
        for(e_it=std::get<2>(*supernode_it).begin();
                e_it!=std::get<2>(*supernode_it).end();++e_it){
            edges_to_highlight[*e_it]="red";
        }
        for(e_it=std::get<3>(*supernode_it).begin();
                e_it!=std::get<3>(*supernode_it).end();++e_it){
            edges_to_highlight[*e_it]="blue";
        }

    }
    std::string filename("DependencyGraph_supernode_highlight_");
    filename+=Instruction::getOpcodeName(opCode);
    filename+=".dot";
    write_dot(filename);
    size_t new_Nodes_count=0;
    clearHighlights();
    for(supernode_it=supernode_list.begin();
            supernode_it!=supernode_list.end(); ++supernode_it){
        //Edges previous layer ( initialized with in-edges)
        errs()<<"* Analysing new supernode\n";
        std::vector<vertex_t> previous_layer_vertex;
        for(e_it=std::get<1>(*supernode_it).begin();
                e_it!=std::get<1>(*supernode_it).end();++e_it){
            previous_layer_vertex.push_back(source(*e_it,ddg));
        }
        while(previous_layer_vertex.size()>1){
            std::vector<vertex_t> current_layer_vertex;
            errs()<<"STARTING NEW LAYER\n";

            for(size_t i=0; i< previous_layer_vertex.size()-1;i+=2){
                errs()<<"i is equal to: "<<i<<" total size = "<<previous_layer_vertex.size()<<"\n";
                errs()<<"handling node "<<ddg[previous_layer_vertex[i]].name <<" with id "<<previous_layer_vertex[i]<<" and "<<ddg[previous_layer_vertex[i+1]].name <<" "<<previous_layer_vertex[i+1]<<"\n"; 
                Instruction* operandInst_0 = ddg[previous_layer_vertex[i]].inst;
                Instruction* operandInst_1 = ddg[previous_layer_vertex[i+1]].inst;
                Instruction* insertionPoint = operandInst_1->getNextNode(); 
                const char *opCodeName = Instruction::getOpcodeName(opCode);
                std::string name = opCodeName +std::string(".sup.") +std::to_string(new_Nodes_count++);
                Instruction *newOp = BinaryOperator::Create((llvm::Instruction::BinaryOps)opCode,operandInst_0, operandInst_1,Twine(name),insertionPoint); 
                vertex_t newOpVertex = boost::add_vertex(ddg);
                ddg[newOpVertex].inst = newOp;
                ddg[newOpVertex].name = newOp->getName();
                add_edge(previous_layer_vertex[i],newOpVertex,ddg); 
                add_edge(previous_layer_vertex[i+1],newOpVertex,ddg); 
                current_layer_vertex.push_back(newOpVertex);
            }
            if(previous_layer_vertex.size()%2){
                current_layer_vertex.push_back(previous_layer_vertex.back());
            }
            previous_layer_vertex.clear();
            previous_layer_vertex=current_layer_vertex;
        }

        edge_t out_supernode_edge = std::get<3>(*supernode_it).front();
        vertex_t out_supernode_vertex = target(out_supernode_edge,ddg);
        Instruction *out_supernode_instruction = ddg[out_supernode_vertex].inst;
        if(isa<StoreInst>(out_supernode_instruction)){
            StoreInst *out_supernode_instruction_store = (StoreInst*)out_supernode_instruction;
            Value* ptrOperand =out_supernode_instruction_store->getPointerOperand();
            vertex_t last_supernode_vertex = previous_layer_vertex.front();
            Instruction* last_supernode_instruction = ddg[last_supernode_vertex].inst;
            errs()<<"creating store for instruction" <<*last_supernode_instruction<<"\n";
            Instruction* newStore = new StoreInst(last_supernode_instruction,ptrOperand);
            ddg[out_supernode_vertex].inst=newStore;
        }
        add_edge(previous_layer_vertex.front(),out_supernode_vertex,ddg);

        for(it=std::get<0>(*supernode_it).begin();
                it!=std::get<0>(*supernode_it).end();++it){
            clear_vertex(*it,ddg); 
        }

    }
    std::list<vertex_t> vertex_to_remove;
    for(supernode_it=supernode_list.begin();
            supernode_it!=supernode_list.end(); ++supernode_it){
        //removing old nodes 
        for(it=std::get<0>(*supernode_it).begin();
                it!=std::get<0>(*supernode_it).end();++it){
            ddg[*it].mark_remove=true;
        }
    }

    BasicBlock* bb= ddg[0].inst->getParent();
    dumpBasicBlockIR("bb_before_instruction_removal.ll",bb);

    vertex_it_t vi,vi_end,next;
    boost::tie(vi,vi_end) = vertices(ddg);
    for(next=vi; vi !=vi_end;vi=next){
        ++next;
        if(ddg[*vi].mark_remove){
            Instruction* toRemove=  ddg[*vi].inst;
            errs()<<"\nErasing instruction attached to node : "<<ddg[*vi].name<<" ID:" <<*vi <<": ";
            if(removedInstructions.find(toRemove)==removedInstructions.end())
                replaceAndErase(toRemove);
            remove_vertex(*vi,ddg);
        }
    }
    dumpBasicBlockIR("bb_after_instruction_removal.ll",bb);
    regenerateBasicBlock(bb);
    dumpBasicBlockIR("bb_after_instruction_reorder.ll",bb);
    //update read_nodes and write_nodes (after modifying the graph the vertex id might change)
    read_nodes=std::list<vertex_t>();
    write_nodes=std::list<vertex_t>();
    vertex_it_t it_1,vi_end_1,next_1;
    boost::tie(it_1,vi_end_1) = vertices(ddg);
    for(next_1=it_1;it_1 !=vi_end_1;it_1=next_1){
        ++next_1;
        Instruction *I = ddg[*it_1].inst;
        if(isa<LoadInst>(I)){
            read_nodes.push_back(*it_1);
        }else{
            if(isa<StoreInst>(I)){
                write_nodes.push_back(*it_1);
            }
        }
    }
}

void DependencyGraph::regenerateBasicBlock(BasicBlock *bb){
    errs()<<"Called regenerateBasicBlock\n";
    Instruction *returnI=NULL;
    BasicBlock::reverse_iterator inst, inst_e,next;
    for(inst = bb->rbegin(),inst_e = bb->rend();
            inst != inst_e; inst=next){
        if(isa<ReturnInst>(*inst))
            returnI=&*inst;
        next=inst;
        next++;
        inst->removeFromParent();
    }
    typedef std::list<vertex_t> InstructionOrder;
    InstructionOrder instruction_order;
    boost::topological_sort(ddg, std::front_inserter(instruction_order));

    errs() << "instruction ordering...\n";
    for (InstructionOrder::iterator i = instruction_order.begin();
            i != instruction_order.end(); ++i){
        if(ddg[*i].elementPtrInst!=NULL)
            bb->getInstList().push_back(ddg[*i].elementPtrInst);
        bb->getInstList().push_back(ddg[*i].inst);
    }
    bb->getInstList().push_back(returnI);
}


ArrayReference DependencyGraph::solveElementPtr(BasicBlock *BB, StringRef elementPtrID){
    StringRef arrayName;
    APInt offset;
    for(BasicBlock::reverse_iterator inst = BB->rbegin(),inst_e = BB->rend();
            inst != inst_e; ++inst){
        Instruction *I = &*inst;
        if(elementPtrID == I->getName()){
            Value *v = I->getOperand(0);
            arrayName=v->getName();
            Value *v1= I->getOperand(1);

            if(ConstantInt* CI = dyn_cast<ConstantInt>(v1)){
                offset = CI->getValue();
            }
        }

    }
    ArrayReference arrayRefInfo = {arrayName, offset};
    return arrayRefInfo;
}

int DependencyGraph::getLatency(vertex_t v){
    return getVertexLatency(ddg,v,config);
}

void DependencyGraph::computeL2_L1_transfertimes(){
    std::map<std::string,int> arrayInfo_read;
    std::map<std::string,int> arrayInfo_write;

    // collect data and sort them in lexicographic order
    //std::list<vertex_t>::iterator it;
    //for(it = read_nodes.begin(); it!= read_nodes.end(); it++){

    vertex_it_t it,vi_end,next;
    boost::tie(it,vi_end) = vertices(ddg);
    for(next=it;it !=vi_end;it=next){
        ++next;
        Instruction *I = ddg[*it].inst;
        if(isa<LoadInst>(I)){
            std::string arrayName = ddg[*it].arrayName;
            int arraySize = ddg[*it].arrayOffset;
           if(arrayInfo_read.find(arrayName) != arrayInfo_read.end()){
               int currArraySize = arrayInfo_read[arrayName]; 
               if (currArraySize < arraySize + 1){
                    arrayInfo_read[arrayName] = arraySize+1;
               }
           }else{
                arrayInfo_read.insert(std::pair<std::string,int>(arrayName,arraySize+1));
           }
        }
    }
    boost::tie(it,vi_end) = vertices(ddg);
    for(next=it;it !=vi_end;it=next){
        ++next;
        Instruction *I = ddg[*it].inst;
        if(isa<StoreInst>(I)){
            std::string arrayName = ddg[*it].arrayName;
            int arraySize = ddg[*it].arrayOffset;
           if(arrayInfo_write.find(arrayName) != arrayInfo_write.end()){
               int currArraySize = arrayInfo_write[arrayName]; 
               if (currArraySize < arraySize + 1){
                    arrayInfo_write[arrayName] = arraySize+1;
               }
           }else{
                arrayInfo_write.insert(std::pair<std::string,int>(arrayName,arraySize+1));
           }
       }
    }   
     int nextAvailableL2Slot =0;
    for(auto arrayInfoIt = arrayInfo_read.begin();
            arrayInfoIt != arrayInfo_read.end();
            arrayInfoIt ++){
        std::string arrayName = arrayInfoIt->first;
        int size = arrayInfoIt->second;
        l2_model.L2_baseAddress.insert(std::pair<std::string,int>(arrayName,nextAvailableL2Slot));
        l2_model.add_L2_partition(arrayName,nextAvailableL2Slot,size);
        nextAvailableL2Slot+= size;
    }
    l2_model.add_L2_operation(0,Optype::READ_FROM_L2,0,nextAvailableL2Slot-1,0);
    for(auto arrayInfoIt = arrayInfo_write.begin();
            arrayInfoIt != arrayInfo_write.end();
            arrayInfoIt ++){
        std::string arrayName = arrayInfoIt->first;
        int size = arrayInfoIt->second;
        l2_model.L2_baseAddress.insert(std::pair<std::string,int>(arrayName,nextAvailableL2Slot));
        l2_model.add_L2_partition(arrayName,nextAvailableL2Slot,size);
        nextAvailableL2Slot+= size;
    }
    // get initial startup time
    int bitwidth_l2 = config.resource_database.bitwidth_l2;
    int bitwidth_l1 = config.resource_database.bitwidth_register_file;
    int bitwidth_l1_l2_ratio = bitwidth_l1 / bitwidth_l2;
    int clock_l1 = config.resource_database.clock_frequency;
    int clock_l2 = config.resource_database.clock_l2;
    int depth_l2 = config.resource_database.depth_l2;
    std::string type_l2 = config.resource_database.type_l2;
    int clock_ratio_l1_l2 = ceil(clock_l1/((double)clock_l2));
    double clock_remainder = ceil(clock_l1/((double)clock_l2))- (clock_l1/((double)clock_l2));
    int technology = config.resource_database.technology_l2;
    int startupLatencyL2_read = config.resource_database.startup_read_latency_l2; 
    int read_cycle_latency=resources_database::getL2ReadLatency(depth_l2,clock_l2,bitwidth_l2,type_l2,technology);
    boost::tie(it,vi_end) = vertices(ddg);
    for(next=it;it !=vi_end;it=next){
        ++next;
        Instruction *I = ddg[*it].inst;
        if(isa<LoadInst>(I)){
            std::string arrayName = ddg[*it].arrayName;
            int arrayOffset = ddg[*it].arrayOffset;
            int arrayBaseAddress= l2_model.L2_baseAddress[arrayName];
            int elementL2_index = arrayBaseAddress+arrayOffset;
            int arrivalTime =  startupLatencyL2_read + 
                (read_cycle_latency*(elementL2_index +1)*(bitwidth_l1_l2_ratio)*clock_ratio_l1_l2)
                -(floor(read_cycle_latency*(elementL2_index+1)*bitwidth_l1_l2_ratio * clock_remainder)) ;
            ddg[*it].schedules[ASAP] = arrivalTime;
        }
    }
    return;
}

void DependencyGraph::max_par_schedule(){
    errs()<<"Called max_par_schedule()\n";
    std::list<vertex_t> instruction_order;

    //ASAP
    boost::topological_sort(ddg, std::front_inserter(instruction_order));
    std::vector<int> clocks(num_vertices(ddg),0);

    //SET the arrival time of data to L1 here!
    computeL2_L1_transfertimes();
    std::list<vertex_t>::iterator it;
    for(it = read_nodes.begin(); it!= read_nodes.end(); it++){
        clocks[*it]=ddg[*it].schedules[ASAP];
    }

    for (std::list<vertex_t>::iterator i = instruction_order.begin();
            i != instruction_order.end(); ++i){
        if (boost::in_degree (*i, ddg) > 0) {
            in_edge_it_t j, j_end;
            int maxdist = 0;
            for(boost::tie(j,j_end) = in_edges(*i,ddg); j!= j_end;++j){
                int v_source_id = source(*j,ddg);
                errs()<<v_source_id<<"previous node inst: "<<*(ddg[v_source_id].inst);
                errs()<<", latency: "<<getLatency(v_source_id)<<"\n";
                maxdist = std::max(clocks[v_source_id]+getLatency(v_source_id)-1,maxdist);
            }
            clocks[*i]=maxdist+1;
            ddg[*i].schedules[ASAP] = maxdist+1;
            if(schedule.size() >= (unsigned) maxdist+1){
                std::list<vertex_t> current_cycle = schedule[maxdist];
                current_cycle.push_back(*i);
            }
            else{
                std::list<vertex_t> current_cycle;
                current_cycle.push_back(*i);
                schedule.push_back(current_cycle);
            }
        }else{
          //  clocks[*i]=0;
            //ddg[*i].schedules[ASAP] =0;
            int asapClock = ddg[*i].schedules[ASAP];
            if(schedule.size() >= (unsigned) asapClock+1){
                std::list<vertex_t> current_cycle = schedule[asapClock];
                current_cycle.push_back(*i);
            }else{
                while(schedule.size()<(unsigned) asapClock+1){
                    std::list<vertex_t> current_cycle;
                    schedule.push_back(current_cycle);
                }
                //std::list<vertex_t> current_cycle = schedule[asapClock];
                schedule[asapClock].push_back(*i);
            }
        }
    }
    asap_scheduled = true; 
    write_dot("DependencyGraph_final_schedule_asap_DBG1_.dot",ASAP);
    //ALAP
    int latency= schedule.size(); 
    std::vector<int> clocks_alap(num_vertices(ddg),0);
    for (std::list<vertex_t>::reverse_iterator r_i = instruction_order.rbegin();
            r_i != instruction_order.rend(); ++r_i){
        if (boost::out_degree (*r_i, ddg) > 0) {
            out_edge_it_t out_j, out_j_end;
            int maxdist = 0;
            for(boost::tie(out_j,out_j_end) = out_edges(*r_i,ddg);out_j!= out_j_end;++out_j){
                int v_target_id = target(*out_j,ddg);
                errs()<<"v_target_id:"<<v_target_id<<", clocks_alap[v_target_id]:"<<clocks_alap[v_target_id]<<"\n";
                maxdist = std::max(clocks_alap[v_target_id]+(getLatency(*r_i)-1),maxdist);
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
    alap_scheduled = true; 
}

void DependencyGraph::sequential_schedule(){
    errs()<<"Called sequential_schedule()\n";
    std::list<vertex_t> instruction_order;
    size_t cycle=0;
    boost::topological_sort(ddg, std::front_inserter(instruction_order));
    for (std::list<vertex_t>::iterator i = instruction_order.begin();
            i != instruction_order.end(); ++i){

        if (boost::in_degree (*i, ddg) > 0) {
            in_edge_it_t j, j_end;
            int maxLatency = 0;
            int cycle_due_to_parents;
            for(boost::tie(j,j_end) = in_edges(*i,ddg); j!= j_end;++j){
                int v_source_id = source(*j,ddg);
                //int v_source_additional_latency =ddg[v_source_id].schedules[SEQUENTIAL]+getLatency(v_source_id);
                int next_free_cycle = ddg[v_source_id].schedules[SEQUENTIAL]+getLatency(v_source_id);
                cycle_due_to_parents=std::max(next_free_cycle,cycle_due_to_parents); 
                maxLatency = std::max(getLatency(v_source_id),maxLatency);
            }
            int cycle_=cycle;
            cycle=std::max(cycle_,cycle_due_to_parents);
            //cycle=std::max(cycle_inc,maxLatency);
            for(int j=0; j<maxLatency;j++){
                std::list<vertex_t> newCycle;
                if(j==maxLatency-1)
                    newCycle.push_back(*i);
                schedule_sequential.push_back(newCycle);
            }
            ddg[*i].schedules[SEQUENTIAL] = cycle++;

        }else{
            std::list<vertex_t> newCycle;
            newCycle.push_back(*i);
            schedule_sequential.push_back(newCycle);
            ddg[*i].schedules[SEQUENTIAL] = cycle++;
        }
    }

}

Instruction* DependencyGraph::getElementPtr(BasicBlock *BB, StringRef elementPtrID){
    StringRef arrayName;
    APInt offset;
    Instruction *resultElementPtr;
    for(BasicBlock::reverse_iterator inst = BB->rbegin(),inst_e = BB->rend();
            inst != inst_e; ++inst){
        Instruction *I = &*inst;
        if(elementPtrID == I->getName()){
            resultElementPtr = I;
            break;
        }

    }
    return resultElementPtr;
}

void DependencyGraph::clearHighlights(){
    vertices_to_highlight.clear();
    edges_to_highlight.clear();
    return;
}
