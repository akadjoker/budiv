#include "VM.hpp"
#include "Utils.hpp"
#include "Parser.hpp"

GarbageCollector GC;


void Value::cleanup()
{
   if (type == ValueType::OBJ && string)
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

ObjString::ObjString(const char* str): GCObject(ObjType::STRING)
{
    length = strlen(str);
    data = new char[length + 1];
    for (int i = 0; i <= length; i++)
    {
        data[i] = str[i];
    }
}

ObjString::ObjString(int value): GCObject(ObjType::STRING)
{
    length = snprintf(nullptr, 0, "%d", value);
    data = new char[length + 1];
    snprintf(data, length + 1, "%d", value);
}

ObjString::ObjString(double value): GCObject(ObjType::STRING)
{
    length = snprintf(nullptr, 0, "%f", value);
    data = new char[length + 1];
    snprintf(data, length + 1, "%f", value);
}

ObjString::~ObjString()
{
    //   INFO("deleting string: %s", data);
    delete[] data;
}


GarbageCollector::GarbageCollector()
    : head(nullptr), roots(nullptr), rootCount(0), rootCapacity(32)
{}

GarbageCollector::~GarbageCollector()
{
    INFO("deleting garbage collector");

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


const ObjString* AS_STRING(const Value& value) { return value.string; }
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
    v.string = GC.allocate<ObjString>(value);
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
    size_t len = strlen("Function");
    strncpy(name, "Function", len);
    name[len] = '\0';
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
    strncpy(name, n, len);
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
    priority_list = nullptr;
    priority_list_size = 0;
    priority_list_capacity = 0;
    current_priority_index = 0;
    next_process_id = 1;
    current_frame = 0;
    frame_completed = false;
    must_exit = false;
    exit_value = 0;
    priority_dirty = false;

    priority_list_capacity = 32;
    priority_list = (Process**)realloc(priority_list, priority_list_capacity * sizeof(Process*));
    parser = new Parser(this);
    main_process = add_process("_main_", true, 0);
    
}

Interpreter::~Interpreter()
{
    main_process = nullptr;
    delete parser;
    clear();
    delete first_instance;

    for (u32 i = 0; i < functions.getSize (); i++)
    {
        delete functions[i];
    }
    functions.clear();

    for (u32 i = 0; i < processes.getSize (); i++)
    {
        delete processes[i];
    }
    processes.clear();
 
    for (u32 i = 0; i < raw_processes.getSize (); i++)
    {
        delete raw_processes[i];
    }
    raw_processes.clear();  

    constants.clear();

    for (u32 i = 0; i < natives.getSize (); i++)
    {
        delete natives[i];
    }
    natives.clear();

    free(priority_list);
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
    priority_list_size = 0;
    priority_dirty = false;
    main_process =      add_process("_main_", true, 0);

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
    size_t len = strlen(name);
    strncpy(process->name, name, len);
    process->name[len] = '\0';
    processes.push_back(process);
    return process;
}

bool Interpreter::call_process(Process* process, int32_t priority)
{
    process->priority = priority;
    process->last_priority = priority;

    // Insert at end of linked list
    if (!first_instance)
    {
        first_instance = process;
    }
    else
    {
        Process* current = first_instance;
        while (current->next)
        {
            current = current->next;
        }
        current->next = process;
        process->prev = current;
    }

    priority_dirty = true;
    return true;
}

Process* Interpreter::add_process(const char* name, bool root,int32_t priority)
{
    Process* process = new Process(this, root);
    size_t len = strlen(name);
    strncpy(process->name, name, len);
    process->name[len] = '\0';
    process->priority = priority;
    process->last_priority = priority;

    // Insert at end of linked list
    if (!first_instance)
    {
        first_instance = process;
    }
    else
    {
        Process* current = first_instance;
        while (current->next)
        {
            current = current->next;
        }
        current->next = process;
        process->prev = current;
    }

    priority_dirty = true;
    return process;
}

void Interpreter::rebuild_priority_list()
{
    if (!priority_dirty) return;

    priority_list_size = 0;

    // Collect alive processes
    Process* current = first_instance;
    while (current)
    {
        if (current->is_alive())
        {
            if (priority_list_size >= priority_list_capacity)
            {
                // Resize if needed
                priority_list_capacity *= 2;
                priority_list = (Process**)realloc(priority_list, priority_list_capacity * sizeof(Process*));
            }
            priority_list[priority_list_size++] = current;
        }
        current = current->next;
    }

    // Simple bubble sort by priority (good enough for small lists)
    for (uint32_t i = 0; i < priority_list_size - 1; i++)
    {
        for (uint32_t j = 0; j < priority_list_size - i - 1; j++)
        {
            if (priority_list[j]->priority < priority_list[j + 1]->priority)
            {
                Process* temp = priority_list[j];
                priority_list[j] = priority_list[j + 1];
                priority_list[j + 1] = temp;
            }
        }
    }

    priority_dirty = false;
}

void Interpreter::instance_reset_iterator_by_priority()
{
    rebuild_priority_list();
    current_priority_index = 0;
}

Process* Interpreter::instance_next_by_priority()
{
    while (current_priority_index < priority_list_size)
    {
        Process* process = priority_list[current_priority_index++];
        if (process && process->is_alive())
        {
            return process;
        }
    }
    return nullptr;
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

void Interpreter::cleanup_dead_processes()
{
    Process* current = first_instance;
    while (current)
    {
        Process* next = current->next;

        if (current->status == STATUS_DEAD)
        {
            // Remove from linked list
            if (current->prev)
            {
                current->prev->next = current->next;
            }
            else
            {
                first_instance = current->next;
            }

            if (current->next)
            {
                current->next->prev = current->prev;
            }

            //     printf("Process %s removed (dead)\n", current->name);
            delete current;
            priority_dirty = true;
        }

        current = next;
    }
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
        if (current->pid == pid)
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
        if (current->pid == pid)
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
 

    natives.push_back(native);

    main_process->push(STRING(name));
    main_process->push(NATIVE(native));


    if (!define(name, main_process->stack[1]))
    {
        WARNING("Native %s already defined", name);
    }

    main_process->pop();
    main_process->pop();
}

u32 Interpreter::run()
{
    must_exit = false;

    while (has_alive_processes() && !must_exit)
    {
        frame_completed = false;
        current_frame++;

      //  printf("\n=== FRAME %u ===\n", current_frame);

        instance_reset_iterator_by_priority();
        Process* i = instance_next_by_priority();
        uint32_t i_count = 0;

        while (i && !must_exit)
        {
            if (i->frame_percent < 100)
            {
                ProcessStatus status = i->status;

                // ✅ VERIFICAR SE PROCESSO AINDA ESTÁ VIVO ANTES DE EXECUTAR
                if (status == STATUS_RUNNING)
                {
                 //   printf("Executing process: %s (priority: %d)\n", i->name, i->priority);

        
                    bool still_running = i->run();
                    
                    if (still_running && i->status == STATUS_RUNNING)
                    {
                        i_count++;
                    }
                    else
                    {
                        // Processo morreu/foi morto
                        WARNING("Process %s terminated (status: %d)\n", 
                               i->name, (int)i->status);
                    }

                    if (must_exit) break;
                }
                else if (status == STATUS_DEAD || status == STATUS_KILLED)
                {
                    // Processo já morreu, não executar
                    WARNING("Process %s is dead/killed, skipping\n", i->name);
                }
            }

            i = instance_next_by_priority();
        }

        // Frame completion logic
        if (i_count == 0)
        {
            frame_completed = true;
            
            // Update processes
            Process* current = first_instance;
            while (current)
            {
                current->saved_status = current->status;

                if (current->status == STATUS_DEAD
                    || current->status == STATUS_KILLED
                    || current->status == STATUS_RUNNING)
                {
                    current->frame_percent -= 100;
                }

                if (current->last_priority != current->priority)
                {
                    current->saved_priority = current->priority;
                    current->last_priority = current->priority;
                    priority_dirty = true;
                }

                current = current->next;
            }

            cleanup_dead_processes();
        //    printf("Frame completed!\n");
        }
    }

    return exit_value;
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
    clear();
    if (parser->lexer->LoadFromFile(path))
    {
        return parser->compile();
    }
    return false;
}
// u32 Interpreter::run()
// {
//     must_exit = false;

//     while (has_alive_processes() && !must_exit)
//     {
//         frame_completed = false;
//         current_frame++;

//         printf("\n=== FRAME %u ===\n", current_frame);

//         // Reset iterator por prioridade
//         instance_reset_iterator_by_priority();
//         Process* i = instance_next_by_priority();
//         uint32_t i_count = 0;

//         // Executa todos os processos que precisam executar neste frame
//         while (i && !must_exit)
//         {
//             if (i->frame_percent < 100)
//             {
//                 ProcessStatus status = i->status;

//                 if (status == STATUS_RUNNING)
//                 {
//                     printf("Executing process: %s (priority: %d)\n", i->name,i->priority);

//                     // Run instance
//                     if (i->run())
//                     {
//                         i_count++;
//                     }

//                     if (must_exit) break;
//                 }
//                 else if (status != STATUS_KILLED && status != STATUS_DEAD)
//                 {
//                     // Skip sleeping/frozen/waiting processes
//                 }
//             }

//             i = instance_next_by_priority();
//         }

//         // Se nenhum processo executou, o frame está completo
//         if (i_count == 0)
//         {
//             frame_completed = true;

//             // Update internal vars
//             Process* current = first_instance;
//             while (current)
//             {
//                 current->saved_status = current->status;

//                 // Decrease frame_percent for running/dead/killed processes
//                 if (current->status == STATUS_DEAD
//                     || current->status == STATUS_KILLED
//                     || current->status == STATUS_RUNNING)
//                 {
//                     current->frame_percent -= 100;
//                 }

//                 // Check priority changes
//                 if (current->last_priority != current->priority)
//                 {
//                     current->saved_priority = current->priority;
//                     current->last_priority = current->priority;
//                     priority_dirty = true;
//                 }

//                 current = current->next;
//             }

//             // Remove dead processes
//             cleanup_dead_processes();

//             printf("Frame completed!\n");
//         }
//     }

//     return exit_value;
// }

 ObjFunction*  Interpreter::add_function(const char* name, u8 arity)
 {
     ObjFunction* function = new ObjFunction(name);
     function->arity = arity;
     functions.push_back(function);
     return function;
 }

ObjFunction* Interpreter::find_function(const char* name)
{
   for (u32 i = 0; i < functions.getSize(); i++)
   {
       ObjFunction* function = functions[i];
       if (strcmp(function->name, name) == 0)
       {
           return function;
       }
   }
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

ObjProcess::ObjProcess() 
{
    
    size_t len = strlen("Process");
    strncpy(name, "Process", len);
    name[len] = '\0';
    process = nullptr;
    function = nullptr;
}

ObjProcess::ObjProcess(const String& n)  
{

     
    size_t len = n.length();
    strncpy(name, n.c_str(), len);
    name[len] = '\0';
    process = nullptr;
    function = nullptr;
}

ObjProcess::ObjProcess(const char* n) 
{
    
    size_t len = strlen(n);
    strncpy(name, n, len);
    name[len] = '\0';
    process = nullptr;
    function = nullptr;
}
