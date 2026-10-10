# TJVM Notes

> These notes are written alongside the implementation so the project can also become the basis for a technical blog series.

---

# 1. What Are We Building?

TJVM is intended to become a serious JVM implementation rather than a small bytecode demo.

The long-term goal is a **hybrid C++/Java runtime** with:

- Class-file loading and parsing
- Class linking and initialization
- JVM bytecode interpretation
- JVM runtime structures
- Objects and arrays
- Memory management
- Garbage collection
- Threading and synchronization
- Native runtime integration
- Runtime profiling
- JIT compilation
- Debugging and diagnostics
- Compatibility tests
- Performance benchmarks

The important goal is not to recreate HotSpot line-for-line.

Instead, TJVM should implement the JVM execution model while making explicit engineering choices about the internals.

A good mental model is:

```text
                 Java Program
                      |
                      v
                  javac
                      |
                      v
                 .class file
                      |
                      v
                     TJVM
        +-------------+-------------+
        |             |             |
   Class System   Execution     Memory System
        |             |             |
    ClassLoader    Interpreter      Heap
    Linker         Frames           Objects
    Verifier       Bytecode         Arrays
    Constant Pool  JIT               GC
        |             |             |
        +-------------+-------------+
                      |
                      v
                Native Runtime
                      |
              +-------+-------+
              |       |       |
           Threads   OS I/O  Native API
```

---

# 2. Why Build a JVM?

Java source code does not execute directly on the CPU.

The normal Java pipeline is:

```text
Hello.java
    |
    | javac
    v
Hello.class
    |
    v
JVM
    |
    v
CPU / OS
```

The `.class` file contains JVM bytecode and class metadata.

The JVM is the program responsible for understanding that bytecode and providing the runtime needed by Java programs.

By implementing TJVM, we are effectively implementing the machinery that normally sits between Java bytecode and the underlying machine.

This makes the project a systems/runtime project rather than a normal application.

---

# 3. Why C++ + Java?

A JVM does not have to be written in Java.

The JVM is an implementation of a virtual machine specification. The implementation language is a design choice.

TJVM uses a hybrid design:

```text
C++
 |
 +-- Class loading internals
 +-- Runtime representation
 +-- Interpreter
 +-- Heap / object management
 +-- Garbage collector
 +-- Threads / monitors
 +-- Native integration
 +-- JIT / machine-code generation

Java
 |
 +-- Runtime classes
 +-- Higher-level runtime services
 +-- Developer tooling
 +-- Diagnostics
```

C++ is used for the VM core because we want direct control over:

- memory
- object layout
- execution frames
- native threads
- synchronization
- machine-code generation

Java can then be used where a higher-level runtime/tooling layer is useful.

## Bootstrapping idea

The host machine already has a real JVM such as HotSpot installed.

That host JVM can run TJVM's Java-side components during development.

Eventually the important distinction is:

```text
Host JVM
    |
    | runs TJVM tooling/runtime components
    v
TJVM
    |
    | executes
    v
Java bytecode
```

TJVM itself does not use HotSpot to execute the target Java program's bytecode once the native runtime is responsible for execution.

---

# 4. The JVM Lifecycle

A useful high-level model is:

```text
JVM startup
    |
    v
Initialization
    |
    v
Class Loading
    |
    v
Class Linking
    |
    +--> Verification
    +--> Preparation
    +--> Resolution
    |
    v
Class Initialization
    |
    v
Execution
    |
    +--> Interpreter
    +--> JIT
    |
    v
Runtime / Memory / Threads / GC
```

Important distinction:

Some details seen in a specific JVM implementation are implementation-specific.

For example:

- which garbage collector is selected
- how memory is physically allocated
- whether Class Data Sharing (CDS) is used
- exactly how startup arguments are processed

These are not universal requirements imposed by the JVM specification.

For TJVM, we should implement the required JVM semantics and choose our own internal designs.

---

# 5. JVM Initialization

Conceptually, TJVM startup will eventually look like:

```text
TJVM starts
    |
    +-- Parse command-line arguments
    +-- Build classpath
    +-- Detect/configure runtime resources
    +-- Create runtime state
    +-- Initialize heap
    +-- Initialize method/class metadata storage
    +-- Initialize class loader
    +-- Initialize GC
    +-- Start execution
```

