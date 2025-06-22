#include "VM.hpp"
#include "Utils.hpp"
#include "Parser.hpp"
#include <raylib.h>
GarbageCollector GC;


void Value::cleanup()
{
   if (type == ValueType::STRING && string)
   {
       delete []string;
       string = nullptr;
   }        
}

Value Value::clone() const 
{ 
    Value v;
    v.type = type;
    v.flags = flags;
    switch (type)
    {
        case ValueType::BOOL: v.boolean = boolean; break;
        case ValueType::NUMBER: v.number = number; break;
        case ValueType::STRING: v.string = new ObjString(*string); break;
        case ValueType::OBJ: v.string = string; break;
        default: break;
    }
    return v;
}

ObjString::ObjString():GCObject(ObjType::STRING) 
{
    length = 0;
    data = nullptr;
}

ObjString::ObjString(const char* str) :GCObject(ObjType::STRING)
{
    length = strlen(str);
    data = new char[length + 1];
    strcpy(data, str);
    data[length] = '\0';
}

ObjString::ObjString(const char* str, size_t length) :GCObject(ObjType::STRING)
{
    this->length = length;
    data = new char[length + 1];
    strcpy(data, str);
    data[length] = '\0';

}

ObjString::ObjString(int value) :GCObject(ObjType::STRING)
{
    length = snprintf(nullptr, 0, "%d", value);
    data = new char[length + 1];
    snprintf(data, length + 1, "%d", value);
}

ObjString::ObjString(double value) :GCObject(ObjType::STRING)
{
    length = snprintf(nullptr, 0, "%f", value);
    data = new char[length + 1];
    snprintf(data, length + 1, "%f", value);
}

ObjString::~ObjString()
{
   //INFO("deleting string: %s", data);
    if (!data) return;
    delete[] data;
}


GarbageCollector::GarbageCollector()
    : head(nullptr), roots(nullptr), rootCount(0), rootCapacity(32)
{}

GarbageCollector::~GarbageCollector()
{
    INFO("deleting garbage collector");
    for(u32 i = 0; i < stringPool.size(); i++)
    {
        delete stringPool[i];
    }

    while (head)
    {
        GCObject* next = head->next;
        delete head;
        head = next;
    }
    delete[] roots;
}


void GarbageCollector::addObject(GCObject* obj)
{
    if (head)
    {
        head->prev = obj;
    }
    obj->next = head;
    obj->prev = nullptr;
    head = obj;
}


void GarbageCollector::removeObject(GCObject* obj)
{
    if (obj->prev)
    {
        obj->prev->next = obj->next;
    }
    else
    {
        head = obj->next;
    }

    if (obj->next)
    {
        obj->next->prev = obj->prev;
    }
}


void GarbageCollector::addRoot(GCObject** root)
{
    if (rootCount >= rootCapacity)
    {
        int newCapacity = rootCapacity == 0 ? 4 : rootCapacity * 2;
        GCObject*** newRoots = new GCObject**[newCapacity];

        for (int i = 0; i < rootCount; i++)
        {
            newRoots[i] = roots[i];
        }

        delete[] roots;
        roots = newRoots;
        rootCapacity = newCapacity;
    }

    roots[rootCount++] = root;
}

void GarbageCollector::collect()
{
    // Fase Mark
    for (int i = 0; i < rootCount; i++)
    {
        if (*roots[i] != nullptr)
        {
            markObject(*roots[i]);
        }
    }

    // Fase Sweep
    GCObject* current = head;
    while (current)
    {
        GCObject* next = current->next;

        if (current->isMarked)
        {
            current->isMarked = false; // Reset para próxima coleta
        }
        else
        {
            removeObject(current);
            delete current;
        }

        current = next;
    }
}


int GarbageCollector::countObjects()
{
    int count = 0;
    GCObject* current = head;
    while (current)
    {
        count++;
        current = current->next;
    }
    return count;
}

ObjString* GarbageCollector::newString(const String& str) 
{
    ObjString* string = new ObjString(str.c_str(), str.length());
    stringPool.push_back(string);
    return string;
}

