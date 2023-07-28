#include <fstream>
#include <iostream>
#include <string>

#include <cctype>

#include <CLI/CLI.hpp>
#include <catch.hpp>

#include "Lexer.hpp"

std::vector<SourceCode> read_raw_file(std::vector<std::string> filepaths) {
    // Read every single file in the filepaths and append them to a vector
    // mapping of filepaths and their raw content
    std::vector<SourceCode> raw_source(filepaths.size());
    for(const auto& filename : filepaths) {
        std::clog << "Reading source file: " << filename << std::endl;
        if(filename.length() > 3) {
            auto extension = filename.substr(filename.length() - 3);
            if(extension != "raj" || extension != "jar") {
                std::cerr << "\033[1;31m File: " << filename << " is not of correct file extension [.raj | .jar]\033[0m"
                          << std::endl;
                std::cerr << "\033[1;31m Extension is `" << extension << "`\033[0m" << std::endl;
                std::cerr << "\033[1;31m Exiting... \033[0m" << std::endl;
            }
        }

        std::ifstream file(filename);
        if(!file) {
            std::cerr << "\033[1;31m Error opening file \033[0m" << filename << std::endl;
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
                lsm.state = LexerStates::Space;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = "";
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
                lsm.state = LexerStates::Space;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = "";
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
                lsm.state = LexerStates::Space;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = ch;
                lexemes.push_back(Lexeme(accumulator));
                accumulator = "";
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
            // case LexerStates::Other:
            //     break;
        }
    }
    return lexemes;
}

int main() {
    CLI::App app{"Raj Language Compiler"};

    std::vector<std::string> source_files = {"examples/declaration_array.raj"};

    std::vector<SourceCode> raw_file = read_raw_file(source_files);
    for(const auto& file : raw_file) {
        std::vector<Lexeme> lexemes = lex_file(file);
    }

    return 0;
}