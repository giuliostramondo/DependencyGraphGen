#ifndef ARCHITECTURE_HPP
#define ARCHITECTURE_HPP

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"
#include "llvm/Bitcode/LLVMBitCodes.h"
#include "Graph_Utils.hpp"
#include <utility> // for std::pair 
#include <unordered_map>

using namespace llvm;
class Architecture{
    class vertex_writer {
        public:
            // constructor - needs reference to graph we are coloring
            vertex_writer(DataDependencyGraph& g, 
            std::map<vertex_t,std::string> vertices_to_highlight =std::map<vertex_t,std::string>(),
            Schedule to_print = NONE) : ddg( g ), 
            vertices_to_highlight(vertices_to_highlight), to_print(to_print) {}
        
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
                        case ARCHITECTURAL:
                            cycle_asap = std::to_string(ddg[e].schedules[ARCHITECTURAL]);
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

    class edges_writer {
        public:
            // constructor - needs reference to graph we are coloring
            edges_writer(DataDependencyGraph& g,std::map<edge_t, std::string> edges_to_highlight): ddg( g ), edges_to_highlight(edges_to_highlight) {}
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



    class FunctionalUnit: public std::list<vertex_t>{
        public:
        unsigned opCode;
        unsigned extra_description;
        unsigned earliest_free_slot=0;
        std::string label;

    };
    public:
        Architecture(DataDependencyGraph& g,int latency,mem_comp_paramJSON_format config): 
            ddg(g), maxLatency(latency),config(config) {};
        void generateArchitecturalMapping();
        void generateSmallestArchitecturalMapping();
        void write_dot(std::string filename);
        void describe();
        void mergeFUs();
    private:
        DataDependencyGraph& ddg;
        int maxLatency;
        mem_comp_paramJSON_format config;
        void performALAPSchedule();
        //Map between OpCodes and list of FUs (implementing the opcode)
        //Each FU contains a list of vertices of the ddg that will execute
        //std::map<unsigned,std::list<std::list<vertex_t>>> units;
        std::map<unsigned,std::list<FunctionalUnit>> units;
        
}; 
#endif

