#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>

#include <CLI/CLI.hpp>
#include <catch.hpp>
#include <magic_enum_all.hpp>
#include <magic_enum.hpp>

#include "AST.hpp"
#include "Lexer.hpp"

#include "logging.hpp"

extern LogLevel currentLogLevel;

int main(int argc, char** argv) {
    CLI::App    app{"Raj Language Compiler"};
    std::string file;
    // TODO:: Pass file into the source_files vector
    app.add_option("-f,--file", file, "Require a source file")->required()->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);

    currentLogLevel                                 = LogLevel::ERROR;
    std::vector<std::filesystem::path> source_files = {"examples/function_calling.raj"};
    std::vector<SourceCode>            raw_file     = read_raw_file(source_files);

    for(const auto& file : raw_file) {
        currentLogLevel                                   = LogLevel::ERROR;
        std::vector<std::tuple<Lexeme, Location>> lexemes = lex_file(file);
        lexemes                                           = filter_spaces(lexemes);
        // Basic debugging
        LOG_DEBUG("Lexemes Identified: ")
        for(const auto& lex : lexemes) {
            LOG_DEBUG(std::get<0>(lex).tokens + " " +
                      std::string(magic_enum::enum_name(std::get<0>(lex).lexeme_type)))
        }
        // Generate the AST for the lexemes
        currentLogLevel = LogLevel::DEBUG;
        generate_ast(lexemes);
    }

    return 0;
}