ObjString* GarbageCollector::newString(const char* str) 
{ 
    ObjString* string = new ObjString(str);
  //  stringPool.push_back(string);
    return string;
}


void GarbageCollector::markObject(GCObject* obj)
{
    if (!obj || obj->isMarked) return;

    obj->isMarked = true;
    obj->trace(this); // Objeto marca suas próprias referências
}


void GarbageCollector::mark(GCObject* obj) { markObject(obj); }

Value INTEGER(int value)
{
    Value v;
    v.number = static_cast<double>(value);
    v.type = ValueType::NUMBER;
    return v;
}
Value NUMBER(double value)
{
    Value v;
    v.number = value;
    v.type = ValueType::NUMBER;
    return v;
}
Value BOOLEAN(bool value)
{
    Value v;
    v.boolean = value;
    v.type = ValueType::BOOL;
    return v;
}
Value NIL()
{
    Value v;
    v.number = 0;
    v.type = ValueType::NIL;
    return v;
}

Value NATIVE(ObjNative* native) 
{ 
    Value v;
    v.native = native;
    v.type = ValueType::NATIVE;
 //   GC.addObject(v.native);
    return v;
}

Value PROCESS(ObjProcess* process) 
{
    Value v;
    v.process = process;
  //  GC.addObject(v.process);
    v.type = ValueType::PROCESS;
    return v;
}

Value FUNCTION(ObjFunction* function) 
{ 
    Value v;
    v.function = function;
    v.type = ValueType::FUNCTION;
 //   GC.addObject(v.function);
    return v;
}


bool IS_INTEGER(const Value& value) { return value.type == ValueType::NUMBER; }
bool IS_BOOLEAN(const Value& value) { return value.type == ValueType::BOOL; }
bool IS_NUMBER(const Value& value) { return value.type == ValueType::NUMBER; }
bool IS_STRING(const Value& value) { return value.type == ValueType::STRING; }
bool IS_NIL(const Value& value) { return value.type == ValueType::NIL; }
bool IS_FUNCTION(const Value& value) { return value.type == ValueType::FUNCTION; }
bool IS_NATIVE(const Value& value) { return value.type == ValueType::NATIVE; }
bool IS_PROCESS(const Value& value) { return value.type == ValueType::PROCESS; }
bool IS_FALSEY(const Value& value) { return value.isFalsey(); }
bool IS_TRUTHY(const Value& value) { return value.isTruthy(); }


ObjString* AS_STRING(const Value& value) { return value.string; }
double AS_NUMBER(const Value& value) { return value.number; }
int AS_INTEGER(const Value& value) { return static_cast<int>(value.number); }
bool AS_BOOLEAN(const Value& value) { return value.boolean; }
ObjFunction* AS_FUNCTION(const Value& value) { return value.function; }
ObjNative* AS_NATIVE(const Value& value) { return value.native; }

ObjProcess* AS_PROCESS(const Value& value) { return value.process; }

bool MATCH(const Value& value, const Value& with)
{
    if (value.type != with.type) return false;
    if (IS_STRING(value) && IS_STRING(with))
    {
        if (AS_STRING(value)->length != AS_STRING(with)->length) return false;
        return strcmp(AS_STRING(value)->data, AS_STRING(with)->data) == 0;
    }
    else if (IS_NUMBER(value) && IS_NUMBER(with))
        return fabs(AS_NUMBER(value) - AS_NUMBER(with))
            < 0.01953; // TODO: use epsilon error margin
    else if (IS_BOOLEAN(value) && IS_BOOLEAN(with))
        return AS_BOOLEAN(value) == AS_BOOLEAN(with);
    else if (IS_NIL(value))
        return true;
    
    return false;
}


Value STRING(const char* value)
{
    Value v;
    v.string = GC.allocate<ObjString>(value); // <ObjString>(value>
    v.type = ValueType::STRING;
    return v;
}

