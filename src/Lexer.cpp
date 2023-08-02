

#include <iostream>
#include <regex>
#include <string>

#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include "Lexer.hpp"

SourceCode::SourceCode() {
    LOG_INFO("Initializing Null Source Code")
    this->path         = "/null/path";
    this->raw_document = "";
}

SourceCode::SourceCode(const std::filesystem::path& filename, std::string content) {
    LOG_INFO("Initializing Source Code: " + std::string(filename))
    this->path         = filename;
    this->raw_document = std::move(content);
}

SourceCode::~SourceCode() = default;

LexingStateMachine::LexingStateMachine() {
    this->state = LexerStates::Other;
}

LexingStateMachine::~LexingStateMachine() = default;

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

    if(std::regex_match(tokens, all_space) || tokens.empty()) {
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
    else if(tokens == "return") {
        this->lexeme_type = LexemeClass::Return;
    }
    else if(tokens == "->") {
        this->lexeme_type = LexemeClass::RightArrow;
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
        LOG_ERROR("Lexeme not recognized: " + tokens)
        LOG_ERROR("Length: " << (tokens.length()))
        LOG_ERROR("Throwing...")
        throw std::runtime_error("Lexeme not recognized");
    }
}

Lexeme::~Lexeme() = default;

Location::Location() {
    this->line   = 0;
    this->column = 0;
    this->file   = "NULL_FILE.txt";
}

Location::Location(size_t line, size_t column, const std::string& file) {
    this->line   = line;
    this->column = column;
    this->file   = file;
}

std::ostream& operator<<(std::ostream& os, const Location& loc) {
    os << loc.file.string() << ":" << loc.line << ":" << loc.column << std::endl;
    return os;
}

Location::~Location() = default;

std::vector<SourceCode> read_raw_file(const std::vector<std::filesystem::path>& file_paths) {
    // Read every single file in the file paths and append them to a vector
    // mapping of file paths and their raw content
    std::vector<SourceCode> raw_source;
    for(const auto& filename : file_paths) {
        LOG_INFO("Reading source file: " + filename.string())
        auto extension = filename.extension().string();
        if(extension != ".raj" && extension != ".jar") {
            LOG_ERROR("File: " << filename << "is not of correct file extension [.raj | .jar]")
            LOG_ERROR("Extension is `" << extension << "`")
        }

        std::ifstream file(filename);
        if(!file) {
            LOG_ERROR("Error opening file" << filename)
        }
        std::string content((std::istreambuf_iterator<char>(file)),
                            (std::istreambuf_iterator<char>()));
        file.close();
        raw_source.emplace_back(filename, content + "\n");
    }
    return raw_source;
}

std::string escape_regex(const std::string& input) {
    std::string result;
    for(char c : input) {
        if(c == '\\' || c == '.' || c == '*' || c == '+' || c == '?' || c == '(' || c == ')' ||
           c == '[' || c == ']' || c == '{' || c == '}' || c == '|' || c == '^' || c == '$' ||
           c == '/') {
            result.push_back('\\'); // Add an escape character before the special character
        }
        result.push_back(c);
    }
    return result;
}

std::string generate_regex_from_strings(const std::vector<std::string>& strings) {
    std::string regexStr;
    for(const auto& str : strings) {
        if(!regexStr.empty()) {
            regexStr += "|"; // Use the pipe symbol for alternation
        }
        regexStr += escape_regex(str);
    }
    return regexStr;
}

std::vector<std::tuple<Lexeme, Location>> generate_operator_lexemes(std::string s, const Location& loc) {
    std::vector<std::string> ops = {
        // Order matters here preference is given to the first string
        "||", "==", "!=", "<=", ">=", "+=", "-=", "*=", "/=", "%=", "&&",
        "^^", "->", "<-", "+",  "-",  "*",  "/",  "%",  "=",  "!",  "&",
        "|",  "^",  "<",  ">",  "(",  ")",  "[",  "]",  "{",  "}",  "\'",
    };

    std::regex                                 words_regex(generate_regex_from_strings(ops));
    std::regex_iterator<std::string::iterator> iter(s.begin(), s.end(), words_regex);
    std::regex_iterator<std::string::iterator> end;
    std::vector<std::string>                   matches;
    std::vector<Location>                      locations;
    uint                                       matched_length = 0;
    while(iter != end) {
        matches.push_back(iter->str()); // Store the matched substring in the vector
        matched_length += iter->str().length();
        locations.emplace_back(loc.line, loc.column + matched_length, loc.file);
        ++iter;
    }
    if(matched_length != s.length()) {
        LOG_ERROR("Error generating operator lexemes")
        LOG_ERROR("Matched length: " << matched_length)
        LOG_ERROR("Actual length: " << s.length() << "String: " << s)
        LOG_ERROR("Throwing...")
        throw std::runtime_error("Error generating operator lexemes");
    }
    std::vector<std::tuple<Lexeme, Location>> lexemes;
    for(size_t i = 0; i < matches.size(); i++) {
        std::string match = matches[i];
        Location    l     = locations[i];
        Lexeme      lexeme(match);
        lexemes.emplace_back(lexeme, l);
    }
    return lexemes;
}

