#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <cctype>

#include <CLI/CLI.hpp>
#include <catch.hpp>
#include <magic_enum_all.hpp>

#include "Lexer.hpp"

int main() {
    CLI::App app{"Raj Language Compiler"};

    std::vector<std::filesystem::path> source_files = {"examples/declaration_array.raj"};

    std::vector<SourceCode> raw_file = read_raw_file(source_files);
    for(const auto& file : raw_file) {
        // std::cout << "file.path: " << file.path << std::endl;
        // std::cout << "file.raw_document: " << file.raw_document << std::endl;
        std::vector<Lexeme> lexemes = lex_file(file);

        std::cout << std::endl << "Lexemes Identified: " << std::endl;
        for(const auto& lex : lexemes) {
            std::cout << lex.tokens << " " << magic_enum::enum_name(lex.lexeme_type) << std::endl;
        }
    }

    return 0;
}