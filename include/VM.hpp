
#include "String.hpp"
#include "Chunk.hpp"
#include "Vector.hpp"
#include "Map.hpp"
 


class Value;
class ObjFunction;
class ObjNative;
class Interpreter;
class ObjString;
class ObjProcess;
class Parser;
class Process;


Value INTEGER(int value);
Value NUMBER(double value);
Value STRING(const char* value);
Value BOOLEAN(bool value);
Value NIL();
Value FUNCTION(ObjFunction* function);
Value NATIVE(ObjNative* native);
Value PROCESS(ObjProcess* process);

bool IS_BOOLEAN(const Value& value);
bool IS_NUMBER(const Value& value);
bool IS_STRING(const Value& value);
bool IS_NIL(const Value& value);
bool IS_FUNCTION(const Value& value);
bool IS_NATIVE(const Value& value);
bool IS_PROCESS(const Value& value);

bool IS_FALSEY(const Value& value);
bool IS_TRUTHY(const Value& value);

bool MATCH(const Value& value, const Value& with);
void PRINT_VALUE(const Value& value);

const ObjString* AS_STRING(const Value& value);
double AS_NUMBER(const Value& value);
int     AS_INTEGER(const Value& value);
bool    AS_BOOLEAN(const Value& value);
ObjFunction *AS_FUNCTION(const Value& value);
ObjNative   *AS_NATIVE(const Value& value);
ObjProcess  *AS_PROCESS(const Value& value);

enum class ValueType
{
    NIL,
    BOOL,
    NUMBER,
    STRING,
    FUNCTION,
    NATIVE,
    PROCESS,
    OBJ
};


enum class ObjType
{
    FUNCTION,
    NATIVE,
    PROCESS,
    CLOSURE,
    UPVALUE,
    STRING
};


enum ProcessStatus : u32
{
    STATUS_RUNNING = 1,
    STATUS_SLEEPING = 2,
    STATUS_FROZEN = 4,
    STATUS_WAITING = 8,
    STATUS_PAUSED = 16,
    STATUS_DEAD = 32,
    STATUS_KILLED = 64
};


enum OpCode : u32
{
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_DUP,
    OP_HALT,
    OP_RETURN,
    OP_PRINT,
    OP_CALL,
    OP_FRAME,

    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    OP_MODULO,
    OP_POWER,

    OP_AND,
    OP_OR,
    OP_XOR,
    
    //OP_EQUAL_EQUAL,
    OP_BANG_EQUAL,
    OP_GREATER_EQUAL,
    OP_LESS_EQUAL,
    OP_NOT_EQUAL,
    OP_NOT,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,

    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_DEFINE_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,

    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_JUMP_IF_TRUE,
    OP_LOOP,

    OP_NOW,
    OP_BREAK,
    OP_CONTINUE


    // Process-specific opcodes
    // OP_FRAME, // frame(ms) - yield for specified milliseconds
    // OP_YIELD, // yield execution to other processes
    // OP_SPAWN, // spawn new process
    // OP_KILL, // kill a process
    // OP_GET_PID, // get current process ID
    // OP_SLEEP // sleep process for N frames
};



class GarbageCollector;

class GCObject {
public:
    ObjType type;
    bool isMarked;
    GCObject* next;
    GCObject* prev;

    GCObject(ObjType type)
        : type(type), isMarked(false), next(nullptr), prev(nullptr)
    {}
    virtual ~GCObject() {}
    virtual void trace(GarbageCollector* gc) {}
};


class ObjString : public GCObject {
public:
    char* data;
    int length;
    ObjString(): GCObject(ObjType::STRING)
    {
        data = nullptr;
        length = 0;
    }
    ObjString(const char* str);
    ObjString(int value);
    ObjString(double value);


    ~ObjString();
};


struct LoopContext 
{
    int loopStart;
    ValueArray<int> breakJumps;
};


typedef Value (*NativeFn)(int argCount, Value* args);

class ObjNative : public GCObject 
{
public:
    NativeFn function;
    ObjNative(NativeFn function): GCObject(ObjType::NATIVE), function(function) {}
};

class ObjFunction : public GCObject 
{
 
public:
    u8 arity;
    Chunk chunk;
    char name[64];
    Vector<LoopContext> loopStack;
    ObjFunction();
    ObjFunction(const String& n);
    ObjFunction(const char* n);

};


class ObjProcess  : public GCObject 
{
 
public:
  
    char name[64];
    Process* process;
    ObjFunction* function;
    ObjProcess();
    ObjProcess(const String& n);
    ObjProcess(const char* n);
};


struct Value
{
    u8 flags;
    ValueType type;
    union 
    {
        bool boolean;
        double number;
        ObjString* string;
        ObjFunction* function;
        ObjNative* native;
        ObjProcess* process;
    };

    Value(): flags(0), type(ValueType::NIL) {}



