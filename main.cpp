#include <iostream>
#include <string>
#include <sstream>
#include <exception>
#include <typeinfo>
#include <cctype>
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <fstream>


class ActivationRecord {
    private:
        std::string procedureName;
        std::unordered_map<std::string, int>
            memory;
        int scope;
    public:
        ActivationRecord(std::string procedureName) {
            this->procedureName = procedureName;
        }

        void setScope(int scope) {
            this->scope = scope;
        }

        // void declare(VariableNode *varNode) {
        //     std::string name = varNode->variableToken->value;
            
        // }

        int lookup(std::string name) {
            int result = -1;
            if (memory.find(name) != memory.end())
                result = memory[name];
            return result;
        }

        // the value might change
        void assign(std::string name, int value) {
            memory[name] = value;
        }

        const std::string toString() {
            std::stringstream ss;
            ss << "Activation record: Name = \"" << procedureName
                << "\", Scope = " << scope << "\n";
            for (auto pair : memory) {
                ss << " { \"" << pair.first << "\" = " << pair.second << " }\n";
            }
            return ss.str();
        }
};

// Main class for the call stack, which holes activation records.
class CallStack {
    private:
        std::vector<std::unique_ptr<ActivationRecord>> records;
        int top = -1;

    public:
        CallStack() {};

        bool isEmpty() {
            return top == -1;
        }

        ActivationRecord* peek() {
            ActivationRecord* result = nullptr;
            if (!isEmpty() && top + 1 == records.size())
                result = records[top].get();
            return result;
        }

        void pop() {
            records.erase(records.end() - 1);
            top--;
        }

        void push(std::unique_ptr<ActivationRecord> record) {
            top++;
            ActivationRecord *ptr = record.get();
            records.push_back(std::move(record));

            ptr->setScope(top);
        }

        void print() {
            std::cout << "Call stack:\n";
            for (auto it = records.begin(); it != records.end(); ++it) {
                std::cout << (*it)->toString() << "\n";
            }
        }
};

// ----------------------------------------------------------------------------
enum class TokenType {
    ADD,
    SUB,
    MUL,
    DIV,
    INT_DIV,
    LPAREN,
    RPAREN,
    INT,
    END_OF_FILE,

    BEGIN,
    END,
    COMMA,
    DOT,
    SEMI,
    COLON,
    ASSIGN,
    VARIABLE,
    PROCEDURE,
    PROGRAM,
    PROGRAM_NAME,
    VAR,

    INTEGER,
    REAL,
};


const std::string tokenType_tostring(TokenType aTokenType) {
    switch (aTokenType) {
        case TokenType::ADD: return "ADD";
        case TokenType::SUB: return "SUB";
        case TokenType::MUL: return "MUL";
        case TokenType::DIV: return "DIV";
        case TokenType::INT_DIV: return "INT_DIV";
        case TokenType::INT: return "INT";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::END_OF_FILE: return "EOF";

        case TokenType::BEGIN: return "BEGIN";
        case TokenType::END: return "END";
        case TokenType::COMMA: return "COMMA";
        case TokenType::DOT: return "DOT";
        case TokenType::SEMI: return "SEMI";
        case TokenType::COLON: return "COLON";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::VARIABLE: return "VARIABLE";
        case TokenType::PROCEDURE: return "PROCEDURE";
        case TokenType::PROGRAM: return "PROGRAM";
        case TokenType::PROGRAM_NAME: return "PROGRAM_NAME";
        case TokenType::VAR: return "VAR";
        case TokenType::INTEGER: return "INTEGER";
        case TokenType::REAL: return "REAL";
    }
    return "Unknown";
}

enum class ErrorCode {
    UNEXPECTED_TOKEN,
    UNDECLARED_ID,
    DUPLICATE_ID,
    DUPLICATE_PROCEDURE,
    NONE,
};
const std::string error_tostring(ErrorCode errorType) {
    switch (errorType) {
        case ErrorCode::UNEXPECTED_TOKEN:
            return "Unexpected token";
        case ErrorCode::UNDECLARED_ID:
            return "undeclared identifier";
        case ErrorCode::DUPLICATE_ID:
            return "duplicate identifier";
        case ErrorCode::DUPLICATE_PROCEDURE:
            return "duplicate procedure";
    }
    return "";
}

// ---------------------------------------------------------------------


class Token {
    public:
        static std::unordered_map<std::string, TokenType> KEYWORDS;
        TokenType tokenType;
        std::string value;
        int lineno;
        int column;
        Token(TokenType aTokenType, const std::string& aValue, int lineno, int column);
        void print();
        const std::string toString() {
            std::stringstream ss;
            ss << "{ TokenType::" << tokenType_tostring(tokenType) << " with value \'" << value << "\', line " << lineno << ", col " << column << " }";
            return ss.str();
        }
};

std::unordered_map<std::string, TokenType> Token::KEYWORDS = {
    {"begin", TokenType::BEGIN},
    {"end", TokenType::END},
    {"program", TokenType::PROGRAM},
    {"var", TokenType::VAR},
    {"procedure", TokenType::PROCEDURE},
    {"integer", TokenType::INTEGER},
    {"real", TokenType::REAL},
    {"div", TokenType::INT_DIV},
};
Token::Token(TokenType aTokenType, const std::string& aValue, int lineno, int column) {
    tokenType = aTokenType;
    value = aValue;
    this->lineno = lineno;
    this->column = column;
}
void Token::print() {
    std::cout << "Token: { TokenType: " << tokenType_tostring(tokenType) << " | Value: \"" << value << "\" }\n";   
}

// ---------------------------------------------------------------------

// the base class
class Error: public std::exception {
    protected:
        std::shared_ptr<Token> token;
        ErrorCode code;
        std::string message;
        std::string msg;
    public:
    // what is error code used for?
        Error(const std::string& message, std::shared_ptr<Token> token = nullptr, ErrorCode code = ErrorCode::NONE) : message(message) {
            this->token = token;
            this->code = code;
        }
        const char *what() const noexcept override {
            std::string result = "Base error class";
            return message.c_str();
        }
};

// this will be thrown by the Lexer itself, just needs a message
// since the Lexer builds the message by itself
class LexerError: public Error {
    public:
        // Lexer: unexpected char c at (5:2)
        LexerError(const std::string &message) 
        : Error(message) {}

        const char *what() const noexcept override {
            return message.c_str();
        }
};

class ParserError: public Error {
    private:
        TokenType expected;
        std::shared_ptr<Token> got;
    public:
        ParserError(TokenType expected, std::shared_ptr<Token> got) : Error("") {
            this->expected = expected;
            this->got = got;
            load_message();
        }
        void load_message() {
            std::stringstream ss;
            ss.str("");
            ss <<  "ParserError: expected token \'" << tokenType_tostring(expected) << "\', got \'" << got->toString() << "\' token"; 
            message = ss.str();
        }
        const char *what() const noexcept override {
            return message.c_str();
        }
};

