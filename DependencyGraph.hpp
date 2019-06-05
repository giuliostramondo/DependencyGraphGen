#ifndef DEPENDENCY_GRAPH_HPP
#define DEPENDENCY_GRAPH_HPP

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"

#include<fstream> // To write to file
//Boost graph includes 
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graphviz.hpp> // Export/Import dot files

using namespace llvm;

struct ArrayReference{
    StringRef arrayName;
    APInt offset;
};

struct Vertex{
    Instruction *inst;
    std::string name;};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,Vertex, boost::no_property> DataDependencyGraph;
typedef boost::graph_traits<DataDependencyGraph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<DataDependencyGraph>::edge_descriptor edge_t;
typedef DataDependencyGraph::in_edge_iterator in_edge_it_t;

// A hash function for vertices.
//struct vertex_hash:std::unary_function<vertex_t, std::size_t> {
//  std::size_t operator()(vertex_descriptor const& u) const {
//    std::size_t seed = 0;
//    boost::hash_combine(seed, u[0]);
//    boost::hash_combine(seed, u[1]);
//    return seed;
//  }
//};

// A hash function for vertices.
//struct edge_hash:std::unary_function<edge_t, std::size_t> {
//  std::size_t operator()(edge_t const& u) const {
//    std::size_t seed = 0;
//    boost::hash_combine(seed, u[0]);
//    boost::hash_combine(seed, u[1]);
//    return seed;
//  }
//};
std::string replaceAll(StringRef inString, char toReplace, char replacement);

class DependencyGraph {
    class color_writer {
        public:
            // constructor - needs reference to graph we are coloring
            color_writer(DataDependencyGraph& g ) : myGraph( g ) {}
            // functor that does the coloring
            template <class VertexOrEdge>
                void operator()(std::ostream& out, const VertexOrEdge& e) const {
                    // check if this is the edge we want to color red
                    //if(edges_to_highlight.find(e) != edges_to_highlight.end())
                    //    out << "[color=red]";
                }
        private:
            DataDependencyGraph& myGraph;
    };

    class vertex_writer {
        public:
            // constructor - needs reference to graph we are coloring
            vertex_writer(DependencyGraph& g ) : ddg( g.ddg ), vertices_to_highlight(g.vertices_to_highlight) {}
            // functor that does the coloring
            template <class VertexOrEdge>
                void operator()(std::ostream& out, const VertexOrEdge& e) const {
                    errs()<<"Hey, I am the node writer dude\n";
                    errs()<<"I have a list of "<<vertices_to_highlight.size()<<" vertices to highlight\n";
                    Instruction *inst =  ddg[e].inst;
                    std::string name = ddg[e].name;
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
                    if( vertices_to_highlight.find(e) != vertices_to_highlight.end()){
                        out <<"[color=red;label=\""<<name<<"\";shape="<<shape<<"]";
                    }else{
                        out <<"[label=\""<<name<<"\";shape="<<shape<<"]";
                    }
                }
        private:
            DataDependencyGraph& ddg;
            std::unordered_set<vertex_t> vertices_to_highlight;

    };

    public:
    DependencyGraph(): ddg(0){} 
    int inst_count=0;
    void populateGraph(BasicBlock *BB);
    void write_dot(std::string fileName);
    void supernode_opt();

    private:
    DataDependencyGraph ddg;
    std::unordered_map<std::string, Instruction*> nodeNameToInstructionMap;
    std::unordered_map<Instruction*,vertex_t> InstructionToVertexMap;
    std::list<vertex_t> write_nodes;
    std::list<vertex_t> read_nodes;
    std::unordered_set<vertex_t> vertices_to_highlight;
    //std::unordered_set<edge_t> edges_to_highlight;
    ArrayReference solveElementPtr(BasicBlock *BB, StringRef elementPtrID);
};
#endif 
