
#include "Lexer.hpp"
#include "Utils.hpp"


Lexer::Lexer()
{
    start = NULL;
    current = NULL;
    line = 1;
    panicMode = false;
    allocatedBuffer = NULL;
    initialize();
}

Lexer::~Lexer() { cleanup(); }


void Lexer::cleanup()
{
    INFO("cleaning up lexer");
    if (allocatedBuffer != NULL)
    {
        free(allocatedBuffer);
        allocatedBuffer = NULL;
        start = NULL;
        current = NULL;
    }
}

static char* readFile(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (file == NULL)
    {
        ERROR("Failed to open file: %s", path);
        return NULL; // Return NULL on error
    }

    if (fseek(file, 0L, SEEK_END) != 0)
    {
        ERROR("Failed to seek to end of file: %s", path);
        fclose(file);
        return NULL;
    }

    size_t fileSize = ftell(file);
    if (fileSize == (size_t)-1)
    {
        ERROR("Failed to get file size: %s", path);
        fclose(file);
        return NULL;
    }

    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL)
    {
        ERROR("Failed to allocate memory for file: %s", path);
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    buffer[bytesRead] = '\0';
    INFO("Read %zu bytes from file: %s", bytesRead, path);
    fclose(file);
    return buffer;
}
bool Lexer::LoadFromFile(const String& fileName)
{
    char* fileContent = readFile(fileName.c_str());
    if (fileContent == NULL)
    {
        return false; // File couldn't be read
    }

    if (strlen(fileContent) == 0)
    {
        free(fileContent);
        return false; // Empty file
    }


    cleanup();

    start = fileContent;
    current = start;
    allocatedBuffer = fileContent;
    line = 1;
    panicMode = false;

    return true;
}
bool Lexer::Load(String input)
{
    if (input.size() == 0) return false;


    cleanup();

    char* source = strdup(input.c_str());
    if (source == NULL) return false;

    start = source;
    current = source;
    allocatedBuffer = source;
    line = 1;
    panicMode = false;

    return true;
}


char Lexer::peek()
{
    if (current == NULL) return '\0';
    return *current;
}

bool Lexer::isAtEnd()
{
    if (current == NULL) return true;
    return *current == '\0';
}

char Lexer::advance()
{
    if (isAtEnd()) return '\0';
    current++;
    return current[-1];
}

bool Lexer::match(char expected)
{
    if (isAtEnd()) return false;
    if (*current != expected) return false;

    current++;
    return true;
}

char Lexer::peekNext()
{
    if (isAtEnd() || current[1] == '\0') return '\0';
    return current[1];
}

char Lexer::peekAhead(int n)
{
    if (isAtEnd()) return '\0';
    for (int i = 0; i < n; i++)
    {
        if (current[i] == '\0') return '\0';
    }
    return current[n];
}

char Lexer::previous()
{
    if (current <= start)
    {
        ERROR("previous() called at start of buffer! current=%p, start=%p",
              (void*)current, (void*)start);
        return '\0';
    }
    return current[-1];
}


bool Lexer::isDigit(char c) { return (c >= '0' && c <= '9'); }

bool Lexer::isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isAlphaNumeric(char c) { return isAlpha(c) || isDigit(c); }

String Lexer::extractIdentifier(String& str)
{
    String result;
    for (size_t i = 0; i < str.size(); i++)
    {
        char c = str[i];
        if (isAlpha(c) || c == '_')
        {
            result += c;
        }
    }
    return result;
}

void Lexer::Error(String message)
{
    panicMode = true;
    String textLine = String(line);
    String text = message + " at line: " + textLine;
    Log(2, text.c_str());
    panicMode = true;
}


void Lexer::initialize()
{

    keywords.insert("program", TokenType::PROGRAM);

    keywords.insert("nil", TokenType::NIL);

    keywords.insert("def", TokenType::FUNCTION);
    keywords.insert("process", TokenType::PROCESS);

    keywords.insert("and", TokenType::AND);
    keywords.insert("or", TokenType::OR);
    keywords.insert("not", TokenType::NOT);
    keywords.insert("xor", TokenType::XOR);

    keywords.insert("if", TokenType::IF);
    keywords.insert("else", TokenType::ELSE);
    keywords.insert("elif", TokenType::ELIF);

    keywords.insert("while", TokenType::WHILE);
    keywords.insert("for", TokenType::FOR);
    keywords.insert("do", TokenType::DO);

    keywords.insert("loop", TokenType::LOOP);

    keywords.insert("break", TokenType::BREAK);
    keywords.insert("continue", TokenType::CONTINUE);
    keywords.insert("return", TokenType::RETURN);
    keywords.insert("switch", TokenType::SWITCH);
    keywords.insert("case", TokenType::CASE);
    keywords.insert("default", TokenType::DEFAULT);

    keywords.insert("print", TokenType::PRINT);
    keywords.insert("now", TokenType::NOW);
    keywords.insert("frame", TokenType::FRAME);
    keywords.insert("class", TokenType::CLASS);
    keywords.insert("this", TokenType::THIS);
    keywords.insert("len", TokenType::LEN);

    keywords.insert("import", TokenType::IMPORT);

    keywords.insert("var", TokenType::VAR);
    keywords.insert("true", TokenType::TRUE);
    keywords.insert("false", TokenType::FALSE);
}


Token Lexer::string()
{
 
    while (peek() != '"' && !isAtEnd())
    {
        if (peek() == '\n') line++;
        advance();
    }

    if (isAtEnd())
    {
        Error("Unterminated string");
        return Token(TokenType::ERROR, "Unterminated string", "ERROR", line);
    }

    advance();


     const char* stringStart = start + 1;          
     int stringLength = (int)(current - start) - 2;
    String text(stringStart, stringLength);

    Token token = Token(TokenType::STRING, std::move(text), "", line);
     return token;
}