// contains what? Variable name
class SemanticError: public Error {
    private:
        std::shared_ptr<Token> token;
        std::string varName;
        ErrorCode code;
    public:
        SemanticError(std::shared_ptr<Token> token, ErrorCode code) 
        : Error("", nullptr, code) {
            this->token = token;
            this->varName = varName;
            this->code = code;
            load_message();
        }
        void load_message() {
            std::stringstream ss;
            ss.str("");
            ss << "SemanticError: found " << error_tostring(code)
                << " \'" << token->toString() << "\'";
            message = ss.str();
        }
        const char *what() const noexcept override {
            return message.c_str();
        }
};

// --------------------------------------------------------------

class Node;
class NumberNode;
class BinaryOp;
class UnaryOp;
class VariableNode;
class AssignStatement;
class CompoundStatement;
class ProcedureCall;
class EmptyStatement;
class TypeNode;
class VarDeclaration;
class DeclarationRoot;
class Procedure;
class ParamDeclaration;
class Block;
class ProgramNode;

// --------------------------------------------------------------

// Abstract class
class Symbol {
    public:
        enum class Type { INTEGER, REAL, NO_TYPE };
        const std::string name;
        std::shared_ptr<Symbol> type;
    private:
    public:
        Symbol(const std::string &name) : name(name), type(nullptr) {};

        Symbol(const std::string &name, std::shared_ptr<Symbol> type) : name(name), type(type) {};

        virtual void print() = 0;
};

class ProcedureSymbol: public Symbol {
    public:
        ProcedureSymbol(const std::string &name) : Symbol(name) {};
        void print() override {
            std::cout << "Procedure symbol: " << name;
        }
};

class ProgramSymbol: public Symbol {
    public:
        ProgramSymbol(const std::string &name) : Symbol(name) {};
        void print() override {
            std::cout << "Program symbol: " << name;
        }
};

class VarSymbol: public Symbol {
    public:
        VarSymbol(const std::string &name, std::shared_ptr<Symbol> type) : Symbol(name, type) {};

        void print() override {
            std::cout << "Var symbol: " << name << " | ";
            type->print(); 
        }
};

class BuiltinTypeSymbol: public Symbol {
    public:
        BuiltinTypeSymbol(const std::string &name) : Symbol(name) {};
        void print() override {
            std::cout << "Type symbol: " << name;
        }
};


class SymbolTable {
    private:
        std::unordered_map<std::string, std::shared_ptr<Symbol>> map;
        const std::string name; // global or procedure name
        
        // symbols would only be inside this map
    public:
        std::shared_ptr<SymbolTable> enclosingScope;
        int level;
        

        SymbolTable(int level, const std::string &name, const std::shared_ptr<SymbolTable> enclosingScope = nullptr) : level(level), name(name) {
            this->enclosingScope = enclosingScope;
            if (level == 0) {
                define(std::make_unique<BuiltinTypeSymbol>("INTEGER"));
                define(std::make_unique<BuiltinTypeSymbol>("REAL"));
            }
        };
        // for variable symbols, their type symbols will point to the type symbols in the map.
        void define(std::shared_ptr<Symbol> sym) {
            std::string name = sym->name;
            map[name] = sym;
        };
        std::shared_ptr<Symbol> lookup(const std::string &symName, bool local=false) {
            std::shared_ptr<Symbol> result = nullptr;
            auto pair = map.find(symName);
            if (pair != map.end())
                result = pair->second;
            else {
                if (!local && enclosingScope != nullptr)
                    result = enclosingScope->lookup(symName, local);
            }
            return result;
        }
        void print() {
            std::cout << "Scoped symbol table \nLevel: " << level << " | Name: " << name << "\n";
            for (auto &pair : map) {
                std::cout << "Pair: { \"" << pair.first;
                std::cout << "\" --> ";
                pair.second->print();
                std::cout << " }\n";
            }
        }
};

// --------------------------------------------------------------

class Visitor {
    public:
        Visitor() {};
        virtual ~Visitor() {};
        virtual void visitNumberNode(NumberNode *node) {};
        virtual void visitBinaryOp(BinaryOp *node) {};
        virtual void visitUnaryOp(UnaryOp *node) {};
        virtual void visitVariableNode(VariableNode *node) {};
        virtual void visitCompoundStatement(CompoundStatement *node) {};
        virtual void visitAssignStatement(AssignStatement *node) {};
        virtual void visitProcedureCall(ProcedureCall *node) {};
        virtual void visitEmptyStatement(EmptyStatement *node) {};
        virtual void visitVarDeclaration(VarDeclaration *node) {};
        virtual void visitDeclarationRoot(DeclarationRoot *node) {};
        virtual void visitParamDeclaration(ParamDeclaration *node) {};
        virtual void visitProcedure(Procedure *node) {};
        virtual void visitBlock(Block *node) {};
        virtual void visitProgramNode(ProgramNode *node) {};
};

class Node {
    public:
        Node() {};
        virtual ~Node() {};
        Node(Node *node) {};
        virtual void accept(Visitor *visitor) = 0;
        virtual void print() = 0;
        // some derived classes will have child nodes
};


class NumberNode: public Node {
    public:
        std::shared_ptr<Token> token;
        int value;
        NumberNode(std::shared_ptr<Token> token);
        void accept(Visitor *visitor);
        void print();
    private:
        std::string to_string(int num);
};
NumberNode::NumberNode(std::shared_ptr<Token> token) {
    this->token = token;
    this->value = std::stoi(token->value);
}
std::string NumberNode::to_string(int num) {
    std::string res = "";
    while (num > 0) {
        int digit = num % 10;
        res += (char)(digit + '0');
        num /= 10;
    }
    return res;
}
void NumberNode::accept(Visitor *visitor)  {
    visitor->visitNumberNode(this);
}
void NumberNode::print() {
    std::printf("NumberNode: { Value: %d }\n", value);
}


class BinaryOp: public Node {
    public:
        std::shared_ptr<Token> op;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        BinaryOp(std::shared_ptr<Token> op, std::unique_ptr<Node> left, std::unique_ptr<Node> right);
        void accept(Visitor *visitor) override;
        void print() override;
};
BinaryOp::BinaryOp(std::shared_ptr<Token> op, std::unique_ptr<Node> left, std::unique_ptr<Node> right) {
    this->op = op;
    this->left = std::move(left);
    this->right = std::move(right);
}
void BinaryOp::accept(Visitor *visitor) {
    visitor->visitBinaryOp(this);
}
void BinaryOp::print() {
    std::cout << "BinaryOp: { Type: " << tokenType_tostring(op->tokenType) << " }\n";
}


