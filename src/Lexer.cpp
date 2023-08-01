
#include <cstddef>
#include <cstdlib>

#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "Lexer.hpp"

SourceCode::SourceCode() {
    LOG_INFO("Initializing Null Source Code");
    this->path         = "/null/path";
    this->raw_document = "";
}
SourceCode::SourceCode(std::filesystem::path filename, std::string content) {
    LOG_INFO("Initializing Source Code: " + std::string(filename));
    this->path         = filename;
    this->raw_document = content;
}
SourceCode::~SourceCode() { }

LexingStateMachine::LexingStateMachine() {
    this->state = LexerStates::Other;
}

LexingStateMachine::~LexingStateMachine() { }

Lexeme::Lexeme() {
    this->lexeme_type = LexemeClass::Space;
    this->tokens      = "";
}

Lexeme::Lexeme(std::string tokens) {
    // Interpret the Lexeme class from the tokens given
    this->tokens = tokens;
    // Logic for interpreting the type of the lexeme
    std::regex all_space("\\s+");
    std::regex is_integer_literal("(\\+|-)?[0-9]+");
    std::regex is_float_literal("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)");
    std::regex is_type("(i(1|8|16|32|64))|(u(1|8|16|32|64))|(f(32|64))");
    std::regex is_math_operator("[+\\-*/%]");
    std::regex is_comment("#[^\n]*");
    std::regex is_identifier("[_a-zA-Z][_a-zA-Z0-9]*");

    if(std::regex_match(tokens, all_space) || tokens == "") {
        this->lexeme_type = LexemeClass::Space;
    }
    else if(std::regex_match(tokens, is_integer_literal)) {
        this->lexeme_type = LexemeClass::IntegerLiteral;
    }
    else if(std::regex_match(tokens, is_float_literal)) {
        this->lexeme_type = LexemeClass::FloatLiteral;
    }
    else if(std::regex_match(tokens, is_type)) {
        if(tokens[0] == 'i') {
            this->lexeme_type = LexemeClass::IntegerType;
        }
        else if(tokens[0] == 'u') {
            this->lexeme_type = LexemeClass::UIntegerType;
        }
        else if(tokens[0] == 'f') {
            this->lexeme_type = LexemeClass::FloatType;
        }
    }
    else if(tokens == "let") {
        this->lexeme_type = LexemeClass::Declaration;
    }
    else if(tokens == "if" || tokens == "else" || tokens == "?") {
        this->lexeme_type = LexemeClass::Conditional;
    }
    else if(tokens == "{") {
        this->lexeme_type = LexemeClass::CurlL;
    }
    else if(tokens == "}") {
        this->lexeme_type = LexemeClass::CurlR;
    }
    else if(tokens == "[") {
        this->lexeme_type = LexemeClass::SquareL;
    }
    else if(tokens == "]") {
        this->lexeme_type = LexemeClass::SquareR;
    }
    else if(tokens == "(") {
        this->lexeme_type = LexemeClass::ParenL;
    }
    else if(tokens == ")") {
        this->lexeme_type = LexemeClass::ParenR;
    }
    else if(tokens == "<") {
        this->lexeme_type = LexemeClass::ABrackL;
    }
    else if(tokens == ">") {
        this->lexeme_type = LexemeClass::ABrackR;
    }
    else if(tokens == ";") {
        this->lexeme_type = LexemeClass::SemiColon;
    }
    else if(tokens == ":") {
        this->lexeme_type = LexemeClass::Colon;
    }
    else if(tokens == ",") {
        this->lexeme_type = LexemeClass::Comma;
    }
    else if(tokens == "=") {
        this->lexeme_type = LexemeClass::Assignment;
    }
    else if(tokens == "func") {
        this->lexeme_type = LexemeClass::Function;
    }
    else if(std::regex_match(tokens, is_math_operator)) {
        this->lexeme_type = LexemeClass::MathExpression;
    }
    else if(std::regex_match(tokens, is_comment)) {
        this->lexeme_type = LexemeClass::Comment;
    }
    else if(std::regex_match(tokens, is_identifier)) {
        this->lexeme_type = LexemeClass::Identifier;
    }
    else {
        LOG_ERROR("Lexeme not recognized: " + tokens);
        LOG_ERROR("Length: " << (tokens.length()));
        LOG_ERROR("Throwing...");
        throw std::runtime_error("Lexeme not recognized");
    }
}

Lexeme::~Lexeme() { }

Location::Location(size_t line, size_t column, std::string file) {
    this->line   = line;
    this->column = column;
    this->file   = file;
}

Location::~Location() { }

std::vector<SourceCode> read_raw_file(std::vector<std::filesystem::path> filepaths) {
    // Read every single file in the filepaths and append them to a vector
    // mapping of filepaths and their raw content
    std::vector<SourceCode> raw_source;
    for(const auto& filename : filepaths) {
        LOG_INFO("Reading source file: " + filename.string());
        auto extension = filename.extension().string();
        if(extension != ".raj" && extension != ".jar") {
            LOG_ERROR("File: " << filename << "is not of correct file extension [.raj | .jar]");
            LOG_ERROR("Extension is `" << extension << "`");
        }

        std::ifstream file(filename);
        if(!file) {
            LOG_ERROR("Error opening file" << filename);
        }
        std::string content((std::istreambuf_iterator<char>(file)),
                            (std::istreambuf_iterator<char>()));
        file.close();
        raw_source.push_back(SourceCode(filename, content + "\n"));
    }
    return raw_source;
}

