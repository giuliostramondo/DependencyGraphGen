#ifndef DEPENDENCY_GRAPH_HPP
#define DEPENDENCY_GRAPH_HPP

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"
#include "llvm/Bitcode/LLVMBitCodes.h"
#include<fstream> // To write to file
//Boost graph includes 
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/graphviz.hpp> // Export/Import dot files
#include <tuple>

#include "mem_comp_paramJSON.hpp"

using namespace llvm;

struct ArrayReference{
    StringRef arrayName;
    APInt offset;
};

enum vertex_options{ NA, MRAM,SRAM };
struct Vertex{
    Instruction *inst;
    std::string name;
    bool mark_remove=false;

    Instruction *elementPtrInst=NULL;
    size_t cycle_asap_begin;
    size_t cycle_alap_begin;
    vertex_options info = NA;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,Vertex, boost::no_property> DataDependencyGraph;
typedef boost::graph_traits<DataDependencyGraph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<DataDependencyGraph>::edge_descriptor edge_t;
typedef DataDependencyGraph::in_edge_iterator in_edge_it_t;
typedef DataDependencyGraph::out_edge_iterator out_edge_it_t;
typedef DataDependencyGraph::vertex_iterator vertex_it_t;


std::string replaceAll(StringRef inString, char toReplace, char replacement);

class DependencyGraph {
    struct edgeHasher{

        public:
            edgeHasher (DataDependencyGraph& g): ddg(g) {}
            size_t operator()(const edge_t &e) const{
                size_t seed =0;
                boost::hash_combine(seed,source(e,ddg));
                boost::hash_combine(seed,target(e,ddg));
                return seed;
            }
        private:
            DataDependencyGraph& ddg;

    };


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
            vertex_writer(DependencyGraph& g ) : ddg( g.ddg ), vertices_to_highlight(g.vertices_to_highlight), asap_scheduled(g.asap_scheduled), alap_scheduled(g.alap_scheduled) {}
            // functor that does the coloring
            template <class VertexOrEdge>
                void operator()(std::ostream& out, const VertexOrEdge& e) const {
                    Instruction *inst =  ddg[e].inst;
                    std::string name = ddg[e].name;
                    size_t cycle_asap = ddg[e].cycle_asap_begin;
                    size_t cycle_alap = ddg[e].cycle_alap_begin;
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
                    if(asap_scheduled){
                        if( vertices_to_highlight.find(e) != vertices_to_highlight.end()){
                            if(alap_scheduled && cycle_asap != cycle_alap)
                                out <<"[color="<<vertices_to_highlight.at(e)<<";label=\""<<name<<".Cycle:("<<std::to_string(cycle_asap)<<"-"<<std::to_string(cycle_alap)<<")\";shape="<<shape<<"]";
                            else
                                out <<"[color="<<vertices_to_highlight.at(e)<<";label=\""<<name<<".Cycle:("<<std::to_string(cycle_asap)<<"\";shape="<<shape<<"]";

                        }else{
                            if(alap_scheduled && cycle_asap != cycle_alap)
                                out <<"[label=\""<<name<<".Cycle:("<<std::to_string(cycle_asap)<<"-"<<std::to_string(cycle_alap)<<")\";shape="<<shape<<"]";
                            else
                                out <<"[label=\""<<name<<".Cycle:"<<std::to_string(cycle_asap)<<"\";shape="<<shape<<"]";

                        }
                    }else{
                        if( vertices_to_highlight.find(e) != vertices_to_highlight.end()){
                                out <<"[color="<<vertices_to_highlight.at(e)<<";label=\""<<name<<"\";shape="<<shape<<"]";
                        }else{
                                out <<"[label=\""<<name<<"\";shape="<<shape<<"]";

                        }              

                    }
                }
        private:
            DataDependencyGraph& ddg;
            std::map<vertex_t,std::string> vertices_to_highlight;
            bool asap_scheduled;
            bool alap_scheduled;

    };

    public:
    DependencyGraph(mem_comp_paramJSON_format _conf): ddg(0), config(_conf) {
        errs()<<"DependencyGraph constructor\n";
        errs()<<"Latency of MRAM read :"+config.memory_param.mram.read_latency<<"\n";    
    }; 
    int inst_count=0;
    void populateGraph(BasicBlock *BB);
    void write_dot(std::string fileName);
    void supernode_opt();
    //TODO 
    void merge_loads();
    //TODO 
    bool asap_scheduled=false;
    bool alap_scheduled=false;
    void max_par_schedule();
    void regenerateBasicBlock(BasicBlock *BB);
    void dumpBasicBlockIR(std::string fileName,BasicBlock* bb);
    /** Gives back the latency of a given instruction.*/
    int getLatency(vertex_t v); 
    private:
    DataDependencyGraph ddg;
    mem_comp_paramJSON_format config;
    std::unordered_map<std::string, Instruction*> nodeNameToInstructionMap;
    std::unordered_map<Instruction*,vertex_t> InstructionToVertexMap;
    std::list<vertex_t> write_nodes;
    std::list<vertex_t> read_nodes;
    std::map<vertex_t,std::string> vertices_to_highlight;
    std::map<edge_t, std::string> edges_to_highlight;
    ArrayReference solveElementPtr(BasicBlock *BB, StringRef elementPtrID);
    void supernode_opt_inner(unsigned opCode);
    Instruction* getElementPtr(BasicBlock *BB, StringRef elementPtrID);
    void clearHighlights();
    std::vector<std::list<vertex_t>> schedule;
    std::vector<std::list<vertex_t>> schedule_alap;

};
#endif 
