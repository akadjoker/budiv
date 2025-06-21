#pragma once

 
#include "Utils.hpp"
#include "Lexer.hpp"
#include "Raii.hpp"
 

class Parser;
class Interpreter;
class Process;
class ObjFunction;
class Value;

enum Precedence {
    NONE = 0,
    ASSIGNMENT = 1,     // =, +=, -=
    OR = 2,            //  OR
    XOR = 3,           //  XOR (lógico)
    AND = 4,           //  AND
    BIT_OR = 5,        // |
    BIT_XOR = 6,       // ^ (bitwise)
    BIT_AND = 7,       // &
    EQUALITY = 8,      // ==, !=
    COMPARISON = 9,    // <, >, <=, >=
    SHIFT = 10,        // <<, >>
    TERM = 11,         // +, -
    FACTOR = 12,       // *, /, %
    POWER = 13,        // ^
    UNARY = 14,        // -, +, !, ~, NOT
    CALL = 15,         // . () []
    PRIMARY = 16
};






typedef void (Parser::*PrefixFn)(bool canAssign);
typedef void (Parser::*InfixFn)( bool canAssign);


struct  ParseRule
{
  PrefixFn prefix;
  InfixFn infix;
  Precedence precedence;

  ParseRule() : prefix(nullptr), infix(nullptr), precedence(Precedence::NONE) {}
  
  ParseRule(PrefixFn p, InfixFn i, Precedence prec) 
      : prefix(p), infix(i), precedence(prec) {}

    
  ParseRule& operator=(const ParseRule& other) 
    {
        prefix = other.prefix;
        infix = other.infix;
        precedence = other.precedence;
        return *this;
    }

    
} ;


 


class Parser 
{
private:
    Lexer *lexer;
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
    int index;
   // UnorderedMap<TokenType, ParseRule> rules;
    Interpreter* vm;
    ParseRule rules[256];
    Process* current_process;
    ObjFunction* current_function;
    bool call_return;
    void parsePrecedence(Precedence precedence);

    ParseRule *getRule(TokenType type);
 

    //ParseRule rules[256];
    void initRules() ;
    void synchronize();
    void errorAtCurrent(const String& message);
    void error(const String& message);
    void errorAt(const Token& token, const String& message);
    void consume(TokenType type, const String& message);
    bool match(TokenType type);
    bool check(TokenType type);
    bool isAtEnd();
    void advance();


        void beginScope();
        void endScope();

        void expression(bool canAssign = false);
        void equality(bool canAssign = false);
        void comparison(bool canAssign = false);
        void grouping(bool canAssign = false);
        void term(bool canAssign = false);
        void factor(bool canAssign = false);
        void unary(bool canAssign = false);
        void power(bool canAssign = false);
        void call(bool canAssign = false);
        void primary(bool canAssign = false);
        void number(bool canAssign = false);
        void string(bool canAssign = false);
        void binary(bool canAssign = false);
        void and_(bool canAssign = false);
        void or_(bool canAssign = false);
        void xor_(bool canAssign = false);
        void block();
        void ternario(bool canAssign);
        void literal(bool canAssign = false);

        void declaration();
        void statement();

        void printStatement();
        void frameStatement();
        void varDeclaration();//set
        void varProcessDeclaration();//set
        void variable(bool canAssign);//get
        void ifStatement();
        void doStatement();
        void loopStatement();
        void whileStatement();
        void forStatement();
        void returnStatement();
        void switchStatement();
        void funDeclaration();
        void procDeclaration();
    
        void expressionStatement();
        u8 argumentList();

        void emitConstant(Value value);
        void emitByte(u8 byte);
        void emitBytes(u8 byte1, u8 byte2);
        void endProcess();
        int  emitJump(u8 instruction);
        void emitLoop(int loopStart);
        void patchJump(int offset);

        void breakStatement();
        void continueStatement();
        void patchBreakJumps();


        friend class Interpreter;   

public:
 
    Parser(Interpreter* vm);
    ~Parser();




    bool compile();
};