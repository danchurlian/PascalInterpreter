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
        TokenType tokenType;
        std::string value;
        Token(TokenType aTokenType, const std::string& aValue);
        void print();
};
Token::Token(TokenType aTokenType, const std::string& aValue) {
    tokenType = aTokenType;
    value = aValue;
}
void Token::print() {
    std::cout << "Token: { TokenType: " << tokenType_tostring(tokenType) << " | Value: \"" << value << "\" }\n";   
}

class Node {
    public:
        Node();
        Node(Node *node);
        virtual ~Node();
        virtual void print();
        // some derived classes will have child nodes
};
Node::Node() {}
Node::Node(Node *node) { std::cout << "Node constructor with node pointer as parameter\n"; }
Node::~Node() {}
void Node::print() {}

class NumberNode: public Node {
    public:
        std::shared_ptr<Token> token;
        int value;
        NumberNode(std::shared_ptr<Token> token);
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
void NumberNode::print() {
    std::printf("NumberNode: { Value: %d }\n", value);
}


class BinaryOp: public Node {
    public:
        std::shared_ptr<Token> op;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        BinaryOp(std::shared_ptr<Token> op, std::unique_ptr<Node> left, std::unique_ptr<Node> right);
        void print();
};
BinaryOp::BinaryOp(std::shared_ptr<Token> op, std::unique_ptr<Node> left, std::unique_ptr<Node> right) {
    this->op = op;
    this->left = std::move(left);
    this->right = std::move(right);
}
void BinaryOp::print() {
    std::cout << "BinaryOp: { Type: " << tokenType_tostring(op->tokenType) << " }\n";
}


class UnaryOp: public Node {
    public:
        std::shared_ptr<Token> op;
        std::unique_ptr<Node> factor; // only child node
        UnaryOp(std::shared_ptr<Token> op, std::unique_ptr<Node> factor);
        void print();
};
UnaryOp::UnaryOp(std::shared_ptr<Token> op, std::unique_ptr<Node> factor) {
    this->op = op;
    this->factor = std::move(factor);
}
void UnaryOp::print() {
    std::cout << "Unary Operator Token: { Type: " << tokenType_tostring(op->tokenType) << " }\n";
}

class VariableNode: public Node {
    public:
        std::shared_ptr<Token> variableToken;
        std::string name;
        VariableNode(std::shared_ptr<Token> token);
        void print();
};
VariableNode::VariableNode(std::shared_ptr<Token> token) {
    this->variableToken = token;
    this->name = token->value;
}
void VariableNode::print() {
    std::cout << "Variable {\"name\" = \"" << name << "\"}\n";
}

class CompoundStatement: public Node {
    public:
        std::vector<std::unique_ptr<Node>> statementList;
        CompoundStatement(std::vector<std::unique_ptr<Node>>&& list);
        void print();
};
CompoundStatement::CompoundStatement(std::vector<std::unique_ptr<Node>>&& list) {
    this->statementList = std::move(list);
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
        void print();
};
AssignStatement::AssignStatement(std::unique_ptr<Node> variable, std::shared_ptr<Token> assignment, std::unique_ptr<Node> expr) {
    this->left = std::move(variable);
    this->assignment = assignment;
    this->right = std::move(expr);
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
        EmptyStatement();
        void print();
};
EmptyStatement::EmptyStatement() {}
void EmptyStatement::print() {
    std::printf("Empty Statement\n");
}
class TypeNode: public Node {
    public:
        std::unique_ptr<Token> type;
        TypeNode(TokenType tokenType) {
            type = std::make_unique<Token>(tokenType, tokenType_tostring(tokenType));
        }
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
        void print() override {
            // "a : INTEGER"
            VariableNode *varConv = dynamic_cast<VariableNode*>(varNode.get());
            TypeNode *typeConv = dynamic_cast<TypeNode*>(typeNode.get());
            std::cout << varConv->name << " : " << tokenType_tostring(typeConv->type->tokenType) << "\n";
        }
};
class DeclarationRoot: public Node {
    public:
        std::vector<std::unique_ptr<Node>> declarations;
        DeclarationRoot() {}
        DeclarationRoot(std::vector<std::unique_ptr<Node>> &declarations) {
            this->declarations = std::move(declarations);
        }
        void print() override {
            std::cout << "Declaration Root\n";
        }
};
class Block: public Node {
    public:
        std::unique_ptr<Node> decRoot;
        std::unique_ptr<Node> compoundStatement;

