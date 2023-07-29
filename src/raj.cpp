#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <cctype>

#include <CLI/CLI.hpp>
#include <catch.hpp>
#include <magic_enum_all.hpp>

#include "Lexer.hpp"

int main(int argc, char** argv) {
    CLI::App    app{"Raj Language Compiler"};
    std::string file;
    // TODO:: Pass file into the source_files vector
    app.add_option("-f,--file", file, "Require a source file")->required()->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);

    std::vector<std::filesystem::path> source_files = {"examples/declaration_array.raj"};
    std::vector<SourceCode>            raw_file     = read_raw_file(source_files);

    for(const auto& file : raw_file) {
        std::vector<Lexeme> lexemes = lex_file(file);

        // Basic debugging
        std::clog << std::endl << "Lexemes Identified: " << std::endl;
        for(const auto& lex : lexemes) {
            std::clog << lex.tokens << " " << magic_enum::enum_name(lex.lexeme_type) << std::endl;
        }
    }

    return 0;
}