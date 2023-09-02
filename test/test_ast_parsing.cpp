#include <catch.hpp> // Include the Catch header
#include <filesystem>

// Include the header of the code you want to test
#include "AST.hpp"
#include "Lexer.hpp"

std::vector<std::tuple<Lexeme, Location>> filtered_lexemes(std::string input) {
    SourceCode sourceCode = SourceCode(std::filesystem::current_path(), input + " \n");
    std::vector<std::tuple<Lexeme, Location>> lexemes = lex_file(sourceCode);
    lexemes                                           = filter_spaces(lexemes);
    return lexemes;
}

TEST_CASE("Test Case 01: Validate Tokenization") {
    std::string input = "func banana(x : i32) -> i32 { return x * x; }";

    std::vector<std::tuple<Lexeme, Location>> lexemes = filtered_lexemes(input);
    for(int i = 0; i < lexemes.size(); ++i) {
        auto lexeme = lexemes[i];
        LOG_DEBUG(std::get<0>(lexeme).tokens << " " << i);
    }
    REQUIRE(lexemes.size() == 16);
}

TEST_CASE("Test Case 02: Validate Basic Types") {
    // i32
    // f64

    Tree                                      tree;
    vertex_t                                  root;
    std::vector<std::tuple<Lexeme, Location>> lexemes;
    Location                                  root_location = Location(0, 0, "Test Case 02");

    // Subtest 1
    lexemes = filtered_lexemes("i32");
    std::cout << lexemes.size() << std::endl;
    for(int i = 0; i < lexemes.size(); ++i) {
        auto lexeme = lexemes[i];
        LOG_DEBUG(std::get<0>(lexeme).tokens << " " << i);
    }

    auto tuple = parse_type(lexemes, root_location);
    tree       = std::get<0>(tuple);
    root       = std::get<1>(tuple);
    // draw_graph(tree);

    REQUIRE(ename(tree[root].node_class) == ename(ASTNodeClass::Type));
    REQUIRE(tree[root].name == "i32");
    REQUIRE(ename(tree[root].sub_type) == ename(ASTNodeSubType::i32));

    // Subtest 2
    lexemes = filtered_lexemes("f64");

    tuple = parse_type(lexemes, root_location);
    tree  = std::get<0>(tuple);
    root  = std::get<1>(tuple);

    REQUIRE(tree[root].node_class == ASTNodeClass::Type);
    REQUIRE(tree[root].name == "f64");
    REQUIRE(tree[root].sub_type == ASTNodeSubType::f64);
}

TEST_CASE("Test Case 03a: Validate Complex Types") {
    // array<i32>
    // map<i32, i32>
    // (i32, i32)
    // (i32, f64)
    // (i32, array<i32>)
    // (array<i32>, map<i32, i32>)
    // func<i32, f32> -> f32
    // func<i32, f32> -> (i32, f32)

    Tree                                      tree;
    vertex_t                                  root;
    std::vector<std::tuple<Lexeme, Location>> lexemes;
    Location                                  root_location = Location(0, 0, "Test Case 02");

    lexemes = filtered_lexemes("array<i32>");

    auto tuple = parse_type(lexemes, root_location);
    tree       = std::get<0>(tuple);
    root       = std::get<1>(tuple);

    REQUIRE(tree[root].node_class == ASTNodeClass::Type);
    REQUIRE(tree[root].name == "array");
    REQUIRE(tree[root].sub_type == ASTNodeSubType::array);
    auto vertex_set = tree.vertex_set();
    for(auto vertex : vertex_set) {
        if(tree[vertex].name == "i32") {
            REQUIRE(tree[vertex].node_class == ASTNodeClass::Type);
            REQUIRE(tree[vertex].name == "i32");
            REQUIRE(tree[vertex].sub_type == ASTNodeSubType::i32);
        }
        else if(tree[vertex].name == "array") {
            REQUIRE(tree[vertex].node_class == ASTNodeClass::Type);
            REQUIRE(tree[vertex].name == "array");
            REQUIRE(tree[vertex].sub_type == ASTNodeSubType::array);
        }
    }
    draw_graph(tree);

    // lexemes = filtered_lexemes("f64");

    // tuple = parse_type(lexemes);
    // tree  = std::get<0>(tuple);
    // root  = std::get<1>(tuple);

    // REQUIRE(tree[root].node_class == ASTNodeClass::Type);
    // REQUIRE(tree[root].name == "f64");
    // REQUIRE(tree[root].sub_type == ASTNodeSubType::f64);
}