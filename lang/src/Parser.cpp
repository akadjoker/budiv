#include "Parser.hpp"
#include "VM.hpp"
#include "Token.hpp"
#include "Lexer.hpp"
#include <string>


typedef void (*ParseFn)(bool);


void Parser::parsePrecedence(Precedence precedence)
{
    advance();
    // INFO("precedence: %d token: %d (%s)", precedence,
    //      static_cast<u32>(current.type),
    //      tknString(current.type).c_str()); // Adicione esta função helper

    ParseRule* prefixRule = getRule(previous.type);
    if (prefixRule->prefix == NULL)
    {
        error("Expect expression");
        return;
    }

    bool canAssign = precedence <= ASSIGNMENT;
    //    INFO("Call prefix for token: %s", tknString(previous.type).c_str());
    (this->*(prefixRule->prefix))(canAssign);

    while (precedence <= getRule(current.type)->precedence)
    {
        advance();
        ParseRule* infixRule = getRule(previous.type);
        if (infixRule->infix == NULL) break;

        // INFO("Call infix for token: %s", tknString(previous.type).c_str());
        (this->*(infixRule->infix))(canAssign);
    }

    if (canAssign && match(TokenType::EQUAL))
    {
        error("Invalid assignment target.");
    }
}

ParseRule* Parser::getRule(TokenType type)
{
    u32 index = static_cast<u32>(type);
    return &rules[index];
}


void Parser::initRules()
{


    rules[static_cast<u32>(TokenType::LEFT_PAREN)] = { &Parser::grouping, &Parser::call,
Precedence::CALL };
    rules[static_cast<u32>(TokenType::RIGHT_PAREN)] = { NULL, NULL,
                                                        Precedence::NONE };
    rules[static_cast<u32>(TokenType::LEFT_BRACE)] = { NULL, NULL,
                                                       Precedence::NONE };
    rules[static_cast<u32>(TokenType::RIGHT_BRACE)] = { NULL, NULL,
                                                        Precedence::NONE };
    rules[static_cast<u32>(TokenType::COMMA)] = { NULL, NULL,
                                                  Precedence::NONE };
    rules[static_cast<u32>(TokenType::DOT)] = { NULL, NULL, Precedence::NONE };
    rules[static_cast<u32>(TokenType::MINUS)] = { &Parser::unary,
                                                  &Parser::binary,
                                                  Precedence::TERM };
    rules[static_cast<u32>(TokenType::PLUS)] = { NULL, &Parser::binary,
                                                 Precedence::TERM };
    rules[static_cast<u32>(TokenType::SEMICOLON)] = { NULL, NULL,
                                                      Precedence::NONE };
    rules[static_cast<u32>(TokenType::SLASH)] = { NULL, &Parser::binary,
                                                  Precedence::FACTOR };
    rules[static_cast<u32>(TokenType::STAR)] = { NULL, &Parser::binary,
                                                 Precedence::FACTOR };
    rules[static_cast<u32>(TokenType::BANG)] = { NULL, NULL, Precedence::NONE };
    rules[static_cast<u32>(TokenType::BANG_EQUAL)] = { NULL, &Parser::binary,
                                                       Precedence::EQUALITY };
    rules[static_cast<u32>(TokenType::EQUAL)] = { NULL, NULL,
                                                  Precedence::NONE };
    rules[static_cast<u32>(TokenType::EQUAL_EQUAL)] = { NULL, &Parser::binary,
                                                        Precedence::EQUALITY };
    rules[static_cast<u32>(TokenType::GREATER)] = { NULL, &Parser::binary,Precedence::COMPARISON };
    rules[static_cast<u32>(TokenType::GREATER_EQUAL)] = {NULL, &Parser::binary, Precedence::COMPARISON};
    rules[static_cast<u32>(TokenType::LESS)] = { NULL, &Parser::binary,Precedence::COMPARISON };
    rules[static_cast<u32>(TokenType::LESS_EQUAL)] = { NULL, &Parser::binary,Precedence::COMPARISON };
    rules[static_cast<u32>(TokenType::BANG)] = { &Parser::unary, NULL,Precedence::NONE };
    rules[static_cast<u32>(TokenType::IDENTIFIER)] = { &Parser::variable, NULL,Precedence::NONE };
    rules[static_cast<u32>(TokenType::STRING)] = { &Parser::string, NULL,Precedence::NONE };
    rules[static_cast<u32>(TokenType::NUMBER)] = { &Parser::number, NULL,Precedence::NONE };

    rules[static_cast<u32>(TokenType::AND)] = { NULL, &Parser::and_, Precedence::AND };
    rules[static_cast<u32>(TokenType::OR)] = { NULL, &Parser::or_, Precedence::OR };
    rules[static_cast<u32>(TokenType::XOR)] = { NULL, &Parser::xor_, Precedence::XOR };
    
        


    rules[static_cast<u32>(TokenType::CLASS)] = { NULL, NULL,Precedence::NONE };
    rules[static_cast<u32>(TokenType::ELSE)] = { NULL, NULL, Precedence::NONE };
    
    rules[static_cast<u32>(TokenType::FALSE)] = { &Parser::literal, NULL,Precedence::NONE };
    rules[static_cast<u32>(TokenType::TRUE)] = { &Parser::literal, NULL, Precedence::NONE };
    rules[static_cast<u32>(TokenType::NOW)] = { &Parser::literal, NULL, Precedence::NONE };
    



    rules[static_cast<u32>(TokenType::FOR)] = { NULL, NULL, Precedence::NONE };
    rules[static_cast<u32>(TokenType::FUNCTION)] = { NULL, NULL,
                                                     Precedence::NONE };
    rules[static_cast<u32>(TokenType::IF)] = { NULL, NULL, Precedence::NONE };
    rules[static_cast<u32>(TokenType::NIL)] = { NULL, NULL, Precedence::NONE };
    rules[static_cast<u32>(TokenType::PRINT)] = { NULL, NULL,Precedence::NONE };
    rules[static_cast<u32>(TokenType::FRAME)] = { NULL, NULL,Precedence::NONE };
    
    rules[static_cast<u32>(TokenType::RETURN)] = { NULL, NULL,
                                                   Precedence::NONE };
    //  rules[static_cast<u32>(TokenType::SUPER)] = {NULL, NULL,
    //  Precedence::NONE};
    rules[static_cast<u32>(TokenType::THIS)] = { NULL, NULL, Precedence::NONE };
    
    rules[static_cast<u32>(TokenType::VAR)] = { NULL, NULL, Precedence::NONE };
    rules[static_cast<u32>(TokenType::WHILE)] = { NULL, NULL,Precedence::NONE };
}

