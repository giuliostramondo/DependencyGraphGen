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

using namespace llvm;

#define DEBUG_TYPE "dependencyGraph"


cl::opt<std::string> ParameterFilename("dependencyGraphConf", cl::desc("Specify configuration file for the DependencyGraph module"), cl::value_desc("filename"));

cl::opt<int> OptSearchLimit("dependencyGraphOptLimit", cl::desc("Specify a limit for the search of the optimal solution"), cl::value_desc("integer"));

namespace {

    struct DependencyGraphGen : public FunctionPass {
        static char ID; // Pass identification, replacement for typeid
        DependencyGraphGen() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            //Good source on boost :https://www.boost.org/doc/libs/1_48_0/libs/graph/doc/quick_tour.html 
            //Source specific to adjacency_list graph https://www.boost.org/doc/libs/1_48_0/libs/graph/doc/adjacency_list.html 
            //This page was used as a reference to link the boost graph to an llvm instruction:https://stackoverflow.com/questions/3100146/adding-custom-vertices-to-a-boost-graph
            mem_comp_paramJSON_format config;
            resources_database r = resources_database("./data/new_resource_utilization_data.db");
            if(ParameterFilename.getNumOccurrences() > 0){
                errs()<<"Passed configuration file to DependencyGraph: "<<ParameterFilename.c_str()<<"\n";   
                config = parse_mem_comp_paramJSON(ParameterFilename.c_str());
            }else{
                //TODO Check if file exists
                config = parse_mem_comp_paramJSON("./configurationFiles/conf_1.json");
            }
            errs()<< config.compute_param.funtional_unit.mul.latency << "<<LATENCY MUL32\n";
            errs() << "//Data Dependency Graph Generator running on: ";
            errs().write_escaped(F.getName()) << '\n';
            int block=0;
            errs() << "//Iterating over basic blocks of "<< F.getName() << '\n';
            if(F.size()>1){
                errs() <<"There are "<<F.size()<<" basic blocks in the code, something went wrong during the loop unrolling\nQuitting...\n";
                return false;
            }
            for(Function::iterator b = F.begin(), be = F.end(); b != be; ++b) {

                BasicBlock *BB = &*b;
                errs()<< "//Block: " << block << '\n';
                block++;      
                DependencyGraph DG(config);
                DG.populateGraph(BB); 
                DG.write_dot("DependencyGraph_original_DBG1.dot");
                DG.supernode_opt();
                DG.write_dot("DependencyGraph_after_supernode_opt_DBG1.dot");
                DG.max_par_schedule();
                DG.write_dot("DependencyGraph_ASAP_ALAP_schedule_DBG1.dot",ASAP_ALAP);
                DG.sequential_schedule();
                DG.write_dot("DependencyGraph_SEQUENTIAL_schedule_DBG1.dot",SEQUENTIAL);
                std::ofstream csvFile;
                csvFile.open(std::string(ParameterFilename.c_str())+".arch_info.csv");
                csvFile<<"MaxLatency,ActualMaxLatency,Area,StaticPower,DynamicPower,TotalEnergy,";
                bool firstArchitecture=true;
                DG.l2_model.dumpMemoryPartitioning("l2_memory_partitioning.csv");
                for(unsigned i=DG.schedule.size();i<=DG.schedule_sequential.size();i++){
                    Architecture a(DG.ddg,i, DG.config,DG.l2_model);
                    //a.generateArchitecturalMapping();
                    a.performALAPSchedule();
                    //a.generateSmallestArchitecturalMapping_Heu();
                    Architecture *curr_a;
                    if(OptSearchLimit.getNumOccurrences()>0){
                        errs()<<"Selected the BETTER THAN GREEDY algorithm\n";
                        errs()<<"Optimization limit set to "<<OptSearchLimit<<"\n";
                        curr_a=a.generateSmallestArchitecturalMapping_Opt(OptSearchLimit);
                    }else{
                        errs()<<"Selected the GREEDY algorithm\n";
                        a.generateSmallestArchitecturalMapping_Heu();
                        curr_a=&a;
                    
                    }
                    if (firstArchitecture){
                        csvFile<<curr_a->getCSVResourceHeader()<<"\n";
                        csvFile.close();
                        firstArchitecture=false;
                    }

                    //a.describe();
                    curr_a->computeSleepAndWriteBack_L2_Ops();
                    curr_a->dumpSchedule();
                    std::string arcFileName=std::string("Architecture_latency_");
                    arcFileName+=std::to_string(i);
                    arcFileName+=".dot";
                    curr_a->write_dot(arcFileName);
                    std::string arc_schemeFilename=std::string("Architecture_latency_");
                    arc_schemeFilename+=std::to_string(i);
                    arc_schemeFilename+="_schematic.dot";
                    curr_a->write_architecture_dot(arc_schemeFilename);
                    curr_a->appendArchInfoToCSV(std::string(ParameterFilename.c_str())+".arch_info.csv");
                    std::string arc_l2ControllerFilename=std::string("Architecture_latency_");
                    arc_l2ControllerFilename+= std::to_string(i);
                    arc_l2ControllerFilename+= "_l2_memory_controller.csv";
                    curr_a->l2_model.dumpMemoryOperations(arc_l2ControllerFilename);
                }
            }
            return false;
        }
    };
}

char DependencyGraphGen::ID = 0;
static RegisterPass<DependencyGraphGen> X("dependencyGraph", "Pass to produce a Data Dependency Graph");