The exact HotSpot startup process should not be copied blindly.

TJVM is its own implementation.

---

# 6. Class Loading

Class loading is the process of getting a class's binary representation into the JVM runtime.

Conceptually:

```text
classpath
    |
    v
ClassLoader
    |
    v
.class file
    |
    v
raw bytes
    |
    v
ClassFileParser
    |
    v
TJVMClass
```

The three broad ideas are:

1. Locate the binary representation of a class or interface.
2. Derive the class/interface representation from those bytes.
3. Make that information available to the runtime.

Eventually TJVM's class loader will need to deal with:

- class paths
- class dependencies
- arrays
- primitive types
- class initialization
- bootstrap/runtime classes

---

# 7. Class Linking

After loading, the class needs to be linked.

The major stages are:

```text
             Linking
                |
       +--------+--------+
       |        |        |
       v        v        v
  Verification Preparation Resolution
```

## 7.1 Verification

Verification checks whether the class-file contents are structurally valid and suitable for execution.

Examples of things a verifier can care about:

- valid class-file structure
- valid bytecode instructions
- type correctness
- valid references
- valid stack usage

A full verifier is a substantial subsystem and will be added later.

## 7.2 Preparation

Preparation creates/initializes the storage needed for static fields and gives them their default values.

Example:

```java
static int x;
static Object obj;
```

After preparation, conceptually:

```text
x   -> 0
obj -> null
```

This is different from executing explicit initialization code.

For example:

```java
static int x = 42;
```

does not simply mean the value is assigned during generic preparation.

Explicit class initialization happens later.

## 7.3 Resolution

Resolution deals with symbolic references used by class files.

This becomes especially important when we reach the constant pool.

A bytecode instruction may refer to a constant-pool index rather than directly containing the final runtime target.

Conceptually:

```text
bytecode
   |
   | #17
   v
constant pool
   |
   v
symbolic method/field/class reference
   |
   v
resolved runtime entity
```

---

# 8. Class Initialization

Class initialization is where explicit static initialization takes place.

Consider:

```java
class Test {
    static int x = 42;

    static {
        System.out.println("Hello");
    }
}
```

The class compiler can produce a special method:

```text
<clinit>()
```

Conceptually:

```text
Test
 |
 v
<clinit>()
 |
 +-- x = 42
 |
 +-- println("Hello")
```

TJVM will eventually need to support class initialization semantics and execution of `<clinit>`.

This becomes much more important once the bytecode interpreter exists.

---

# 9. What Is a `.class` File?

When `javac` compiles Java source:

```java
public class Hello {
    public static void main(String[] args) {
        System.out.println("Hello");
    }
}
```

the result is:

```text
Hello.class
```

A class file is a **binary format** defined by the JVM specification.

It is not human-readable source code.

A simplified layout is:

```text
.class file
+---------------------------+
| Header                    |
+---------------------------+
| Constant Pool             |
+---------------------------+
| Access Flags              |
+---------------------------+
| This Class                |
+---------------------------+
| Super Class               |
+---------------------------+
| Interfaces                |
+---------------------------+
| Fields                    |
+---------------------------+
| Methods                   |
+---------------------------+
| Attributes                |
+---------------------------+
```

The exact class-file structure is specified by the JVMS.

TJVM's first job is therefore to understand the binary format.

---

# 10. Why Parse the Class File Ourselves?

Normally the JVM implementation reads the `.class` file for us.

TJVM cannot do that if we want to own the runtime.

So the process becomes:

```text
Hello.class
    |
    v
C++ ClassFileReader
    |
    v
C++ ClassFileParser
    |
    v
TJVM internal structures
```

This is the beginning of the class-loading subsystem.

---

# 11. Reader vs Parser

This distinction is important.

## ClassFileReader

Responsible for:

> "How do I get bytes/numbers out of the binary file?"

It handles operations such as:

```text
read_u1()
read_u2()
read_u4()
read_u8()
read_bytes()
```

It should not care what those numbers mean.

## ClassFileParser

Responsible for:

> "What does this sequence of bytes mean according to the JVM class-file format?"

For example:

```text
0xCAFEBABE
    |
    v
valid class-file magic

0x0045
    |
    v
major version 69
```

