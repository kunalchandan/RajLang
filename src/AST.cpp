#include "AST.hpp"
#include "Lexer.hpp"
#include <cstddef>
#include <stdexcept>
#include <vector>

ASTNode::ASTNode() {
    this->node_type = ASTNodeType::Root;
    this->name      = "NULL";
    this->location  = Location();
    this->_color    = "white";
}

ASTNode::ASTNode(ASTNodeType type, std::string name, const Location& location) {
    this->node_type = type;
    this->name      = std::move(name);
    this->location  = location;
    this->_color    = _get_graph_color();
    this->_shape    = _get_graph_shape();
}

std::string ASTNode::_get_graph_color() const {
    // Pick some random light colors
    switch(this->node_type) {
    case ASTNodeType::Root:
        return "#000000";
    case ASTNodeType::Function:
        return "#84B6FF";
    case ASTNodeType::Declaration:
        return "#96c57a";
    case ASTNodeType::Expression:
        return "#ffa55b";
    case ASTNodeType::Argument:
        return "#e278e6";
    case ASTNodeType::Type:
        return "#47dfb9";
    case ASTNodeType::Return:
        return "#b9e2b9";
    }
    return "white";
}

std::string ASTNode::_get_graph_shape() const {
    // Pick some shapes
    switch(this->node_type) {
    case ASTNodeType::Root:
        return "ellipse";
    case ASTNodeType::Function:
        return "signature";
    case ASTNodeType::Declaration:
        return "component";
    case ASTNodeType::Expression:
        return "rect";
    case ASTNodeType::Argument:
        return "component";
    case ASTNodeType::Type:
        return "invhouse";
    case ASTNodeType::Return:
        return "rarrow";
    }
    return "ellipse";
}
ASTNode::~ASTNode() = default;

