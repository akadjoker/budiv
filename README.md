BuLang Div Style

**DIV/BennuGD-inspired Cooperative Process Virtual Machine**

---

### Overview

This project implements a lightweight virtual machine (VM) inspired by classic game engines like DIV Games Studio and BennuGD. The VM is designed to manage hundreds of independent processes (entities/sprites), each with its own execution context, while sharing common bytecode instructions. The architecture is based on the model described in *Crafting Interpreters*, adapted to support cooperative multitasking and entity-driven game logic.

---

### Features

- **Cooperative Multitasking:**
Each process (entity) runs independently, advancing a slice of its logic per frame. The VM scheduler executes all active processes in a round-robin fashion, simulating fibers or coroutines.
- **Shared Bytecode, Isolated Contexts:**
All processes share immutable bytecode (function pointers), but each maintains its own stack, instruction pointer, local variables, and execution state.
- **Efficient Process Management:**
Processes can be created and destroyed dynamically, allowing for scalable management of game objects and logic.
- **Extensible Interpreter:**
The VM is built to be extensible, supporting custom opcodes, priorities, and potential messaging between processes.

---

### How It Works

- **Process Creation:**
When a new process is spawned (e.g., a new enemy or effect), it receives a pointer to the shared function bytecode and initializes its own execution context.
- **Execution Loop:**
The main interpreter loop iterates through all live processes, executing a limited number of instructions per process per frame. This ensures fair CPU time distribution and smooth multitasking.
- **Context Isolation:**
Each process has its own stack, instruction pointer, and local variables, ensuring independent execution even when running the same code.

---

### Example Use Cases

- 2D retro-style games with many autonomous entities
- Game scripting engines requiring lightweight multitasking
- Educational tools for learning about interpreters and virtual machines

---

### Getting Started

1. **Clone the repository**
2. **Build the project** using your preferred C++ toolchain
3. **Run the VM** with sample bytecode or scripts

---

 

### Credits

- Inspired by DIV Games Studio, BennuGD, and the book *Crafting Interpreters* by Robert Nystrom.
- Luis Santos AKA DJOKER
---

### License

This project is open source under the MIT License.

 