So:

```text
Reader
bytes --> numeric values

Parser
numeric values --> JVM concepts
```

This separation keeps the design clean.

---

# 12. Class-File Header

The first fields of a class file are currently represented as:

```cpp
struct ClassFileHeader {
    std::uint32_t magic;
    std::uint16_t minor_version;
    std::uint16_t major_version;
    std::uint16_t constant_pool_count;
};
```

The corresponding binary layout is:

```text
magic                    u4
minor_version            u2
major_version            u2
constant_pool_count      u2
```

Where:

```text
u1 = 1 byte
u2 = 2 bytes
u4 = 4 bytes
u8 = 8 bytes
```

---

# 13. `read_u1()`

`read_u1()` reads exactly one byte.

Conceptually, if the file contains:

```text
CA FE BA BE 00 00 ...
^^
```

the first call to `read_u1()` returns:

```text
CA
```

The next call returns:

```text
FE
```

and so on.

It is the most basic operation in our binary reader.

The underlying code is essentially:

```cpp
std::uint8_t ClassFileReader::read_u1() {
    char byte{};

    if (!file_.read(&byte, sizeof(byte))) {
        throw std::runtime_error(
            "Unexpected end of class file"
        );
    }

    return static_cast<std::uint8_t>(
        static_cast<unsigned char>(byte)
    );
}
```

The important part is:

```cpp
file_.read(...)
```

which consumes data from the binary stream.

---

# 14. `read_u2()`

The JVM class-file format stores multi-byte values in **big-endian order**.

Suppose the file contains:

```text
12 34
```

These two bytes represent:

```text
0x1234
```

Our implementation:

```cpp
std::uint16_t ClassFileReader::read_u2() {
    const auto high = read_u1();
    const auto low = read_u1();

    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(high) << 8) |
        low
    );
}
```

Step by step:

```text
high = 0x12
low  = 0x34
```

Shift the high byte:

```text
0x12 << 8
= 0x1200
```

Combine with the low byte:

```text
0x1200
   |
  OR
0x0034
   |
   v
0x1234
```

So `read_u2()` reconstructs a 16-bit big-endian value.

---

# 15. `read_u4()`

`read_u4()` follows the exact same idea for four bytes.

Suppose the file contains:

```text
CA FE BA BE
```

We read:

```text
b1 = CA
b2 = FE
b3 = BA
b4 = BE
```

Then construct:

```text
CA << 24
FE << 16
BA << 8
BE
```

which results in:

```text
0xCAFEBABE
```

The implementation is:

```cpp
std::uint32_t ClassFileReader::read_u4() {
    const auto b1 = read_u1();
    const auto b2 = read_u1();
    const auto b3 = read_u1();
    const auto b4 = read_u1();

    return (static_cast<std::uint32_t>(b1) << 24) |
           (static_cast<std::uint32_t>(b2) << 16) |
           (static_cast<std::uint32_t>(b3) << 8) |
           static_cast<std::uint32_t>(b4);
}
```

---

# 16. Why Big-Endian Matters

The class-file format defines the byte order.

We cannot assume the host CPU's preferred byte order.

For example, if the bytes are:

```text
12 34
```

TJVM must interpret them as:

```text
0x1234
```

not:

```text
0x3412
```

This is why the reader manually reconstructs multi-byte values.

The reader is effectively saying:

> "I will interpret the file according to the JVM class-file specification, regardless of the host machine's native byte order."

---

# 17. The Class-File Magic Number

Every valid Java class file begins with:

```text
CA FE BA BE
```

This becomes:

```text
0xCAFEBABE
```

This is the class-file **magic number**.

TJVM checks it using:

```cpp
if (header.magic != 0xCAFEBABE) {
    throw std::runtime_error(
        "Invalid class file magic"
    );
}
```

Conceptually:

```text
Input file
    |
    v
Read first 4 bytes
    |
    v
CAFEBABE?
  /     \
yes      no
 |        |
continue  reject
```

This is essentially a binary file signature check.

---

# 18. Reading the Header

The reader currently does:

```cpp
ClassFileHeader ClassFileReader::read_header() {
    ClassFileHeader header{};

    header.magic = read_u4();
    header.minor_version = read_u2();
    header.major_version = read_u2();
    header.constant_pool_count = read_u2();

    return header;
}
```