Token Lexer::number()
{
    while (isDigit(peek())) advance();

    if (peek() == '.' && isDigit(peekNext()))
    {
        advance();
        while (isDigit(peek())) advance();
    }

    String text(start, (int)(current - start));
    return addToken(TokenType::NUMBER, text);
}

Token Lexer::addToken(TokenType type, String literal)
{

    String text(start, (int)(current - start));
    Token token = Token(type, std::move(text), std::move(literal), line);
    return token;
}

Token Lexer::identifier()
{

    while (isAlphaNumeric(peek())) advance();

    // std::cout<<" ,"<<peek()<<" ";

    String text(start, (int)(current - start));
    text.ToLower(); 

    // if (keywords.contains(text))
    // {
    //     TokenType type = keywords[text];
    //     WARNING("keyword: %s", text.c_str());
    //     return addToken(type);
    // }
    // else
    // {
    //     WARNING("identifier: %s", text.c_str());
    //     return addToken(TokenType::IDENTIFIER, text);
    // }


 
    TokenType* type = keywords.find(text);
    if (type != nullptr)
    {
      //  WARNING("keyword: %s", text.c_str());
        return addToken(*type);
    }
    else
    {
      //  WARNING("identifier: %s", text.c_str());
        return addToken(TokenType::IDENTIFIER, text);
    }


    return Token(TokenType::ERROR, "UNKNOWN", "indentifier", line);
}

Token Lexer::scanToken()
{
    skipWhitespace();

    start = current;


    if (isAtEnd()) 
    {
       return Token(TokenType::END_OF_FILE, "EOF", "EOF", line);
    }

    char c = advance();

    if (isDigit(c)) return number();

    if (isAlpha(c)) return identifier();

    switch (c)
    {
        case '(': {
            return addToken(TokenType::LEFT_PAREN);
        }
        case ')': {
            return addToken(TokenType::RIGHT_PAREN);
        }
        case '{': {
            return addToken(TokenType::LEFT_BRACE);
        }
        case '}': {
            return addToken(TokenType::RIGHT_BRACE);
        }
        case '[': {

            return addToken(TokenType::LEFT_BRACKET);
        }
        case ']': {
            return addToken(TokenType::RIGHT_BRACKET);
        }

        case ',': return addToken(TokenType::COMMA);

        case '.': return addToken(TokenType::DOT);

        case '-': {
            if (match('-'))
                return addToken(TokenType::DEC);
            else if (match('='))
                return addToken(TokenType::MINUS_EQUAL);
            else
                return addToken(TokenType::MINUS);
        }
        case '+': {
            if (match('+'))
                return addToken(TokenType::INC);
            else if (match('='))
                return addToken(TokenType::PLUS_EQUAL);
            else
                return addToken(TokenType::PLUS);
        }
        case ';': return addToken(TokenType::SEMICOLON);

        case ':': return addToken(TokenType::COLON);


        case '^': return addToken(TokenType::POWER);


        case '%': return addToken(TokenType::MOD);


        case '*': {
            if (match('='))
                return addToken(TokenType::STAR_EQUAL);
            else
                return addToken(TokenType::STAR);
        }

        case '!':
            return addToken(match('=') ? TokenType::BANG_EQUAL
                                       : TokenType::BANG);

        case '=':
            return addToken(match('=') ? TokenType::EQUAL_EQUAL
                                       : TokenType::EQUAL);

        case '<':
            return addToken(match('=') ? TokenType::LESS_EQUAL
                                       : TokenType::LESS);

        case '>':
            return addToken(match('=') ? TokenType::GREATER_EQUAL
                                       : TokenType::GREATER);


        case '/':
        {        
            if (match('='))
                return addToken(TokenType::SLASH_EQUAL);
            else
                return addToken(TokenType::SLASH);
          }


        case '"': return string();
    }


    Error("Unexpected character");
    return Token(TokenType::ERROR, "Unexpected character", "", line);
}

 

void Lexer::skipWhitespace()
{
    int startLine = line;
    int nestedLevel = 0;
    for (;;)
    {
      
    
        char c = peek();
        switch (c)
        {
            case ' ':
            case '\r':
            case '\t': advance(); break;

            case '\n':
                line++;
                advance();
                break;

            case '/':
                if (peekNext() == '/')
                {
          
                    while (peek() != '\n' && !isAtEnd()) advance();
          
                }
                else if (peekNext() == '*')
                {
                    
                    advance(); // consome '/'
                    advance(); // consome '*'
                    nestedLevel++;

                    while (!isAtEnd())
                    {
                        if (peek() == '*' && peekNext() == '/')
                        {
                            advance(); // consome '*'
                            advance(); // consome '/'
                            nestedLevel--;
                            break; 
                        }
                        if (peek() == '\n')
                        {
                            line++;
                        }
                        advance();
                    }

                    if (nestedLevel > 0)
                    {
                        Error("Unterminated comment");
                        line = startLine;
                    }
                   
                }
                else
                {
            
                    return;
                }
                break;

            default: return; 
        }
         
    }

  
}
 

void Lexer::print()
{
    int line = -1;
    printf("   L  T \n");
    for (;;)
    {
        Token token = scanToken();
        if (token.line != line)
        {
            printf("%4d ", token.line);
            line = token.line;
        }
        else
        {
            printf("   | ");
        }
        printf("%2d '%s'   %s \n", (int)token.type, token.lexeme.c_str(),tknString(token.type).c_str());

        if (token.type == TokenType::END_OF_FILE) break;
    }
}