[[nodiscard]] size_t ast_gen_function(Tree&                                      ast,
                                      std::stack<Tree::vertex_descriptor>&       scope_stack,
                                      std::vector<std::tuple<Lexeme, Location>>& lexemes,
                                      size_t                                     x) {
    // Consume the following tokens that we expect
    // Identifier
    // ParenL
    // Arbitrary number of arguments and types separated by Commas
    // ParenR
    // RightArrow
    // CurlL

    Lexeme   lexeme = std::get<0>(lexemes[x]);
    Location loc    = std::get<1>(lexemes[x]);

    Tree::vertex_descriptor function_node = boost::add_vertex(ast);

    Lexeme expecting_identifier = std::get<0>(lexemes[x + 1]);
    if(expecting_identifier.lexeme_type != LexemeClass::Identifier) {
        LOG_ERROR("Expected Identifier in argument, received "
                  << expecting_identifier.tokens << " of type "
                  << magic_enum::enum_name(expecting_identifier.lexeme_type) << " at "
                  << std::get<1>(lexemes[x + 1]))
        throw std::runtime_error("Received unexpected lexeme"); // Could generate something random?
    }
    boost::add_edge(scope_stack.top(), function_node, ast);
    ast[function_node] = ASTNode(ASTNodeType::Function, expecting_identifier.tokens, loc);

    Lexeme expecting_parenL = std::get<0>(lexemes[x + 2]);
    if(expecting_parenL.lexeme_type != LexemeClass::ParenL) {
        LOG_ERROR("Expected ParenL in argument, received "
                  << expecting_parenL.tokens << " of type "
                  << magic_enum::enum_name(expecting_parenL.lexeme_type) << " at "
                  << std::get<1>(lexemes[x + 2]))
        throw std::runtime_error("Received unexpected lexeme");
    }

    // Find out the number of arguments
    // start is parenL
    auto location_parenR = std::find_if(
        lexemes.begin() + x + 3, lexemes.end(), [](const std::tuple<Lexeme, Location>& x) {
            return std::get<0>(x).lexeme_type == LexemeClass::ParenR;
        });
    if(location_parenR == lexemes.end()) {
        LOG_ERROR("Expected ParenR ')' for function after arguments "
                  << expecting_identifier.tokens << " at location " << std::get<1>(lexemes[x + 1]))
        throw std::runtime_error("Received unexpected lexeme");
    }
    size_t index_parenR = location_parenR - lexemes.begin();
    // Parse arguments
    std::vector<std::tuple<Lexeme, Lexeme, Lexeme>> arguments;
    for(size_t p = x + 3; p < index_parenR; p = p) {
        // Expect the structure of the arguments
        // Identifier
        // Colon
        // Type
        // Comma
        Lexeme expecting_identifier_arg = std::get<0>(lexemes[p]);
        if(expecting_identifier_arg.lexeme_type != LexemeClass::Identifier) {
            LOG_ERROR("Expected Identifier in argument, received "
                      << expecting_identifier_arg.tokens << " of type "
                      << magic_enum::enum_name(expecting_identifier_arg.lexeme_type) << " at "
                      << std::get<1>(lexemes[p]))
            throw std::runtime_error("Received unexpected lexeme");
        }
        Lexeme expecting_colon = std::get<0>(lexemes[p + 1]);
        if(expecting_colon.lexeme_type != LexemeClass::Colon) {
            LOG_ERROR("Expected Colon in argument, received "
                      << expecting_colon.tokens << " of type "
                      << magic_enum::enum_name(expecting_colon.lexeme_type) << " at "
                      << std::get<1>(lexemes[p + 1]))
            throw std::runtime_error("Received unexpected lexeme");
        }
        Lexeme expecting_type = std::get<0>(lexemes[p + 2]);
        // TODO:: This type checking will need to be much better
        if(expecting_type.lexeme_type != LexemeClass::FloatType &&
           expecting_type.lexeme_type != LexemeClass::IntegerType &&
           expecting_type.lexeme_type != LexemeClass::UIntegerType) {
            LOG_ERROR("Expected Type in argument, received "
                      << expecting_type.tokens << " of type "
                      << magic_enum::enum_name(expecting_type.lexeme_type) << " at "
                      << std::get<1>(lexemes[p + 2]))
            throw std::runtime_error("Received unexpected lexeme");
        }
        // Can expect comma or ParenR
        Lexeme expecting_comma = std::get<0>(lexemes[p + 3]);
        if(expecting_comma.lexeme_type != LexemeClass::Comma && p + 3 != index_parenR) {
            LOG_ERROR("Expected Comma in argument, received "
                      << expecting_comma.tokens << " of type "
                      << magic_enum::enum_name(expecting_comma.lexeme_type) << " at "
                      << std::get<1>(lexemes[p + 3]))
            throw std::runtime_error("Received unexpected lexeme");
        }
        arguments.emplace_back(expecting_identifier_arg, expecting_colon, expecting_type);
        p += 4;
    }

    // Write the arguments to the graph
    for(auto arg : arguments) {
        // Create the argument node
        Tree::vertex_descriptor argument_node = boost::add_vertex(ast);
        Tree::vertex_descriptor type_node     = boost::add_vertex(ast);
        // Add the argument node to the function node
        boost::add_edge(function_node, argument_node, ast);
        boost::add_edge(argument_node, type_node, ast);
        std::string arg_name  = std::get<0>(arg).tokens + " : " + std::get<2>(arg).tokens;
        std::string type_name = std::get<2>(arg).tokens;
        ast[argument_node]    = ASTNode(ASTNodeType::Argument, arg_name, loc);
        ast[type_node]        = ASTNode(ASTNodeType::Type, type_name, loc);
    }

    // Get the return type and add it to the graph
    size_t location_return_rightArrow = location_parenR - lexemes.begin() + 1;
    Lexeme expecting_right_arrow      = std::get<0>(lexemes[location_return_rightArrow]);
    if(expecting_right_arrow.lexeme_type == LexemeClass::RightArrow) {
        size_t location_return_parenL = location_parenR - lexemes.begin() + 2;
        Lexeme expecting_return_type  = std::get<0>(lexemes[location_return_parenL]);
        // TODO:: should validate that we have a return statement
        if(expecting_return_type.lexeme_type == LexemeClass::ParenL) {
            // TODO:: Parse return value types, arbitrary number
        }
        else if(expecting_return_type.lexeme_type == LexemeClass::FloatType ||
                expecting_return_type.lexeme_type == LexemeClass::IntegerType ||
                expecting_return_type.lexeme_type == LexemeClass::UIntegerType) {
            // Single return type
            Location                loc_return_type = std::get<1>(lexemes[location_return_parenL]);
            Tree::vertex_descriptor return_type     = boost::add_vertex(ast);
            boost::add_edge(function_node, return_type, ast);
            ast[return_type] =
                ASTNode(ASTNodeType::Return, expecting_return_type.tokens, loc_return_type);
            LOG_DEBUG("Adding single return type in " << ast[function_node].name)
        }
        else {
            LOG_ERROR("Expected type or ParenL after arguments, received "
                      << expecting_identifier.tokens << " of type "
                      << magic_enum::enum_name(expecting_identifier.lexeme_type) << " at "
                      << std::get<1>(lexemes[location_return_parenL]))
            throw std::runtime_error("Received unexpected lexeme");
        }
    }
    else if(expecting_right_arrow.lexeme_type == LexemeClass::CurlL) {
        // Implies that the return type is void
        Tree::vertex_descriptor return_type = boost::add_vertex(ast);
        boost::add_edge(function_node, return_type, ast);
        // void return type put location of curlL
        ast[return_type] =
            ASTNode(ASTNodeType::Return, "void", std::get<1>(lexemes[location_return_rightArrow]));
    }
    else {
        LOG_ERROR("Expected RightArrow '->' after arguments, received "
                  << expecting_right_arrow.tokens << " of type "
                  << magic_enum::enum_name(expecting_right_arrow.lexeme_type) << " at "
                  << std::get<1>(lexemes[location_return_rightArrow]))
        throw std::runtime_error("Received unexpected lexeme");
    }

    auto location_curlL = std::find_if(lexemes.begin() + location_return_rightArrow,
                                       lexemes.end(),
                                       [](const std::tuple<Lexeme, Location>& x) {
                                           return std::get<0>(x).lexeme_type == LexemeClass::CurlL;
                                       });
    std::cout << "Joe" << std::endl;
    x = location_curlL - lexemes.begin();
    scope_stack.push(function_node);
    return x;
}

