# Ion

Ion is a low-level systems programming language inspired by C but designed to
be cleaner, more expressive, and easier to extend. The project currently
implements a lexer, token definitions, and a basic driver that reads a source
file and prints the token stream with indentation-aware formatting.

This repository serves as the starting point for the language front end,
providing the groundwork for parsing and later compilation.

---

## Project Goals

**Low-level control**
Ion preserves C-style direct control over memory, layout, and data types.

**Minimal but expressive syntax**
Ion borrows C’s simplicity but aims to reduce boilerplate, eliminate historical
quirks, and introduce concise constructs where appropriate.

**Predictable compilation pipeline**
The compiler is built in straightforward stages—lexing, parsing, AST
generation, analysis, and code generation—keeping the toolchain transparent and
debuggable.

**Ergonomic standard library**
Planned core utilities (strings, buffers, I/O, math) focus on memory clarity
and explicit behavior.

---

## Current Features

**Lexer**

* Tokenization of identifiers, numbers, strings, structural symbols, and punctuation.
* Correct row/column tracking via `Location`.
* Configurable token definitions via `token.def`.
* Dynamic token array storage using the custom allocator.

**Driver (`main.c`)**

* Loads files using arena allocation.
* Runs the lexer.
* Pretty-prints the token sequence with brace-aware indentation.

**StringView utilities**

* Zero-copy slicing.
* Prefix and predicate-based chopping.
* Used throughout the lexer for performance and clarity.

---

## Example Ion Program

```ion
import("std.string");

proc main() i32 {
    my_i32_variable: i32 = 100;
    my_f32_variable: f32 = 100.0;
    my_string_variable := string:from("Hello World");
    println(my_string_variable);

    {
        {
            println("Hellow");
            println("Hellow");
        }
    }

    return 0;
}
```

Run this file through the current front end to view the token stream.

---

## Running

```
./ionc path/to/source.ion
```

Output is a structured token list that reflects line breaks and scopes.

---

## Planned Work

### Parsing

* Pratt or recursive-descent parser
* AST representation
* Error recovery and diagnostic formatting

### Type system

* Basic primitives: `i32`, `u32`, `f32`, `bool`, `string`
* Type inference for simple cases
* User-defined structs

### Code generation

* Native backend

### Tooling

* Formatter
* LSP server
* Test runner