bool is_operator(int ch) {
    std::vector<char> single_ops = {
        '+', '-', '*', '/', '%', '=', '!', '&', '|', '^', '<', '>', '(', ')', '[', ']', '{', '}'};
    return (std::find(single_ops.begin(), single_ops.end(), ch) != single_ops.end());
}

bool is_double_operator(std::string op) {
    std::vector<std::string> double_ops = {
        "==", "!=", "<=", ">=", "+=", "-=", "*=", "/=", "%=", "&&", "||", "^^"};
    return (std::find(double_ops.begin(), double_ops.end(), op) != double_ops.end());
}

std::vector<std::tuple<Lexeme, Location>> lex_file(SourceCode file) {
    std::vector<std::tuple<Lexeme, Location>> lexemes;
    LexingStateMachine                        lsm         = LexingStateMachine();
    std::string                               accumulator = "";

    size_t line_number   = 1;
    size_t column_number = 1;
    for(size_t x = 0; x < file.raw_document.length(); x++) {
        const char ch = file.raw_document[x];
        column_number++;
        if(ch == '\n') {
            line_number++;
            column_number = 1;
        }
        Location location(line_number, column_number, file.path);

        // Per character do Lexing
        try {
            switch(lsm.state) {
            case LexerStates::Space:
                if(ch == ' ' || ch == '\n' || ch == '\t') {
                    lsm.state = LexerStates::Space;
                    accumulator += ch;
                }
                else if(std::isdigit(ch) || ch == '.') {
                    lsm.state = LexerStates::Number;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else if(ch == '#') {
                    lsm.state = LexerStates::Comment;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else if(std::isalpha(ch) || ch == '_') {
                    lsm.state = LexerStates::Word;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else if(is_operator(ch)) {
                    lsm.state = LexerStates::Operator;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else {
                    lsm.state = LexerStates::Other;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                break;

            case LexerStates::Word:
                if(ch == ' ' || ch == '\n' || ch == '\t') {
                    lsm.state = LexerStates::Space;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else if(std::isalnum(ch) || ch == '_') {
                    lsm.state = LexerStates::Word;
                    accumulator += ch;
                }
                else if(ch == '#') {
                    lsm.state = LexerStates::Comment;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else if(is_operator(ch)) {
                    lsm.state = LexerStates::Operator;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else {
                    lsm.state = LexerStates::Other;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
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
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else if(ch == '#') {
                    lsm.state = LexerStates::Comment;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else if(std::isalpha(ch) || ch == '_') {
                    // token converted to word
                    lsm.state = LexerStates::Word;
                    accumulator += ch;
                }
                else if(is_operator(ch)) {
                    lsm.state = LexerStates::Operator;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else {
                    lsm.state = LexerStates::Other;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                break;

            case LexerStates::Comment:
                if(ch == '\n') {
                    lsm.state = LexerStates::Space;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else {
                    lsm.state = LexerStates::Comment;
                    accumulator += ch;
                }
                break;

            case LexerStates::Operator:
                if(is_double_operator(accumulator)) {
                    lsm.state = LexerStates::Operator;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                // else if(is_operator(ch)) { // Fallback for weirder operators?
                //     lsm.state = LexerStates::Operator;
                //     accumulator += ch;
                // }
                else if(ch == ' ' || ch == '\n' || ch == '\t') {
                    lsm.state = LexerStates::Space;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else if(std::isdigit(ch) || ch == '.') {
                    lsm.state = LexerStates::Number;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else if(std::isalpha(ch) || ch == '_') {
                    lsm.state = LexerStates::Word;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else if(ch == '#') {
                    lsm.state = LexerStates::Comment;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else {
                    lsm.state = LexerStates::Other;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                break;
            case LexerStates::Other:
                if(ch == ' ' || ch == '\n' || ch == '\t') {
                    lsm.state = LexerStates::Space;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else if(ch == '#') {
                    lsm.state = LexerStates::Comment;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else if(std::isdigit(ch) || ch == '.') {
                    lsm.state = LexerStates::Number;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else if(std::isalpha(ch) || ch == '_') {
                    lsm.state = LexerStates::Word;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else if(is_operator(ch)) {
                    lsm.state = LexerStates::Operator;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                else {
                    lsm.state = LexerStates::Other;
                    lexemes.push_back(std::make_tuple(Lexeme(accumulator), location));
                    accumulator = ch;
                }
                break;
            }
        }
        catch(std::runtime_error(err)) {
            LOG_ERROR(err.what());
            LOG_ERROR("Unrecognized lexeme: " << accumulator);
            LOG_ERROR("Failure on line " << file.path << ":" << line_number);
            accumulator = ch;
        }
    }
    return lexemes;
}
