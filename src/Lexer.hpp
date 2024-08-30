#pragma once

#include <algorithm>
#include <boost/assert/source_location.hpp>
#include <filesystem>
#include <fstream>

// String Manipulation
#include <regex>
#include <string>

#include <cstddef>

// Logging
#include "logging.hpp"

class SourceCode {
public:
    /* Raw data for the source code, contains Path and Source */
    std::filesystem::path path;
    std::string           raw_document;

    SourceCode();
    SourceCode(const std::filesystem::path& filename, std::string content);
    ~SourceCode();
};

enum class LexerStates {
    Space,
    Word,
    Number,
    Comment,
    Operator,
    Other,
};

enum class LexemeClass {
    Space, // e.g. \t

    Comment, // e.g. // this is a comment

    Identifier, // e.g. banana
    Declaration, // let
    Assignment, // =
    Conditional, // if, else, ternary: ?
    Return, // return

    IntegerLiteral, // e.g. 153
    FloatLiteral, // e.g. 15.3

    IntegerType, // e.g. i1, i8, i16, i32, i64
    UIntegerType, // e.g. u1, u8, u16, u32, u64
    FloatType, // e.g. f32, f64
    Function, // e.g. func
    Array, // e.g. func
    Map, // e.g. func

    SemiColon, // ;
    Colon, // :
    Comma, // ,

    MathExpression, // +, -, /, *, %, ^
    RightArrow, // ->

    CurlL, // {
    CurlR, // }
    SquareL, // [
    SquareR, // ]
    ParenL, // (
    ParenR, // )
    ABrackL, // <
    ABrackR, // >
};

class LexingStateMachine {
public:
    LexerStates state;

    LexingStateMachine();
    ~LexingStateMachine();
};

class Lexeme {
public:
    // Holds each lexeme which is an enum of type of lexeme and corresponding data
    LexemeClass lexeme_type;
    std::string tokens;
    Lexeme();
    explicit Lexeme(std::string tokens);
    ~Lexeme();
    bool operator==(const Lexeme& rhs) const {
        return (rhs.lexeme_type == this->lexeme_type) && (rhs.tokens == this->tokens);
    }
};

class Location {
public:
    size_t                line;
    size_t                column;
    std::filesystem::path file;
    Location();
    Location(size_t line, size_t column, const std::string& file);
    std::string            to_string();
    boost::source_location to_boost_source_location();
    ~Location();
    friend std::ostream& operator<<(std::ostream& os, const Location& loc);
};

std::vector<std::tuple<Lexeme, Location>> lex_file(SourceCode file);
std::vector<SourceCode> read_raw_file(const std::vector<std::filesystem::path>& file_paths);
std::vector<std::tuple<Lexeme, Location>>
filter_spaces(std::vector<std::tuple<Lexeme, Location>> lexemes);
