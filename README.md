# HACK Assembler

A two-pass assembler for the HACK assembly language, part of the Nand to Tetris course.

## Overview

This assembler translates HACK assembly language (`.asm` files) into HACK machine code (`.hack` files). It implements a two-pass assembly process:

1. **Lexing Pass**: Tokenizes the assembly code, identifies labels, and builds a symbol table
2. **Parsing Pass**: Translates tokenized instructions into 16-bit binary machine code

## Building

```bash
make
```

This compiles the assembler from the following source files:

- `main.c` - Main program and assembly logic
- `token.c` - Token data structures and operations
- `symboltable.c` - Symbol table implementation

## Usage

```bash
./compiler <input.asm> <output.hack>
```

**Example:**

```bash
./compiler test_data/add/Add.asm output.hack
```

## Architecture

### Lexer

- Tokenizes assembly instructions
- Identifies and stores labels in the symbol table
- Outputs intermediate tokenized representation to `temp.lex`
- Handles comments and whitespace

### Parser

- Reads tokenized instructions
- Translates A-instructions and C-instructions to binary
- Manages variable symbol table (RAM allocation starts at address 16)
- Outputs 16-bit binary machine code

### Symbol Tables

- **Labels Table**: Maps label names to ROM addresses (instruction locations)
- **Variables Table**: Maps variable names to RAM addresses (starting at 16)

## Instruction Format

### A-Instructions

Format: `@value`

- `@17` - Literal integer
- `@LOOP` - Label reference
- `@counter` - Variable reference
- `@R0` - Predefined symbol

Binary output: `0vvvvvvvvvvvvvvv` (15-bit value)

### C-Instructions

Format: `dest=comp;jump`

- `D=M+1` - Computation with destination
- `0;JMP` - Unconditional jump
- `D;JGT` - Conditional jump

Binary output: `111accccccdddjjj`

## Test Data

The `test_data/` directory contains example assembly programs:

- `add/` - Simple addition program
- `max/` - Maximum of two numbers
- `rect/` - Rectangle drawing
- `pong/` - Pong game

Each test includes:

- `.asm` - Source assembly file
- `-test.lex` - Expected lexer output
- `-test.hack` - Expected machine code output

## Testing

Run the assembler on test files:

```bash
make run
```

Compare output with expected results:

```bash
diff output.hack test_data/add/add-test.hack
diff temp.lex test_data/add/add-test.lex
```

## Predefined Symbols

| Symbol | Value | Description           |
| ------ | ----- | --------------------- |
| R0-R15 | 0-15  | Virtual registers     |
| SP     | 0     | Stack pointer         |
| LCL    | 1     | Local segment base    |
| ARG    | 2     | Argument segment base |
| THIS   | 3     | This pointer          |
| THAT   | 4     | That pointer          |
| SCREEN | 16384 | Screen memory map     |
| KBD    | 24576 | Keyboard memory map   |

## Clean Build

```bash
make clean
```

Removes object files, executable, and temporary files.

## Project Structure

```
.
├── main.c              # Main assembler logic
├── token.c/h           # Token handling
├── symboltable.c/h     # Symbol table implementation
├── Makefile            # Build configuration
├── test_data/          # Test assembly programs
│   ├── add/
│   ├── max/
│   ├── pong/
│   └── rect/
└── README.md           # This file
```

## Course Information

Part of **COMSM1302 - Overview of Computer Architecture**  
MSc Computer Science  
University of Bristol