class UnaryOp: public Node {
    public:
        std::shared_ptr<Token> op;
        std::unique_ptr<Node> factor; // only child node
        UnaryOp(std::shared_ptr<Token> op, std::unique_ptr<Node> factor);
        void accept(Visitor *visitor) override;
        void print() override;
};
UnaryOp::UnaryOp(std::shared_ptr<Token> op, std::unique_ptr<Node> factor) {
    this->op = op;
    this->factor = std::move(factor);
}
void UnaryOp::accept(Visitor *visitor) {
    visitor->visitUnaryOp(this);
}
void UnaryOp::print() {
    std::cout << "UnaryOp: { Type: " << tokenType_tostring(op->tokenType) << " }\n";
}


// Does not store a type
class VariableNode: public Node {
    public:
        std::shared_ptr<Token> variableToken;
        std::string name;
        VariableNode(std::shared_ptr<Token> token);
        void accept(Visitor *visitor) override;
        void print() override;
};
VariableNode::VariableNode(std::shared_ptr<Token> token) {
    this->variableToken = token;
    this->name = token->value;
}
void VariableNode::accept(Visitor *visitor) {
    visitor->visitVariableNode(this);
}
void VariableNode::print() {
    std::cout << "Variable {\"name\" = \"" << name << "\"}\n";
}

class CompoundStatement: public Node {
    public:
        std::vector<std::unique_ptr<Node>> statementList;
        CompoundStatement(std::vector<std::unique_ptr<Node>>&& list);
        void accept(Visitor *visitor) override;
        void print() override;
};
CompoundStatement::CompoundStatement(std::vector<std::unique_ptr<Node>>&& list) {
    this->statementList = std::move(list);
}
void CompoundStatement::accept(Visitor *visitor) {
    visitor->visitCompoundStatement(this);
}
void CompoundStatement::print() {
    std::cout << "Compound Statement\n";
}


class ProcedureCall: public Node {
    public:
        std::shared_ptr<Token> procedure;
        std::vector<std::unique_ptr<Node>> args;

        ProcedureCall(std::shared_ptr<Token> &procedure, std::vector<std::unique_ptr<Node>> &&args) {
            this->procedure = procedure;
            this->args = std::move(args);
        }
        void accept(Visitor *visitor) override {
            visitor->visitProcedureCall(this);
        }
        void print() override {
            std::cout << "Procedure call { " << procedure->value << "( ... ) }\n";
        }
};


class AssignStatement: public Node {
    public:
        std::unique_ptr<Node> left;
        std::shared_ptr<Token> assignment;
        std::unique_ptr<Node> right;      
        AssignStatement(std::unique_ptr<Node> variable, std::shared_ptr<Token> assignment, std::unique_ptr<Node> expr);
        void accept(Visitor *visitor) override;
        void print();
};
AssignStatement::AssignStatement(std::unique_ptr<Node> variable, std::shared_ptr<Token> assignment, std::unique_ptr<Node> expr) {
    this->left = std::move(variable);
    this->assignment = assignment;
    this->right = std::move(expr);
}
void AssignStatement::accept(Visitor *visitor) {
    visitor->visitAssignStatement(this);
}
void AssignStatement::print() {
    Node *leftRaw = left.get();
    if (leftRaw == nullptr) return;
    if (typeid(*left) == typeid(VariableNode)) {
        VariableNode *node = dynamic_cast<VariableNode*>(leftRaw);
        std::cout << "Assignment Statement { " << node->name << " = ... }\n";
    }
}


class EmptyStatement: public Node {
    public:
        EmptyStatement() {};
        void accept(Visitor *visitor) override;
        void print() override;
};
void EmptyStatement::accept(Visitor *visitor) {
    visitor->visitEmptyStatement(this);
}
void EmptyStatement::print() {
    std::printf("Empty Statement\n");
}


class TypeNode: public Node {
    public:
        std::unique_ptr<Token> type;
        TypeNode(TokenType tokenType) {
            type = std::make_unique<Token>(tokenType, tokenType_tostring(tokenType), 0, 0);
        }
        void accept(Visitor *visitor) override {}
        void print() override {}
};


// tokenType must be INTEGER or REAL
class VarDeclaration: public Node {
    public:
        std::unique_ptr<Node> varNode;
        std::unique_ptr<Node> typeNode;
        VarDeclaration(std::unique_ptr<Node> varNode, TokenType tokenType) {
            this->varNode = std::move(varNode);
            this->typeNode = std::make_unique<TypeNode>(tokenType);
        }
        void accept(Visitor *visitor) override {
            visitor->visitVarDeclaration(this);
        }
        void print() override {
            // "a : INTEGER"
            VariableNode *varConv = dynamic_cast<VariableNode*>(varNode.get());
            TypeNode *typeConv = dynamic_cast<TypeNode*>(typeNode.get());
            std::cout << "VAR -> " << varConv->name << " : " << tokenType_tostring(typeConv->type->tokenType) << "\n";
        }
};
class DeclarationRoot: public Node {
    public:
        std::vector<std::unique_ptr<Node>> declarations;
        DeclarationRoot() {}
        DeclarationRoot(std::vector<std::unique_ptr<Node>> &declarations) {
            this->declarations = std::move(declarations);
        }
        void accept(Visitor *visitor) override {
            visitor->visitDeclarationRoot(this);
        }
        void print() override {
            std::cout << "Declaration Root\n";
        }
};
class Block: public Node {
    public:
        std::vector<std::unique_ptr<Node>> varDeclarations; // replace decRoot;
        std::unique_ptr<Node> compoundStatement;
        std::vector<std::unique_ptr<Node>> procedures;

        Block( 
            std::unique_ptr<Node> compoundStatement, std::vector<std::unique_ptr<Node>>&& procedures,
            std::vector<std::unique_ptr<Node>>&& varDeclarations
        ) {
            this->compoundStatement = std::move(compoundStatement); 
            this->procedures = std::move(procedures);
            this->varDeclarations = std::move(varDeclarations);
        }
        void accept(Visitor *visitor) override {
            visitor->visitBlock(this);
        }
        void print() override {
            std::cout << "Block\n";
        }
};

class ParamDeclaration: public Node {
    public:
        std::unique_ptr<Node> varNode;
        std::unique_ptr<Node> typeNode;
        ParamDeclaration(std::unique_ptr<Node> &&varNode, TokenType tokenType) {
            this->varNode = std::move(varNode);
            this->typeNode = std::make_unique<TypeNode>(tokenType);
        }
        void accept(Visitor *visitor) override {
            visitor->visitParamDeclaration(this);
        }
        void print() override {
            // "a : INTEGER"
            std::cout << "PARAM -> ";
            VariableNode *varConv = dynamic_cast<VariableNode*>(varNode.get());
            TypeNode *typeConv = dynamic_cast<TypeNode*>(typeNode.get());
            std::cout << varConv->name << " : " << tokenType_tostring(typeConv->type->tokenType) << "\n";
        }
};
class Procedure: public Node {
    public:
        std::shared_ptr<Token> id;
        std::unique_ptr<Node> block;
        std::vector<std::unique_ptr<Node>> paramDeclarations;

