#include "VM.hpp"
#include "Utils.hpp"


u32 Process::nextPID = 1;




 

Process::Process(    Interpreter* interpreter,bool isRoot)
{
    this->interpreter = interpreter;
    size_t len = strlen("Process");
    strncpy(name, "Process", len);
    name[len] = '\0';
    id = nextPID++;
    priority = 0;
    status = STATUS_RUNNING;
    frame_percent = 0;
    saved_status = status;
 
    next = nullptr;
    prev = nullptr;
    frameCount = 0;
    localCount = 0;
    scopeDepth = 0;
    defineLocals = 0;
    stackTop = &stack[0];
    function = nullptr;
    frame_timer = 0.0;     
    frame_interval = 1.0/60.0; 
    frame_speed_multiplier = 1.0; 
    root = isRoot;
    if (isRoot)
    {
        function = new ObjFunction("__main__");
    }

}

Process::~Process()
{
  //  INFO("deleting process: %s", name);
 
    if (function != nullptr && root) 
    {
        delete function;
        function = nullptr;
    }
    



}

void Process::setFrameSpeed(double speed_multiplier)
{
    // speed_multiplier: 1.0 = normal, 2.0 = duplo, 0.5 = metade
    frame_speed_multiplier = speed_multiplier;
    
    // Recalcular intervalo baseado na velocidade atual
    double base_fps = 60.0;
    double adjusted_fps = base_fps * speed_multiplier;
    frame_interval = 1.0 / adjusted_fps;
}

 
void Process::pauseForSeconds(double seconds)
{
    frame_timer = -seconds; // Timer negativo = pausa
}


int Process::addLocal(const char* name) 
{
   
    if (localCount >= UINT8_COUNT)
    {
        runtimeError("Too many local variables in function.");
        return -1;
    }
    
     
    Local *local = &locals[localCount++];
    
    local->len = strlen(name);
    strncpy(local->name, name, local->len);
    local->name[local->len] = '\0';
    local->isArg = true;
    local->depth = 0;



    
    return localCount - 1;
}

int Process::addLocal(const char* name,size_t len, bool isArg) 
{
    if (localCount >= UINT8_COUNT)
    {
        runtimeError("Too many local variables in function.");
        return -1;
    }
    
     
    Local *local = &locals[localCount++];
    
    strncpy(local->name, name, len);
    local->name[len] = '\0';
    local->len = len;
    local->isArg = isArg;
    local->depth = -1;
    
    return localCount - 1;
}

static bool identifiersEqual(const char* a,size_t aLen, const char* b, size_t bLen)
{
    if (aLen != bLen) return false;   
    return memcmp(a, b, aLen) == 0;
}

int Process::resolveLocal(const char* name,size_t len)
{
    
    for (int i = localCount - 1; i >= 0; i--) 
    {
        Local* local = &locals[i];
        if (identifiersEqual(local->name, local->len, name, len)) 
        {
            if (local->depth == -1) 
            {
                runtimeError("Can't read local variable in its own initializer.");
                return -1;
            }
        return i;
        }
  }

  return -1;
}

void Process::markInitialized() 
{
      if (scopeDepth == 0) return;
      locals[localCount - 1].depth = scopeDepth;
}

void Process::init_locals() 
{
    
    // addLocal("x", 4, false);
    // addLocal("y", 4, false);
    // addLocal("angle", 6, false);
    push(std::move(NUMBER(30)));//angle
    push(std::move(NUMBER(2)));//y
    push(std::move(NUMBER(360)));//x
    // markInitialized();
}

void Process::resetStack()
{
    stackTop = stack;
    frameCount = 0;
}

    void Process::push(Value value)
    {
        *stackTop = value;
        stackTop++;
    }

    Value Process::pop()
    {
        stackTop--;
        return *stackTop;
    }

    void Process::popn(int n) 
    {
        if (n < 0 || n > stackTop - stack) 
        {
            runtimeError("Stack underflow in popn");
            n = stackTop - stack;  
        }
        stackTop -= n;
    }

    Value Process::peek(int distance) { return stackTop[-1 - distance]; }

