# SFort95 Interpreter

An interpreter for SFort95, a small subset of Fortran 95, written in C++ for NJIT's CS280 (Programming Language Concepts) in Spring 2024.

This is the third part of a three-part course project:

1. [Lexical Analyzer for SFort95](https://github.com/iaalcantara17/Lexical-Analyzer-for-SFort95-Language) turns source text into tokens.
2. [Parser for SFort95](https://github.com/iaalcantara17/Parser-for-SFort95-Language) checks the token stream against the grammar.
3. This repo extends the parser so it also runs the program as it parses it.

## What it does

The interpreter reads one SFort95 source file, parses it with a recursive-descent parser, and executes each statement as it's recognized. It keeps a symbol table for declared variables and their types, and a separate table for their current values.

The language it handles:

- A program wrapped in `PROGRAM name` ... `END PROGRAM name`
- Declarations with `INTEGER`, `REAL`, and `CHARACTER`, including `CHARACTER (LEN = n)` and initial values (`integer:: b = 2`)
- Assignment, `PRINT *, ...` with a comma-separated list, block `IF (...) THEN ... ELSE ... END IF`, and a one-line `IF (...) stmt`
- Arithmetic `+ - * /`, exponent `**`, string concatenation `//`, and comparisons `== < >`
- Case-insensitive keywords and `!` comments

`CHARACTER` variables are padded with spaces or cut to their declared length on assignment.

The interpreter prints the line number and an error message when it finds:

- a variable used without being declared, or declared twice
- a variable used before it has a value
- a non-string value assigned to a `CHARACTER` variable
- operand types that don't work with an operator or a comparison
- a sign (`+` or `-`) in front of a non-numeric value
- syntax errors, such as a missing `END IF`, `THEN` branch statement, or closing parenthesis

When the whole program runs it prints `Successful Execution`. Otherwise it prints `Unsuccessful Interpretation` and the error count.

## Example

From `PA 3 Test Cases (Updated)/test4`, with blank lines removed:

```fortran
PROGRAM circle
	!Testing Block-If Else-clauseand string assignment
	REAL :: r, a, p
	integer:: b = 2
	CHARACTER (LEN = 4):: str1, str2
	r = 3
	a = (3.14) * r * r
	IF ( r == 5) THEN
	  p = 2 * 3.14 * b
	  print *, a, p, r
	else
		str1 = "CS"
		str2 = "Program"
		print *, str1, str2, " ", b
	END IF
END PROGRAM circle
```

Expected output (`test4.correct`):

```
CS  Prog 2
(DONE)

Successful Execution
```

`"Program"` is cut to `"Prog"` because `str2` was declared with `LEN = 4`.

## Tech stack

C++ (standard library only: `map`, `queue`, `stack`, streams).

## Files

| File | What it is |
| --- | --- |
| `PA_3_I_Alcantara.cpp` | My parser and interpreter: every grammar function from `Prog` down to `Factor`, plus the symbol and value tables |
| `prog3.cpp` | Test driver with `main`, part of the course framework. It opens the source file and calls `Prog` |
| `lex.cpp`, `lex.h` | Lexical analyzer and token definitions, part of the course framework |
| `val.h` | The `Value` class that holds integer, real, string, and boolean results, part of the course framework |
| `parserInterp.h` | Declarations for the parser functions, part of the course framework |
| `GivenparserInterpPart.cpp` | Partial starter code that came with the assignment |
| `parser.cpp` | A parser-only version from the previous assignment. It isn't part of this build |
| `PA 3 Test Cases (Updated)/` | 14 test programs, each with a `.correct` file of the expected output |

## How to build and run

You need a C++ compiler such as `g++`.

`val.h` declares the `Value` operations (`+`, `-`, `*`, `/`, `==`, `<`, `>`, `Catenate`, and `Power`), but the file that implements them, `val.cpp`, isn't in this repo. The build won't link until a `val.cpp` with those definitions sits next to the other files. With it in place:

```sh
g++ -std=c++17 -o sfort95 prog3.cpp PA_3_I_Alcantara.cpp lex.cpp val.cpp
./sfort95 "PA 3 Test Cases (Updated)/test4"
```

Compare the output with the matching `.correct` file.

## What I'd point a reader to

- `AssignStmt` in `PA_3_I_Alcantara.cpp`: the type check for `CHARACTER` targets and the padding and cutting of fixed-length strings.
- The `Parser` namespace at the top of the same file: a one-token pushback that lets each grammar function peek at the next token and hand it back.
- `BlockIfStmt`: it evaluates the condition first and, when it's false, skips ahead to the `ELSE` branch.

## Status

This is finished coursework. Known limits:

- A false `IF` condition skips tokens until the next `ELSE` or `END`, so an `IF` block nested inside a skipped branch isn't handled.
- The header comment at the top of `PA_3_I_Alcantara.cpp` still describes the earlier parser assignment.