Value SHARED_STRING(const char* value)
{
    Value v;
    v.string = GC.newString(value);
    v.type = ValueType::STRING;
    return v;
}

 
void PRINT_VALUE(const Value& value)
{
     switch (value.type)
    {
        case ValueType::NIL: printf("nil"); break;
        case ValueType::BOOL: printf("%s", value.boolean ? "true" : "false"); break;
        case ValueType::NUMBER: printf("%f", value.number); break;
        case ValueType::STRING: printf("%s", value.string->data); break;
        case ValueType::OBJ: printf("object"); break;
        case ValueType::FUNCTION: printf("<%s>", value.function->name); break;
        case ValueType::NATIVE: printf("<native>"); break;
        case ValueType::PROCESS: printf("<process>"); break;
        default: printf("unknown"); break;
    }
}

void Value::print()
{
    switch (type)
    {
        case ValueType::NIL: printf("nil\n"); break;
        case ValueType::BOOL: printf("%s\n", boolean ? "true" : "false"); break;
        case ValueType::NUMBER: printf("N:%f\n", number); break;
        case ValueType::STRING: printf("S:%s\n", string->data); break;
        case ValueType::OBJ: printf("object\n"); break;
        case ValueType::FUNCTION: printf("<%s>\n", function->name); break;
        case ValueType::NATIVE: printf("<native>\n"); break;
        case ValueType::PROCESS: printf("<process>\n"); break;
        default: printf("unknown\n"); break;
    }
}


ObjFunction::ObjFunction():  arity(0)
{
    memcpy(name, "function", 7);
    name[7] = '\0';
}
ObjFunction::ObjFunction(const String &n): arity(0)
{
    size_t len = n.length();
    strncpy(name, n.c_str(), len);
    name[len] = '\0';
}
ObjFunction::ObjFunction(const char *n):  arity(0)
{
    size_t len = strlen(n);
    memccpy(name, n, '\0', len);
    name[len] = '\0';
}


ObjProcess* Interpreter::add_raw_process(const char* name) 
{ 
    ObjProcess* process = new ObjProcess(name);
    raw_processes.push_back(process);
    return process;
}

Interpreter::Interpreter()
{
    first_instance = nullptr;
    last_instance = nullptr;

    queu_processes.reserve(512);
 
    next_process_id = 1;
    current_frame = 0;
    frame_completed = false;
    must_exit = false;
    exit_value = 0;
    first_instance = nullptr;
    last_instance = nullptr;
    parser = new Parser(this);
    main_process = add_process("_main_", true, 0);
    first_instance = main_process;
    panicMode = false;
    
}

Interpreter::~Interpreter()
{
    clear();
   // main_process = nullptr;
    delete parser;
   // delete first_instance;
    delete main_process;

    if (queu_processes.getSize () > 0)
    {
        for (u32 i = 0; i < queu_processes.getSize (); i++)
        {
            delete queu_processes[i];
        }
        queu_processes.clear();
    }

    // for (u32 i = 0; i < functions.getSize (); i++)
    // {
    //   // delete functions[i];
    // }
    // functions.clear();

    for (u32 i = 0; i < processes.getSize (); i++)
    {
        delete processes[i];
    }
    processes.clear();
 
    for (u32 i = 0; i < raw_processes.getSize (); i++)
    {
      // delete raw_processes[i];
    }
    raw_processes.clear();  

    constants.clear();

    // for (u32 i = 0; i < natives.getSize (); i++)
    // {
    //     delete natives[i];
    // }
    // natives.clear();


    auto all = globals.get_all_pairs();
    for ( auto& kv : all) 
    {
        if (IS_FUNCTION(kv.value))
        {
            delete kv.value.function;
        } else 
        if (IS_NATIVE(kv.value))
        {
            delete kv.value.native;
        } else 
        if (IS_PROCESS(kv.value))
        {
           delete kv.value.process;
        }
        
//        kv.value.print();
    }
    globals.clear();

 
}