        Procedure(std::shared_ptr<Token> id, std::unique_ptr<Node>&& block, std::vector<std::unique_ptr<Node>> params) {
            this->id = id;
            this->block = std::move(block);
            this->paramDeclarations = std::move(params);
        }
        void print() override {
            std::cout << "Procedure \"" << id->value << "\"\n";
        }
        void accept(Visitor *visitor) override {
            visitor->visitProcedure(this);
        }
};
class ProgramNode: public Node {
    public:
        std::shared_ptr<Token> programName;
        std::unique_ptr<Node> block;

        ProgramNode(std::shared_ptr<Token> programName, std::unique_ptr<Node> &&block) {
            this->programName = programName;
            this->block = std::move(block);
        }
        void accept(Visitor *visitor) override {
            visitor->visitProgramNode(this);
        }
        void print() override {
            std::cout << "Program \"" << programName->value << ".pas\" \n";
        }
};


// --------------------------------------------------------------

class Lexer {
    private:
        std::string text;
        int pos;
        int lineno = 1;
        int column = 0;
        void error();
        char peek();
        void advance();
        void skip_comment();
        void skip_whitespace();
        std::string integer();
        std::string identifier();
    public:
        char currentChar;
        Lexer(const std::string &aText);
        std::shared_ptr<Token> get_next_token();
    
};
Lexer::Lexer(const std::string &aText) {
    text = aText;
    pos = 0;
    currentChar = text[pos];
}
void Lexer::error() {
    std::stringstream ss;
    ss.str("");
    ss << "Lexer error: Found unexpected char \'" 
    << currentChar << "\' at line " << lineno << " column " << column;

    throw LexerError(ss.str());
}
char Lexer::peek() {
    int nextPos = pos + 1;
    if (nextPos >= text.size()) {
        return '\0';
    }
    return text[nextPos];
}
void Lexer::advance() {
    ++pos;
    ++column;
    if (pos >= text.size()) {
        currentChar = '\0';
    }
    else {
        currentChar = text[pos];
    }
    if (currentChar == '\n') {
        ++lineno;
        column = 0;
    }
}
void Lexer::skip_comment() {
    while (currentChar != '}' && currentChar != '\0') {
        advance();
    }
    if (currentChar == '}')
        advance();
}
void Lexer::skip_whitespace() {
    while(currentChar == ' ' || currentChar == '\n') {
        advance();
    }
}
std::string Lexer::integer() {
    std::string result = "";
    while(currentChar - '0' >= 0 && currentChar - '0' <= 9) {
        // result.append(currentChar);
        result += currentChar;
        advance();
    }
    return result;
}
std::string Lexer::identifier() {
    std::string result = "";
    while (currentChar != '\0' && isalnum(currentChar)) {
        result += currentChar;
        advance();
    }
    return result;
}
std::shared_ptr<Token> Lexer::get_next_token() {
    if (currentChar == '\0') {
        return std::make_shared<Token>(TokenType::END_OF_FILE, "EOF", lineno, column);
    }
    while (currentChar == ' ' || currentChar == '\n' || currentChar == '{') {
        if (currentChar == ' ' || currentChar == '\n') {
            skip_whitespace();
        }
        else {
            skip_comment();
        }
    }
    int tokenLine = lineno;
    int tokenColumn = column;
    if (currentChar - '0' >= 0 && currentChar - '0' <= 9) {
        return std::make_shared<Token>(TokenType::INT, integer(), tokenLine, tokenColumn);
    }
    else if (isalnum(currentChar)) {
        std::string id = identifier();
        std::string lexeme = id;
        std::transform(lexeme.begin(), lexeme.end(), lexeme.begin(), ::tolower);
        auto pair = Token::KEYWORDS.find(lexeme);
        if (pair != Token::KEYWORDS.end()) {
            return std::make_shared<Token>(pair->second, pair->first, tokenLine, tokenColumn);
        }
        return std::make_shared<Token>(TokenType::VARIABLE, id, tokenLine, tokenColumn);
    }
    switch (currentChar) { 
        case '+': 
            advance();
            return std::make_shared<Token>(TokenType::ADD, "+", tokenLine, tokenColumn);
        case '-':
            advance();
            return std::make_shared<Token>(TokenType::SUB, "-", tokenLine, tokenColumn);
        case '*':
            advance();
            return std::make_shared<Token>(TokenType::MUL, "*", tokenLine, tokenColumn);
        case '/':
            advance();
            return std::make_shared<Token>(TokenType::DIV, "/", tokenLine, tokenColumn);
        case '(':
            advance();
            return std::make_shared<Token>(TokenType::LPAREN, "(", tokenLine, tokenColumn);
        case ')':
            advance();
            return std::make_shared<Token>(TokenType::RPAREN, ")", tokenLine, tokenColumn);
        case ':':
            if (peek() == '=') {
                advance();
                advance();
                return std::make_shared<Token>(TokenType::ASSIGN, ":=", tokenLine, tokenColumn);  
            } 
            advance();
            return std::make_shared<Token>(TokenType::COLON, ":", tokenLine, tokenColumn);
        case ',':
            advance();
            return std::make_shared<Token>(TokenType::COMMA, ",", tokenLine, tokenColumn);
        case '.':
            advance();
            return std::make_shared<Token>(TokenType::DOT, ".", tokenLine, tokenColumn);
        case ';':
            advance();
            return std::make_shared<Token>(TokenType::SEMI, ";", tokenLine, tokenColumn);
    }

    // std::string errormsg = "Invalid token ";
    error();
    return nullptr;
}

// -------------------------------------------------------------------------

