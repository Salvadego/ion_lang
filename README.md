# Íon

Íon is an experimental low-level systems programming language designed for
**kernels, runtimes, and bare-metal software**, while remaining practical for
servers, tools, and other performance-critical applications.

Íon aims to be:

A language that prioritizes explicitness, predictability, and direct control over
hardware, avoiding hidden behavior and complex safety machinery.

---

## Project Status

Íon is in an **early experimental stage**.

This repository currently contains a **compiler frontend prototype**, used to
validate syntax, performance characteristics, and architectural decisions.

### Implemented

* Source file loading
* Lexer
* Token definitions
* Precise source location tracking (file, line, column)
* Arena-based allocation
* Token stream inspection with indentation awareness

### Not Yet Implemented

* Parser
* AST representation
* Semantic analysis
* Type system
* Memory region enforcement
* Code generation
* Tooling (formatter, LSP, debugger support)

The language design described below represents the **intended direction**, not
fully implemented behavior.

---

## Repository Scope

This repository contains the **reference Íon compiler frontend**, written in C.

Design goals for the compiler itself:

* Fast compilation
* Explicit memory management
* Minimal dependencies
* No parser generators
* Easy to debug and extend
* Transparent compilation pipeline

The frontend is intentionally simple and avoids hidden allocations or
framework-heavy abstractions.

---

## Core Philosophy

Íon is not clever.
Íon is not magical.
Íon is honest.

### Hard Goals

* Zero mandatory runtime
* Zero mandatory standard library
* No hidden allocations
* No hidden control flow
* Fast compilation
* Easy to reason about generated machine code
* Assembly as a first-class citizen

---

## Design Direction

* Kernel-oriented
* Data-oriented
* Explicit over abstraction
* Orthogonal features
* Minimal syntax sugar
* No “safety by force”

Íon does not try to make invalid programs impossible.
It tries to make **dangerous intent explicit and local**.

---

## Memory Model (Design Goal)

### Explicit Address Spaces

Pointers are typed by **intent**, not just by pointee type:

```ion
ptr<T>            // generic pointer
ptr<T, phys>      // physical memory
ptr<T, mmio>      // memory-mapped IO
ptr<T, stack>     // stack memory
ptr<T, heap>      // heap memory
```

This model is intended to eliminate entire classes of kernel bugs **without**
a borrow checker.

Planned properties:

* Arrays do not decay into pointers
* Pointer arithmetic is explicit
* Address space violations are compile-time errors

---

## Regions, Scopes, and Aliasing (Design Goal)

Íon models memory using three orthogonal concepts:

| Concept      | Purpose                                    |
| ------------ | ------------------------------------------ |
| **Regions**  | Classify memory lifetime and address space |
| **Scopes**   | Restrict where pointers may escape         |
| **Aliasing** | Model who may invalidate memory            |

Conceptually:

```text
ptr<T, Region, Scope, Alias>
```

This system is designed to prevent:

* Stack dangling pointers
* Arena dangling pointers
* Thread escape bugs
* Most accidental use-after-free

Without introducing mandatory runtime overhead.

---

## No Mandatory Standard Library

Íon does **not** ship a traditional standard library.

Instead, the language defines a **small, frozen core vocabulary** so
independently written code can interoperate without adapters.

### Core Types (Planned)

* Integers: `i8 … i64`, `u8 … u64`, `usize`, `isize`
* `bool`, `void`
* `ptr`
* `slice`
* `option<T>`
* `result<T, E>`

There is **no built-in string type**.

Text is represented as:

* `[]u8` (slice with explicit length)
* `[N]u8` or `ptr<u8>` with `0` termination

---

## Slices (Design Goal)

Pointers are just addresses.
Length lives in a separate, explicit type.

```ion
slice<T> {
    data: ptr<T>
    len: usize
}
```

Slices are intended to:

* Never allocate
* Carry length explicitly
* Allow optional bounds checking
* Be removable at compile time

This reflects common kernel programming practice.

---

## Syntax Principles

* Simple, predictable control flow
* No implicit conversions
* No hidden temporaries
* No implicit allocations

### Method-style syntax (pure sugar)

```ion
func (c: ptr<Client>) Login(): i32 {
    return 0
}

client.Login() // passes `client` as the first parameter
```

This is syntax sugar only — no hidden dispatch or vtables.

---

## Assembly Integration (Design Goal)

Assembly is a first-class language feature.

```ion
asm {
    mov rax, cr3
    mov cr3, rbx
} clobbers { rax }
```

Planned properties:

* Structured assembly blocks
* Explicit clobbers
* No ABI guessing
* Support for pure assembly functions

---

## Symbol and ABI Control (Design Goal)

```ion
@symbol("_start")
func entry() {
    ...
}
```

No implicit name mangling.
All linkage and ABI details are explicit.

---

## Modules

Each file is a module.

A file may optionally **return a symbol table or structure**, Lua-style:

```ion
return {
    Client = Client,
    Login  = Login,
}
```

This avoids implicit globals and keeps linkage explicit.

---

## What Íon Is Not

* No garbage collection
* No borrow checker
* No hidden lifetimes
* No implicit compile-time execution
* No “smart” pointers

Everything that matters is visible in the source.

---

## Why “Íon”

Íons are:

* Fundamental
* Simple
* Predictable
* Directly interacting with physical systems

Íon the language follows the same idea:

**small primitives, explicit interactions, no surprises.**

---

