#include <iostream>
#include <string>
#include <exception>
#include <typeinfo>
#include <cctype>
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <fstream>

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

class Token {
    public:
        static std::unordered_map<std::string, TokenType> KEYWORDS;
        TokenType tokenType;
        std::string value;
        Token(TokenType aTokenType, const std::string& aValue);
        void print();
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
Token::Token(TokenType aTokenType, const std::string& aValue) {
    tokenType = aTokenType;
    value = aValue;
}
void Token::print() {
    std::cout << "Token: { TokenType: " << tokenType_tostring(tokenType) << " | Value: \"" << value << "\" }\n";   
}

// --------------------------------------------------------------

class Node;
class NumberNode;
class BinaryOp;
class UnaryOp;
class VariableNode;
class AssignStatement;
class CompoundStatement;
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
        int level;
        const std::string name; // global or procedure name
        // symbols would only be inside this map
    public:
        SymbolTable(int level, const std::string &name) : level(level), name(name) {
            define(std::make_unique<BuiltinTypeSymbol>("INTEGER"));
            define(std::make_unique<BuiltinTypeSymbol>("REAL"));
        };
        // for variable symbols, their type symbols will point to the type symbols in the map.
        void define(std::shared_ptr<Symbol> &&sym) {
            std::string name = sym->name;
            map[name] = sym;
        };
        std::shared_ptr<Symbol> lookup(const std::string &symName) {
            std::shared_ptr<Symbol> result = nullptr;
            auto pair = map.find(symName);
            if (pair != map.end())
                result = pair->second;
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
            type = std::make_unique<Token>(tokenType, tokenType_tostring(tokenType));
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
        char currentChar;
        void error(const std::string &message);
        char peek();
        void advance();
        void skip_comment();
        void skip_whitespace();
        std::string integer();
        std::string identifier();
    public:
        Lexer(const std::string &aText);
        std::shared_ptr<Token> get_next_token();
    
};
Lexer::Lexer(const std::string &aText) {
    text = aText;
    pos = 0;
    currentChar = text[pos];
}
void Lexer::error(const std::string &message) {
    throw std::runtime_error("Lexer error: " + message);
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
    if (pos >= text.size()) {
        currentChar = '\0';
    }
    else {
        currentChar = text[pos];
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
        return std::make_shared<Token>(TokenType::END_OF_FILE, "EOF");
    }
    while (currentChar == ' ' || currentChar == '\n' || currentChar == '{') {
        if (currentChar == ' ' || currentChar == '\n') {
            skip_whitespace();
        }
        else {
            skip_comment();
        }
    }
    if (currentChar - '0' >= 0 && currentChar - '0' <= 9) {
        return std::make_shared<Token>(TokenType::INT, integer());
    }
    if (isalnum(currentChar)) {
        std::string id = identifier();
        std::string lexeme = id;
        std::transform(lexeme.begin(), lexeme.end(), lexeme.begin(), ::tolower);
        auto pair = Token::KEYWORDS.find(lexeme);
        if (pair != Token::KEYWORDS.end()) {
            return std::make_shared<Token>(pair->second, pair->first);
        }
        return std::make_shared<Token>(TokenType::VARIABLE, id);
    }
    switch (currentChar) { 
        case '+': 
            advance();
            return std::make_shared<Token>(TokenType::ADD, "+");
        case '-':
            advance();
            return std::make_shared<Token>(TokenType::SUB, "-");
        case '*':
            advance();
            return std::make_shared<Token>(TokenType::MUL, "*");
        case '/':
            advance();
            return std::make_shared<Token>(TokenType::DIV, "/");
        case '(':
            advance();
            return std::make_shared<Token>(TokenType::LPAREN, "(");
        case ')':
            advance();
            return std::make_shared<Token>(TokenType::RPAREN, ")");
        case ':':
            if (peek() == '=') {
                advance();
                advance();
                return std::make_shared<Token>(TokenType::ASSIGN, ":=");  
            } 
            advance();
            return std::make_shared<Token>(TokenType::COLON, ":");
        case ',':
            advance();
            return std::make_shared<Token>(TokenType::COMMA, ",");
        case '.':
            advance();
            return std::make_shared<Token>(TokenType::DOT, ".");
        case ';':
            advance();
            return std::make_shared<Token>(TokenType::SEMI, ";");
    }

    std::string errormsg = "Invalid token ";
    error(errormsg + "\'" + currentChar + "\'" + " found");
    return nullptr;
}

// -------------------------------------------------------------------------

class Parser {
    private:
        std::unique_ptr<Lexer> lexer;
        std::shared_ptr<Token> currentToken;
        void error(const std::string &message);
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
void Parser::error(const std::string &message) {
    throw std::runtime_error("Parser error: " +message);
}
void Parser::eat(TokenType aTokenType) {
    if (currentToken->tokenType != aTokenType) {
        std::string errormsg = "expected token \'" +tokenType_tostring(aTokenType)+ "\', got \'" +tokenType_tostring(currentToken->tokenType)+ "\' token";
        std::cout << currentToken << std::endl;
        error(errormsg);
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
        error("missing \'END\' token");
    }
    if (currentToken->tokenType == TokenType::END) {
        std::unique_ptr<Node>statement = std::unique_ptr<Node>(emptyStatement());
        list.push_back(std::move(statement));
        return std::move(list);
    }
    if (currentToken->tokenType == TokenType::BEGIN) {
        list.push_back(std::unique_ptr<Node>(compoundStatement()));
        eat(TokenType::SEMI);
        return statementList(list);
    }

    std::unique_ptr<Node> statement = assignStatement();
    list.push_back(std::move(statement));
    if (currentToken->tokenType == TokenType::SEMI) {
        eat(TokenType::SEMI);
    }
    // this->error("Statement list error: edge case reached");
    return statementList(list);
}
std::unique_ptr<Node> Parser::assignStatement() {
    std::shared_ptr<Token> variable = currentToken;
    eat(TokenType::VARIABLE);
    std::unique_ptr<Node> variableNode = std::make_unique<VariableNode>(variable);

    std::shared_ptr<Token> assign = currentToken;
    eat(TokenType::ASSIGN);

    std::unique_ptr<Node> right = factor();
    std::unique_ptr<Node> newNode = std::make_unique<AssignStatement>(std::move(variableNode), assign, std::move(right));
    return newNode;
}
std::unique_ptr<Node> Parser::emptyStatement() {
    return std::make_unique<EmptyStatement>();
}
std::unique_ptr<Node> Parser::factor() {
    std::shared_ptr<Token> current = currentToken;
    if (current->tokenType == TokenType::INT) {
        eat(TokenType::INT);
        return std::make_unique<NumberNode>(current); // passing raw pointer into NumberNode constructor, creating a new shared_ptr
    }
    if (current->tokenType == TokenType::VARIABLE) {
        eat(TokenType::VARIABLE);
        return std::make_unique<VariableNode>(current);
    }
    if (current->tokenType == TokenType::ADD || current->tokenType == TokenType::SUB) {
        switch (current->tokenType) {
            case TokenType::ADD: eat(TokenType::ADD); break;
            case TokenType::SUB: eat(TokenType::SUB); break;
        }
        std::unique_ptr<Node> factorNode = factor();
        std::unique_ptr<Node> unaryOp = std::make_unique<UnaryOp>(current, std::move(factorNode));
        return unaryOp;
    }
    if (current->tokenType == TokenType::LPAREN) {
        eat(TokenType::LPAREN);
        std::unique_ptr<Node> exprRoot = expr();
        eat(TokenType::RPAREN);
        return exprRoot;
    }
    this->error("Found an invalid factor");
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

    public:
        SemanticAnalyzer() {
            symTable = std::make_shared<SymbolTable>(1, "global");
        };

        // Should only be called by interpreter
        // Should be called when this visitor end of life
        std::shared_ptr<SymbolTable> transferSymTable() {
            return symTable;
        }

        void print_table() {
            symTable->print();
        }

        void visitVariableNode(VariableNode *node) override {
            std::string name = node->name;
            if (symTable->lookup(name) == nullptr) {
                throw std::runtime_error("SemanticAnalyzer found undeclared variable \"" +name+ "\"");
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
            node->left->accept(this);
            node->right->accept(this);
        }

        void visitCompoundStatement(CompoundStatement *node) override {
            for (auto &child : node->statementList) {
                child->accept(this);
            }
        }

        void visitVarDeclaration(VarDeclaration *node) override {
            VariableNode *varNode = dynamic_cast<VariableNode*>(node->varNode.get());
            const std::string varName = varNode->name;
            TypeNode *typeNode = dynamic_cast<TypeNode*>(node->typeNode.get());
            const std::string typeName = tokenType_tostring(typeNode->type->tokenType);

            if (symTable->lookup(varNode->name)) {
                throw std::runtime_error("SemanticAnalyzer found duplicate variable \"" +varNode->name+ "\"");
            }

            std::shared_ptr<Symbol> typeSym = symTable->lookup(typeName);
            std::shared_ptr<VarSymbol> varSymbol = std::make_shared<VarSymbol>(varNode->name, typeSym);
            symTable->define(varSymbol);
        }

        void visitDeclarationRoot(DeclarationRoot *node) override {
            for (auto &child : node->declarations) {
                child->accept(this);
            }
        }

        void visitBlock(Block *node) override {
            for (auto &varDeclaration : node->varDeclarations) {
                varDeclaration->accept(this);
            }
            node->compoundStatement->accept(this);
        }

        void visitProgramNode(ProgramNode *node) override {
            node->block->accept(this);
        }
};

// ------------------------------------------------------------------------

class EvalVisitor: public Visitor {
    private:
        std::unordered_map<Node*, int> nodeValues;
        std::unordered_map<std::string, int> varValues;
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
            if (varValues.find(node->name) == varValues.end()) {
                error("Variable \"" +node->name+ "\" was not initialized");
                return;
            }
            nodeValues[node] = varValues[node->name];
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
            varValues[varName] = rightValue;
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
        void visitBlock(Block *node) {
            for (auto &varDeclaration : node->varDeclarations) {
                varDeclaration->accept(this);
            }
            node->compoundStatement->accept(this);
        }
        void visitProgramNode(ProgramNode *node) {
            node->block->accept(this);
        }
};

// ------------------------------------------------------------------------

class PrintVisitor: public Visitor {
    private:
        int level;
        void print_with_tabs(int numTabs, const std::string &msg) {
            for (int i = 0; i < numTabs; ++i) {
                std::printf("    ");
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
    }
    catch(const std::exception& e) {
        error(e.what());
    }
}
void Interpreter::print_postorder() {
    std::unique_ptr<Visitor> printVisitor = std::make_unique<PrintVisitor>();
    root->accept(printVisitor.get());
}
void Interpreter::build_symbol_table() {
    std::unique_ptr<SemanticAnalyzer> builder = std::make_unique<SemanticAnalyzer>();
    try {
        root->accept(builder.get());
        builder->print_table();
    }
    catch (const std::runtime_error& e) {
        error(e.what());
    }
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
    catch (std::runtime_error e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}