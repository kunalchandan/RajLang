#pragma once

#include <stack>
#include <tuple>
#include <utility>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/property_map/property_map.hpp>
#include <magic_enum_all.hpp>

#include "Lexer.hpp"

enum ASTNodeType {
    Root,

    Function,
    Type,
    Argument,
    Return,

    Expression, // e.g x = 1 + 2
    Declaration, // e.g x : i32
};

class ASTNode {
public:
    ASTNodeType node_type;
    std::string name;
    Location    location;
    std::string _color;
    std::string _style = "filled";
    std::string _shape;

    ASTNode();
    ASTNode(ASTNodeType type, std::string name, const Location& location);
    [[nodiscard]] std::string _get_graph_color() const;
    [[nodiscard]] std::string _get_graph_shape() const;
    ~ASTNode();
};

class ASTEdge {
public:
    std::string name;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, ASTNode, ASTEdge>
                                                     Tree;
typedef boost::graph_traits<Tree>::vertex_descriptor vertex_t;
typedef boost::graph_traits<Tree>::edge_descriptor   edge_t;

void generate_ast(std::vector<std::tuple<Lexeme, Location>> lexemes);