class Parser {
    private:
        std::unique_ptr<Lexer> lexer;
        std::shared_ptr<Token> currentToken;
        void error(TokenType expected, std::shared_ptr<Token> got);
        void eat(TokenType aTokenType);
        std::unique_ptr<Node> program(); 
        std::shared_ptr<Token> program_name();
        std::unique_ptr<Node> procedure();
        std::vector<std::unique_ptr<Node>> paramList();
        std::vector<std::unique_ptr<Node>> paramDecLine();
        std::unique_ptr<Node> block();
        std::vector<std::unique_ptr<Node>> procedureList();
        std::vector<std::unique_ptr<Node>> declarationList();
        std::vector<std::unique_ptr<Node>> varList();
        std::unique_ptr<Node> compoundStatement();
        std::vector<std::unique_ptr<Node>> statementList(std::vector<std::unique_ptr<Node>> &list);
        std::unique_ptr<Node> assignStatement();
        std::unique_ptr<Node> procedureCall();
        std::vector<std::unique_ptr<Node>> argList(std::vector<std::unique_ptr<Node>> &list);
        std::unique_ptr<Node> emptyStatement();
        std::unique_ptr<Node> factor();
        std::unique_ptr<Node> term();
        std::unique_ptr<Node> expr();
    public:
        Parser(const std::string &aText);
        ~Parser();
        void print_tokens();
        std::unique_ptr<Node> parse();
};
Parser::Parser(const std::string &aText) {
    lexer = std::make_unique<Lexer>(aText);
    currentToken = lexer->get_next_token();
}
Parser::~Parser() {}
void Parser::print_tokens() {
    while(currentToken != nullptr) {
        currentToken->print();
        if (currentToken->tokenType == TokenType::END_OF_FILE) {
            break;
        }
        currentToken = std::shared_ptr<Token>(lexer->get_next_token());
    }
}
// only called by eat()
void Parser::error(TokenType expected, std::shared_ptr<Token> got) {
    throw ParserError(expected, got);
}
void Parser::eat(TokenType aTokenType) {
    if (currentToken->tokenType != aTokenType) {
        std::string errormsg = "expected token \'" +tokenType_tostring(aTokenType)+ "\', got \'" +tokenType_tostring(currentToken->tokenType)+ "\' token";
        error(aTokenType, currentToken);
    }
    currentToken = lexer->get_next_token();
}
std::unique_ptr<Node> Parser::program() {
    eat(TokenType::PROGRAM);
    std::shared_ptr<Token> name = program_name();
    eat(TokenType::SEMI);
    std::unique_ptr<Node> blockNode = block();
    eat(TokenType::DOT);
    std::unique_ptr<Node> root = std::make_unique<ProgramNode>(name, std::move(blockNode));
    return root;
}
std::shared_ptr<Token> Parser::program_name() {
    std::shared_ptr<Token> name = currentToken;
    eat(TokenType::VARIABLE);
    return name;
}
// PROCEDURE VARIABLE (LPAREN PARAM_LIST RPAREN)? SEMI BLOCK SEMI;
std::unique_ptr<Node> Parser::procedure() {
    eat(TokenType::PROCEDURE);
    std::shared_ptr<Token> name = currentToken;
    eat(TokenType::VARIABLE);

    std::vector<std::unique_ptr<Node>> paramDeclarations;

    if (currentToken->tokenType == TokenType::LPAREN) {
        eat(TokenType::LPAREN);
        paramDeclarations = paramList();
        eat(TokenType::RPAREN);
    }

    eat(TokenType::SEMI);
    std::unique_ptr<Node> blockNode = block();
    eat(TokenType::SEMI);
    return std::make_unique<Procedure>(
        name, std::move(blockNode), std::move(paramDeclarations));
}
// paramDecLine (SEMI paramDecLine)*
// parse through all param arguments between LPAREN and RPAREN
std::vector<std::unique_ptr<Node>> Parser::paramList() {
    std::vector<std::unique_ptr<Node>> list;
    
    std::vector<std::unique_ptr<Node>> lineDecList = paramDecLine();
    list.insert(list.end(), 
        std::make_move_iterator(lineDecList.begin()), 
        std::make_move_iterator(lineDecList.end()));

    if (currentToken->tokenType == TokenType::SEMI) {
        eat(TokenType::SEMI);
        std::vector<std::unique_ptr<Node>> nextList = paramList();
        list.insert(list.end(),
            std::make_move_iterator(nextList.begin()),
            std::make_move_iterator(nextList.end())
        );
    }
    return list;
}
// variable (COMMA variable)* COLON type
std::vector<std::unique_ptr<Node>> Parser::paramDecLine() {
    std::vector<std::unique_ptr<Node>> varsUsing = varList();
    eat(TokenType::COLON);

    TokenType decType = currentToken->tokenType;
    switch (currentToken->tokenType) {
        case TokenType::REAL:
            eat(TokenType::REAL);
            break;
        default:
            eat(TokenType::INTEGER);
    }
    std::vector<std::unique_ptr<Node>> decList;
    for (auto &var : varsUsing) {
        std::unique_ptr<Node> paramDec = 
            std::make_unique<ParamDeclaration>(std::move(var), decType);
        decList.push_back(std::move(paramDec));
    }
    return decList;
}
std::unique_ptr<Node> Parser::block() {
    std::vector<std::unique_ptr<Node>> declarations;
    if (currentToken->tokenType == TokenType::VAR) {
        eat(TokenType::VAR);
        declarations = declarationList();
    }
    std::vector<std::unique_ptr<Node>> procedures = procedureList();
    std::unique_ptr<Node> statementRoot = compoundStatement();

    return std::make_unique<Block>(std::move(statementRoot), 
        std::move(procedures), std::move(declarations));
}
std::vector<std::unique_ptr<Node>> Parser::procedureList() {
    std::vector<std::unique_ptr<Node>> list;
    while(currentToken->tokenType == TokenType::PROCEDURE) {
        std::unique_ptr<Node> proc = procedure();
        list.push_back(std::move(proc));
    }
    return list;
}
std::vector<std::unique_ptr<Node>> Parser::declarationList() {
    std::vector<std::unique_ptr<Node>> list;

    while(currentToken->tokenType == TokenType::VARIABLE) {
        std::vector<std::unique_ptr<Node>> varListResult = varList();
        eat(TokenType::COLON);
        std::shared_ptr<Token> typeToken = currentToken;
        if (currentToken->tokenType == TokenType::INTEGER) {
            eat(TokenType::INTEGER);
        }
        else {
            eat(TokenType::REAL);
        }
        eat(TokenType::SEMI);

        for (auto &varNode : varListResult) {
            std::unique_ptr<VarDeclaration> varDecNode = std::make_unique<VarDeclaration>(std::move(varNode), typeToken->tokenType);
            list.push_back(std::move(varDecNode));
        }
    }

    return list;
}
// list of variable nodes
std::vector<std::unique_ptr<Node>> Parser::varList() {
    std::vector<std::unique_ptr<Node>> list;
    list.push_back(std::make_unique<VariableNode>(currentToken));
    eat(TokenType::VARIABLE);
    
    while(currentToken->tokenType == TokenType::COMMA) {
        eat(TokenType::COMMA);
        list.push_back(std::make_unique<VariableNode>(currentToken));
        eat(TokenType::VARIABLE);
    }
    return list;
}
std::unique_ptr<Node> Parser::compoundStatement() {
    eat(TokenType::BEGIN);
    std::vector<std::unique_ptr<Node>> list;
    list = std::move(statementList(list));
    eat(TokenType::END);

    return std::make_unique<CompoundStatement>(std::move(list));
}
std::vector<std::unique_ptr<Node>> Parser::statementList(std::vector<std::unique_ptr<Node>> &list) {
    if (currentToken->tokenType == TokenType::END_OF_FILE) {
        return std::move(list);
    }
    if (currentToken->tokenType == TokenType::END) {
        std::unique_ptr<Node>statement = std::unique_ptr<Node>(emptyStatement());
        list.push_back(std::move(statement));
        return std::move(list);
    }
    else if (currentToken->tokenType == TokenType::BEGIN) {
        list.push_back(std::unique_ptr<Node>(compoundStatement()));
        eat(TokenType::SEMI);
        return statementList(list);
    }

    // normal circumstance
    std::unique_ptr<Node> statement;
    if (currentToken->tokenType == TokenType::VARIABLE && lexer->currentChar == '(') {
        statement = procedureCall();
    } else {
        statement = assignStatement();
    }
    list.push_back(std::move(statement));
    if (currentToken->tokenType == TokenType::SEMI) {
        eat(TokenType::SEMI);
    }
    return statementList(list);
}
std::unique_ptr<Node> Parser::assignStatement() {
    std::shared_ptr<Token> variable = currentToken;
    eat(TokenType::VARIABLE);
    std::unique_ptr<Node> variableNode = std::make_unique<VariableNode>(variable);

    std::shared_ptr<Token> assign = currentToken;
    eat(TokenType::ASSIGN);

    std::unique_ptr<Node> right = expr();
    std::unique_ptr<Node> newNode = std::make_unique<AssignStatement>(std::move(variableNode), assign, std::move(right));
    return newNode;
}

