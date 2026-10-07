## tjvm

readme is basically notes for me until i complete ts!

````
Hello.class
│
├── Magic number
├── Version
├── Constant Pool
├── Access flags
├── This class
├── Super class
├── Interfaces
├── Fields
├── Methods
└── Attributes


 CA FE BA BE 00 00 00 46 ...
  └── magic numbers (the first 4 bytes)
````

magic numbers → 'this is a java class file'

magic          → u4
minor_version  → u2
major_version  → u2

# ROADMAP (professional!)

                         TJVM
                          │
        ┌─────────────────┼──────────────────┐
        │                 │                  │
    CLASS SYSTEM        EXECUTION          MEMORY
        │                 │                  │
    Class Loader       Interpreter          Heap
    Parser             Frames               Object Model
    Linker             Operand Stack        Allocator
    Verifier           Bytecode             GC
    Constant Pool      Dispatch             Generations
        │                 │                  │
        └─────────────────┼──────────────────┘
                          │
                      NATIVE RUNTIME
                          │
              ┌───────────┼───────────┐
              │           │           │
            Threads      I/O/OS       Native
            Monitors     Signals      Methods
              │           │           │
              └───────────┼───────────┘
                          │
                         JIT
                          │
                   ┌──────┴──────┐
                   │             │
                   IR        x86-64 Code

# Initialization of JVM

1) Input Validation :  JVM arguments, artifact to be executed, and classpath.
2) Detecting System Resources : processors, system memory, and system services
3) Preparing the environment
4) Choosing Garbage Collector : (<1792) ? Serial GC : GC1
5) CDS Archives : Cached Data Storage of class files
6) Method Area : special off-heap memory location where class data will be stored 

# Class Loading

a three-step process of: 

1) the JVM locating the binary representation of a class or interface 
2) deriving the class or interface from it 
3)  loading that information into the JVM method area

# Class Linking

1) Verification : ensuring class or structure is structurally correct
2) Preparation : initialization of static fields in a class to their default values.
3) Resolution : resolve the symbolic references in the Constant Pool of a class

# Class Initialization

involves assigning a ConstantValue to static fields and executing any static
initializers in a class if present

````
Component                    Language	                    Reason
Class-file parser               C++	            Binary parsing and class metadata
Bytecode interpreter   	        C++         	Direct control over VM execution
Operand stacks / frames	        C++	            Runtime memory representation
Heap allocator	                C++	            Explicit memory management
Garbage collector               C++	            Object reachability and reclamation
JIT compiler	                C++	            Native code generation
Threading / monitors	        C++	            OS integration and synchronization
Core class libraries	     Java + C++	        Java APIs with native implementations where necessary
Debugger / profiler UI	        Java	        Easier tooling and visualization
VM management APIs	     Java + C++	        Diagnostics backed by native runtime data
````

# Expected Milestones

```
TJVM 0.1
  ↓
Class-file parser
Constant pool
Class loader
Runtime representation

TJVM 0.2
  ↓
Interpreter
Frames
Operand stack
Methods
Control flow

TJVM 0.3
  ↓
Objects
Arrays
Inheritance
Exceptions

TJVM 0.4
  ↓
Java runtime layer
Native bridge
I/O
Strings

TJVM 0.5
  ↓
Threads
Monitors
Synchronization

TJVM 0.6
  ↓
Mark-sweep GC

TJVM 0.7
  ↓
Generational GC
GC telemetry

TJVM 0.8
  ↓
Profiler
Hot method detection

TJVM 0.9
  ↓
x86-64 baseline JIT

TJVM 1.0
  ↓
Optimization
Compatibility suite
Benchmarks
Debugger
Documentation
```