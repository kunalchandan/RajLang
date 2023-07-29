#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <cctype>

#include <CLI/CLI.hpp>
#include <catch.hpp>
#include <magic_enum_all.hpp>

#include "Lexer.hpp"

std::vector<SourceCode> read_raw_file(std::vector<std::filesystem::path> filepaths) {
    // Read every single file in the filepaths and append them to a vector
    // mapping of filepaths and their raw content
    std::vector<SourceCode> raw_source;
    for(const auto& filename : filepaths) {
        std::clog << "Reading source file: " << filename << std::endl;
        auto extension = filename.extension().string();
        if(extension != ".raj" && extension != ".jar") {
            std::cerr << "\033[1;31m    File: " << filename;
            std::cerr << " is not of correct file extension [.raj | .jar] \033[0m" << std::endl;
            std::cerr << "\033[1;31m    Extension is `" << extension << "`\033[0m" << std::endl;
            // std::cerr << "\033[1;31m Exiting... \033[0m" << std::endl;
        }

        std::ifstream file(filename);
        if(!file) {
            std::cerr << "\033[1;31m    Error opening file \033[0m" << filename << std::endl;
        }
        std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
        file.close();
        raw_source.push_back(SourceCode(filename, content));
    }
    return raw_source;
}
std::vector<Lexeme> lex_file(SourceCode file) {
    std::vector<Lexeme> lexemes;
    LexingStateMachine  lsm         = LexingStateMachine();
    std::string         accumulator = "";

    for(const auto& ch : file.raw_document) {

        std::cout << ch;
        // Per character do Lexing
        // lsm.consume(ch);
        switch(lsm.state) {
        case LexerStates::Space:
            if(ch == ' ' || ch == '\n' || ch == '\t') {
                lsm.state = LexerStates::Space;
                accumulator += ch;
            }
            else if(std::isdigit(ch) || ch == '.') {
                lsm.state = LexerStates::Number;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
            }
            else if(ch == '#') {
                lsm.state = LexerStates::Comment;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
            }
            else if(std::isalpha(ch) || ch == '_') {
                lsm.state = LexerStates::Word;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
            }
            else {
                lsm.state = LexerStates::Other;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
            }
            break;

        case LexerStates::Word:
            if(std::isalnum(ch) || ch == '_') {
                lsm.state = LexerStates::Word;
                accumulator += ch;
            }
            else if(ch == '#') {
                lsm.state = LexerStates::Comment;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
            }
            else {
                lsm.state = LexerStates::Other;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
            }
            break;

        case LexerStates::Number:
            if(std::isdigit(ch) || ch == '.' || ch == 'e' || ch == 'E') {
                // Special case for floating point numbers
                lsm.state = LexerStates::Number;
                accumulator += ch;
            }
            else if(ch == ' ' || ch == '\n' || ch == '\t') {
                lsm.state = LexerStates::Space;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
            }
            else if(ch == '#') {
                lsm.state = LexerStates::Comment;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
            }
            else if(std::isalpha(ch) || ch == '_') {
                // token converted to word
                lsm.state = LexerStates::Word;
                accumulator += ch;
            }
            else {
                lsm.state = LexerStates::Other;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
            }
            break;

        case LexerStates::Comment:
            if(ch == '\n') {
                lsm.state = LexerStates::Space;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
            }
            else {
                lsm.state = LexerStates::Comment;
                accumulator += ch;
            }
            break;
        case LexerStates::Other:
            if(ch == ' ' || ch == '\n' || ch == '\t') {
                lsm.state = LexerStates::Space;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
            }
            else if(ch == '#') {
                lsm.state = LexerStates::Comment;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
            }
            else if(std::isdigit(ch) || ch == '.') {
                lsm.state = LexerStates::Number;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
            }
            else if(std::isalpha(ch) || ch == '_') {
                lsm.state = LexerStates::Word;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
            }
            else {
                lsm.state = LexerStates::Other;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
            }
            break;
        }
    }
    return lexemes;
}

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