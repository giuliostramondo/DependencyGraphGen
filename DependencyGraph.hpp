#ifndef DEPENDENCY_GRAPH_HPP
#define DEPENDENCY_GRAPH_HPP

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"
#include "llvm/Bitcode/LLVMBitCodes.h"
#include<fstream> // To write to file
//Boost graph includes 

#include <tuple>

#include "Architecture.hpp"
#include "Graph_Utils.hpp"
#include "mem_comp_paramJSON.hpp"

using namespace llvm;

struct ArrayReference{
    StringRef arrayName;
    APInt offset;
};


std::string replaceAll(StringRef inString, char toReplace, char replacement);

class DependencyGraph {

    class color_writer {
        public:
            // constructor - needs reference to graph we are coloring
            color_writer(DependencyGraph& g ): ddg( g.ddg ), edges_to_highlight(g.edges_to_highlight) {}
            // functor that does the coloring
            template <class VertexOrEdge>
                void operator()(std::ostream& out, const VertexOrEdge& e) const {
                    // check if this is the edge we want to color red
                    if(edges_to_highlight.find(e) != edges_to_highlight.end())
                        out << "[color="<<edges_to_highlight.at(e)<<"]";
                }
        private:
            DataDependencyGraph& ddg;
            std::map<edge_t, std::string> edges_to_highlight;
    };

    class vertex_writer {
        public:
            // constructor - needs reference to graph we are coloring
            vertex_writer(DependencyGraph& g, Schedule to_print = NONE) : ddg( g.ddg ), vertices_to_highlight(g.vertices_to_highlight), to_print(to_print) {}
            // functor that does the coloring
            template <class VertexOrEdge>
                void operator()(std::ostream& out, const VertexOrEdge& e) const {
                    Instruction *inst =  ddg[e].inst;
                    std::string name = ddg[e].name;

                    std::string vertex_label="[";
                    if( vertices_to_highlight.find(e) != vertices_to_highlight.end()){
                        vertex_label.append("color=");
                        vertex_label.append(vertices_to_highlight.at(e));
                        vertex_label.append(";");

                    }
                    vertex_label.append("label=\"");
                    vertex_label.append(name);
                    std::string cycle_asap; 
                    std::string cycle_alap; 
                    std::string cycle_sequential; 
                    switch(to_print){
                        case ASAP_ALAP:
                            cycle_asap = std::to_string(ddg[e].schedules[ASAP]);
                            cycle_alap = std::to_string(ddg[e].schedules[ALAP]);
                            if(cycle_asap.compare(cycle_alap)){
                                vertex_label.append(".Cycle:(");
                                vertex_label.append(cycle_asap);
                                vertex_label.append("-");
                                vertex_label.append(cycle_alap);
                                vertex_label.append(");");
                            }else{
                                cycle_asap = std::to_string(ddg[e].schedules[ASAP]);
                                vertex_label.append(".Cycle:");
                                vertex_label.append(cycle_asap);
                            }
                            break;
                        case ASAP:
                            cycle_asap = std::to_string(ddg[e].schedules[ASAP]);
                            vertex_label.append(".Cycle:");
                            vertex_label.append(cycle_asap);
                            break;
                        case ALAP:
                            cycle_alap = std::to_string(ddg[e].schedules[ALAP]);
                            vertex_label.append(".Cycle:");
                            vertex_label.append(cycle_alap);
                            break;
                        case SEQUENTIAL:
                            cycle_sequential= std::to_string(ddg[e].schedules[SEQUENTIAL]);
                            vertex_label.append(".Cycle:");
                            vertex_label.append(cycle_sequential);
                            break;
                        default:;

                    } 
                    vertex_label.append("\";");
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
                    vertex_label.append("shape="); 
                    vertex_label.append(shape); 
                    vertex_label.append("]");
                    out << vertex_label;
                }
    private:
        DataDependencyGraph& ddg;
        std::map<vertex_t,std::string> vertices_to_highlight;
        bool asap_scheduled;
        bool alap_scheduled;
        Schedule to_print;

};
    protected:
        friend class Instruction;

    public:
     DependencyGraph(mem_comp_paramJSON_format _conf): ddg(0), config(_conf) {
            errs()<<"DependencyGraph constructor\n";
            errs()<<"Latency of MRAM read :"+config.memory_param.mram.read_latency<<"\n";    
        }; 
    int inst_count=0;
    void populateGraph(BasicBlock *BB);
    void write_dot(std::string fileName, Schedule schedule = NONE);
    void supernode_opt();
    //TODO 
    void merge_loads();
    //TODO 
    bool asap_scheduled=false;
    bool alap_scheduled=false;
    void max_par_schedule();
    void sequential_schedule();
    void regenerateBasicBlock(BasicBlock *BB);
    void dumpBasicBlockIR(std::string fileName,BasicBlock* bb);
    /** Gives back the latency of a given instruction.*/
    int getLatency(vertex_t v); 
    DataDependencyGraph ddg;
    private:
    mem_comp_paramJSON_format config;
    std::unordered_map<std::string, Instruction*> nodeNameToInstructionMap;
    std::unordered_map<Instruction*,vertex_t> InstructionToVertexMap;
    std::list<vertex_t> write_nodes;
    std::list<vertex_t> read_nodes;
    std::map<vertex_t,std::string> vertices_to_highlight;
    std::map<edge_t, std::string> edges_to_highlight;
    ArrayReference solveElementPtr(BasicBlock *BB, StringRef elementPtrID);
    void supernode_opt_inner(unsigned opCode);
    void replaceAndErase(Instruction *I);
    Instruction* getElementPtr(BasicBlock *BB, StringRef elementPtrID);
    void clearHighlights();
    std::vector<std::list<vertex_t>> schedule;
    std::vector<std::list<vertex_t>> schedule_alap;
    std::vector<std::list<vertex_t>> schedule_sequential;
    //std::list<Architecture> architectures;
};
#endif 
