
#include "Lexer.hpp"

SourceCode::SourceCode() {
    this->path         = "/null/path";
    this->raw_document = "";
}
SourceCode::SourceCode(std::string filename, std::string content) {
    std::cout << "Initializing SourceCode: " << filename << std::endl;
    this->path         = filename;
    this->raw_document = content;
}
SourceCode::~SourceCode() { }

LexingStateMachine::LexingStateMachine() {
    this->state = LexerStates::Other;
}

LexingStateMachine::~LexingStateMachine() { }

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
        std::cerr << "Lexeme not recognized: " << tokens << std::endl;
        std::cerr << "\033[1;31m^" << tokens << "^\033[0m Length: " << tokens.length() << std::endl;
    }
}

Lexeme::~Lexeme() { }


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
