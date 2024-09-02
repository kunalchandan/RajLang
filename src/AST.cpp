#include "AST.hpp"
#include "Lexer.hpp"
#include "logging.hpp"
#include <boost/assert/source_location.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>
#include <stack>
#include <tuple>
#include <vector>

ASTNode::ASTNode() {
    this->node_class = ASTNodeClass::Root;
    this->name       = "NULL";
    this->location   = Location();
    this->_color     = "white";
}

ASTNode::ASTNode(ASTNodeClass    type,
                 ASTNodeSubType  subtype,
                 std::string     name,
                 const Location& location) {
    this->node_class = type;
    this->sub_type   = subtype;
    this->name       = std::move(name);
    this->location   = location;
    this->_color     = _get_graph_color();
    this->_shape     = _get_graph_shape();
}

std::string ASTNode::_get_graph_color() const {
    // Pick some random light colors
    switch(this->node_class) {
    case ASTNodeClass::Root:
        return "#000000";
    case ASTNodeClass::Function:
        return "#84B6FF";
    case ASTNodeClass::Declaration:
        return "#96c57a";
    case ASTNodeClass::Expression:
        return "#ffa55b";
    case ASTNodeClass::Argument:
        return "#e278e6";
    case ASTNodeClass::Type:
        return "#47dfb9";
    case ASTNodeClass::Return:
        return "#b9e2b9";
    }
    return "white";
}

std::string ASTNode::_get_graph_shape() const {
    // Pick some shapes
    switch(this->node_class) {
    case ASTNodeClass::Root:
        return "ellipse";
    case ASTNodeClass::Function:
        return "signature";
    case ASTNodeClass::Declaration:
        return "component";
    case ASTNodeClass::Expression:
        return "rect";
    case ASTNodeClass::Argument:
        return "component";
    case ASTNodeClass::Type:
        return "invhouse";
    case ASTNodeClass::Return:
        return "rarrow";
    }
    return "ellipse";
}
ASTNode::~ASTNode() = default;