Value Process::top()
{
    return *stackTop;
}


bool Process::is_alive() const
{
    return status != STATUS_DEAD && status != STATUS_KILLED;
}

bool Process::should_execute() const
{
    return status == STATUS_RUNNING && frame_percent < 100;
}

 bool Process::call(ObjFunction* function, int argCount)
 {
        if (argCount != function->arity)
        {
            runtimeError("Expected " + String(function->arity)
                         + " arguments but got " + String(argCount)
                         + ".");

            return false;
        }

        if (frameCount == FRAMES_MAX)
        {
            runtimeError("Call Stack overflow.");
            return false;
        }

        CallFrame* frame = &frames[frameCount++];
        currentFrame = frame;
        frame->function = function;
        frame->ip = function->chunk.code;
        frame->slots = stackTop - argCount - 1;
       // frame->slots = stackTop - argCount;
        return true;
 }



 void Process::writeChunk(u8 instruction, int line) 
 {
    function->chunk.write(instruction, line);
 }

 void Process::runtimeError(const String& message)
 {
         ERROR("Runtime error: %s", message.c_str());

        // for (int i = frameCount - 1; i >= 0; i--)
        // {
        //     CallFrame* frame = &frames[i];
        //     ObjFunction* function = frame->function;
        //     size_t instruction = frame->ip - function->chunk.code - 1;
        //     ERROR("[line %d] in %s()", function->chunk.lines[instruction], function->name);   
            
            
        // }

         resetStack();
}

u32 Process::simpleInstruction(Chunk* chunk,const char *name, u32 offset)
{
    printf("%s\n", name);
    return offset + 1;
}

u32 Process::byteInstruction(Chunk* chunk, const char* name, u32 offset)
{
    u8 slot = chunk->code[offset + 1];
    printf("%-16s %4d\n", name, slot);
    return offset + 2;
}

u32 Process::jumpInstruction(Chunk* chunk,const char *name, u32 sign, u32 offset)
{
    u16 jump = (u16)chunk->code[offset + 1] << 8;
    jump |= chunk->code[offset + 2];
    printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
    return offset + 3;
}

void Process::disassemble() 
{
    disassembleCode(&function->chunk, function->name);
}

void Process::disassembleCode(Chunk* chunk,const char* name) 
{
    printf("================== %s ==================\n", name);
    printf("\n");
    for (size_t offset = 0; offset < chunk->count;)
    {
        offset = disassembleInstruction(chunk,offset);
    }
    printf("\n");
}

u32 Process::constantInstruction(Chunk* chunk,const char *name, u32 offset)
{

    u8 constant = chunk->code[offset + 1];

    printf("%-16s %4d '", name, constant);
    Value value =interpreter->constants[constant];
    PRINT_VALUE(value);
    printf("'\n");

    return offset + 2;
}

