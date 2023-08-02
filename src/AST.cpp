#include "AST.hpp"

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
}

std::string ASTNode::_get_graph_color() const {
    // Pick some random light colors
    switch(this->node_type) {
    case ASTNodeType::Root:
        return "#000000";
    case ASTNodeType::Function:
        return "#6496E1";
    case ASTNodeType::Declaration:
        return "#96c57a";
    case ASTNodeType::Expression:
        return "#ffa55b";
    case ASTNodeType::Argument:
        return "#e278e6";
    case ASTNodeType::Type:
        return "#47dfb9";
        break;
    }
    return "white";
}

ASTNode::~ASTNode() = default;

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
            // Consume the following tokens that we expect
            // Identifier
            // ParenL
            // Arbitrsty number of arguments and types separated by Commas
            // ParenR
            // RightArrow

            Tree::vertex_descriptor function_node        = boost::add_vertex(ast);
            Lexeme                  expecting_identifier = std::get<0>(lexemes[x + 1]);
            if(expecting_identifier.lexeme_type != LexemeClass::Identifier) {
                LOG_ERROR("Expected Identifier in argument, received "
                          << expecting_identifier.tokens << " of type "
                          << magic_enum::enum_name(expecting_identifier.lexeme_type) << " at "
                          << std::get<1>(lexemes[x + 1]))
                return; // Could generate something random?
            }
            Lexeme expecting_parenL = std::get<0>(lexemes[x + 2]);
            if(expecting_parenL.lexeme_type != LexemeClass::ParenL) {
                LOG_ERROR("Expected ParenL in argument, received "
                          << expecting_parenL.tokens << " of type "
                          << magic_enum::enum_name(expecting_parenL.lexeme_type) << " at "
                          << std::get<1>(lexemes[x + 2]))
                return;
            }

            // Find out the number of arguments
            // start is parenL
            auto location_parenR = std::find_if(
                lexemes.begin() + x + 3, lexemes.end(), [](const std::tuple<Lexeme, Location>& x) {
                    return std::get<0>(x).lexeme_type == LexemeClass::ParenR;
                });
            if(location_parenR == lexemes.end()) {
                LOG_ERROR("Expected ParenR ')' for function after arguments "
                          << expecting_identifier.tokens << " at location "
                          << std::get<1>(lexemes[x + 1]))
                return;
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
                              << magic_enum::enum_name(expecting_identifier_arg.lexeme_type)
                              << " at " << std::get<1>(lexemes[p]))
                    return;
                }
                Lexeme expecting_colon = std::get<0>(lexemes[p + 1]);
                if(expecting_colon.lexeme_type != LexemeClass::Colon) {
                    LOG_ERROR("Expected Colon in argument, received "
                              << expecting_colon.tokens << " of type "
                              << magic_enum::enum_name(expecting_colon.lexeme_type) << " at "
                              << std::get<1>(lexemes[p + 1]))
                    return;
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
                    return;
                }
                // Can expect comma or ParenR
                Lexeme expecting_comma = std::get<0>(lexemes[p + 3]);
                if(expecting_comma.lexeme_type != LexemeClass::Comma && p + 3 != index_parenR) {
                    LOG_ERROR("Expected Comma in argument, received "
                              << expecting_comma.tokens << " of type "
                              << magic_enum::enum_name(expecting_comma.lexeme_type) << " at "
                              << std::get<1>(lexemes[p + 3]))
                    return;
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

            boost::add_edge(scope_stack.top(), function_node, ast);
            ast[function_node] = ASTNode(ASTNodeType::Function, expecting_identifier.tokens, loc);
            // ast[function_node].name = expecting_identifier.tokens;

            scope_stack.push(function_node);
        }
        else if(lexeme.lexeme_type == LexemeClass::CurlR) {
            scope_stack.pop();
        }
        else if(lexeme.lexeme_type == LexemeClass::Identifier) {
        }
    }
    // for(auto vd : boost::make_iterator_range(vertices(ast))) {
    //     std::cout << "Vertex descriptor #" << vd << " degree:" << degree(vd, ast)
    //               << " id:" << ast[vd].id << " color:" << ast[vd].color << "\n";
    // }
    // Print the AST
    LOG_INFO("AST Generated")
    std::ofstream             outFile("tree_visualization.dot");
    boost::dynamic_properties dp;
    dp.property("label", boost::get(&ASTNode::name, ast));
    dp.property("node_id", boost::get(boost::vertex_index, ast));
    dp.property("label", boost::get(&ASTEdge::name, ast));
    dp.property("fillcolor", boost::get(&ASTNode::_color, ast));
    dp.property("style", boost::get(&ASTNode::_style, ast));
    write_graphviz_dp(outFile, ast, dp);
    outFile.close();
    LOG_INFO("AST Graph in tree_visualization.dot")
}
