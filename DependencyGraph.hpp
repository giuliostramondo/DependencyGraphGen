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

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,Vertex, boost::no_property> DataDependencyGraph;
typedef boost::graph_traits<DataDependencyGraph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<DataDependencyGraph>::edge_descriptor edge_t;

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
                    if( boost::source( e, myGraph ) == 0 &&
                            boost::target( e, myGraph ) == 2  )
                        out << "[color=red]";
                }
        private:
            DataDependencyGraph& myGraph;
    };

    class vertex_writer {
        public:
            // constructor - needs reference to graph we are coloring
            vertex_writer(DataDependencyGraph& g ) : ddg( g ) {}
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
                    out <<"[label=\""<<name<<"\";shape="<<shape<<"]";
                }
        private:
            DataDependencyGraph& ddg;
    };

    public:
    DependencyGraph(): ddg(0){} 
    int inst_count=0;
    void populateGraph(BasicBlock *BB);
    void write_dot(std::string fileName);

    private:
    DataDependencyGraph ddg;
    std::unordered_map<std::string, Instruction*> nodeNameToInstructionMap;
    std::unordered_map<Instruction*,vertex_t> InstructionToVertexMap;
    ArrayReference solveElementPtr(BasicBlock *BB, StringRef elementPtrID);
};
#endif 