If the file begins conceptually with:

```text
CA FE BA BE
00 00
00 45
00 1D
```

then TJVM gets:

```text
magic               = 0xCAFEBABE
minor_version       = 0
major_version       = 69
constant_pool_count = 29
```

The parser has now consumed the beginning of the binary structure.

---

# 19. Why the Parser Exists Separately

The `ClassFileReader` only understands byte sizes.

For example:

```text
read_u4()
```

means:

> Read four bytes.

It does not know whether those four bytes represent:

- a magic number
- an integer
- part of a method structure
- an attribute
- something else

The parser adds the meaning:

```text
read_u4()
    |
    v
magic field
```

This gives us:

```text
                 .class file
                      |
                      v
              ClassFileReader
                      |
                raw values
                      |
                      v
              ClassFileParser
                      |
              JVM structures
```

---

# 20. Current TJVM Code Structure

At this point the repository is intended to look like:

```text
tjvm/
├── CMakeLists.txt
├── .gitignore
│
├── include/
│   └── tjvm/
│       ├── class_file.hpp
│       └── class_file_reader.hpp
│
├── src/
│   ├── main.cpp
│   └── class_file_reader.cpp
│
├── docs/
│   └── architecture.md
│
└── notes.md
```

The next files will be added as the class-file subsystem grows.

---

# 21. Current Data Flow

Right now the implementation conceptually does:

```text
Hello.java
    |
    | javac
    v
Hello.class
    |
    v
ClassFileReader
    |
    +--> magic
    +--> minor version
    +--> major version
    +--> constant pool count
    |
    v
ClassFileHeader
```

The next stage will extend the flow into:

```text
.class
   |
   v
Header
   |
   v
Constant Pool
```

The constant pool is the next concept to study.

---

# 22. Current Status

### Completed

- TJVM project direction
- Hybrid C++/Java architecture
- Long-term JVM roadmap
- JVM lifecycle overview
- Class loading overview
- Class linking overview
- Class initialization overview
- `.class` file fundamentals
- Binary reader design
- Big-endian handling
- Class-file magic validation
- Class-file header parsing

### Current milestone

**Class-file parsing**

```text
Header
  |
  v
Constant Pool  <-- NEXT
  |
  v
Fields
  |
  v
Methods
  |
  v
Attributes
```

---

# 23. Learning Principle

Do not treat TJVM as:

> "A bunch of code I copied to make a JVM."

Treat it as:

> "A sequence of implementations that turn a binary class file into an executing Java program."

Each subsystem should answer three questions:

1. What does the JVM specification define?
2. What implementation choice is TJVM making?
3. How will we test that our implementation is correct?

This keeps the project both technically serious and understandable.

---

# 24. Blog Material

Possible first blog chapter:

> **Building TJVM: Why I Decided to Build a JVM**

Possible section order:

```text
Why build a JVM?
        |
        v
What actually happens when javac runs?
        |
        v
What is a .class file?
        |
        v
Why does the JVM need a class-file format?
        |
        v
How TJVM reads raw binary data
        |
        v
Reading the class-file header
```

A good recurring style for the blog is:

```text
Problem
   |
Why the JVM needs it
   |
TJVM design
   |
Implementation
   |
Testing
   |
What comes next
```

---

# 25. Important Corrections to Remember

Some JVM notes found online describe **specific HotSpot implementation behavior** rather than requirements of the JVM specification.

Examples include:

- automatic garbage collector selection
- exact heap organization
- CDS implementation
- method-area implementation details
- specific startup resource heuristics

For TJVM, always distinguish:

```text
JVM Specification
        vs
HotSpot / OpenJDK implementation detail
```

That distinction will matter a lot as TJVM becomes more advanced.

---

# Next Topic

## The Constant Pool

Before writing more code, understand:

- what the constant pool is
- why class files need it
- why entries refer to each other
- one concrete example such as:

```text
Methodref
   |
   +--> Class
   |      |
   |      +--> Utf8 "java/lang/Object"
   |
   +--> NameAndType
          |
          +--> Utf8 "<init>"
          |
          +--> Utf8 "()V"
```

Only after that should we implement the constant-pool parser.
