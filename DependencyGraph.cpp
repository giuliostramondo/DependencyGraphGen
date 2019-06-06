#include "DependencyGraph.hpp"




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
        for(it=std::get<0>(*supernode_it).begin();
                it!=std::get<0>(*supernode_it).end();++it){
            vertices_to_highlight.insert(*it);
        }
    }

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