void Interpreter::clear()
{
    if (!first_instance)
    {
        return;
    }
  
    Process* current = first_instance;
    while (current)
    {
        Process* next = current->next;
        delete current;
        current = next;
    }
    first_instance = nullptr;   
    main_process =      add_process("_main_", true, 0);
    first_instance = main_process;
    panicMode = false;

}

 u32 Interpreter::addConstant(Value value) 
 { 
    u32 count =  constants.getSize();
    for (u32 i = 0; i < count; i++)
    {
        if (MATCH(value, constants[i]))
        {
            return i;
        }
    }
    constants.push_back(std::move(value));
    return constants.getSize() - 1;
 }

 Process* Interpreter::create_process(const char* name)
{
    Process* process = new Process(this, true);
    memccpy(process->name, name, '\0', strlen(name));
    processes.push_back(process);
    return process;
}


Process* Interpreter::queue_process(const char* name,   int32_t priority)
{
    Process* process = new Process(this, false);
    memccpy(process->name, name, '\0', strlen(name));
    process->priority = priority;
 
    process->frame_timer = 0.0;
    process->frame_interval = 1.0 / 60.0; // Default to 60 FPS
    process->priority = priority;

    process->next = nullptr;
    process->prev = nullptr;

    queu_processes.push_back(process);

    return process;
}

Process* Interpreter::add_process(const char* name, bool root, int32_t priority)
{
    Process* process = new Process(this, root);
    memccpy(process->name, name, '\0', strlen(name));
    process->priority = priority;
 
    process->frame_timer = 0.0;
    process->frame_interval = 1.0 / 60.0; // Default to 60 FPS
    process->priority = priority;

    process->next = nullptr;
    process->prev = nullptr;

    
  if (!first_instance) 
  {
        first_instance = process;
        last_instance = process;
    } else 
    {
        last_instance->next = process;
        process->prev = last_instance;
        last_instance = process;
    }

    return process;
}

 

void Interpreter::request_exit(s32 value)
{
    must_exit = true;
    exit_value = value;
}

u32 Interpreter::instance_count()
{
    uint32_t count = 0;
    Process* current = first_instance;
    while (current)
    {
        if (current->is_alive())
        {
            count++;
        }
        current = current->next;
    }
    return count;
}


void Interpreter::runtimeError(const String& message)
{
    ERROR("Runtime error: %s", message.c_str());
}




bool Interpreter::kill_process(const char* name)
{
    Process* current = first_instance;
    while (current)
    {
        if (strcmp(current->name, name) == 0)
        {
            current->status = STATUS_KILLED;
            break;
        }
        current = current->next;
    }
    return false;
}

bool Interpreter::kill_process(u32 pid)
{
    Process* current = first_instance;
    while (current)
    {
        if (current->id == pid)
        {
            current->status = STATUS_KILLED;
            break;
        }
        current = current->next;
    }
    return false;
}


