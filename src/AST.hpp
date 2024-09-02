#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

#include <stack>
#include <tuple>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/property_map/property_map.hpp>
#include <magic_enum_all.hpp>

#include "Lexer.hpp"

enum ASTNodeClass {
    Root,

    Function,
    Type,
    Argument,
    Return,

    Expression, // e.g x = 1 + 2
    Declaration, // e.g x : i32
};

enum ASTNodeSubType {
    boolean,

    i8,
    i16,
    i32,
    i64,
    i128,

    u8,
    u16,
    u32,
    u64,
    u128,

    f32,
    f64,

    array,
    tuple,
    map,

    func,
    none,
};

class ASTNode {
public:
    ASTNodeClass   node_class;
    ASTNodeSubType sub_type;
    std::string    name;
    Location       location;
    std::string    _color;
    std::string    _style = "filled";
    std::string    _shape;

    ASTNode();
    ASTNode(ASTNodeClass type, ASTNodeSubType subtype, std::string name, const Location& location);
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

void draw_graph(Tree ast);

std::tuple<Tree, vertex_t> parse_type(std::vector<std::tuple<Lexeme, Location>>& type_lexemes,
                                      std::stack<vertex_t>&                      type_stack,
                                      Tree&                                      type_tree,
                                      Location                                   root_location);
Tree                       generate_ast(std::vector<std::tuple<Lexeme, Location>> lexemes);