void Parser::errorAtCurrent(const String& message)
{
    errorAt(current, message);
}

void Parser::error(const String& message) { errorAt(previous, message); }

void Parser::errorAt(const Token& token, const String& message)
{
    if (panic_mode) return;
    panic_mode = true;
    if (token.type == TokenType::END_OF_FILE)
    {
        ERROR("[line %d] Error (%s) at end", token.line, message.c_str());
    }
    else
    {
        ERROR("[line %d] Error '%s' at  %s ", token.line, token.lexeme.c_str(),
              message.c_str());
    }

    had_error = true;
}

void Parser::consume(TokenType type, const String& message)
{
    if (current.type == type)
    {
        advance();
        return;
    }
    errorAtCurrent(message);
}

bool Parser::match(TokenType type)
{
    if (!check(type)) return false;
    advance();
    return true;
}

bool Parser::isAtEnd() { return current.type == TokenType::END_OF_FILE; }

bool Parser::check(TokenType type) { return current.type == type; }


void Parser::emitConstant(Value value)
{
    u32 index = vm->addConstant(std::move(value));
    emitBytes(OP_CONSTANT, index);
}

void Parser::emitByte(u8 byte)
{
    current_function->chunk.write(byte, current.line);
}

void Parser::emitBytes(u8 byte1, u8 byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

void Parser::endProcess()
{
    current_process->writeChunk(OP_HALT, 0);
    current_process->call(current_function, 0);
}

int Parser::emitJump(u8 instruction)
{
    emitByte(instruction);
    emitByte(0xFF);
    emitByte(0xFF);
    return current_function->chunk.count - 2;
}

void Parser::emitLoop(int loopStart)
{
    emitByte(OP_LOOP);
    int offset = current_function->chunk.count - loopStart + 2;
    emitByte((offset >> 8) & 0xFF);
    emitByte(offset & 0xFF);
}


void Parser::patchJump(int offset)
{
    int jump = current_function->chunk.count - offset - 2;
    if (jump > UINT16_MAX)
    {
        error("Too much code to jump over");
        return;
    }
    current_function->chunk.code[offset] = jump >> 8;
    current_function->chunk.code[offset + 1] = jump & 0xFF;
}



void Parser::synchronize()
{
    panic_mode = false;
    while (current.type != TokenType::END_OF_FILE)
    {
        if (previous.type == TokenType::SEMICOLON) return;
        switch (current.type)
        {
            case TokenType::CLASS:
            case TokenType::FUNCTION:
            case TokenType::VAR:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::FRAME:
            case TokenType::DO:
            case TokenType::SWITCH:
            case TokenType::CASE:
            case TokenType::ELSE:
            case TokenType::DEFAULT:
            case TokenType::RETURN: return;
            default: break;
        }
        advance();
    }
}


Parser::Parser(Interpreter* vm)
{
    initRules();
    this->vm = vm;
    had_error = false;
    panic_mode = false;
    lexer = new Lexer();
    call_return = false;
}

Parser::~Parser() { delete lexer; }


void Parser::advance()
{
    previous = current;
    for (;;)
    {
        current = lexer->scanToken();
        if (current.type != TokenType::ERROR) break;
        errorAtCurrent(current.lexeme);
    }
}


bool Parser::compile()
{
    current_process =   vm->main_process;
    current_function = current_process->function;

   // INFO("Parsing started");


    advance();

    while (!match(TokenType::END_OF_FILE))
    {

        declaration();
    }


    endProcess();

  //  INFO("Parsing done");

    return !had_error;
}


void Parser::beginScope() { current_process->scopeDepth++; }

void Parser::endScope()
{
    current_process->scopeDepth--;

    while (current_process->localCount > 0 && current_process->locals[current_process->localCount - 1].depth> current_process->scopeDepth && !current_process->locals[current_process->localCount - 1].isArg)
    {
        emitByte(OP_POP);
        current_process->localCount--;
    }
}

void Parser::declaration()
{
    if (match(TokenType::VAR))
    {
        varDeclaration();
    } else if (match(TokenType::FUNCTION))
    {
        funDeclaration();
    } else if (match(TokenType::PROCESS))
    {
        procDeclaration();
    }
    else
    {
        statement();
    }

    if (panic_mode) synchronize();
}

void Parser::statement()
{
    ///  INFO("TokenType::PRINT value: %d", static_cast<int>(TokenType::PRINT));

    if (match(TokenType::PRINT))
    {

        printStatement();
    } else if (match(TokenType::FRAME))
    {
        frameStatement();
    }
    else if (match(TokenType::IF))
    {
        ifStatement();
    }
    else if (match(TokenType::WHILE))
    {
        whileStatement();
    }
    else if (match(TokenType::FOR))
    {
        forStatement();
    }
    else if (match(TokenType::DO))
    {
        doStatement();
    } else if (match(TokenType::LOOP))
    {
        loopStatement();
    }
    else if (match(TokenType::SWITCH))
    {
        switchStatement();
    } else if (match(TokenType::RETURN))
    {
        returnStatement();
    }else if (match(TokenType::BREAK))
    {
        breakStatement();
    } else if (match(TokenType::CONTINUE))
    {
        continueStatement();
    }
    else if (match(TokenType::LEFT_BRACE))
    {
        beginScope();
        block();
        endScope();
    }
    else
    {

        expressionStatement();
    }
}

void Parser::printStatement()
{
    // INFO("print statement");
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'print'.");
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);

    // //slow version but it works
    // consume(TokenType::LEFT_PAREN, "Expect '(' after 'print'.");
    // int argCount = 0;
    // if (!check(TokenType::RIGHT_PAREN)) 
    // {
    //     do 
    //     { 
    //         expression();
    //         argCount++;
    //     } while (match(TokenType::COMMA));
    //     emitConstant(NUMBER(argCount));
    // } else 
    // {
    //     emitConstant(NUMBER(0));
    // }
    // emitByte(OP_PRINT); 
    // consume(TokenType::RIGHT_PAREN, "Expect ')' after expression(s).");
    // consume(TokenType::SEMICOLON, "Expect ';' after value.");
}

