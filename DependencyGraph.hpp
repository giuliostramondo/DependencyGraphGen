#ifndef DEPENDENCY_GRAPH_HPP
#define DEPENDENCY_GRAPH_HPP

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"

#include<fstream> // To write to file
//Boost graph includes 
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graphviz.hpp> // Export/Import dot files

using namespace llvm;

struct ArrayReference{
    StringRef arrayName;
    APInt offset;
};

struct Vertex{
    Instruction *inst;
    std::string name;};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,Vertex, boost::no_property> DataDependencyGraph;
typedef boost::graph_traits<DataDependencyGraph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<DataDependencyGraph>::edge_descriptor edge_t;

std::string replaceAll(StringRef inString, char toReplace, char replacement){
    std::string str_data = inString.str();
    std::replace(str_data.begin(), str_data.end(),toReplace,replacement);
    return str_data;
}

class DependencyGraph {
    class color_writer {
        public:

            // constructor - needs reference to graph we are coloring
            color_writer(DataDependencyGraph& g ) : myGraph( g ) {}

            // functor that does the coloring
            template <class VertexOrEdge>
                void operator()(std::ostream& out, const VertexOrEdge& e) const {

                    // check if this is the edge we want to color red
                    if( boost::source( e, myGraph ) == 0 &&
                            boost::target( e, myGraph ) == 2  )
                        out << "[color=red]";
                }
        private:
            DataDependencyGraph& myGraph;
    };

    class vertex_writer {
        public:

            // constructor - needs reference to graph we are coloring
            vertex_writer(DataDependencyGraph& g ) : ddg( g ) {}

            // functor that does the coloring
            template <class VertexOrEdge>
                void operator()(std::ostream& out, const VertexOrEdge& e) const {
                    Instruction *inst =  ddg[e].inst;
                    std::string name = ddg[e].name;

                    // check if this is the edge we want to color red
                    std::string shape;
                    if(inst->getOpcodeName()== StringRef("store")){
                        shape="triangle";
                    }else{
                        if(inst->getOpcodeName()== StringRef("load")){
                            shape="invtriangle";
                        }else{
                            shape="ellipse";
                        }
                    }
                    out <<"[label=\""<<name<<"\";shape="<<shape<<"]";
                }
        private:
            DataDependencyGraph& ddg;
    };

    public:
    DependencyGraph(): ddg(0){} 
    int inst_count=0;
    void populateGraph(BasicBlock *BB){
        errs() << "populating ddg ... ";
        for(BasicBlock::iterator inst = BB->begin(),inst_e = BB->end(); inst != inst_e; ++inst){
            Instruction *I = &*inst;
            // For DEBUG
            Value *destination = cast<Value>(I);
            errs()<< "//(DDG CLASS) Instruction:" << inst_count <<", "<< I->getOpcodeName() <<" "<< I->getName()<< " " << destination->getName() <<'\n';
            // For Debug
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
    void write_dot(std::string fileName){
        std::ofstream output_dot_file;
        output_dot_file.open(fileName);
        boost::write_graphviz(output_dot_file,ddg,vertex_writer(ddg),color_writer(ddg));
    }

    private:
    DataDependencyGraph ddg;
    std::unordered_map<std::string, Instruction*> nodeNameToInstructionMap;
    std::unordered_map<Instruction*,vertex_t> InstructionToVertexMap;
    ArrayReference solveElementPtr(BasicBlock *BB, StringRef elementPtrID){
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
};
#endif 
