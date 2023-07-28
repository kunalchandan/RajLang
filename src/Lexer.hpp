#include <algorithm>
#include <map>
#include <regex>

#include <iostream>
#include <string>

class SourceCode {
public:
    /* Raw data for the source code, contains Path and Source */
    std::string path;
    std::string raw_document;

    SourceCode();
    SourceCode(std::string filename, std::string content);
    ~SourceCode();
};

enum class LexerStates {
    Space,
    Word,
    Number,
    Comment,
};

enum class LexemeClass {
    Space, // e.g. \t
    NewLine, // e.g. \n

    Comment, // e.g. // this is a comment

    Identifier, // e.g. banana
    Declaration, // let
    Assignment, // =
    Conditional, // if, else, ?

    IntegerLiteral, // e.g. 153
    FloatLiteral, // e.g. 15.3

    Integer, // e.g. i1, i8, i16, i32, i64
    UInteger, // e.g. u1, u8, u16, u32, u64
    Float, // e.g. f32, f64
    Function, // e.g. func

    SemiColon, // ;
    Colon, // :
    Comma, // ,

    MathExpression, // +, -, /, *, %, ^

    CurlL, // {
    CurlR, // }
    SquareL, // [
    SquareR, // ]
    ParenL, // (
    ParenR, // )
};

class LexingStateMachine {
public:
    LexerStates state;

    static std::map<LexerStates, LexerStates> transition_table;

    LexingStateMachine();
    ~LexingStateMachine();
};

class Lexeme {
public:
    // Holds each lexeme which is an enum of type of lexeme and corresponding data
    LexemeClass lexeme_type;
    std::string tokens;
    Lexeme(std::string tokens);
    ~Lexeme();
};
