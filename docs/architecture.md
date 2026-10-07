# TJVM Architecture

TJVM is a JVM implementation built around a native C++ runtime
with Java-based runtime components and developer tooling.

The goal is not to reproduce a specific JVM implementation such
as HotSpot internally. Instead, TJVM aims to implement the JVM
execution model while making deliberate engineering choices around
memory management, garbage collection, execution, and compilation.

## Project Goals

TJVM is intended to become a substantial JVM implementation capable
of executing real Java bytecode produced by `javac`.

The long-term goals include:

- Class loading and linking
- JVM bytecode interpretation
- Object and array management
- Java exceptions
- Multithreading and synchronization
- Garbage collection
- Native runtime integration
- Runtime profiling
- JIT compilation
- Debugging and diagnostic tooling
- Compatibility and performance testing

## Architecture Overview

```text
                         TJVM
                          |
          +---------------+---------------+
          |               |               |
          v               v               v
   CLASS SYSTEM       EXECUTION        MEMORY
          |               |               |
     Class Loader      Interpreter       Heap
     Class Parser      Frames            Objects
     Linker            Operand Stack     Arrays
     Verifier          Bytecode          GC
     Constant Pool     Dispatch           |
          |               |               |
          +---------------+---------------+
                          |
                          v
                   NATIVE RUNTIME
                          |
              +-----------+-----------+
              |           |           |
              v           v           v
           Threads      OS I/O     Native API
              |
              v
             JIT
              |
      +-------+-------+
      |               |
      v               v
   Profiling       Machine Code
```

## Current Milestone

The first milestone is a binary Java class-file reader.

Before TJVM can execute Java bytecode, it needs to understand the
binary format produced by `javac`.

The first subsystem therefore focuses on reading and validating
the class-file structure.

Current data flow:

```text
.class file
    |
    v
ClassFileReader
    |
    v
ClassFileHeader
```

At this stage TJVM understands:

- The class-file magic number
- Minor version
- Major version
- Constant-pool count

Later milestones will extend this into a complete class-file
representation.

## Why C++?

The core VM requires direct control over several low-level systems,
including:

- Memory allocation
- Object representation
- Execution frames
- Garbage collection
- Thread management
- Native integration
- Machine-code generation

C++ provides low-level control while still giving us abstractions
such as classes, RAII, containers, and standard concurrency
facilities.

Java will be used for parts of the runtime and developer tooling
where running code in the JVM environment is useful.

## Core Components

### Class File System

Responsible for understanding Java `.class` files.

```text
classfile/
├── ClassFileReader
├── ClassFile
├── ConstantPool
├── FieldInfo
├── MethodInfo
├── AttributeInfo
└── CodeAttribute
```

The class-file subsystem should know how to decode binary class-file
data, but it should not be responsible for executing bytecode.

### Class Loader

Responsible for locating and loading classes.

```text
classpath
    |
    v
ClassLoader
    |
    v
.class bytes
    |
    v
ClassFileReader
    |
    v
TJVMClass
```

Future responsibilities include:

- Class-path lookup
- Class dependency loading
- Array classes
- Primitive classes
- Class initialization
- Bootstrap classes

### Linker

Responsible for preparing loaded classes for execution.

The intended pipeline is:

```text
Load
  |
  v
Verify
  |
  v
Prepare
  |
  v
Resolve
  |
  v
Initialize
```

Each stage will have a separate responsibility so that class loading
logic does not become coupled to runtime execution.

### Runtime

The runtime models the JVM execution environment.

Conceptually:

```text
Runtime
|
+-- Thread Manager
|
+-- Frames
|
+-- Heap
|
+-- Objects
|
+-- Arrays
|
+-- Monitors
|
+-- Native Method Registry
```

### Interpreter

The interpreter executes JVM bytecode instructions.

```text
bytecode
    |
    v
decode
    |
    v
instruction
    |
    v
runtime state
```

Execution is based around JVM stack frames containing:

```text
Frame
├── Local Variables
├── Operand Stack
├── Program Counter
└── Current Method
```

The interpreter will initially provide the baseline execution engine
used before JIT compilation.

### Memory System

TJVM will maintain its own runtime representation of objects and
arrays.

Conceptually:

```text
Heap
|
+-- Objects
|
+-- Arrays
|
+-- Metadata
└-- Free Memory
```

The memory subsystem will be designed independently from the
interpreter so that garbage collection and allocation strategies
can evolve without rewriting bytecode execution.

### Garbage Collector

TJVM will initially implement a simple tracing collector and later
evolve toward a generational design.

Planned progression:

```text
Mark / Sweep
     |
     v
Generational Collection
     |
     v
GC Profiling / Optimization
```

The collector will expose metrics such as:

- Allocation rate
- Heap usage
- Objects reclaimed
- Collection count
- Collection pause duration
- Promotion rate

### Threading and Synchronization

TJVM will eventually support JVM concurrency primitives including:

