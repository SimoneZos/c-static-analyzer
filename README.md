# C Static Analyzer (myPreCompiler)

A custom static analysis tool built in C that parses source code files to validate variable declarations, check for C-standard naming conventions, resolve custom typedef definitions, and track variable usage. 

Developed as an academic project for an Operating Systems course.

## Features

* Lexical Validation: Checks if variable names adhere strictly to C standards.
* Type Checking: Validates against standard C types and dynamically registers user-defined types via typedef.
* Usage Tracking: Implements a robust substring-isolated search to accurately count variable usage without false positives.
* Dynamic Memory: Efficiently handles arbitrary numbers of variables using dynamic memory allocation.

## How to Build and Run

Compile the project using gcc:
```bash
gcc -Wall -Wextra -o precompiler main.c analyzer.c

./precompiler -i <input_file.c> [-o <output_file.txt>] [-v]