std::tuple<Tree, vertex_t> parse_type(std::vector<std::tuple<Lexeme, Location>>& type_lexemes,
                                      std::stack<vertex_t>&                      type_stack,
                                      Tree&                                      type_tree,
                                      Location                                   root_location) {
    /// returns type tree and reference to root node
    /// Expecting a sequence of lexemes that look like any of the following examples:
    // i32
    // f64
    // array<i32>
    // map<i32, i32>
    // (i32, i32)
    // (i32, f64)
    // (i32, array<i32>)
    // (array<i32>, map<i32, i32>)
    // func<i32, f32> -> f32
    // func<i32, f32> -> (i32, f32)

    LOG_DEBUG("Calling Parse_type " << ename(std::get<0>(type_lexemes[0]).lexeme_type))
    for(size_t i = 0; i < type_lexemes.size(); i++) {
        Lexeme   lexeme = std::get<0>(type_lexemes[i]);
        Location loc    = std::get<1>(type_lexemes[i]);
        LOG_INFO("  0> Adding " << ename(lexeme.lexeme_type) << " tokens " << lexeme.tokens)
        if(lexeme.lexeme_type == LexemeClass::ParenL) {
            // push to stack
            vertex_t tuple_type              = boost::add_vertex(type_tree);
            type_tree[tuple_type].name       = lexeme.tokens;
            type_tree[tuple_type].node_class = ASTNodeClass::Type;
            type_tree[tuple_type].sub_type   = ASTNodeSubType::tuple;
            type_tree[tuple_type].location   = loc;
            type_stack.push(tuple_type);
        }
        else if(lexeme.lexeme_type == LexemeClass::ParenR) {
            // pop from stack
            type_stack.pop();
        }
        else if(lexeme.lexeme_type == LexemeClass::FloatType) {
            // Add child
            vertex_t float_type = boost::add_vertex(type_tree);
            if(lexeme.tokens == "f32") {
                type_tree[type_stack.top()].sub_type = ASTNodeSubType::f32;
            }
            else if(lexeme.tokens == "f64") {
                type_tree[type_stack.top()].sub_type = ASTNodeSubType::f64;
            }
            type_tree[float_type].name       = lexeme.tokens;
            type_tree[float_type].node_class = ASTNodeClass::Type;
            type_tree[float_type].location   = loc;
            if(!type_stack.empty()) {
                boost::add_edge(type_stack.top(), float_type, type_tree);
            }
            else {
                type_stack.push(float_type);
            }
        }
        else if(lexeme.lexeme_type == LexemeClass::IntegerType) {
            // Add child
            LOG_DEBUG("Parse Int")
            LOG_DEBUG(" " << ename(type_tree[type_stack.top()].sub_type))
            LOG_DEBUG(" " << ename(type_tree[type_stack.top()].node_class))
            vertex_t integer_type = boost::add_vertex(type_tree);
            if(lexeme.tokens == "i8") {
                type_tree[type_stack.top()].sub_type = ASTNodeSubType::i8;
            }
            else if(lexeme.tokens == "i16") {
                type_tree[type_stack.top()].sub_type = ASTNodeSubType::i16;
            }
            else if(lexeme.tokens == "i32") {
                type_tree[type_stack.top()].sub_type = ASTNodeSubType::i32;
            }
            else if(lexeme.tokens == "i64") {
                type_tree[type_stack.top()].sub_type = ASTNodeSubType::i64;
            }
            else if(lexeme.tokens == "i128") {
                type_tree[type_stack.top()].sub_type = ASTNodeSubType::i128;
            }
            type_tree[integer_type].name       = lexeme.tokens;
            type_tree[integer_type].node_class = ASTNodeClass::Type;
            type_tree[integer_type].location   = loc;
            if(!type_stack.empty()) {
                boost::add_edge(type_stack.top(), integer_type, type_tree);
            }
            else {
                type_stack.push(integer_type);
            }
        }
        else if(lexeme.lexeme_type == LexemeClass::UIntegerType) {
            // Add child
            vertex_t integer_type = boost::add_vertex(type_tree);
            if(lexeme.tokens == "u8") {
                type_tree[type_stack.top()].sub_type = ASTNodeSubType::u8;
            }
            else if(lexeme.tokens == "u16") {
                type_tree[type_stack.top()].sub_type = ASTNodeSubType::u16;
            }
            else if(lexeme.tokens == "u32") {
                type_tree[type_stack.top()].sub_type = ASTNodeSubType::u32;
            }
            else if(lexeme.tokens == "u64") {
                type_tree[type_stack.top()].sub_type = ASTNodeSubType::u64;
            }
            else if(lexeme.tokens == "u128") {
                type_tree[type_stack.top()].sub_type = ASTNodeSubType::u128;
            }
            type_tree[integer_type].name       = lexeme.tokens;
            type_tree[integer_type].node_class = ASTNodeClass::Type;
            type_tree[integer_type].location   = loc;
            if(!type_stack.empty()) {
                boost::add_edge(type_stack.top(), integer_type, type_tree);
            }
            else {
                type_stack.push(integer_type);
            }
        }
        else if(lexeme.lexeme_type == LexemeClass::Array) {
            // Recursive call after <
            vertex_t array_type              = boost::add_vertex(type_tree);
            type_tree[array_type].name       = lexeme.tokens;
            type_tree[array_type].node_class = ASTNodeClass::Type;
            type_tree[array_type].sub_type   = ASTNodeSubType::array;
            type_tree[array_type].location   = loc;
            type_stack.push(array_type);
            // assert that the next one is a <
            if(std::get<0>(type_lexemes[i + 1]).lexeme_type != LexemeClass::ABrackL) {
                throw BaseException(
                    loc.file, loc.line, loc.column, "Token after 'array' is not '<'");
            }
            int matching_brack = 0; // Store the altitude of brackets, then return index
            for(size_t h_i = i + 1; h_i < type_lexemes.size(); h_i++) {
                LOG_DEBUG("    " << ename(std::get<0>(type_lexemes[h_i]).lexeme_type) << "\t"
                                 << h_i)

                if(std::get<0>(type_lexemes[h_i]).lexeme_type == LexemeClass::ABrackL) {
                    matching_brack += 1;
                }
                else if(std::get<0>(type_lexemes[h_i]).lexeme_type == LexemeClass::ABrackR) {
                    matching_brack -= 1;
                }
                if(matching_brack == 0) {
                    matching_brack = h_i;
                    break;
                }
                if(h_i == type_lexemes.size()) {
                    throw BaseException(boost::source_location(), "Cannot find matching '>'");
                }
            }
            auto subtype = std::vector<std::tuple<Lexeme, Location>>(
                type_lexemes.begin() + i + 1, type_lexemes.begin() + matching_brack + 1);
            LOG_DEBUG("recursive call made for "
                      << "[" << i + 1 << ", " << matching_brack << "]")
            std::tuple<Tree, vertex_t> parsed_subtree =
                parse_type(subtype, type_stack, type_tree, root_location);
            i = matching_brack; // BEWARE: Skip to matching bracket after parsing
            boost::add_edge(array_type, std::get<1>(parsed_subtree), type_tree);
        }
        else if(lexeme.lexeme_type == LexemeClass::Map) {
            // TODO
            // Recursive call after <
        }
        else if(lexeme.lexeme_type == LexemeClass::Function) {
            // TODO
            // Recursive call after <
            // expect ->
            // recursive call
        }
        else if(lexeme.lexeme_type == LexemeClass::ABrackL) {
            // assume (map | array | func) are at top
            if((!type_stack.empty()) &&
               (type_tree[type_stack.top()].sub_type == ASTNodeSubType::map ||
                type_tree[type_stack.top()].sub_type == ASTNodeSubType::func ||
                type_tree[type_stack.top()].sub_type == ASTNodeSubType::array)) {
                LOG_DEBUG("ABrackL Not empty stack and child of [map|func|array] type. "
                          << ename(type_tree[type_stack.top()].sub_type))
            }
            else {
                LOG_DEBUG("ABrackL start of AnonTupleTupe! " << i)
                vertex_t tuple              = boost::add_vertex(type_tree);
                type_tree[tuple].name       = "AnonTypeTuple";
                type_tree[tuple].node_class = ASTNodeClass::Type;
                type_tree[tuple].sub_type   = ASTNodeSubType::tuple;
                type_tree[tuple].location   = loc;

                type_stack.push(tuple);
            }
            // top = ASTNode(ASTNodeClass::Type, ASTNodeSubType::none, "Anon Tuple", loc);
        }
        else if(lexeme.lexeme_type == LexemeClass::Comma) {
            // Do nothing
        }
    }

    return std::tuple(type_tree, type_stack.top());
}

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
                  << ename(expecting_identifier.lexeme_type) << " at "
                  << std::get<1>(lexemes[x + 1]))
        throw std::runtime_error("Received unexpected lexeme"); // Could generate something random?
    }
    boost::add_edge(scope_stack.top(), function_node, ast);
    ast[function_node] =
        ASTNode(ASTNodeClass::Function, ASTNodeSubType::func, expecting_identifier.tokens, loc);

    Lexeme expecting_parenL = std::get<0>(lexemes[x + 2]);
    if(expecting_parenL.lexeme_type != LexemeClass::ParenL) {
        LOG_ERROR("Expected ParenL in argument, received " << expecting_parenL.tokens << " of type "
                                                           << ename(expecting_parenL.lexeme_type)
                                                           << " at " << std::get<1>(lexemes[x + 2]))
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
                      << ename(expecting_identifier_arg.lexeme_type) << " at "
                      << std::get<1>(lexemes[p]))
            throw std::runtime_error("Received unexpected lexeme");
        }
        Lexeme expecting_colon = std::get<0>(lexemes[p + 1]);
        if(expecting_colon.lexeme_type != LexemeClass::Colon) {
            LOG_ERROR("Expected Colon in argument, received "
                      << expecting_colon.tokens << " of type " << ename(expecting_colon.lexeme_type)
                      << " at " << std::get<1>(lexemes[p + 1]))
            throw std::runtime_error("Received unexpected lexeme");
        }
        Lexeme expecting_type = std::get<0>(lexemes[p + 2]);
        // TODO:: This type checking will need to be much better
        if(expecting_type.lexeme_type != LexemeClass::FloatType &&
           expecting_type.lexeme_type != LexemeClass::IntegerType &&
           expecting_type.lexeme_type != LexemeClass::UIntegerType) {
            LOG_ERROR("Expected Type in argument, received "
                      << expecting_type.tokens << " of type " << ename(expecting_type.lexeme_type)
                      << " at " << std::get<1>(lexemes[p + 2]))
            throw std::runtime_error("Received unexpected lexeme");
        }
        // Can expect comma or ParenR
        Lexeme expecting_comma = std::get<0>(lexemes[p + 3]);
        if(expecting_comma.lexeme_type != LexemeClass::Comma && p + 3 != index_parenR) {
            LOG_ERROR("Expected Comma in argument, received "
                      << expecting_comma.tokens << " of type " << ename(expecting_comma.lexeme_type)
                      << " at " << std::get<1>(lexemes[p + 3]))
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
        ast[argument_node] = ASTNode(ASTNodeClass::Argument, ASTNodeSubType::none, arg_name, loc);
        ast[type_node]     = ASTNode(ASTNodeClass::Type, ASTNodeSubType::none, type_name, loc);
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
            LOG_WARNING("NOT IMPLEMENTED: Parse arbitrary number return value types")
            throw std::runtime_error("Unable to parse arbitrary number return value types");
        }
        else if(expecting_return_type.lexeme_type == LexemeClass::FloatType ||
                expecting_return_type.lexeme_type == LexemeClass::IntegerType ||
                expecting_return_type.lexeme_type == LexemeClass::UIntegerType) {
            // Single return type
            Location                loc_return_type = std::get<1>(lexemes[location_return_parenL]);
            Tree::vertex_descriptor return_type     = boost::add_vertex(ast);
            boost::add_edge(function_node, return_type, ast);
            ast[return_type] = ASTNode(ASTNodeClass::Return,
                                       ASTNodeSubType::none,
                                       expecting_return_type.tokens,
                                       loc_return_type);
            LOG_DEBUG("Adding single return type in " << ast[function_node].name)
        }
        else {
            LOG_ERROR("Expected type or ParenL after arguments, received "
                      << expecting_identifier.tokens << " of type "
                      << ename(expecting_identifier.lexeme_type) << " at "
                      << std::get<1>(lexemes[location_return_parenL]))
            throw std::runtime_error("Received unexpected lexeme");
        }
    }
    else if(expecting_right_arrow.lexeme_type == LexemeClass::CurlL) {
        // Implies that the return type is void
        Tree::vertex_descriptor return_type = boost::add_vertex(ast);
        boost::add_edge(function_node, return_type, ast);
        // void return type put location of curlL
        ast[return_type] = ASTNode(ASTNodeClass::Return,
                                   ASTNodeSubType::none,
                                   "void",
                                   std::get<1>(lexemes[location_return_rightArrow]));
    }
    else {
        LOG_ERROR("Expected RightArrow '->' after arguments, received "
                  << expecting_right_arrow.tokens << " of type "
                  << ename(expecting_right_arrow.lexeme_type) << " at "
                  << std::get<1>(lexemes[location_return_rightArrow]))
        throw std::runtime_error("Received unexpected lexeme");
    }

    auto location_curlL = std::find_if(lexemes.begin() + location_return_rightArrow,
                                       lexemes.end(),
                                       [](const std::tuple<Lexeme, Location>& x) {
                                           return std::get<0>(x).lexeme_type == LexemeClass::CurlL;
                                       });
    x                   = location_curlL - lexemes.begin();
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

[[nodiscard]] Tree generate_ast(std::vector<std::tuple<Lexeme, Location>> lexemes) {
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
            try {
                x = ast_gen_function(ast, scope_stack, lexemes, x);
            }
            catch(std::exception& e) {
                LOG_ERROR(e.what())
                throw;
            }
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
            ast[anonymous_scope] =
                ASTNode(ASTNodeClass::Function, ASTNodeSubType::func, scope_name, loc);
            scope_stack.push(anonymous_scope);
        }
    }
    draw_graph(ast);
    return ast;
}