Process* Interpreter::find_process(const char* name)
{
    Process* current = first_instance;
    while (current)
    {
        if (strcmp(current->name, name) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

Process* Interpreter::find_process(u32 pid)
{
    Process* current = first_instance;
    while (current)
    {
        if (current->id == pid)
        {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}


bool Interpreter::has_alive_processes() const
{
    Process* current = first_instance;
    while (current)
    {
        if (current->is_alive())
        {
            return true;
        }
        current = current->next;
    }
    return false;
}

 void Interpreter::defineNative(const char* name, NativeFn function) 
 {
    if (main_process == nullptr) 
    {
        WARNING("No main process defined");
        return;

    }

    ObjNative *native = new ObjNative(function);
 

   // natives.push_back(native);

    main_process->push(SHARED_STRING(name));
    main_process->push(NATIVE(native));


    if (!define(name, main_process->stack[1]))
    {
        WARNING("Native %s already defined", name);
    }

    main_process->pop();
    Value b =main_process->pop();
    delete b.string;
 


}

void Interpreter::defineNatives(const NativeReg* natives) 
{
    for (int i = 0; natives[i].name != nullptr; ++i) 
    {
        defineNative(natives[i].name, natives[i].function);
    }
}



 
bool Interpreter::define(const char* name, Value value) 
{
    if (globals.contains(name))
    {
      //  WARNING("Variable %s already defined", name);
        globals[name] = std::move(value);
        return true;
    }
    globals.insert(name, std::move(value));
    return true;
}

bool Interpreter::contains(const char* name) 
{ 
    return globals.contains(name);
}

Value Interpreter::get(const char* name)
{
    if (globals.contains(name))
    {
        return globals[name];
    }
    return Value();
}

bool Interpreter::compile(const char* source)
{ 
    clear();
    
    if (parser->lexer->Load(source))
    {
        return parser->compile();
    }
    return false; 
}
bool Interpreter::compile_file(const char* path)
 {
  //  clear();
    if (parser->lexer->LoadFromFile(path))
    {
        return parser->compile();
    }
    return false;
}

void Interpreter::remove_process_from_list(Process* process)
{
    if (!process) return;
    
    if (process->prev)
        process->prev->next = process->next;
    else
        first_instance = process->next;  
        
    if (process->next)
        process->next->prev = process->prev;
    else
        last_instance = process->prev;  
    
    delete process;
}

 

u32 Interpreter::run()
{
    must_exit = false;
    
    while ((!must_exit || !panicMode) && !WindowShouldClose())
    {
        
        if (queu_processes.getSize() > 0)
        {
            Process* process = queu_processes.back();
            queu_processes.pop_back();
            
                if (!first_instance) 
                {
                        first_instance = process;
                        last_instance = process;
                    } else 
                    {
                        last_instance->next = process;
                        process->prev = last_instance;
                        last_instance = process;
                    }
           
        }


        BeginDrawing();
        ClearBackground(BLACK);
        
        double deltaTime = GetFrameTime();
        current_frame++;
        
 
        Process* i = first_instance;
        uint32_t i_count = 0;
        uint32_t dead_count = 0;
        
        while (i)
        {
            Process* next = i->next; // Safe iteration
            ProcessStatus status = i->status;
            
            if (status == STATUS_RUNNING)
            {
                i->frame_timer += deltaTime;
                
                if (i->frame_timer >= i->frame_interval)
                {
                    bool still_running = i->run();
                    
                    if (still_running && i->status == STATUS_RUNNING)
                    {
                        i_count++;
                        i->frame_timer -= i->frame_interval; // Maintain timing precision
                    }
                }
                else
                {
                    i_count++; // Count waiting processes as active
                }
                
                // Render active, non-root processes
                if (i->status == STATUS_RUNNING && !i->root)
                {
                    double x = i->stack[ID_X].number;
                    double y = i->stack[ID_Y].number;
                    DrawCircle(x, y, 5, WHITE);
                    // Optional: DrawText(TextFormat("FPS: %.0f", 1.0/i->frame_interval), x, y-20, 12, GRAY);
                }
            }
            else if (status == STATUS_DEAD || status == STATUS_KILLED)
            {
                dead_count++;
                remove_process_from_list(i); // Updates last_instance if needed
            }
            
            if (must_exit) break;
            
            i = next;
        }
        
        DrawFPS(10, 10);
        DrawText(TextFormat("Processes: %d", i_count), 10, 30, 20, WHITE);
        DrawText(TextFormat("Dead cleaned: %d", dead_count), 10, 50, 20, RED);
        
        
        EndDrawing();
    }
    
    return exit_value;
}
 ObjFunction*  Interpreter::add_function(const char* name, u8 arity)
 {
     ObjFunction* function = new ObjFunction(name);
     function->arity = arity;
  //   functions.push_back(function);
     return function;
 }

ObjFunction* Interpreter::find_function(const char* name)
{
//    for (u32 i = 0; i < functions.getSize(); i++)
//    {
//        ObjFunction* function = functions[i];
//        if (strcmp(function->name, name) == 0)
//        {
//            return function;
//        }
//    }
   return nullptr;
}



void Interpreter::disassemble()
{
   Process* current = first_instance;
    while (current)
    {
        current->disassemble();
        current = current->next;
    }
}

void Interpreter::Error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    Log(2, format, args);
    va_end(args);
    panicMode = true;
}

void Interpreter::Warning(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    Log(1, format, args);
    va_end(args);
}

void Interpreter::Info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    Log(0, format, args);
    va_end(args);
}

 

bool Interpreter::registerVariable(const char *name, Value value)
{
     if (globals.contains(name))
     {
        WARNING("Variable %s already defined", name);
         return false;
     }
     globals.insert(name, std::move(value));
     return true;
}

bool Interpreter::registerNumber(const char *name, double value)
{
     if (globals.contains(name))
     {
        WARNING("Variable %s already defined", name);
         return false;
     }
     globals.insert(name, std::move(NUMBER(value)));
     return true;
}

bool Interpreter::registerInteger(const char *name, int value)
{
     if (globals.contains(name))
     {
        WARNING("Variable %s already defined", name);
         return false;
     }
     globals.insert(name, std::move(INTEGER(value)));
     return true;
}

bool Interpreter::registerString(const char *name, const char *value)
{
     if (globals.contains(name))
     {
        WARNING("Variable %s already defined", name);
         return false;
     }
     globals.insert(name, std::move(STRING(value)));
     return true;
}

bool Interpreter::registerBoolean(const char *name, bool value)
{
     if (globals.contains(name))
     {
        WARNING("Variable %s already defined", name);
         return false;
     }
     globals.insert(name, std::move(BOOLEAN(value)));
     return true;
}

void Interpreter::push(Value v)
{
     main_process->push(std::move(v));
}

Value Interpreter::pop()
{
    return main_process->pop();
}

Value Interpreter::peek(int offset)
{
    return main_process->peek(offset);
}

Value Interpreter::top()
{
    return main_process->top();
}

long Interpreter::pop_int()
{
    Value value = pop();
    if (IS_NUMBER(value))
    {
        double number = AS_NUMBER(value);
        return (long)number;
    }
    Error("Expected number but got :");
    value.print();
    return 0;
}

double Interpreter::pop_double()
{
    Value value = pop();
    if (IS_NUMBER(value))
    {
        double number = AS_NUMBER(value);
        return number;
    }
    Error("Expected number but got :");
    value.print();
    return 0;
}

float Interpreter::pop_float()
{
    Value value = pop();
    if (IS_NUMBER(value))
    {
        double number = AS_NUMBER(value);
        return (float)number;
    }
    Error("Expected number but got :");
    value.print();
    return 0;
}

long Interpreter::pop_long()
{
    Value value = pop();
    if (IS_NUMBER(value))
    {
        double number = AS_NUMBER(value);
        return (long)number;
    }
    Error("Expected number but got :");
    value.print();
    return 0;
}

String Interpreter::pop_string()
{
    Value value = pop();
    if (IS_STRING(value))
    {
        ObjString* a = AS_STRING(value);
        String str(a->data, a->length);
        return str;
    }
    Error("Expected string but got :");
    value.print();
    return "";
}

bool Interpreter::pop_bool()
{
    Value value = pop();
    if (IS_BOOLEAN(value))
    {
        return AS_BOOLEAN(value);
    }
    Error("Expected bool but got :");
    value.print();   
    return false;
}

bool Interpreter::pop_nil()
{
    Value value = pop();
    if (IS_NIL(value))
    {
        return true;
    }
    Error("Expected nil but got :");
    value.print();
    return false;
}

void Interpreter::push_int(int value)
{
    double number = static_cast<double>(value);
     push(std::move(NUMBER(number)));
}

void Interpreter::push_double(double value)
{
    return push(std::move(NUMBER(value)));
}

void Interpreter::push_bool(bool value)
{
    return push(std::move(BOOLEAN(value)));
}

void Interpreter::push_nil()
{
    return push(std::move(NIL()));
}

void Interpreter::push_string(const char *value)
{
    return push(std::move(STRING(value)));
}

void Interpreter::push_string(const String &str)
{
    return push(std::move(STRING(str.c_str())));
}


ObjProcess::ObjProcess() 
{
    
    
    strncpy(name, "Process", sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0'; 
    process = nullptr;
    function = nullptr;
}

ObjProcess::ObjProcess(const String& n)  
{

     
    size_t len = n.length();
    memccpy(name, n.c_str(), '\0', len);
    process = nullptr;
    function = nullptr;
}

ObjProcess::ObjProcess(const char* n) 
{
    
    size_t len = strlen(n);
    memccpy(name, n, '\0', len);

    process = nullptr;
    function = nullptr;
}