    bool isTruthy() const
    {
        switch (type)
        {
            case ValueType::NIL: return false;
            case ValueType::BOOL: return boolean;
            case ValueType::NUMBER: return number != 0.0;
            case ValueType::OBJ: return true; // Strings and other objects are truthy
            case ValueType::STRING: return string->length != 0;

            default: return false;
        }
    }
    bool isFalsey() const { return !isTruthy(); }
    void print();
    void cleanup();
    Value clone() const;
};



class GarbageCollector {
private:
    GCObject* head;
    GCObject*** roots;
    int rootCount;
    int rootCapacity;

public:
    GarbageCollector();
    ~GarbageCollector();
    void addObject(GCObject* obj);
    void removeObject(GCObject* obj);
    template <typename T, typename... Args> T* allocate(Args&&... args)
    {
        T* obj = new T(args...);
        addObject(obj);
        return obj;
    }

    void addRoot(GCObject** root);
    void collect();
    int countObjects();


private:
    void markObject(GCObject* obj);

public:
    void mark(GCObject* obj);
};


struct Local
{
    char name[128]{ '\0' };
    u32 len;
    int depth;
    bool isArg;
};

class CallFrame {
public:
    ObjFunction* function; // Function being called
    uint8_t* ip; // Instruction pointer
    Value* slots; // Pointer to function's stack window

    CallFrame(): function(nullptr), ip(nullptr), slots(nullptr) {}
};



class Process 
{
private:
    static u32 nextPID;
    static const s32 FRAMES_MAX = 1024;
    static const s32 STACK_MAX = FRAMES_MAX * 256;
    static const s32 UINT8_COUNT = 512; 




     Local locals[UINT8_COUNT];
    int localCount;
    int defineLocals;
    int scopeDepth;

    CallFrame frames[FRAMES_MAX];
    CallFrame* currentFrame;
    int frameCount;

    Value stack[STACK_MAX];
    Value* stackTop;
  

    void runtimeError(const String& message);
    void disassembleCode(Chunk* chunk, const char* name);
    u32 disassembleInstruction(Chunk* chunk, u32 offset);
    u32 constantInstruction(Chunk* chunk, const char* name, u32 offset);
    u32 byteInstruction(Chunk* chunk, const char* name, u32 offset);
    u32 jumpInstruction(Chunk* chunk, const char* name, u32 sign, u32 offset);
    u32 simpleInstruction(Chunk* chunk, const char* name, u32 offset);
    void markInitialized();




    friend class GarbageCollector;
    friend class Interpreter;
    friend class Parser;
    Interpreter* interpreter;

public:
    u32 pid; // Process ID
    char name[64];
    u32 id;
    s32 priority;
    ProcessStatus status;
    s32 frame_percent;
    ProcessStatus saved_status;
    s32 saved_priority;
    s32 last_priority;
    ObjFunction* function;
    bool root;


    Process* next;
    Process* prev;


    int addLocal(const char* name,size_t len,bool isArg);
    int resolveLocal(const char* name,size_t len);
    

    void resetStack();
    void push(Value value);
    Value pop();
    Value peek(int distance);
    void popn(int n);

    bool is_alive() const;
    bool should_execute() const;

    bool call(ObjFunction* function, int argCount);

    void writeChunk(u8 instruction, int line);

    Process(Interpreter* interpreter,bool isRoot);
    ~Process();

    bool run();

    void disassemble();
    bool isEmpty();
};


class Interpreter {

    Process* first_instance;
    Process** priority_list;
    Process* main_process; 
    u32 priority_list_size;
    u32 priority_list_capacity;
    u32 current_priority_index;

    u32 next_process_id;
    u32 current_frame;
    bool frame_completed;
    bool must_exit;
    s32 exit_value;
    bool priority_dirty;
    ValueArray<ObjFunction*> functions;
    ValueArray<ObjNative*> natives;
    ValueArray<Process*> processes;
    Parser* parser;
    UnorderedMap<String, Value> globals;
    ValueArray<Value> constants;
    friend class Parser;
    friend class Process;

public:
    Interpreter();
    ~Interpreter();
    void clear();
    Process* add_process(const char* name, bool root, int32_t priority = 0);
    Process* create_process(const char* name);

    bool call_process(Process* process, int32_t priority = 0);


    ObjFunction* add_function(const char* name, u8 arity = 0);
    ObjFunction* find_function(const char* name);
    void rebuild_priority_list();
    void instance_reset_iterator_by_priority();
    Process* instance_next_by_priority();
    u32 instance_count();
    void cleanup_dead_processes();
    bool has_alive_processes() const;
    bool kill_process(const char* name);
    bool kill_process(u32 pid);
    Process* find_process(const char* name);
    Process* find_process(u32 pid);
    void request_exit(s32 value = 0);
    u32 run();

    bool define(const char* name, Value value);
    bool contains(const char* name );
    Value get(const char* name);
    u32 addConstant(Value value);

    bool compile(const char* source);
    bool compile_file(const char* path);

    void runtimeError(const String& message);

    void disassemble();

    void defineNative(const char* name, NativeFn function);
};