void Parser::frameStatement() 
{
    // INFO("frame statement");
 
    if (match(TokenType::LEFT_PAREN))
    {
 
        expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
    } else 
    {
      emitConstant(NUMBER(100));
    }
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    emitByte(OP_FRAME);
}

void Parser::variable(bool canAssign)
{
    String name = previous.lexeme;

    u8 SET = OP_SET_GLOBAL;
    u8 GET = OP_GET_GLOBAL;

    int arg = current_process->resolveLocal(name.c_str(), name.length());

    if (arg != -1)
    {

        SET = OP_SET_LOCAL;
        GET = OP_GET_LOCAL;
    }
    else
    {
        arg = vm->addConstant(STRING(name.c_str()));
    }

    if (canAssign && match(TokenType::EQUAL))
    {
        expression();
        emitBytes(SET, arg);
    }
    else
    {
        emitBytes(GET, arg);
    }
}



void Parser::ifStatement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
    
    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP); // Remove condition from stack
    statement();
    
    Vector<int> endJumps; // Para saltar todos os elifs/else
    endJumps.push_back(emitJump(OP_JUMP));
    
    // Handle elif chains
    while (match(TokenType::ELIF)) 
    {
        patchJump(thenJump);
        emitByte(OP_POP); // Remove false condition
        
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'elif'.");
        expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after elif condition.");
        
        thenJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP); // Remove condition
        statement();
        
        endJumps.push_back(emitJump(OP_JUMP));
    }
    
    // Handle final else
    if (match(TokenType::ELSE)) 
    {
        patchJump(thenJump);
        emitByte(OP_POP); // Remove false condition
        statement();
    } else 
    {
        patchJump(thenJump);
        emitByte(OP_POP); // Remove false condition
    }
    
    // Patch all end jumps
    for (int jump : endJumps) 
    {
        patchJump(jump);
    }
}

