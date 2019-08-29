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
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/JSON.h"

#include <utility> // for std::pair 
#include <unordered_map>
#include<fstream> // To write to file

//Boost graph includes 
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graphviz.hpp> // Export/Import dot files

#include "mem_comp_paramJSON.hpp"
#include "DependencyGraph.hpp"
#include "resource_database_interface.hpp"
#include "ErrorLog.hpp"

using namespace llvm;

#define DEBUG_TYPE "dependencyGraph"


cl::opt<std::string> ParameterFilename("dependencyGraphConf", cl::desc("Specify configuration file for the DependencyGraph module"), cl::value_desc("filename"));

cl::opt<int> OptSearchLimit("dependencyGraphOptLimit", cl::desc("Specify a limit for the search of the optimal solution"), cl::value_desc("integer"),cl::init(0));

namespace {

    struct DependencyGraphGen : public FunctionPass {
        static char ID; // Pass identification, replacement for typeid
        DependencyGraphGen() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            //Good source on boost :https://www.boost.org/doc/libs/1_48_0/libs/graph/doc/quick_tour.html 
            //Source specific to adjacency_list graph https://www.boost.org/doc/libs/1_48_0/libs/graph/doc/adjacency_list.html 
            //This page was used as a reference to link the boost graph to an llvm instruction:https://stackoverflow.com/questions/3100146/adding-custom-vertices-to-a-boost-graph
            mem_comp_paramJSON_format config;
            resources_database r = resources_database("./data/resource_utilization_data.db");
            if(ParameterFilename.getNumOccurrences() > 0){
                errs()<<"Passed configuration file to DependencyGraph: "<<ParameterFilename.c_str()<<"\n";   
                config = parse_mem_comp_paramJSON(ParameterFilename.c_str());
            }else{
                //TODO Check if file exists
                config = parse_mem_comp_paramJSON("./configurationFiles/conf_1.json");
            }
            errs() << "//Data Dependency Graph Generator running on: ";
            errs().write_escaped(F.getName()) << '\n';
            errs() << "//Iterating over basic blocks of "<< F.getName() << '\n';
            if(F.size()>1){
                errs() <<"There are "<<F.size()<<" basic blocks in the code, something went wrong during the loop unrolling\nQuitting...\n";
                report_error("There are "+std::to_string(F.size())+" basic blocks in the code, something went wrong during the loop unrolling\n");
                return false;
            }
                BasicBlock *BB = &F.front();
                DependencyGraph DG(config,BB);
                DG.computeSchedules();
                DG.performArchitecturalDSE(ParameterFilename,OptSearchLimit);
            return false;
        }
    };
}

char DependencyGraphGen::ID = 0;
static RegisterPass<DependencyGraphGen> X("dependencyGraph", "Pass to produce a Data Dependency Graph");


