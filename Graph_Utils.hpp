#ifndef GRAPH_UTILS_HPP
#define GRAPH_UTILS_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/graphviz.hpp> // Export/Import dot files
#include "llvm/IR/Instructions.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/copy.hpp>
#include "mem_comp_paramJSON.hpp"
#include "resource_database_interface.hpp"
#include "register_file_model.hpp"
#include "ErrorLog.hpp"
#include <random>
#include <stdlib.h>     /* srand, rand */
#include <time.h> 
#include <algorithm>

using namespace llvm;
enum vertex_options{ NA, MRAM,SRAM };
enum Schedule{
    ASAP,
    ALAP,
    SEQUENTIAL,
    ARCHITECTURAL,
    ASAP_ALAP,
    NONE
};



struct Vertex{
    Instruction *inst;
    std::string name;
    bool mark_remove=false;
    std::string arrayName="";
    int arrayOffset = -1;
    Instruction *elementPtrInst=NULL;
    std::vector<size_t>schedules{0,0,0,0};
    vertex_options info = NA;
    std::string FU = "";
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,Vertex, boost::no_property> DataDependencyGraph;
typedef boost::graph_traits<DataDependencyGraph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<DataDependencyGraph>::edge_descriptor edge_t;
typedef boost::graph_traits<DataDependencyGraph>::edge_iterator edge_it_t;
typedef DataDependencyGraph::in_edge_iterator in_edge_it_t;
typedef DataDependencyGraph::out_edge_iterator out_edge_it_t;
typedef DataDependencyGraph::vertex_iterator vertex_it_t;


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

//Getter function for vertex from configuration file
int getVertexLatency(DataDependencyGraph& ddg,vertex_t v,mem_comp_paramJSON_format config);
double getVertexArea(DataDependencyGraph& ddg,vertex_t v,mem_comp_paramJSON_format config, 
        int registerFileAVGDepth = 5);
double getVertexDynamicPower(DataDependencyGraph& ddg,vertex_t v,mem_comp_paramJSON_format config,
        int registerFileAVGDepth = 5);
double getVertexStaticPower(DataDependencyGraph& ddg,vertex_t v,mem_comp_paramJSON_format config,
        int registerFileAVGDepth = 5);

void alltopologicalSortUtil(
        DataDependencyGraph& g,
        std::vector<vertex_t>& res,
        std::map<vertex_t,bool> visited,
        std::map<vertex_t,int> indegree,
        std::list<std::list<vertex_t>> &topological_sorts,
        std::mt19937 rng);
std::list<std::list<vertex_t>> alltopologicalSort(
        DataDependencyGraph& g); 
void alltopologicalSortUtil_rev(
        DataDependencyGraph& g,
        std::vector<vertex_t>& res,
        std::map<vertex_t,bool> visited,
        std::map<vertex_t,int> indegree,
        std::list<std::list<vertex_t>> &topological_sorts,
        std::mt19937 rng);
std::list<std::list<vertex_t>> alltopologicalSort(
        DataDependencyGraph& g); 
std::list<std::list<vertex_t>> *alltopologicalSort_rev(
        DataDependencyGraph& g, unsigned optLimit); 
std::list<vertex_t> sortVerticesByASAP(DataDependencyGraph &g);
class vertex_writer {
    public:
        // constructor - needs reference to graph we are coloring
        vertex_writer(DataDependencyGraph& g, 
        std::map<vertex_t,std::string> vertices_to_highlight =std::map<vertex_t,std::string>(),
        Schedule to_print = NONE, std::map<vertex_t,unsigned> architectural_schedule = std::map<vertex_t,unsigned>()) : ddg( g ), 
        vertices_to_highlight(vertices_to_highlight), to_print(to_print),architectural_schedule(architectural_schedule) {}
    
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
                std::string cycle_architectural; 
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
                        //cycle_asap = std::to_string(ddg[e].schedules[ARCHITECTURAL]);
                        if(isa<LoadInst>(inst)){
                            cycle_asap = std::to_string(ddg[e].schedules[ASAP]);
                            cycle_architectural = std::to_string(architectural_schedule.at(e));
                            vertex_label.append(".ASAPCycle:");
                            vertex_label.append(cycle_asap);
                            vertex_label.append(".ArchitecturalCycle:");
                            vertex_label.append(cycle_architectural);
                        }else{
                            cycle_asap = std::to_string(architectural_schedule.at(e));
                            vertex_label.append(".Cycle:");
                            vertex_label.append(cycle_asap);
                        }
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
    std::map<vertex_t,unsigned> architectural_schedule; 

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


#endif 