void Parser::breakStatement()
{

    

    if (current_function->loopStack.empty()) 
    {
        error("Cannot use 'break' outside of loop");
        return;
    }
    int jump = emitJump(OP_JUMP);
    current_function->loopStack.back().breakJumps.push_back(jump);

    consume(TokenType::SEMICOLON, "Expect ';' after 'break'");
   
}

void Parser::continueStatement()
{
    if (current_function->loopStack.empty()) 
    {
        error("Cannot use 'continue' outside of loop");
        return;
    }
    emitLoop(current_function->loopStack.back().loopStart);
    consume(TokenType::SEMICOLON, "Expect ';' after 'continue'");
    
   
}
void Parser::patchBreakJumps()
{
    const ValueArray<int> breakJumps = current_function->loopStack.back().breakJumps;
    for (u32 i=0; i< breakJumps.getSize(); i++) 
    {
        patchJump(breakJumps[i]); 
    }
    current_function->loopStack.pop_back();

    
}

void Parser::doStatement() 
{
   
   

    int loopStart     = current_function->chunk.count;   

    LoopContext ctx;
    ctx.loopStart = loopStart;
    ctx.breakJumps.clear();
    current_function->loopStack.push_back(ctx);

    statement();
    
    consume(TokenType::WHILE, "Expect 'while' after loop body in do-while statement.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
    consume(TokenType::SEMICOLON, "Expect ';' after do-while condition.");
    
    

    int exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP); 
    
    emitLoop(loopStart);


    patchJump(exitJump);
    emitByte(OP_POP);

    patchBreakJumps();
    
    

 
  

}

void Parser::loopStatement() 
{
    
    int loopStart = current_function->chunk.count;
 
    LoopContext ctx;
    ctx.loopStart = loopStart;
    ctx.breakJumps.clear();
    current_function->loopStack.push_back(ctx);

    statement();
    emitLoop(loopStart);
    
     

    patchBreakJumps();
 

 
}