        Block(std::unique_ptr<Node> decRoot, std::unique_ptr<Node> compoundStatement) {
            this->decRoot = std::move(decRoot);
            this->compoundStatement = std::move(compoundStatement); 
        }
        void print() override {
            std::cout << "Block\n";
        }
};
class ProgramNode: public Node {
    public:
        std::shared_ptr<Token> programName;
        std::unique_ptr<Node> block;

        ProgramNode(std::shared_ptr<Token> programName, std::unique_ptr<Node> block) {
            this->programName = programName;
            this->block = std::move(block);
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
        // TokenNode *newToken = new TokenNode(TokenType::END_OF_FILE, 0);
        return std::make_shared<Token>(TokenType::END_OF_FILE, "EOF");
    }
    if (currentChar == ' ' || currentChar == '\n') {
        skip_whitespace();
    }
    if (currentChar - '0' >= 0 && currentChar - '0' <= 9) {
        return std::make_shared<Token>(TokenType::INT, integer());
    }
    if (isalnum(currentChar)) {
        std::string id = identifier();
        std::string lexeme = id;
        std::transform(lexeme.begin(), lexeme.end(), lexeme.begin(), ::tolower);

        if (lexeme.compare("begin") == 0) {
            return std::make_shared<Token>(TokenType::BEGIN, "BEGIN");
        }
        if (lexeme.compare("end") == 0) {
            return std::make_shared<Token>(TokenType::END, "END");
        }
        if (lexeme.compare("program") == 0) {
            return std::make_shared<Token>(TokenType::PROGRAM, "END");
        }
        if (lexeme.compare("var") == 0) {
            return std::make_shared<Token>(TokenType::VAR, "VAR");
        }
        if (lexeme.compare("integer") == 0) {
            return std::make_shared<Token>(TokenType::INTEGER, "INTEGER");
        }
        if (lexeme.compare("real") == 0) {
            return std::make_shared<Token>(TokenType::REAL, "REAL");
        }
        if (lexeme.compare("div") == 0) {
            return std::make_shared<Token>(TokenType::INT_DIV, "DIV");
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
        std::shared_ptr<Token> currentToken; // should this be a raw pointer?
        void error(const std::string &message);
        void eat(TokenType aTokenType);
        std::unique_ptr<Node> program(); 
        std::shared_ptr<Token> program_name();
        std::unique_ptr<Node> block();
        std::unique_ptr<Node> declarationRoot();
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
std::unique_ptr<Node> Parser::block() {
    std::unique_ptr<Node> decRoot = declarationRoot();
    std::unique_ptr<Node> statementRoot = compoundStatement();
    return std::make_unique<Block>(std::move(decRoot), std::move(statementRoot));
}
std::unique_ptr<Node> Parser::declarationRoot() {
    if (currentToken->tokenType == TokenType::VAR) {
        eat(TokenType::VAR);
        std::vector<std::unique_ptr<Node>> list = declarationList();
        return std::make_unique<DeclarationRoot>(list);
    }
    return std::make_unique<DeclarationRoot>();
}
// only runs when there is at least a variable to be declared
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


class Interpreter {
    private:
        std::unique_ptr<Parser> parser;
        std::unordered_map<std::string, int> GLOBAL_SCOPE;
        std::unique_ptr<Node> root;
        void error(const std::string &message);
        int eval_expr(Node *root);
        void eval_helper(Node *root);
        void print_expr_postorder(Node *root, int level);
        void print_with_tabs(int numTabs, const std::string &msg);
        void print_postorder_helper(Node *root, int level);
        // void destructor_helper(Node *root);
    public:
        Interpreter(const std::string &aText);
        // ~Interpreter();
        void interpret();
        void print_postorder();
        void print_global_scope();
};
Interpreter::Interpreter(const std::string &aText) {
    parser = std::make_unique<Parser>(aText);
    root = parser->parse();
    std::cout << "Interpreter is ready\n";
}
// void Interpreter::destructor_helper(Node *root) {
//     if (root == nullptr) return;

//     if (typeid(*root) == typeid(UnaryOp)) {
//         UnaryOp *op = dynamic_cast<UnaryOp*>(root);
//         destructor_helper(op->factor);
//     }

//     if (typeid(*root) == typeid(BinaryOp)) {
//         BinaryOp *op = dynamic_cast<BinaryOp*>(root);
//         destructor_helper(op->left);
//         destructor_helper(op->right);
//     }

//     if (typeid(*root) == typeid(AssignStatement)) {
//         AssignStatement *statement = dynamic_cast<AssignStatement*>(root);
//         // destructor_helper(statement->left);
//         // destructor_helper(statement->right);
//     }   

//     if (typeid(*root) == typeid(CompoundStatement)) {
//         CompoundStatement *comp = dynamic_cast<CompoundStatement*>(root);
//         for (Node *child : comp->statementList) {
//             destructor_helper(child);
//         }
//     }
//     delete root;
//     return;
// }
// Interpreter::~Interpreter() {
//     delete root;
// }


void Interpreter::error(const std::string &message) {
    throw std::runtime_error("Interpreter failed to interpret expression: " +message);
}
int Interpreter::eval_expr(Node *root) {
    if (root == nullptr) return 0;

    if (typeid(*root) == typeid(NumberNode)) {
        NumberNode *numNode = dynamic_cast<NumberNode*>(root);
        return numNode->value;
    }

    if (typeid(*root) == typeid(VariableNode)) {
        VariableNode *node = dynamic_cast<VariableNode*>(root);
        if (GLOBAL_SCOPE.find(node->name) == GLOBAL_SCOPE.end()) {
            error("Variable \"" +node->name+ "\" was not declared");
        }
        return GLOBAL_SCOPE[node->name];
    }

    if (typeid(*root) == typeid(UnaryOp)) {
        UnaryOp *op = dynamic_cast<UnaryOp*>(root);
        int childResult = eval_expr(op->factor.get());
        switch (op->op->tokenType) {
            case TokenType::ADD: return childResult;
            case TokenType::SUB: return -childResult;
        }    
    }

    if (typeid(*root) == typeid(BinaryOp)) {
        BinaryOp *op = dynamic_cast<BinaryOp*>(root);
        int leftResult = eval_expr(op->left.get());
        int rightResult = eval_expr(op->right.get());
    
        switch (op->op->tokenType) {
            case TokenType::ADD: return leftResult + rightResult;
            case TokenType::SUB: return leftResult - rightResult;
            case TokenType::MUL: return leftResult * rightResult;
            case TokenType::DIV: return leftResult / rightResult;
            case TokenType::INT_DIV: return leftResult / rightResult;
        }
    }

    error("Evaluation helper failed");
    return 0;
}
void Interpreter::eval_helper(Node *root) {
    if (root == nullptr) return;

    if (typeid(*root) == typeid(AssignStatement)) {
        AssignStatement *statement = dynamic_cast<AssignStatement*>(root);
        Node *leftRaw = statement->left.get();
        Node *rightRaw = statement->right.get();
        // if (leftRaw == nullptr || rightRaw == nullptr) return; 
        if (typeid(*leftRaw) == typeid(VariableNode)) {
            VariableNode *variableNode = dynamic_cast<VariableNode*>(leftRaw);
            std::string key = variableNode->name;
            if (GLOBAL_SCOPE.find(key) == GLOBAL_SCOPE.end()) {
                error("Variable \"" +key+ "\" was not declared");
            }

            int value = eval_expr(rightRaw);
            this->GLOBAL_SCOPE[key] = value;
        }
        return;
    }   

    if (typeid(*root) == typeid(CompoundStatement)) {
        CompoundStatement *comp = dynamic_cast<CompoundStatement*>(root);
        comp->statementList;
        for (std::unique_ptr<Node>& child : comp->statementList) {
            eval_helper(child.get());
        }
        return;
    }

    if (typeid(*root) == typeid(EmptyStatement)) {
        return;
    }

    if (typeid(*root) == typeid(VarDeclaration)) {
        VarDeclaration *dec = dynamic_cast<VarDeclaration*>(root);
        VariableNode *var = dynamic_cast<VariableNode*>(dec->varNode.get());
        std::string name = var->name;
        GLOBAL_SCOPE[name] = 0;
        return;
    }

    if (typeid(*root) == typeid(DeclarationRoot)) {
        DeclarationRoot *decRoot = dynamic_cast<DeclarationRoot*>(root);
        for (auto &varDec : decRoot->declarations) {
            eval_helper(varDec.get());
        }
        return;
    }

    if (typeid(*root) == typeid(Block)) {
        Block *node = dynamic_cast<Block*>(root);
        eval_helper(node->decRoot.get());
        eval_helper(node->compoundStatement.get());
    }

    if (typeid(*root) == typeid(ProgramNode)) {
        ProgramNode *node = dynamic_cast<ProgramNode*>(root);
        eval_helper(node->block.get());
        return;
    }


}
void Interpreter::interpret() {
    eval_helper(root.get());
}
void Interpreter::print_with_tabs(int numTabs, const std::string &msg) {
    for (int i = 0; i < numTabs; ++i) {
        std::printf("    ");
    }
    std::cout << msg;
}
void Interpreter::print_expr_postorder(Node *root, int level) {
    if (root == nullptr) return;
    if (typeid(*root) == typeid(UnaryOp)) {
        UnaryOp *op = dynamic_cast<UnaryOp*>(root);
        print_expr_postorder(op->factor.get(), level+1);
        print_with_tabs(level, "");
        op->print();
    }
    if (typeid(*root) == typeid(BinaryOp)) {
        BinaryOp *op = dynamic_cast<BinaryOp*>(root);
        print_expr_postorder(op->left.get(), level+1);
        print_expr_postorder(op->right.get(), level+1);
        print_with_tabs(level, "");
        op->print();
    }
    if (typeid(*root) == typeid(NumberNode)) {
        print_with_tabs(level, "");
        root->print();
        return;
    }

    if (typeid(*root) == typeid(VariableNode)) {
        VariableNode *node = dynamic_cast<VariableNode*>(root);
        print_with_tabs(level, "");
        node->print();
        return;
    }
}
// polymorphism and down casting
void Interpreter::print_postorder_helper(Node *root, int level) {
    if (root == nullptr) return; // nullcheck

    if (typeid(*root) == typeid(VariableNode)) {
        VariableNode *node = dynamic_cast<VariableNode*>(root);
        print_with_tabs(level, "");
        node->print();
        return;
    }
    if (typeid(*root) == typeid(CompoundStatement)) {
        CompoundStatement *comp = dynamic_cast<CompoundStatement*>(root);
        for (std::unique_ptr<Node>& child : comp->statementList) {
            print_postorder_helper(child.get(), level+1);
        }
        print_with_tabs(level,"");
        comp->print();
        return;
    }
    if (typeid(*root) == typeid(AssignStatement)) {
        AssignStatement *statement = dynamic_cast<AssignStatement*>(root);
        print_postorder_helper(statement->left.get(), level+1);
        print_with_tabs(level+1, ":=\n");
        print_expr_postorder(statement->right.get(), level+1);
        print_with_tabs(level,"");
        statement->print();
        return;
    }

    if (typeid(*root) == typeid(DeclarationRoot)) {
        DeclarationRoot *decRoot = dynamic_cast<DeclarationRoot*>(root);
        if (decRoot->declarations.empty()) {
            print_with_tabs(level+1,"");
            std::cout << "No variables declared\n";
        }
        else {
            for (auto &varDec : decRoot->declarations) {
                print_with_tabs(level+1,"");
                varDec->print();
            }
        }
        print_with_tabs(level,"");
        decRoot->print();
        return;
    }

    if (typeid(*root) == typeid(Block)) {
        Block *block = dynamic_cast<Block*>(root);
        print_postorder_helper(block->decRoot.get(), level+1);
        print_postorder_helper(block->compoundStatement.get(), level+1);
        print_with_tabs(level,"");
        block->print();
        return;
    }

    if (typeid(*root) == typeid(ProgramNode)) {
        ProgramNode *program = dynamic_cast<ProgramNode*>(root);
        print_postorder_helper(program->block.get(), level+1);
        print_with_tabs(level,"");
        program->print();
        return;
    }
    
    print_with_tabs(level,"");
    root->print();
}
void Interpreter::print_postorder() {
    print_postorder_helper(root.get(), 0);
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
        std::unique_ptr<Interpreter> interpreter = std::make_unique<Interpreter>(input);
        interpreter->print_postorder();
        interpreter->interpret();
        interpreter->print_global_scope();
        std::cout << "Done" << std::endl;
    }
    catch (std::runtime_error e) {
        std::cout << "Exception raised. " << e.what() << std::endl;
    }
    return 0;
}