bool is_operator(int ch) {
    std::vector<char> single_ops = {
        '+', '-', '*', '/', '%', '=', '!', '&', '|',  '^',
        '<', '>', '(', ')', '[', ']', '{', '}', '\'',
    };
    return (std::find(single_ops.begin(), single_ops.end(), ch) != single_ops.end());
}

std::vector<std::tuple<Lexeme, Location>> lex_file(SourceCode file) {
    std::vector<std::tuple<Lexeme, Location>> lexemes;
    LexingStateMachine                        lsm         = LexingStateMachine();
    std::string                               accumulator;

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
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                else if(ch == '#') {
                    lsm.state = LexerStates::Comment;
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                else if(std::isalpha(ch) || ch == '_') {
                    lsm.state = LexerStates::Word;
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                else if(is_operator(ch)) {
                    lsm.state = LexerStates::Operator;
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                else {
                    lsm.state = LexerStates::Other;
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                break;

            case LexerStates::Word:
                if(ch == ' ' || ch == '\n' || ch == '\t') {
                    lsm.state = LexerStates::Space;
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                else if(std::isalnum(ch) || ch == '_') {
                    lsm.state = LexerStates::Word;
                    accumulator += ch;
                }
                else if(ch == '#') {
                    lsm.state = LexerStates::Comment;
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                else if(is_operator(ch)) {
                    lsm.state = LexerStates::Operator;
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                else {
                    lsm.state = LexerStates::Other;
                    lexemes.emplace_back(Lexeme(accumulator), location);
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
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                else if(ch == '#') {
                    lsm.state = LexerStates::Comment;
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                else if(std::isalpha(ch) || ch == '_') {
                    // token converted to word
                    lsm.state = LexerStates::Word;
                    accumulator += ch;
                }
                else if(is_operator(ch)) {
                    lsm.state = LexerStates::Operator;
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                else {
                    lsm.state = LexerStates::Other;
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                break;

            case LexerStates::Comment:
                if(ch == '\n') {
                    lsm.state = LexerStates::Space;
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                else {
                    lsm.state = LexerStates::Comment;
                    accumulator += ch;
                }
                break;

            case LexerStates::Operator:
                if(is_operator(ch)) {
                    lsm.state = LexerStates::Operator;
                    accumulator += ch;
                }
                else if(ch == ' ' || ch == '\n' || ch == '\t') {
                    lsm.state       = LexerStates::Space;
                    auto op_lexemes = generate_operator_lexemes(accumulator, location);
                    lexemes.insert(lexemes.end(), op_lexemes.begin(), op_lexemes.end());
                    accumulator = ch;
                }
                else if(std::isdigit(ch) || ch == '.') {
                    lsm.state       = LexerStates::Number;
                    auto op_lexemes = generate_operator_lexemes(accumulator, location);
                    lexemes.insert(lexemes.end(), op_lexemes.begin(), op_lexemes.end());
                    accumulator = ch;
                }
                else if(std::isalpha(ch) || ch == '_') {
                    lsm.state       = LexerStates::Word;
                    auto op_lexemes = generate_operator_lexemes(accumulator, location);
                    lexemes.insert(lexemes.end(), op_lexemes.begin(), op_lexemes.end());
                    accumulator = ch;
                }
                else if(ch == '#') {
                    lsm.state       = LexerStates::Comment;
                    auto op_lexemes = generate_operator_lexemes(accumulator, location);
                    lexemes.insert(lexemes.end(), op_lexemes.begin(), op_lexemes.end());
                    accumulator = ch;
                }
                else {
                    lsm.state       = LexerStates::Other;
                    auto op_lexemes = generate_operator_lexemes(accumulator, location);
                    lexemes.insert(lexemes.end(), op_lexemes.begin(), op_lexemes.end());
                    accumulator = ch;
                }
                break;
            case LexerStates::Other:
                if(ch == ' ' || ch == '\n' || ch == '\t') {
                    lsm.state = LexerStates::Space;
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                else if(ch == '#') {
                    lsm.state = LexerStates::Comment;
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                else if(std::isdigit(ch) || ch == '.') {
                    lsm.state = LexerStates::Number;
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                else if(std::isalpha(ch) || ch == '_') {
                    lsm.state = LexerStates::Word;
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                else if(is_operator(ch)) {
                    lsm.state = LexerStates::Operator;
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                else {
                    lsm.state = LexerStates::Other;
                    lexemes.emplace_back(Lexeme(accumulator), location);
                    accumulator = ch;
                }
                break;
            }
        }
        catch(const std::runtime_error& err) {
            LOG_ERROR(err.what())
            LOG_ERROR("Unrecognized lexeme: " << accumulator)
            LOG_ERROR("Failure on line " << file.path << ":" << line_number)
            accumulator = ch;
        }
    }
    return lexemes;
}

std::vector<std::tuple<Lexeme, Location>>
filter_spaces(std::vector<std::tuple<Lexeme, Location>> lexemes) {
    // Filter out spaces using the modern C++ STL
    // https://stackoverflow.com/a/21204788/8679866
    std::vector<std::tuple<Lexeme, Location>> filtered;
    std::copy_if(lexemes.begin(),
                 lexemes.end(),
                 std::back_inserter(filtered),
                 [](std::tuple<Lexeme, Location> i) {
                     return std::get<0>(i).lexeme_type != LexemeClass::Space;
                 });
    return filtered;
}