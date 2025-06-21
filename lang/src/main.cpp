//#include "TestString.hpp"
//#include "TestChunk.hpp"
//#include "TestStack.hpp"
//#include "TestVector.hpp"
//#include "TesteQueue.hpp"
//#include "TestRai.hpp"
//#include "TesteMap.hpp"

#include "Config.hpp"
#include "VM.hpp"
#include "Utils.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
extern GarbageCollector GC;



//#define DEBUG_MEMORY

#ifdef DEBUG_MEMORY
static int allocation_count = 0;
static int deallocation_count = 0;

void* operator new(size_t size) {
    allocation_count++;
    return malloc(size);
}


void* operator new[](size_t size) {
    allocation_count++;
    return malloc(size);
}

void operator delete(void* ptr) {
    (void)ptr;
    deallocation_count++;
    free(ptr);
}

void operator delete[](void* ptr) {
    (void)ptr;
    deallocation_count++;
    free(ptr);
}

void operator delete(void*, long unsigned int) {}
void operator delete[](void*, long unsigned int) {}

void checkMemoryLeaks() 
{
    printf("=== MEMORY LEAK CHECK ===\n");
    printf("Allocations: %d\n", allocation_count);
    printf("Deallocations: %d\n", deallocation_count);
    if (allocation_count != deallocation_count) 
    {
        printf("MEMORY LEAK DETECTED!\n");
    } else 
    {
        printf("No memory leaks detected.\n");
    }

   
}
#endif


static Value clockNative(int argCount, Value* args) 
{
  return NUMBER((double)clock() / CLOCKS_PER_SEC);
}

static Value write_Native(int argCount, Value* args) 
{
  for (int i = 0; i < argCount; i++) 
  {
    PRINT_VALUE(args[i]);
  }

  return NIL();
}
static Value writeln_Native(int argCount, Value* args) 
{
  for (int i = 0; i < argCount; i++) 
  {
    PRINT_VALUE(args[i]);
  }
  printf("\n");
  return NIL();
}


int main()
{

 

   Interpreter vm;

   vm.defineNative("clock", clockNative);
   vm.defineNative("write", write_Native  );
   vm.defineNative("writeln", writeln_Native);

   if (vm.compile_file("main.bu"))
   {



        vm.disassemble();

      //  Process* main = vm.find_process("_main_");
      //   main->run();
       
        vm.run();
   }
 
  vm.clear();

     INFO("Objects before collection: %d", GC.countObjects());
     GC.collect();
     INFO("Objects after collection: %d", GC.countObjects());


#ifdef DEBUG_MEMORY
    checkMemoryLeaks();
    #endif
   
    return 0;
}