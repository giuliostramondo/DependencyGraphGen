#ifndef GRAPH_UTILS_HPP
#define GRAPH_UTILS_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/graphviz.hpp> // Export/Import dot files
#include "llvm/IR/Instructions.h"
#include <boost/graph/adjacency_list.hpp>

using namespace llvm;
enum vertex_options{ NA, MRAM,SRAM };
enum Schedule{
    ASAP,
    ALAP,
    SEQUENTIAL,
    ASAP_ALAP,
    NONE
};

struct Vertex{
    Instruction *inst;
    std::string name;
    bool mark_remove=false;

    Instruction *elementPtrInst=NULL;
    std::vector<size_t>schedules{0,0,0};
    vertex_options info = NA;
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

#endif 