void Parser::whileStatement()
{
  
    int loopStart = current_function->chunk.count;

      LoopContext ctx;
    ctx.loopStart = loopStart;
    ctx.breakJumps.clear();
    current_function->loopStack.push_back(ctx);
    
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");

    int exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();

    emitLoop(loopStart);
    
    
    patchJump(exitJump);
    emitByte(OP_POP); 

    patchBreakJumps();
    


}

 
void Parser::forStatement()
{
   // beginScope();

  

    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

    // Initializer
    // if (match(TokenType::SEMICOLON))
    // {
    //     // No initializer.
    // }
    // else if (match(TokenType::VAR))
    // {
    //     varDeclaration();
    // }
    // else
    // {
         expressionStatement();
    // }

    int loopStart = current_function->chunk.count;
    LoopContext ctx;
    ctx.loopStart = loopStart;
    ctx.breakJumps.clear();
    current_function->loopStack.push_back(ctx);
    int exitJump = -1;

    // Condition
    if (!match(TokenType::SEMICOLON))
    {
        expression();
        consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");
        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP); // Remove condition
    }

    // Increment
    int bodyJump = -1;
    int incrementStart = -1;
    if (!match(TokenType::RIGHT_PAREN))
    {
        bodyJump = emitJump(OP_JUMP);
        incrementStart = current_function->chunk.count;
        expression();
        emitByte(OP_POP); // Remove increment result
        consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
        current_function->loopStack.back().loopStart = loopStart;
    }

    // Body
    statement();
    emitLoop(loopStart);
    if (exitJump != -1)
    {
        patchJump(exitJump);
        emitByte(OP_POP); // Remove false condition
    }

    patchBreakJumps();
 
   // endScope();
}

void Parser::ternario(bool canAssign)
{
 
    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    
    parsePrecedence(ASSIGNMENT); // Valor se true
    
    int elseJump = emitJump(OP_JUMP);
    patchJump(thenJump);
    emitByte(OP_POP);
    
    consume(TokenType::COLON, "Expect ':' in ternary operator.");
    parsePrecedence(ASSIGNMENT); // Valor se false
    
    patchJump(elseJump);
}


void Parser::switchStatement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'switch'.");
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after switch condition.");
    consume(TokenType::LEFT_BRACE, "Expect '{' before switch cases.");
    Vector<int> endJumps;
    endJumps.reserve(32);
    int caseCount = 0;
    while (match(TokenType::CASE))
    {
        emitByte(OP_DUP);
        expression();
        consume(TokenType::COLON, "Expect ':' after case value.");
        emitByte(OP_EQUAL);
        int caseJump = emitJump(OP_JUMP_IF_FALSE);
        statement();
        endJumps.push_back(emitJump(OP_JUMP));
        patchJump(caseJump);
        emitByte(OP_POP); // Pop case value if false
        caseCount++;
    }
    emitByte(OP_POP); // Pop switch value
    if (match(TokenType::DEFAULT))
    {
        consume(TokenType::COLON, "Expect ':' after default case.");
        statement();
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after switch cases.");
    if (caseCount == 0 && !match(TokenType::DEFAULT)) // Check after parsing
    {
        error("Switch statement must have at least one case or a default case.");
        return;
    }
    for (u32 i = 0; i < endJumps.size(); i++)
    {
        patchJump(endJumps[i]);
    }
}

void Parser::call(bool canAssign) 
{
  uint8_t argCount = argumentList();
  emitBytes(OP_CALL, argCount);
}