// name LPAREN expr (COMMA expr)* RPAREN
std::unique_ptr<Node> Parser::procedureCall() {
    std::shared_ptr<Token> proc = currentToken;
    eat(TokenType::VARIABLE);
    eat(TokenType::LPAREN);

    std::vector<std::unique_ptr<Node>> args;
    args = argList(args);

    eat(TokenType::RPAREN);

    std::unique_ptr<Node> node = std::make_unique<ProcedureCall>(proc, std::move(args));

    return node;
}

// expr (, expr)*
std::vector<std::unique_ptr<Node>> Parser::argList(std::vector<std::unique_ptr<Node>> &list) {
    std::unique_ptr<Node> express = expr();

    list.push_back(std::move(express));

    if (currentToken->tokenType == TokenType::COMMA) {
        eat(TokenType::COMMA);
        list = argList(list);
    }
    return std::move(list);
}
std::unique_ptr<Node> Parser::emptyStatement() {
    return std::make_unique<EmptyStatement>();
}
std::unique_ptr<Node> Parser::factor() {
    std::shared_ptr<Token> current = currentToken;
    // regular number node
    if (current->tokenType == TokenType::INT) {
        eat(TokenType::INT);
        return std::make_unique<NumberNode>(current); // passing raw pointer into NumberNode constructor, creating a new shared_ptr
    }
    // case of a variable
    if (current->tokenType == TokenType::VARIABLE) {
        eat(TokenType::VARIABLE);
        return std::make_unique<VariableNode>(current);
    }
    // check for unary operator
    if (current->tokenType == TokenType::ADD || current->tokenType == TokenType::SUB) {
        switch (current->tokenType) {
            case TokenType::ADD: eat(TokenType::ADD); break;
            case TokenType::SUB: eat(TokenType::SUB); break;
        }
        std::unique_ptr<Node> factorNode = factor();
        std::unique_ptr<Node> unaryOp = std::make_unique<UnaryOp>(current, std::move(factorNode));
        return unaryOp;
    }
    // check for an expression
    if (current->tokenType == TokenType::LPAREN) {
        eat(TokenType::LPAREN);
        std::unique_ptr<Node> exprRoot = expr();
        eat(TokenType::RPAREN);
        return exprRoot;
    }
    return nullptr;
}
std::unique_ptr<Node> Parser::term() {
    std::unique_ptr<Node> root = factor();
    while(currentToken->tokenType == TokenType::MUL ||
    currentToken->tokenType == TokenType::DIV ||
    currentToken->tokenType == TokenType::INT_DIV) {
        std::shared_ptr<Token> op = currentToken;
        switch (op->tokenType) {
            case TokenType::MUL:
                eat(TokenType::MUL);
                break;
            case TokenType::DIV:
                eat(TokenType::DIV);
                break;
            default:
                eat(TokenType::INT_DIV);
                break;
        }
        std::unique_ptr<Node> right = factor();
        std::unique_ptr<Node> opNode = std::make_unique<BinaryOp>(op, std::move(root), std::move(right));
        root = std::move(opNode);
    }
    return root;
}
std::unique_ptr<Node> Parser::expr() {
    std::unique_ptr<Node> root = term();
    while(currentToken->tokenType == TokenType::ADD ||
    currentToken->tokenType == TokenType::SUB) {
        std::shared_ptr<Token> op = currentToken;
        switch(op->tokenType) {
            case TokenType::ADD:
                eat(TokenType::ADD);
                break;
            default:
                eat(TokenType::SUB);
                break;
        }
        std::unique_ptr<Node> right = term();
        std::unique_ptr<Node> opNode = std::make_unique<BinaryOp>(op, std::move(root), std::move(right));
        root = std::move(opNode);
    }
    return root;
} 
std::unique_ptr<Node> Parser::parse() {
    return program();
}

// ------------------------------------------------------------------------

class SemanticAnalyzer: public Visitor {
    private:
        std::shared_ptr<SymbolTable> symTable;
        std::shared_ptr<SymbolTable> currentScope;
        std::shared_ptr<SymbolTable> builtinsScope;

    public:
        SemanticAnalyzer() {
            builtinsScope = std::make_shared<SymbolTable>(0, "builtins");
            symTable = std::make_shared<SymbolTable>(1, "global", builtinsScope);
            currentScope = symTable;
        };

        // Should only be called by interpreter
        // Should be called when this visitor end of life
        std::shared_ptr<SymbolTable> transferSymTable() {
            return symTable;
        }

        void print_table() {
            builtinsScope->print();
        }

        void visitVariableNode(VariableNode *node) override {
            std::string name = node->name;
            SymbolTable *curr = currentScope.get();
            if (currentScope->lookup(name) == nullptr) {
                throw SemanticError(node->variableToken, ErrorCode::UNDECLARED_ID);
            }
        }

        void visitUnaryOp(UnaryOp *node) override {
            node->factor->accept(this);
        }

        void visitBinaryOp(BinaryOp *node) override {
            node->left->accept(this);
            node->right->accept(this);
        }

        void visitAssignStatement(AssignStatement *node) override {
            node->right->accept(this);
            node->left->accept(this);
        }

        void visitCompoundStatement(CompoundStatement *node) override {
            for (auto &child : node->statementList) {
                child->accept(this);
            }
        }

