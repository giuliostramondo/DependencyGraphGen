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
                DependencyGraph DG;
                DG.populateGraph(BB); 
                DG.supernode_opt();
                DG.write_dot("DependencyGraph_class_boost_generated_ddg_color_writer.dot");
            }
            return false;
        }
    };
}

char DependencyGraphGen::ID = 0;
static RegisterPass<DependencyGraphGen> X("dependencyGraph", "Pass to produce a Data Dependency Graph");


