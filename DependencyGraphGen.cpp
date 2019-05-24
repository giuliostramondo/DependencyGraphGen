//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"


#include <utility> // for std::pair 
#include <unordered_map>
#include<fstream> // To write to file

//Boost graph includes 
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graphviz.hpp> // Export/Import dot files
using namespace llvm;

#define DEBUG_TYPE "dependencyGraph"

STATISTIC(HelloCounter, "Counts number of functions greeted");
//namespace std {
//    template<>
//    struct hash<Instruction>
//    {
//      std::size_t operator()(const Instruction& s) const noexcept { return (std::size_t)&s; }
//    };
//}
namespace {

  // Hello - The first implementation, without getAnalysisUsage.
  struct ArrayReference{
     StringRef arrayName;
     APInt offset;
  };

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

  int count_vertices(BasicBlock *BB){
        int node_count=0;
        for(BasicBlock::iterator inst = BB->begin(),inst_e = BB->end(); inst != inst_e; ++inst){
            Instruction *I = &*inst;
            if(I->getOpcodeName() != StringRef("getelementptr") && I->getOpcodeName() != StringRef("ret")){
                 node_count++; 
            }
        }
        return node_count;
  }

  std::string replaceAll(StringRef inString, char toReplace, char replacement){
      std::string str_data = inString.str();
      std::replace(str_data.begin(), str_data.end(),toReplace,replacement);
      return str_data;
  }

  struct DependencyGraphGen : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    DependencyGraphGen() : FunctionPass(ID) {}
    