u32 Process::disassembleInstruction(Chunk* chunk, u32 offset) 
{ 
     printf("%04d ", offset);
    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1])
    {
        printf("   | ");
    }
    else
    {
        printf("%4d ", chunk->lines[offset]);
    }
    u8 instruction = chunk->code[offset];

    switch (instruction)
    {
            case OP_CONSTANT:
            {
                 return constantInstruction( chunk, "CONSTANT", offset);
            }
            case OP_NIL:
            {
                return simpleInstruction(chunk, "NIL", offset);
            }
            case OP_TRUE:
            {
                return simpleInstruction(chunk, "TRUE", offset);
            }
            case OP_FALSE:
            {
                return simpleInstruction(chunk, "FALSE", offset);
            }
            case OP_POP:
            {
                return simpleInstruction(chunk, "POP", offset);
            }
            case OP_NOW:
            {
                return simpleInstruction(chunk, "NOW", offset);
            }
            case OP_DUP:
            {
                return simpleInstruction(chunk, "DUP", offset);
            }
            case OP_HALT:
            {
                return simpleInstruction(chunk, "HALT", offset);
            }
            case OP_RETURN:
            {
                return simpleInstruction(chunk, "RETURN", offset);
            }
            case OP_PRINT:
            {
                return simpleInstruction(chunk, "PRINT", offset);
            }
            case OP_CALL:
            {
                
                return byteInstruction(chunk, "CALL", offset);
            }
            case OP_ADD:
            {
                
                return simpleInstruction(chunk, "ADD", offset);
            }
            case OP_SUBTRACT:
            {
                
                return simpleInstruction(chunk, "SUBTRACT", offset);
            }
            case OP_MULTIPLY:
            {
                
                return simpleInstruction(chunk, "MULTIPLY", offset);
            }
            case OP_DIVIDE:
            {
                
                return simpleInstruction(chunk, "DIVIDE", offset);
            }
            case OP_EQUAL:
            {
                
                return simpleInstruction(chunk, "EQUAL", offset);
            }
            case OP_GREATER:
            {
                
                return simpleInstruction(chunk, "GREATER", offset);
            }
            case OP_LESS:
            {
                
                return simpleInstruction(chunk, "LESS", offset);
            }
            case OP_NEGATE:
            {
                
                return simpleInstruction(chunk, "NEGATE", offset);
            }
            case OP_DEFINE_GLOBAL:
            {
                return constantInstruction(chunk, "DEFINE_GLOBAL", offset);
            }
            case OP_GET_GLOBAL:
            {
                return constantInstruction(chunk, "GET_GLOBAL", offset);
            }
            case OP_SET_GLOBAL:
            {
                return constantInstruction(chunk, "SET_GLOBAL", offset);
            }
            case OP_GET_LOCAL:
            {
                return byteInstruction(chunk, "GET_LOCAL", offset);
            }
            case OP_SET_LOCAL:
            {
                return byteInstruction(chunk, "SET_LOCAL", offset);
            }
            case OP_DEFINE_LOCAL:
            {
                   return constantInstruction(chunk, "DEFINE_LOCAL", offset);
            }
            case OP_JUMP:
            {
                return jumpInstruction(chunk, "JUMP", 1, offset);
            }
            case OP_JUMP_IF_FALSE:
            {
                return jumpInstruction(chunk, "JUMP_IF_FALSE", -1, offset);
            }
            case OP_JUMP_IF_TRUE:
            {
                return jumpInstruction(chunk, "JUMP_IF_TRUE", -1, offset);
            }
            case OP_LOOP:
            {
                return jumpInstruction(chunk, "LOOP", -1, offset); 
            }
            case OP_FRAME:
            {
                return simpleInstruction(chunk, "FRAME", offset);
            }
            case OP_XOR:
            {
                return simpleInstruction(chunk, "XOR", offset);
            }

            default:
            {
                printf("Unknow instruction %d\n", instruction);
                return chunk->count;
                break;
            }
    }

    return offset;
}