        void visitVarDeclaration(VarDeclaration *node) override {
            VariableNode *varNode = dynamic_cast<VariableNode*>(node->varNode.get());
            std::shared_ptr<Token> varToken = varNode->variableToken;
            const std::string varName = varNode->name;
            if (currentScope->lookup(varNode->name, true)) {
                throw SemanticError(varToken, ErrorCode::DUPLICATE_ID);
            }

            TypeNode *typeNode = dynamic_cast<TypeNode*>(node->typeNode.get());
            const std::string typeName = tokenType_tostring(typeNode->type->tokenType);


            std::shared_ptr<Symbol> typeSym = currentScope->lookup(typeName);
            std::shared_ptr<VarSymbol> varSymbol = std::make_shared<VarSymbol>(varNode->name, typeSym);
            currentScope->define(varSymbol);
        }

        // void visitDeclarationRoot(DeclarationRoot *node) override {
        //     for (auto &child : node->declarations) {
        //         child->accept(this);
        //     }
        // }

        void visitParamDeclaration(ParamDeclaration *node) {
            VariableNode* varNode = dynamic_cast<VariableNode*>(node->varNode.get());
            std::shared_ptr<Token> varToken = varNode->variableToken;
            const std::string name = varNode->name;

            if (currentScope->lookup(varNode->name, true)) {
                throw SemanticError(varToken, ErrorCode::DUPLICATE_ID);
            }

            TypeNode* typeNode = dynamic_cast<TypeNode*>(node->typeNode.get());
            const std::string typeName = tokenType_tostring(typeNode->type->tokenType);

            std::shared_ptr<Symbol> paramSym = std::make_shared<VarSymbol>(name, symTable->lookup(typeName));
            currentScope->define(paramSym);
        }

        void visitProcedure(Procedure *node) {
            const std::string procedureName = node->id->value;
            std::shared_ptr<Token> procedureToken = node->id;
            if (symTable->lookup(procedureName)) {
                throw SemanticError(procedureToken, ErrorCode::DUPLICATE_PROCEDURE);
            } else {
                std::shared_ptr<Symbol> procSym = std::make_shared<ProcedureSymbol>(procedureName);
                symTable->define(procSym);

                // increment the scope and change current scope
                currentScope = std::make_shared<SymbolTable>(currentScope->level + 1, procedureName, currentScope);

                for (auto &param : node->paramDeclarations) {
                    param->accept(this);
                }
                node->block->accept(this);
                currentScope->print();

                // decrement the scope
                currentScope = currentScope->enclosingScope;
            }
        }

        void visitBlock(Block *node) override {
            for (auto &varDeclaration : node->varDeclarations) {
                varDeclaration->accept(this);
            }
            for (auto &procedure : node->procedures) {
                procedure->accept(this);
            }
            node->compoundStatement->accept(this);
        }

        void visitProgramNode(ProgramNode *node) override {
            const std::string name = node->programName->value;
            std::shared_ptr<ProgramSymbol> sym = std::make_shared<ProgramSymbol>(name);
            builtinsScope->define(sym);
            node->block->accept(this);
            currentScope->print();
        }
};

// ------------------------------------------------------------------------

class EvalVisitor: public Visitor {
    private:
        std::unordered_map<Node*, int> nodeValues;
        std::unordered_map<std::string, int> varValues;
        std::unique_ptr<CallStack> callStack = std::make_unique<CallStack>();

        void error(const std::string &msg) {
            std::string errormsg = "EvalVisitor error: ";
            throw std::runtime_error(errormsg +msg+ "\n");
        }
    public:
        EvalVisitor() {};
        std::unordered_map<std::string, int> getVarValues() {
            return varValues;
        }
        void visitNumberNode(NumberNode *node) override {
            nodeValues[node] = node->value;
        }
        void visitBinaryOp(BinaryOp *node) override {
            node->left->accept(this);
            node->right->accept(this);
            try {
                int leftVal = nodeValues[node->left.get()];
                int rightVal = nodeValues[node->right.get()];
                int result = 0;
                switch (node->op->tokenType) {
                    case TokenType::ADD: result = leftVal + rightVal; break;
                    case TokenType::SUB: result = leftVal - rightVal; break;
                    case TokenType::MUL: result = leftVal * rightVal; break;
                    case TokenType::DIV: result = leftVal / rightVal; break;
                    case TokenType::INT_DIV: result = leftVal / rightVal; break;
                    default: error("Unknown binary op value");
                }
                nodeValues[node] = result;
            }
            catch (std::runtime_error &e) {
                std::string errormsg = "Invalid node left and right values ";
                error(errormsg + e.what());
            }
        }
        void visitUnaryOp(UnaryOp *node) override {
            node->factor->accept(this);
            int factorVal = nodeValues[node->factor.get()];
            switch (node->op->tokenType) {
                case TokenType::SUB: nodeValues[node] = -factorVal; break;
                case TokenType::ADD: nodeValues[node] = factorVal; break;
                default: error("Invalid unary operator token");
            }
        }
        // only for right-hand side evaluation (math expressions)
        void visitVariableNode(VariableNode *node) override {
            // if (varValues.find(node->name) == varValues.end()) {
            //     error("Variable \"" +node->name+ "\" was not initialized");
            //     return;
            // }
            ActivationRecord *ar = callStack->peek();
            nodeValues[node] = ar->lookup(node->name);
        }
        void visitAssignStatement(AssignStatement *node) {
            VariableNode *leftNode = dynamic_cast<VariableNode*>(node->left.get());
            std::string varName = leftNode->name;
            // if (varValues.find(varName) == varValues.end()) {
            //     error("Variable \"" +varName+ "\" was not declared");
            //     return;
            // }
            node->right->accept(this);
            int rightValue = nodeValues[node->right.get()];
            ActivationRecord *ar = callStack->peek();
            // assert that it cannot be empty
            ar->assign(varName, rightValue);
            // varValues[varName] = rightValue;
        }
        void visitEmptyStatement(EmptyStatement *node) {
            // nothing
        }
        void visitCompoundStatement(CompoundStatement *node) {
            for (auto &child : node->statementList) {
                child->accept(this);
            }
        }
        void visitVarDeclaration(VarDeclaration *node) {
            VariableNode *varNode = dynamic_cast<VariableNode*>(node->varNode.get());
            varValues[varNode->name] = 0;
        }
        void visitDeclarationRoot(DeclarationRoot *node) {
            for (auto &child : node->declarations) {
                child->accept(this);
            }
        }
        void visitProcedure(Procedure* node) {
            // add the stack
            callStack->push(std::make_unique<ActivationRecord>(
                node->id->value
            ));
            // pop the stack
            callStack->pop();
        }
        void visitBlock(Block *node) {
            for (auto &varDeclaration : node->varDeclarations) {
                varDeclaration->accept(this);
            }
            node->compoundStatement->accept(this);
        }
        void visitProgramNode(ProgramNode *node) {
            callStack->push(std::make_unique<ActivationRecord>(
                node->programName->value
            ));
            node->block->accept(this);
            callStack->print();
            callStack->pop();
        }
};