void Parser::funDeclaration() 
{
    ObjFunction* prefunction = current_function;
    consume(TokenType::IDENTIFIER, "Expect 'def' before function name.");
    
    
    String name = previous.lexeme;
    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");
    
    // INFO("function name: %s", name.c_str());
    
    
    u32 nameIndex = vm->addConstant(STRING(name.c_str()));

    current_function = vm->add_function(name.c_str(), 0);
    beginScope();

    current_process->addLocal(name.c_str(), name.length(), true);
    current_process->markInitialized();
    
    if (!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            current_function->arity++;
            if (current_function->arity >= 255)
            {
                error("Can't have more than 255 parameters.");
            }
            
            consume(TokenType::IDENTIFIER, "Expect parameter name.");
            String paramName = previous.lexeme;
            
            //INFO("parameter name: %s", paramName.c_str());
            current_process->addLocal(paramName.c_str(), paramName.length(), true);
            current_process->markInitialized();
            
      
            
            
            
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
    consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
    
    
    call_return = false;
    
    block();
    endScope();
    
    if (!call_return)
    {
        emitByte(OP_NIL);
        emitByte(OP_RETURN);
    }
    

   
    int functionIndex = vm->addConstant(FUNCTION(current_function));
    //int functionIndex =current_process->addConstant(STRING(name.c_str()));
    current_function = prefunction;
    
    emitBytes(OP_CONSTANT,    functionIndex);
    emitBytes(OP_DEFINE_GLOBAL, nameIndex);

    
}

void Parser::procDeclaration() 
{
    Process* preProcess = current_process;
    
    ObjFunction* prefunction = current_function;


    
    
    consume(TokenType::IDENTIFIER, "Expect 'process' before process name.");
    
    
    String name = previous.lexeme;
    consume(TokenType::LEFT_PAREN, "Expect '(' after process name.");
    
    
    u32 nameIndex = vm->addConstant(STRING(name.c_str()));
    current_process = vm->create_process(name.c_str());
    current_function = current_process->function;
    current_process->addLocal("x");
    current_process->addLocal("y");
    current_process->addLocal("angle");

   beginScope();


    // current_process->addLocal("y", 1, true);
    // current_process->markInitialized();
    
    // current_process->addLocal(name.c_str(), name.length(), true);
    // current_process->markInitialized();
    // u32 index = vm->addConstant(STRING(name.c_str()));
    // emitBytes(OP_DEFINE_LOCAL, index); 
    if (!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            current_function->arity++;
            if (current_function->arity >= 255)
            {
                error("Can't have more than 255 parameters.");
            }
            
            consume(TokenType::IDENTIFIER, "Expect parameter name.");
            String paramName = previous.lexeme;
            //current_process->addLocal(paramName.c_str(), paramName.length(), true);
            //current_process->markInitialized();
            current_process->addLocal(paramName.c_str());
            //    u32 index = vm->addConstant(STRING(paramName.c_str()));
            //   emitBytes(OP_SET_LOCAL, index); 
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
    consume(TokenType::LEFT_BRACE, "Expect '{' before process body.");

        

    
    call_return = false;
    
    while (!check(TokenType::RIGHT_BRACE) && !check(TokenType::END_OF_FILE))
    {
        if (match(TokenType::VAR))
        {
            
            varProcessDeclaration();

            //varDeclaration();
        }
        else
        {
            statement();
        }
        
        if (panic_mode) synchronize();
    }
    
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");




    endScope();

 
    
    emitByte(OP_HALT);
    
    ObjProcess* process= vm->add_raw_process(name.c_str());
    process->process  = current_process;
    process->function = current_function;
    
    //  process->process->disassembleCode(&process->function->chunk, "process");
   
    current_process  = preProcess;
    current_function = prefunction;
    int functionIndex = vm->addConstant(PROCESS(process));

    
    emitBytes(OP_CONSTANT,    functionIndex);
    emitBytes(OP_DEFINE_GLOBAL, nameIndex);


}


u8 Parser::argumentList()
{
    u8 count = 0;
    if (!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            expression(false);
            if (count >= 255)
            {
                error("Can't have more than 255 arguments pass to a task");
                return 0;
            }
            count++;
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
    return count;
}

void Parser::returnStatement()
{
    call_return = true;
    if (match(TokenType::SEMICOLON))
    {
        emitByte(OP_NIL);
        emitByte(OP_RETURN);
    }
    else
    {
        expression();
        consume(TokenType::SEMICOLON, "Expect ';' after return value.");
        emitByte(OP_RETURN);
    }

}

void Parser::varProcessDeclaration()
{
   consume(TokenType::IDENTIFIER, "Expect variable name.");
    String name = previous.lexeme;
    current_process->addLocal(name.c_str(), name.length(), false);
    current_process->markInitialized();
    if (match(TokenType::EQUAL))
    {
        expression();
    }
    else
    {
       emitByte(OP_NIL);
    }

    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
   // emitBytes(OP_SET_LOCAL, index); 
}

void Parser::varDeclaration()
{
    consume(TokenType::IDENTIFIER, "Expect variable name.");
    String name = previous.lexeme;
    if (current_process->scopeDepth > 0)
    {

        current_process->addLocal(name.c_str(), name.length(), false);

        if (match(TokenType::EQUAL))
        {
            expression();
        }
        else
        {
            emitByte(OP_NIL);

        }

        current_process->markInitialized();
        consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
        return;
    }


    u32 index = vm->addConstant(STRING(name.c_str()));

    if (match(TokenType::EQUAL))
    {
        expression();
    }
    else
    {
        emitByte(OP_NIL);
    }

    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    emitBytes(OP_DEFINE_GLOBAL, index);
}


void Parser::expressionStatement()
{

    expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

void Parser::expression(bool canAssign) { parsePrecedence(ASSIGNMENT); }

void Parser::grouping(bool canAssign)
{

    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}


void Parser::number(bool canAssign)
{
    Value value = NUMBER(std::stod(previous.lexeme.c_str()));

    emitConstant(std::move((value)));
}

void Parser::string(bool canAssign)
{

    Value value = STRING(previous.lexeme.c_str());
    emitConstant(std::move(value));
}
void Parser::literal(bool canAssign) 
{
    if  (previous.type==TokenType::NIL) emitByte(OP_NIL);
    else if (previous.type==TokenType::TRUE) emitByte(OP_TRUE);
    else if (previous.type==TokenType::FALSE) emitByte(OP_FALSE);
    else if (previous.type==TokenType::NOW) emitByte(OP_NOW);
    
}


void Parser::binary(bool canAssign)
{


    TokenType operatorType = previous.type;

    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch (operatorType)
    {
        case TokenType::PLUS: emitByte(OP_ADD); break;
        case TokenType::MINUS: emitByte(OP_SUBTRACT); break;
        case TokenType::STAR: emitByte(OP_MULTIPLY); break;
        case TokenType::SLASH: emitByte(OP_DIVIDE); break;
        case TokenType::MOD: emitByte(OP_MODULO); break;
        case TokenType::POWER: emitByte(OP_POWER); break;
        case TokenType::EQUAL_EQUAL: emitByte(OP_EQUAL); break;
        case TokenType::BANG_EQUAL: emitByte(OP_NOT_EQUAL); break;
        case TokenType::LESS: emitByte(OP_LESS); break;
        case TokenType::GREATER: emitByte(OP_GREATER); break;
        case TokenType::LESS_EQUAL: emitByte(OP_LESS_EQUAL); break;
        case TokenType::GREATER_EQUAL: emitByte(OP_GREATER_EQUAL); break;
        default: return; // Unreachable.
    }
}
void Parser::and_(bool canAssign)
{

    int endJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);

    parsePrecedence(AND);

    patchJump(endJump);
}

void Parser::or_(bool canAssign)
{

    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);

    patchJump(elseJump);

    emitByte(OP_POP);

    parsePrecedence(OR);

    patchJump(endJump);
}

void Parser::xor_(bool canAssign)
{

    parsePrecedence(XOR);
    emitByte(OP_XOR); 

}


void Parser::block()
{

    while (!check(TokenType::RIGHT_BRACE) && !check(TokenType::END_OF_FILE))
    {
        declaration();
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
}
void Parser::unary(bool canAssign)
{


    TokenType operatorType = previous.type;

    // Compile the operand.
    parsePrecedence(UNARY);

    // Emit the operator instruction.
    switch (operatorType)
    {
        case TokenType::MINUS: emitByte(OP_NEGATE); break;
        case TokenType::BANG: emitByte(OP_NOT); break;
        default: return; // Unreachable.
    }
}
