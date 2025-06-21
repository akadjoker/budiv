#pragma once

#include "Config.hpp"
#include "String.hpp"
#include "Token.hpp"

#include "Map.hpp"
#include "Stack.hpp"
#include "Vector.hpp"



class Lexer
{
private:
    friend class Parser;
    char* allocatedBuffer; 
    const char* start;
    const char* current;
    int line;
    bool panicMode;

    UnorderedMap<String,TokenType> keywords;
 
    Vector<String> variables;

 
    String programName;



    void handleIdentifier();
 

    char peek();
    char advance();
    bool match(char expected);
    char peekNext();
    char peekAhead(int n);
    char previous();

    bool isAtEnd();
    bool isDigit(char c);
    bool isAlpha(char c);
    bool isAlphaNumeric(char c);
   

    Token identifier();
    Token number();
    Token string();



    void skipWhitespace();

    Token addToken(TokenType type, String literal="");

    void Error(String message);


    String extractIdentifier( String &str);

 
    

    

public:
    Lexer();
    virtual ~Lexer();
    void initialize();
    bool Load(String input);
    bool LoadFromFile(const String &fileName);
    Token scanToken();
 
    void print();

    void cleanup();
};