bool Process::run( )
{
    if (frameCount < 1)
    {
        runtimeError("Empty frames.");
        status = STATUS_DEAD;
        return false;
    }


   for (;;) 
    {
    
    CallFrame* frame = &frames[frameCount - 1];
    currentFrame = frame;
    if (frame->ip >= frame->function->chunk.code + frame->function->chunk.count)
    {
        status = STATUS_RUNNING;
        return false;
    } 
    
    #define READ_BYTE() (*frame->ip++)
    #define READ_SHORT() (frame->ip += 2,(uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
    #define READ_CONSTANT() (interpreter->constants[READ_BYTE()])

    const bool debug = false;



    if (debug)
    {
        printf("          \n");
        for (Value* slot = stack; slot < stackTop; slot++)
        {
            printf("|\t");
            PRINT_VALUE(*slot);
            printf("\n");
        }
        printf("\n");
    }
        uint8_t instruction  = READ_BYTE();
        switch (instruction)
        {
            case OP_CONSTANT:
            {
                Value value = READ_CONSTANT();
                push(value);
                break;
            }
            case OP_NIL:
            {
                push(NIL());
                break;
            }
            case OP_TRUE:
            {
                push(BOOLEAN(true));
                break;
            }
            case OP_FALSE:
            {
                push(BOOLEAN(false));
                break;
            }
            case OP_POP:
            {
                pop();
                break;
            }
            case OP_DUP:
            {
                Value value = pop();
                push(value);
                push(value);
                break;
            }
            case OP_HALT:
            {
               // WARNING("Process '%s' exited", name);
                status = STATUS_DEAD;
                return false;
            }
            case OP_XOR:
            {
                Value b = pop();
                Value a = pop();
                bool result = (IS_TRUTHY(a) != IS_TRUTHY(b));  
                push(BOOLEAN(result));
                break;
            }
            case OP_RETURN:
            {
                Value result = pop();
                frameCount--;
                if (frameCount == 0)
                {
                    pop();
                    INFO("Process '%s' finished", name);
                    status = STATUS_DEAD;
                    return false;
                }
               // WARNING("Process '%s' function returned", name);
                
                stackTop = frame->slots;
                push(result);
                frame = &frames[frameCount - 1];
                currentFrame = frame;
                break;
            }
            case OP_PRINT:
            {
                
                Value value = pop();
                PRINT_VALUE(value);

                // Value count = pop();
                // if (count.type != ValueType::NUMBER)
                // {
                //     runtimeError("In print expected a number.");
                //     return false;
                // }
                // int len = AS_INTEGER(count);
                // int index = len-1;
                // for (int i = 0; i < len; i++)
                // {
                //     Value value = peek(index);
                //     index--;
                //     PRINT_VALUE(value);
                // }
                // popn(len);
                
                printf("\n");
                break;
            }
            case OP_CALL:
            {
 
        
                int argCount = READ_BYTE();
                Value value= peek(argCount);

               // value.print();
     

                if (IS_FUNCTION(value))
                {
                    ObjFunction* function = AS_FUNCTION(value);
                    if (argCount != function->arity)
                    {
                        ERROR("In call function '%s' expected %d arguments, got %d.", function->name, function->arity, argCount);
                        runtimeError("In Call function");
                        return false;
                    }
                    
                    if (!call(function, argCount))
                    {

                        status = STATUS_DEAD;
                        return false;
                    }

           
                }
                else if (IS_NATIVE(value))
                {
       
                        ObjNative *obj_native = AS_NATIVE(value);
                        NativeFn native = obj_native->function;
                        Value result = native(argCount, stackTop - argCount);
                        stackTop -= argCount + 1;
                        push(result);
                     //   return true;

                } 
                else if (IS_PROCESS(value))
                {
                    ObjProcess* process = AS_PROCESS(value);
                    
                  //  INFO("Process '%s' called", process->name);

                    Process* child = interpreter->queue_process(process->name,  100);
                    
              
 



                    CallFrame* cframe = &child->frames[child->frameCount++];
                    child->defineLocals= argCount;
                    cframe->function =  process->process->function;
                    cframe->ip = process->function->chunk.code;
                    cframe->slots = child->stackTop;
                    child->init_locals();
                    for (int i = argCount-1; i >= 0; i--)
                    {
                        Value arg = peek(i);
                        child->push(arg);
                    }
        
                    

                   // disassembleCode(&process->process->function->chunk, process->name);
                   // frame->slots = stackTop - argCount - 1;
       
                   return true;
                }
 
    
               // runtimeError("In call expected a function.");
          

               // disassembleCode(&function->chunk, function->name);


                break;
            }
            case OP_FRAME:
            {

       
                 Value frameValue = pop();
                double frame_param = AS_NUMBER(frameValue);
                
                // Interpretar o parâmetro frame():
                // frame(100) = velocidade normal (60fps)
                // frame(50)  = metade da velocidade (30fps)  
                // frame(200) = velocidade dupla (120fps)
                // frame(1)   = muito lento (0.6fps)
      
                double base_fps = 60.0;  
                double target_fps = (frame_param / 100.0) * base_fps;
                
      
                if (target_fps <= 0.0) target_fps = 0.1;
                
                frame_interval = 1.0 / target_fps;
                frame_timer = 0.0; // Reset timer
                
                status = STATUS_RUNNING;
                //goto break_all;

                return true; 
            }
            case OP_ADD:
            {
                
                if (peek(0).type == ValueType::NUMBER && peek(1).type == ValueType::NUMBER)
                {
                    Value b = pop();
                    Value a = pop();
                    push(NUMBER(a.number + b.number));
                }
                else if (peek(0).type == ValueType::STRING && peek(1).type == ValueType::STRING)
                {
                    Value b = pop();
                    Value a = pop();
                    const char* textA = a.string->data;
                    const char* textB = b.string->data;

                    int length = snprintf(nullptr, 0, "%s%s", textA, textB);
                    if (length > 255)
                    {
                        runtimeError("String too long.");
                        return false;
                    }
                    char text[256];
                    snprintf(text, length + 1, "%s%s", textA, textB);
                    text[length] = '\0';
                    push(STRING(text));
                } else if (peek(0).type == ValueType::STRING && peek(1).type == ValueType::NUMBER)
                {
                
                    Value b = pop();
                    pop();
                    const char* textA = b.string->data;
                    
                    char text[256];
                    int length = snprintf(text, sizeof(text), "%s%d", textA, (int)b.number);
                    
                    if (length < 256) 
                    {
                        push(STRING(text));
                    } else 
                    {
                        char* dynText = (char*)malloc(length + 1);
                        if (!dynText) 
                        {
                            runtimeError("Memory allocation failed.");
                            return false;
                        }
                        snprintf(dynText, length + 1, "%s%d", textA, (int)b.number);
                        push(STRING(dynText));
                        free(dynText);
                    }
                }

                else
                {
                    PRINT_VALUE(peek(0));
                    PRINT_VALUE(peek(1));
                    runtimeError("Operation 'add' not supported.");
                    return false;
                }
                break;
            }
            case OP_SUBTRACT:
            {
                if (peek(0).type == ValueType::NUMBER && peek(1).type == ValueType::NUMBER)
                {
                    Value b = pop();
                    Value a = pop();
                    push(NUMBER(a.number - b.number));
                }
                else
                {
                    runtimeError("Operation 'sub' not supported.");
                }
                break;
            }
            case OP_MULTIPLY:
            {
                if (peek(0).type == ValueType::NUMBER && peek(1).type == ValueType::NUMBER)
                {
                    Value b = pop();
                    Value a = pop();
                    push(NUMBER(a.number * b.number));
                }
                else
                {
                    runtimeError("Operation 'mul' not supported.");
                }
                break;
            }
            case OP_DIVIDE:
            {
                if (peek(0).type == ValueType::NUMBER && peek(1).type == ValueType::NUMBER)
                {
                    Value b = pop();
                    Value a = pop();
                    push(NUMBER(a.number / b.number));
                }
                else
                {
                    runtimeError("Operation 'div' not supported.");
                }
                break;
            }
            case OP_NEGATE:
            {
                if (peek(0).type == ValueType::NUMBER)
                {
                    Value value = pop();
                    push(NUMBER(-value.number));
                }
                else
                {
                    runtimeError("Operation 'neg' not supported.");
                }
                break;
            }
            case OP_EQUAL:
            {
                Value b = pop();
                Value a = pop();
                push(BOOLEAN(MATCH(a, b)));
                break;
            }
            case OP_GREATER:
            {
                if (peek(0).type != ValueType::NUMBER || peek(1).type != ValueType::NUMBER)
                {
                    runtimeError("Operation '>' not supported.");
                    return false;
                }
                Value b = pop();
                Value a = pop();
                push(BOOLEAN(a.number > b.number));
                break;
            }
            case OP_LESS:
            {
                if (peek(0).type != ValueType::NUMBER || peek(1).type != ValueType::NUMBER)
                {
                    runtimeError("Operation '<' not supported.");
                    return false;
                }
                Value b = pop();
                Value a = pop();
                push(BOOLEAN(a.number < b.number));
                break;
            }
            case OP_BANG_EQUAL:
            {
                if (peek(0).type != ValueType::NUMBER || peek(1).type != ValueType::NUMBER)
                {
                    runtimeError("Operation '!=' not supported.");
                    return false;
                }
                Value b = pop();
                Value a = pop();
                push(BOOLEAN(a.number != b.number));
                break;
            }
            case OP_GREATER_EQUAL:
            {
                if (peek(0).type != ValueType::NUMBER || peek(1).type != ValueType::NUMBER)
                {
                    runtimeError("Operation '>=' not supported.");
                    return false;
                }
                Value b = pop();
                Value a = pop();
                push(BOOLEAN(a.number >= b.number));
                break;
            }
            case OP_LESS_EQUAL:
            {
                if (peek(0).type != ValueType::NUMBER || peek(1).type != ValueType::NUMBER)
                {
                    runtimeError("Operation '<=' not supported.");
                    return false;
                }
                Value b = pop();
                Value a = pop();
                push(BOOLEAN(a.number <= b.number));
                break;
            }

            case OP_DEFINE_GLOBAL:
            {
                //INFO("Define Global");
                Value name = READ_CONSTANT();
                if (name.type != ValueType::STRING)
                {
                    PRINT_VALUE(name);
                    runtimeError("Variable name must be a string.");
                    return false;
                }
                Value value = peek(0);
                if (interpreter->define(AS_STRING(name)->data, std::move(value)))
                {
                   // INFO("Variable '%s' defined.", AS_STRING(name)->data);
                    pop();
                }
                break;
            }
            case OP_GET_GLOBAL:
            {
                Value name = READ_CONSTANT();
                if (name.type != ValueType::STRING)
                {
                    runtimeError("Variable name must be a string.");
                    return false;
                }
                if (interpreter->contains(AS_STRING(name)->data))
                {
                    Value value =interpreter->get(AS_STRING(name)->data);
                    push(value);
                }
                else
                {
                    runtimeError("Get Global");
                    ERROR("Undefined variable '%s'.", AS_STRING(name)->data);
                    return false;
                }
                break;
            }
            case OP_SET_GLOBAL:
            {
                Value name = READ_CONSTANT();
                if (name.type != ValueType::STRING)
                {
                    runtimeError("Variable name must be a string.");
                    return false;
                }
                 interpreter->define(AS_STRING(name)->data, peek(0));
                 break;
            }
            case OP_GET_LOCAL:
            {
                u8 slot = READ_BYTE();
                Value value = frame->slots[slot];
                // printf("GET_LOCAL[%d] = ", slot);
                // PRINT_VALUE(value);
                // printf("\n");
                push(value);
                break;
            }
            case OP_SET_LOCAL:
            {
                u8 slot = READ_BYTE();
                Value value = peek(0);
                // printf("SET_LOCAL[%d] = ", slot);
                // PRINT_VALUE(value);
                // printf("\n");
                frame->slots[slot] = value;
     
                break;
            }
            case     OP_DEFINE_LOCAL:
            {

                Value name = READ_CONSTANT();
                if (name.type != ValueType::STRING)
                {
                    PRINT_VALUE(name);
                    runtimeError("Variable name must be a string.");
                    return false;
                }
                

                defineLocals++;
                Value value = pop();
                printf("DEFINE_LOCAL[%d] = ", defineLocals);
                PRINT_VALUE(value);
                printf("\n");
                frame->slots[defineLocals] = value;
                break;
            }
            case OP_JUMP:
            {
                u16 offset = READ_SHORT();
                frame->ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE:
            {
                u16 offset = READ_SHORT();
                if (IS_FALSEY(peek(0)))
                {
                    frame->ip += offset;
                }
                break;
            }
            case OP_JUMP_IF_TRUE:
            {
                u16 offset = READ_SHORT();
                if (IS_TRUTHY(peek(0)))
                {
                    frame->ip += offset;
                }
                break;
            }
            case OP_LOOP:
            {
                u16 offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }
            case OP_NOW:
            {
                push(NUMBER(time_now()));
                break;
            }
         
            default:
            {
                runtimeError("Unimplemented opcode."+String(instruction));
                status = STATUS_DEAD;
                return false;
            }
        }
    
    }
 
     break_all:
        return status == STATUS_RUNNING;
    
    //INFO("Process '%s' running", name);

    #undef READ_BYTE
    #undef READ_SHORT
    #undef READ_CONSTANT
    return status == STATUS_RUNNING;
}
