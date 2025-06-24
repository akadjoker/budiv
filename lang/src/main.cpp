//#include "TestString.hpp"
//#include "TestChunk.hpp"
//#include "TestStack.hpp"
//#include "TestVector.hpp"
//#include "TesteQueue.hpp"
//#include "TestRai.hpp"
//#include "TesteMap.hpp"

#include <cmath>
#include <cstdlib>
#include <raylib.h>

#include "Config.hpp"
#include "VM.hpp"
#include "Utils.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
extern GarbageCollector GC;


Texture2D dummy;


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


static Value key_down_Native(int argCount, Value* args) 
{
  bool isDown = IsKeyDown(AS_INTEGER(args[0]));
  return BOOLEAN(isDown);
}

static Value key_pressed_Native(int argCount, Value* args) 
{
  bool isPressed = IsKeyPressed(AS_INTEGER(args[0]));
  return BOOLEAN(isPressed);
}

static Value key_released_Native(int argCount, Value* args) 
{
  bool isReleased = IsKeyReleased(AS_INTEGER(args[0]));
  return BOOLEAN(isReleased);
}


static Value key_up_Native(int argCount, Value* args) 
{
  bool isUp = IsKeyUp(AS_INTEGER(args[0]));
  return BOOLEAN(isUp);
}

static Value mouse_x_Native(int argCount, Value* args)
{
  return NUMBER(GetMouseX());
}

static Value mouse_y_Native(int argCount, Value* args)
{
  return NUMBER(GetMouseY());
}

static Value mouse_down_Native(int argCount, Value* args) 
{
  bool isDown = IsMouseButtonDown(AS_INTEGER(args[0]));
  return BOOLEAN(isDown);
}

static Value mouse_pressed_Native(int argCount, Value* args) 
{
  bool isPressed = IsMouseButtonPressed(AS_INTEGER(args[0]));
  return BOOLEAN(isPressed);
}

static Value mouse_released_Native(int argCount, Value* args) 
{
  bool isReleased = IsMouseButtonReleased(AS_INTEGER(args[0]));
  return BOOLEAN(isReleased);
}


static Value mouse_up_Native(int argCount, Value* args) 
{
  bool isUp = IsMouseButtonUp(AS_INTEGER(args[0]));
  return BOOLEAN(isUp);
}


static Color use_color = WHITE;
static Value set_color_Native(int argCount, Value* args) 
{
  use_color.r = AS_INTEGER(args[0]);
  use_color.g = AS_INTEGER(args[1]);
  use_color.b = AS_INTEGER(args[2]);
  use_color.a = AS_INTEGER(args[3]);
  return NIL();
}

static Value darw_circle_Native(int argCount, Value* args) 
{
  DrawCircle(AS_INTEGER(args[0]), AS_INTEGER(args[1]), AS_INTEGER(args[2]), use_color);
  return NIL();
}

static Value darw_rectangle_Native(int argCount, Value* args) 
{
  DrawRectangle(AS_INTEGER(args[0]), AS_INTEGER(args[1]), AS_INTEGER(args[2]), AS_INTEGER(args[3]), use_color);
  return NIL();
}

static Value darw_line_Native(int argCount, Value* args) 
{
  DrawLine(AS_INTEGER(args[0]), AS_INTEGER(args[1]), AS_INTEGER(args[2]), AS_INTEGER(args[3]), use_color);
  return NIL();
}

static Value darw_text_Native(int argCount, Value* args) 
{

  ObjString* str = AS_STRING(args[0]);
  DrawText(str->data, AS_INTEGER(args[1]), AS_INTEGER(args[2]), AS_INTEGER(args[3]), use_color);
  return NIL();
}

// Random number generator (returns [0,1))
Value rand_native(int argCount, Value* args) {
    return NUMBER(static_cast<double>(std::rand()) / RAND_MAX);
}

#include <cstdlib>

// Returns a double in [min, max]
Value random_native(int argCount, Value* args) 
{
    if (argCount != 2 || !IS_NUMBER(args[0]) || !IS_NUMBER(args[1]))
        return NIL();

    double min = AS_NUMBER(args[0]);
    double max = AS_NUMBER(args[1]);

    if (min > max) 
    {
        double temp = min;
        min = max;
        max = temp;
    }

    // Generate a double in [0, 1)
    double r = (double)std::rand() / ((double)RAND_MAX + 1.0);

    // Scale to [min, max]
    double result = min + r * (max - min);

    return NUMBER(result);
}


// Absolute value
Value abs_native(int argCount, Value* args) {
    if (argCount != 1 || !IS_NUMBER(args[0])) return NIL();
    return NUMBER(std::fabs(AS_NUMBER(args[0])));
}

// Sine function (radians)
Value sin_native(int argCount, Value* args) {
    if (argCount != 1 || !IS_NUMBER(args[0])) return NIL();
    return NUMBER(std::sin(AS_NUMBER(args[0])));
}

// Cosine function (radians)
Value cos_native(int argCount, Value* args) {
    if (argCount != 1 || !IS_NUMBER(args[0])) return NIL();
    return NUMBER(std::cos(AS_NUMBER(args[0])));
}

// Tangent function (radians)
Value tan_native(int argCount, Value* args) {
    if (argCount != 1 || !IS_NUMBER(args[0])) return NIL();
    return NUMBER(std::tan(AS_NUMBER(args[0])));
}






int main()
{

 

   Interpreter vm;

   vm.defineNative("clock", clockNative);
   vm.defineNative("write", write_Native  );
   vm.defineNative("writeln", writeln_Native);


   vm.defineNative("rand", rand_native);
   vm.defineNative("random", random_native);
   vm.defineNative("abs", abs_native);
   vm.defineNative("sin", sin_native);
   vm.defineNative("cos", cos_native);
   vm.defineNative("tan", tan_native);

   vm.defineNative("key_down", key_down_Native);
   vm.defineNative("key_pressed", key_pressed_Native);
   vm.defineNative("key_released", key_released_Native);
   vm.defineNative("key_up", key_up_Native);

   vm.defineNative("mouse_down", mouse_down_Native);
   vm.defineNative("mouse_pressed", mouse_pressed_Native);
   vm.defineNative("mouse_released", mouse_released_Native);
   vm.defineNative("mouse_up", mouse_up_Native);

   vm.defineNative("mouse_x", mouse_x_Native);
   vm.defineNative("mouse_y", mouse_y_Native);

   vm.defineNative("set_color", set_color_Native);
   vm.defineNative("draw_circle", darw_circle_Native);
   vm.defineNative("draw_rectangle", darw_rectangle_Native);
   vm.defineNative("draw_line", darw_line_Native);
   vm.defineNative("draw_text", darw_text_Native);


   const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "BuEngine");
  SetTargetFPS(60);
      dummy = LoadTexture("assets/wabbit_alpha.png");
 


  if (vm.compile_file("main.bu"))
  {


      //   vm.disassemble();

      //  Process* main = vm.find_process("_main_");
      //   main->run();
   
      vm.run();
   }

  //  if (!done)
  //  {
  //     vm.disassemble();
  //     vm.run();
  //     done = true;
  //  }

  //  while (!done && !WindowShouldClose())
  //  {
  //     BeginDrawing();
  //     ClearBackground(BLACK);
  

  //     DrawFPS(10, 10);
  //     EndDrawing();
       
  //  }




  vm.clear();

 CloseWindow();   


     INFO("Objects before collection: %d", GC.countObjects());
     GC.collect();
     INFO("Objects after collection: %d", GC.countObjects());


#ifdef DEBUG_MEMORY
    checkMemoryLeaks();
    #endif
   
    return 0;
}