void draw_graph(Tree ast) {
    // Print the AST
    LOG_INFO("AST Generated")
    std::ofstream             outFile("tree_visualization.dot");
    boost::dynamic_properties dp;
    dp.property("label", boost::get(&ASTNode::name, ast));
    dp.property("node_id", boost::get(boost::vertex_index, ast));
    dp.property("label", boost::get(&ASTEdge::name, ast));
    dp.property("fillcolor", boost::get(&ASTNode::_color, ast));
    dp.property("style", boost::get(&ASTNode::_style, ast));
    dp.property("shape", boost::get(&ASTNode::_shape, ast));
    write_graphviz_dp(outFile, ast, dp);
    outFile.close();
    LOG_INFO("AST Graph in tree_visualization.dot")
}

void generate_ast(std::vector<std::tuple<Lexeme, Location>> lexemes) {
    LOG_INFO("Generating AST")

    Tree                    ast;
    Tree::vertex_descriptor root = boost::add_vertex(ast);
    ast[root].name               = "root";

    std::stack<Tree::vertex_descriptor> scope_stack;
    scope_stack.push(root);

    // Insert the root node
    for(int x = 0; x < lexemes.size(); x++) {
        Lexeme   lexeme = std::get<0>(lexemes[x]);
        Location loc    = std::get<1>(lexemes[x]);

        LOG_DEBUG("Lexeme: " << lexeme.tokens)
        if(lexeme.lexeme_type == LexemeClass::Function) {
            x = ast_gen_function(ast, scope_stack, lexemes, x);
        }
        else if(lexeme.lexeme_type == LexemeClass::CurlR) {
            scope_stack.pop();
        }
        else if(lexeme.lexeme_type == LexemeClass::Identifier) {
        }
        else if(lexeme.lexeme_type == LexemeClass::CurlL) {
            vertex_t anonymous_scope = boost::add_vertex(ast);
            boost::add_edge(scope_stack.top(), anonymous_scope, ast);
            std::string scope_name = loc.file.filename().string() + "_" + std::to_string(loc.line) +
                                     std::to_string(loc.column);
            ast[anonymous_scope] = ASTNode(ASTNodeType::Function, scope_name, loc);
            scope_stack.push(anonymous_scope);
        }
    }
    draw_graph(ast);
}
