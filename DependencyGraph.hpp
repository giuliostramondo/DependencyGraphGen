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
#include <boost/graph/graphviz.hpp> // Export/Import dot files
#include <tuple>

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
typedef DataDependencyGraph::out_edge_iterator out_edge_it_t;


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
            vertex_writer(DependencyGraph& g ) : ddg( g.ddg ), vertices_to_highlight(g.vertices_to_highlight) {}
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
                            shape="invtriangle";
                        }else{
                            shape="ellipse";
                        }
                    }
                    if( vertices_to_highlight.find(e) != vertices_to_highlight.end()){
                        out <<"[color="<<vertices_to_highlight.at(e)<<";label=\""<<name<<"\";shape="<<shape<<"]";
                    }else{
                        out <<"[label=\""<<name<<"\";shape="<<shape<<"]";
                    }
                }
        private:
            DataDependencyGraph& ddg;
            std::map<vertex_t,std::string> vertices_to_highlight;

    };

    public:
    DependencyGraph(): ddg(0) {}; 
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
    std::map<vertex_t,std::string> vertices_to_highlight;
    std::map<edge_t, std::string> edges_to_highlight;
    ArrayReference solveElementPtr(BasicBlock *BB, StringRef elementPtrID);
};
#endif 