// ------------------------------------------------------------------------

class PrintVisitor: public Visitor {
    private:
        int level;
        void print_with_tabs(int numTabs, const std::string &msg) {
            for (int i = 0; i < numTabs; ++i) {
                std::printf("  ");
            }
            std::cout << msg;
        };
    public:
        PrintVisitor() {
            level = 0;
        };
        void visitNumberNode(NumberNode *node) override {
            print_with_tabs(level, "");
            node->print();
        }
        void visitBinaryOp(BinaryOp *node) override {
            ++level;
            node->left->accept(this);
            node->right->accept(this);
            --level;
            print_with_tabs(level, "");
            node->print();
        }
        void visitUnaryOp(UnaryOp *node) override {
            ++level;
            node->factor->accept(this);
            --level;
            print_with_tabs(level, "");
            node->print();
        }
        void visitVariableNode(VariableNode *node) override {
            print_with_tabs(level, "");
            node->print();
        };
        void visitCompoundStatement(CompoundStatement *node) override {
            for (auto &statement : node->statementList) {
                ++level;
                statement->accept(this);
                --level;
            }
            print_with_tabs(level, "");
            node->print();
        }
        void visitAssignStatement(AssignStatement *node) override {
            ++level;
            node->left->accept(this);
            --level;

            print_with_tabs(level+1, ":=\n");

            ++level;
            node->right->accept(this);
            --level;

            print_with_tabs(level,"");
            node->print();
        }
        void visitProcedureCall(ProcedureCall *node) {
            ++level;
            for (auto &arg : node->args) {
                arg->accept(this);
            }
            --level;
            print_with_tabs(level, "");
            node->print();
        }
        void visitEmptyStatement(EmptyStatement *node) {
            print_with_tabs(level, "");
            node->print();
        }
        void visitVarDeclaration(VarDeclaration *node) {
            print_with_tabs(level, "");
            node->print();
        }
        void visitDeclarationRoot(DeclarationRoot *node) {
            for (auto &dec : node->declarations) {
                ++level;
                dec->accept(this);
                --level;
            }
            print_with_tabs(level, "");
            node->print();
        }
        void visitBlock(Block *node) {
            ++level;
            for (auto &procedure : node->procedures) {
                procedure->accept(this);
            }
            for (auto &varDeclaration : node->varDeclarations) {
                varDeclaration->accept(this);
            }
            node->compoundStatement->accept(this);
            --level;
            print_with_tabs(level, "");
            node->print();
        }
        void visitParamDeclaration(ParamDeclaration *node) {
            print_with_tabs(level, "");
            node->print();
        }
        void visitProcedure(Procedure *node) {
            ++level;
            node->block->accept(this);
            for (auto &dec : node->paramDeclarations) {
                if (dec != nullptr)
                    dec->accept(this);
            }
            --level;
            print_with_tabs(level,"");
            node->print();
        }
        void visitProgramNode(ProgramNode *node) {
            ++level;
            node->block->accept(this);
            --level;
            print_with_tabs(level, "");
            node->print();
        }
};


// -----------------------------------------------------------------------------

class Interpreter {
    private:
        std::unique_ptr<Parser> parser;
        std::unordered_map<std::string, int> GLOBAL_SCOPE;
        std::unique_ptr<Node> root;
        void error(const std::string &message);
    public:
        Interpreter(const std::string &aText);
        void interpret();
        void print_postorder();
        void build_symbol_table();
        void print_global_scope();
};
Interpreter::Interpreter(const std::string &aText) {
    parser = std::make_unique<Parser>(aText);
    root = parser->parse();
}
void Interpreter::error(const std::string &message) {
    throw std::runtime_error(message);
}
void Interpreter::interpret() {
    std::unique_ptr<EvalVisitor> evalVisitor = std::make_unique<EvalVisitor>();
    try {
        root->accept(evalVisitor.get());
        GLOBAL_SCOPE = evalVisitor->getVarValues();
    } catch(const std::exception& e) {
        error(e.what());
    }
}
void Interpreter::print_postorder() {
    std::unique_ptr<Visitor> printVisitor = std::make_unique<PrintVisitor>();
    root->accept(printVisitor.get());
}
// semantic analysis, throws a Semantic Error
void Interpreter::build_symbol_table() {
    std::unique_ptr<SemanticAnalyzer> builder = std::make_unique<SemanticAnalyzer>();
    root->accept(builder.get());
    builder->print_table();
}
void Interpreter::print_global_scope() {
    std::printf("\nGLOBAL SCOPE: \n");
    for (const auto &pair : GLOBAL_SCOPE) {
        std::cout << "{ [\"" << pair.first <<  "\"] = " << pair.second << " }\n";
    }
}

void print_help() {
    std::printf("\n--HELP--:\n");
    std::printf("This is a pascal program interpreter.\n");
    std::printf("When making assignment statements with rvalues besides a single integer, please note to use () for expressions.\n");
    std::printf("\n");
}

const std::string read_file(const std::string &path) {
    std::ifstream file;
    file.open(path);

    if (!file.is_open()) {
        std::cout << "Could not open file\n";
        std::exit(EXIT_FAILURE);
    }
    std::string result;
    std::string lineBuffer;
    while(std::getline(file, lineBuffer)) {
        result += "\n" + lineBuffer;
    }
    file.close();
    return result;
}

void input_loop() {
    while(true) {
        std::printf("\nPlease enter a PASCAL program. (\"exit\" to exit) (\"help\" for help) : >> ");
        std::string input;
        std::getline(std::cin, input);
        if (input.empty()) {
            continue;
        }
        if (input.compare("exit") == 0) {
            std::exit(EXIT_SUCCESS);
        }
        if (input.compare("help") == 0) {
            print_help();
            continue;
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "Must have a program file path.\n";
        std::exit(EXIT_FAILURE);
    }
    std::string programPath = std::string(argv[1]);
    const std::string input = read_file(programPath);
    
    // std::cout << "Program path is " << programPath << "\n";
    // std::cout << "Input string is " << input << "\n";

    try {
        // std::unique_ptr<SymbolTable> tab = std::make_unique<SymbolTable>();
        // tab->print();
        std::unique_ptr<Interpreter> interpreter = std::make_unique<Interpreter>(input);
        interpreter->print_postorder();
        interpreter->build_symbol_table();
        interpreter->interpret();
        interpreter->print_global_scope();
        std::cout << "Done\n";
    }
    catch (const std::exception& e) {
        const char *errormessage = e.what();
        std::cerr << errormessage << std::endl;
    }
    return 0;
}