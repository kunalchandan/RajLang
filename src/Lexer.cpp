
#include "Lexer.hpp"

SourceCode::SourceCode() {
    this->path         = "/null/path";
    this->raw_document = "";
}
SourceCode::SourceCode(std::string filename, std::string content) {
    this->path         = filename;
    this->raw_document = content;
}
SourceCode::~SourceCode() { }

std::map<LexerStates, LexerStates> LexingStateMachine::transition_table = {
    {LexerStates::Space, LexerStates::Space},
    {LexerStates::Space, LexerStates::Space},
    {LexerStates::Space, LexerStates::Space},
    {LexerStates::Space, LexerStates::Space},
    {LexerStates::Space, LexerStates::Space},
    {LexerStates::Space, LexerStates::Space},
    {LexerStates::Space, LexerStates::Space},
    {LexerStates::Space, LexerStates::Space},
};

LexingStateMachine::LexingStateMachine() {
    this->state = LexerStates::Space;
}

LexingStateMachine::~LexingStateMachine() { }

Lexeme::Lexeme(std::string tokens) {
    // Interpret the Lexeme class from the tokens given
    this->tokens = tokens;
    // Logic for interpreting the type of the lexeme
    std::regex all_space("\\s+");
    std::regex is_integer_literal("(\+|-)[0-9]+");
    std::regex is_float_literal("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)");
    std::regex is_type("(i(1|8|16|32|64)|(u(1|8|16|32|64))|(f(32|64))");
    std::regex is_math_operator("[+\\-*/%]");
    std::regex is_comment("#[^\n]*");
    std::regex is_identifier("[_a-zA-Z][_a-zA-Z0-9]*");

    if(std::regex_match(tokens, all_space)) {
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
            this->lexeme_type = LexemeClass::Integer;
        }
        else if(tokens[0] == 'u') {
            this->lexeme_type = LexemeClass::UInteger;
        }
        else if(tokens[0] == 'f') {
            this->lexeme_type = LexemeClass::Float;
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
        std::cerr << "Lexeme not recognized: " << tokens << std::endl;
    }
}

Lexeme::~Lexeme() { }
