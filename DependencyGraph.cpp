#include "DependencyGraph.hpp"

std::unordered_set<Instruction*> removedInstructions;
void replaceAndErase(Instruction* I) {
    if(I==NULL) return;
    if(isa<LoadInst>(I))return;
    if(isa<StoreInst>(I)){
        I->eraseFromParent();
        return;
    }
    if(removedInstructions.find(I) == removedInstructions.end()){
        errs()<<*I<<" Erased\n";
        //TODO DEBUG HERE 
        //Probably users returns something that is not an Instruction and on which users() cannot be called... 
        //otherwise it seems strange
        for (auto U : I->users()) {
              replaceAndErase((Instruction*)U);
        }
        I->replaceAllUsesWith(UndefValue::get(I->getType()));
        if(I->getParent() != NULL)
            I->eraseFromParent();
        removedInstructions.insert(I);
    }else{
        errs()<<"Already Erased\n";
    }
} 

std::string replaceAll(StringRef inString, char toReplace, char replacement){
    std::string str_data = inString.str();
    std::replace(str_data.begin(), str_data.end(),toReplace,replacement);
    return str_data;
}

void DependencyGraph::populateGraph(BasicBlock *BB){
    errs() << "populating ddg ... ";
    for(BasicBlock::iterator inst = BB->begin(),inst_e = BB->end(); inst != inst_e; ++inst){
        Instruction *I = &*inst;
        if( !(isa<GetElementPtrInst>(I) || isa<ReturnInst>(I))){
            if(isa<LoadInst>(I) || isa<StoreInst>(I)){
                Value *op;
                std::string nodeName;
                if(isa<LoadInst>(I)){
                    op=I->getOperand(0); 
                }
                else{
                    op=I->getOperand(1);
                }
                ArrayReference a;
                if( op->hasName() && op->getName().contains(StringRef("arrayidx"))){
                    a=solveElementPtr(BB,op->getName());
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

void DependencyGraph::write_dot(std::string fileName){
    std::ofstream output_dot_file;
    output_dot_file.open(fileName);
    boost::write_graphviz(output_dot_file,ddg,vertex_writer(*this),color_writer(*this));
}


void DependencyGraph::supernode_opt(){
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
        errs()<<"BINOP_ADD CODE: "<<bitc::BINOP_ADD;
        if( isa<BinaryOperator>(currVertexInstruction) &&
                currVertexInstruction->getOpcode() == Instruction::Add){
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
                            currFrontierInstruction->getOpcode() == Instruction::Add){
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
        //    vertices_to_highlight.insert(source(*in_edge_it,ddg)); 
        
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
            //Instruction* insertionPoint = operandInst_1->getNextNode(); 
            Instruction *newOp = BinaryOperator::Create(Instruction::Add,operandInst_0, operandInst_1); 
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
               Instruction* newStore = new StoreInst(last_supernode_instruction,ptrOperand);
               ddg[out_supernode_vertex].inst=newStore;
               //out_supernode_instruction->eraseFromParent();
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
   
   vertex_it_t vi,vi_end,next;
   boost::tie(vi,vi_end) = vertices(ddg);
    for(next=vi; vi !=vi_end;vi=next){
        ++next;
        if(ddg[*vi].mark_remove){
            Instruction* toRemove=  ddg[*vi].inst;
            errs()<<"\nErasing instruction attached to node : "<<ddg[*vi].name<<" ID:" <<*vi <<": ";
            replaceAndErase(toRemove);
            //toRemove->eraseFromParent();
            remove_vertex(*vi,ddg);
        }
   }
    BasicBlock* bb= ddg[0].inst->getParent();
   // for (BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; ++i) {
   //     Instruction* ii = &*i;
   //     errs() << *ii << "\n";
   // }
   regenerateBasicBlock(bb);
}

void DependencyGraph::regenerateBasicBlock(BasicBlock *bb){
  errs()<<"Called regenerateBasicBlock\n";
  typedef std::list<vertex_t> InstructionOrder;
  InstructionOrder instruction_order;
  boost::topological_sort(ddg, std::front_inserter(instruction_order));
    
  errs() << "instruction ordering: ";
  Instruction *previous_inst=NULL;
  for (InstructionOrder::iterator i = instruction_order.begin();
       i != instruction_order.end(); ++i){
      if(previous_inst == NULL){
        previous_inst=ddg[*i].inst;
      }else{
       if(ddg[*i].inst->getParent()!=NULL)
           ddg[*i].inst->removeFromParent();
       ddg[*i].inst->insertAfter(previous_inst);
       previous_inst=ddg[*i].inst;

      }
    errs()<< *(ddg[*i].inst) << "\n";
  }
  errs() << "\n";
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