- Java threads
- Thread creation
- Thread joining
- Monitors
- `synchronized`
- `wait`
- `notify`
- `notifyAll`

The runtime will maintain JVM-level thread and monitor state while
using native operating-system primitives underneath where necessary.

### Native Runtime

Some Java methods require functionality provided by the VM or the
host operating system.

TJVM will therefore provide a native method registry and bridge.

Conceptually:

```text
Java method
    |
    v
Native method lookup
    |
    v
TJVM runtime
    |
    v
C++ implementation
```

The initial native bridge will be internal to TJVM. Compatibility with
standard JNI interfaces can be added later.

### JIT Compiler

The JIT compiler will allow frequently executed bytecode to be
compiled into native machine code.

Planned pipeline:

```text
Bytecode
    |
    v
Interpreter
    |
    v
Runtime Profiling
    |
    v
Hot Method Detection
    |
    v
Intermediate Representation
    |
    v
Optimization
    |
    v
Native Code Generation
```

Initial optimization work may include:

- Constant folding
- Dead-code elimination
- Basic-block optimization
- Method inlining
- Strength reduction

The first native backend will target x86-64.

Additional architectures may be added later.

## Tooling

TJVM should eventually provide its own command-line tooling.

Examples:

```text
tjvm run <class>
tjvm run <jar>
tjvm inspect <class>
tjvm dump-method <class> <method>
tjvm threads
tjvm heap
tjvm gc
tjvm debug <class>
```

The tooling should be able to expose information from the runtime
without coupling diagnostic code tightly to the execution engine.

## Testing Strategy

Correctness is a core requirement of the project.

TJVM will use multiple levels of testing.

### Unit Tests

Used for isolated components such as:

- Binary readers
- Constant-pool parsing
- Descriptor parsing
- Bytecode decoding
- Heap allocation
- Garbage collection algorithms

### Integration Tests

Used to verify complete runtime behavior.

Examples:

```text
class loading
method invocation
object creation
exceptions
arrays
inheritance
threads
synchronization
```

### Compatibility Tests

The same Java program will be executed using a reference JVM and
TJVM, with observable behavior compared between the two.

```text
Java Test Program
        |
        +--------> Reference JVM
        |              |
        |              v
        |            Output
        |
        +--------> TJVM
                       |
                       v
                     Output

                 Compare Results
```

This allows TJVM to track exactly which JVM behaviors it currently
supports.

### Benchmark Tests

Performance benchmarks will compare:

```text
Reference JVM
TJVM Interpreter
TJVM JIT
```

Benchmarks will cover areas such as:

- Arithmetic
- Method calls
- Object allocation
- Arrays
- Recursion
- Threads
- Garbage collection
- JIT workloads

## Design Principles

### Separation of Responsibilities

Each subsystem should have one clear responsibility.

For example:

`ClassFileReader` reads binary data.

It should not also perform class loading, linking, or bytecode
execution.

### Specification First

When behavior is defined by the JVM specification, the specification
is the primary reference.

Implementation details such as heap layout, garbage-collection
algorithms, and JIT architecture are TJVM design decisions unless
the specification requires otherwise.

### Measurable Engineering

Features should be accompanied by tests and, where appropriate,
benchmarks.

Performance claims should be backed by measurements rather than
assumptions.

### Incremental Compatibility

TJVM will explicitly document which parts of the JVM are supported.

Unsupported behavior should be reported clearly rather than silently
producing incorrect results.

### Understandable Internals

TJVM is intended to be readable by engineers who want to understand
how a JVM works.

Important design decisions will therefore be documented alongside
the implementation.

## Long-Term Architecture

The intended evolution of TJVM is:

```text
TJVM 0.1
  |
  +-- Class File Reader
  +-- Constant Pool
  +-- Class Representation
  |
  v
TJVM 0.2
  |
  +-- Class Loading
  +-- Linking
  +-- Interpreter
  +-- Frames
  |
  v
TJVM 0.3
  |
  +-- Objects
  +-- Arrays
  +-- Inheritance
  +-- Exceptions
  |
  v
TJVM 0.4
  |
  +-- Java Runtime Layer
  +-- Native Bridge
  +-- I/O
  |
  v
TJVM 0.5
  |
  +-- Threads
  +-- Monitors
  +-- Synchronization
  |
  v
TJVM 0.6
  |
  +-- Mark / Sweep GC
  |
  v
TJVM 0.7
  |
  +-- Generational GC
  +-- GC Telemetry
  |
  v
TJVM 0.8
  |
  +-- Runtime Profiler
  +-- Hot Method Detection
  |
  v
TJVM 0.9
  |
  +-- x86-64 Baseline JIT
  |
  v
TJVM 1.0
  |
  +-- JIT Optimizations
  +-- Compatibility Suite
  +-- Benchmarks
  +-- Debugger
  +-- Production-quality Documentation
```

TJVM is intentionally being developed as a long-term systems project.
Each milestone should leave the runtime in a working state and provide
a foundation for the next subsystem.