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

#include "DependencyGraph.hpp"

using namespace llvm;

#define DEBUG_TYPE "dependencyGraph"

STATISTIC(HelloCounter, "Counts number of functions greeted");

namespace {
    // Hello - The first implementation, without getAnalysisUsage.
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
                            shape="triangle";
                    }else{
                            shape="ellipse";
                    }
                    }
                        out <<"[label=\""<<name<<"\";shape="<<shape<<"]";
                }
        private:
            DataDependencyGraph& ddg;
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



            ++HelloCounter;
            errs() << "//Data Dependency Graph Generator running on: ";
            errs().write_escaped(F.getName()) << '\n';
            int block=0;
            errs() << "//Iterating over basic blocks of "<< F.getName() << '\n';
            for(Function::iterator b = F.begin(), be = F.end(); b != be; ++b) {
                BasicBlock *BB = &*b;
                errs()<< "//Block: " << block << '\n';
                block++;      
                
                //const int num_vertices = count_vertices(BB);
                DependencyGraph DG;
                DG.populateGraph(BB); 
                DG.write_dot("DependencyGraph_class_boost_generated_ddg_color_writer.dot");
            }
            return false;
        }
    };
}

char DependencyGraphGen::ID = 0;
static RegisterPass<DependencyGraphGen> X("dependencyGraph", "Pass to produce a Data Dependency Graph");