    bool runOnFunction(Function &F) override {
    //Good source on boost :https://www.boost.org/doc/libs/1_48_0/libs/graph/doc/quick_tour.html 
    //Source specific to adjacency_list graph https://www.boost.org/doc/libs/1_48_0/libs/graph/doc/adjacency_list.html 
    //This page was used as a reference to link the boost graph to an llvm instruction:https://stackoverflow.com/questions/3100146/adding-custom-vertices-to-a-boost-graph
      struct Vertex{Instruction *inst;
                    std::string name;};
      typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,Vertex, boost::no_property> DataDependencyGraph;
      typedef boost::graph_traits<DataDependencyGraph>::vertex_descriptor vertex_t;
      typedef boost::graph_traits<DataDependencyGraph>::edge_descriptor edge_t;
      std::unordered_map<std::string, Instruction*> nodeNameToInstructionMap;
      std::unordered_map<Instruction*,vertex_t> InstructionToVertexMap;

      //Trying to write the property writer
      //template <vertex_t>
      //class label_writer {
      //public: 
      //    lavel_writer(vertex_t _vertex) : name(_name){}
      //    template <class Vertex> 
      //
      //}

      ++HelloCounter;
      errs() << "//Data Dependency Graph Generator running on: ";
      errs().write_escaped(F.getName()) << '\n';
      int block=0;
      errs() << "//Iterating over basic blocks of "<< F.getName() << '\n';
      for(Function::iterator b = F.begin(), be = F.end(); b != be; ++b) {
            BasicBlock *BB = &*b;
            errs()<< "//Block: " << block << '\n';
            block++;      
          const int num_vertices = count_vertices(BB);
          // POSSIBLE FUTURE ISSUE: What happens if there need to be more node later on?
          //DataDependencyGraph ddg(num_vertices);
          DataDependencyGraph ddg(0);


        int inst_count =0;
        errs() << "digraph {"<<'\n';
        
        for(BasicBlock::iterator inst = BB->begin(),inst_e = BB->end(); inst != inst_e; ++inst){
            Instruction *I = &*inst;
            Value *destination = cast<Value>(I);
            errs()<< "//Instruction:" << inst_count <<", "<< I->getOpcodeName() <<" "<< I->getName()<< " " << destination->getName() <<'\n';
            //Only create node for operations and memory accesses
            if(I->getOpcodeName() != StringRef("getelementptr") && I->getOpcodeName() != StringRef("ret")){
                //Verify if there is a pointer indirection
                if(I->getOpcodeName() == StringRef("load") ||I->getOpcodeName() == StringRef("store")){
                    Value *op; 
                    if (I->getOpcodeName() == StringRef("load"))
                        op=I->getOperand(0);
                    else
                        op=I->getOperand(1);

                    ArrayReference a;
                    if( op->hasName() && op->getName().contains(StringRef("arrayidx"))){
                        a=solveElementPtr(BB,op->getName());
                    }else{
                        StringRef ArrayID = op->getName();
                        //a={ArrayID,0};    
                        a.arrayName = ArrayID;
                        a.offset = 0;
                    }
                    StringRef shape;
                    if(I->getOpcodeName() == StringRef("store")){
                        shape = StringRef("triangle");
                    }else{
                        shape=StringRef("invtriangle");
                    }
                    if (I->getOpcodeName() == StringRef("load")){
                        errs()<< replaceAll(I->getName(),'.','_')<< " " <<"[label=\""<< a.arrayName <<"["<<a.offset<<"]\";shape="<<shape<<"]"<<'\n';
                        nodeNameToInstructionMap[I->getName()]=I;

                         vertex_t inst_vertex = boost::add_vertex(ddg);
                         ddg[inst_vertex].inst = I;
                         ddg[inst_vertex].name = (a.arrayName+"["+a.offset.toString(10,false)).str()+"]";

                         InstructionToVertexMap[I]=inst_vertex;
                    }
                    else{
                        errs()<< a.arrayName<<"_"<<a.offset<< " " <<"[label=\""<< a.arrayName <<"["<<a.offset<<"]\";shape="<<shape<<"]"<<'\n';
                        nodeNameToInstructionMap[(a.arrayName+"_"+a.offset.toString(10,false) ).str()]=I;
                        
                        vertex_t inst_vertex = boost::add_vertex(ddg);
                        ddg[inst_vertex].inst = I;
                        ddg[inst_vertex].name = (a.arrayName+"["+a.offset.toString(10,false)).str()+"]";
                        InstructionToVertexMap[I]=inst_vertex;
                        
                        Value *source = I->getOperand(0);
                        Instruction *openrandInstruction = nodeNameToInstructionMap[replaceAll(source->getName(),'.','_')];

                        vertex_t operandVertex= InstructionToVertexMap[openrandInstruction];
                        add_edge(operandVertex,inst_vertex,ddg);
                        errs() << replaceAll(source->getName(),'.','_') << " -> " << a.arrayName<<"_"<<a.offset << '\n';
                    }
                }
                else{
                    errs() << replaceAll(I->getName(),'.','_') << " " <<"[label=\"" << I->getName() << "\";shape=\"ellipse\"]"<<'\n';
                    nodeNameToInstructionMap[replaceAll(I->getName(),'.','_')]=I;
                    vertex_t inst_vertex = boost::add_vertex(ddg);
                    ddg[inst_vertex].inst = I;
                    ddg[inst_vertex].name = I->getName();
                    InstructionToVertexMap[I]= inst_vertex;

                    for(unsigned i=0;i<I->getNumOperands();i++){
                       Value *op = I->getOperand(i);
                        Instruction *openrandInstruction = nodeNameToInstructionMap[replaceAll(op->getName(),'.','_')];
                        vertex_t operandVertex= InstructionToVertexMap[openrandInstruction];
                        vertex_t instructionVertex = InstructionToVertexMap[I];
                        add_edge(operandVertex,instructionVertex,ddg);
                        errs() <<replaceAll(op->getName(),'.','_') <<" -> " << replaceAll(I->getName(),'.','_')<<'\n';

                    }

                }
            }

                
            for(unsigned operandNb=0; operandNb< I->getNumOperands() ; operandNb++){
               Value *op = I->getOperand(operandNb);

               if(op->hasName()){
               StringRef name = op->getName();
               errs() << "//Opearand: " << operandNb <<" " << name  <<'\n';
               }else{

                
               if(ConstantInt* CI = dyn_cast<ConstantInt>(op))
                   errs() << "//Opearand: " << operandNb <<" " << CI->getValue()  <<'\n';
               }
            }
            inst_count++;
        
        }
        errs()<<"}\n";
      errs()<<"BOOST GRAPHVIZ\n";
      std::ofstream output_dot_file;
      output_dot_file.open("boost_generated_ddg.dot");

      boost::write_graphviz(output_dot_file,ddg,boost::make_label_writer(get(&Vertex::name,ddg)));
      output_dot_file.close();
      }
      return false;
    }
  };
}

char DependencyGraphGen::ID = 0;
static RegisterPass<DependencyGraphGen> X("dependencyGraph", "Pass to produce a Data Dependency Graph");


