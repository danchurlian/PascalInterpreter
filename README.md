# My Basic Pascal Interpreter
*By Daniel Chu*  
This is my version of a simple PASCAL Interpreter in progress. It is a self project that reflects studying programming interpreters through this guide here https://ruslanspivak.com/lsbasi-part1/, along with learning the language **C++**.

I started working on this project in August 2025 and have been working on this extensively during my fall semester of my freshman year at UMD. This has been a large step up from my previous projects and it has taught me countless things about programming.  

## Functionality
- The program takes in a ```.txt``` file. Run the ```run``` executable along with the path to your input ```.txt``` file.
- It prints out a representation of the abstract syntax tree architecture in a postorder traversal.
- It prints out a series of symbol tables for each scope of the input program. This is during the semantic analysis phase
- At the end, it prints out the contents of the activation records in the call stack, containing all the local variable values.

## Key Highlights of the Source Code
- This interpreter contains a **Token** class, **Lexer** class, a **Parser** class, and an **Interpreter** class.
- It also contains multiple visitors such as the **SemanticAnalyzer**, **PrintVisitor**, and **EvalVisitor**, which are applications of the *Node Visitor Pattern* designed to reduce heavy coupling with the Nodes.
- This program also contains an **Abstract Syntax Tree** data structure with **Nodes** that result from parsing different *formal grammars*.
- Custom error classes that extend ```std::exception``` for custom error handling. Now these errors provide line and column numbers, which provide more information where the error is occurring.
- The semantic analysis phase involves using the ```SymbolTable```  class, which is a map with a string key and a pointer to a ```Symbol``` object. This process is used to detect undefined variables or duplicated variables, or if the procedure calls do not match their respective procedure declarations.
- The execution phase involves using a **Call Stack**, which contains **stack frames** or **activation records**. Each **activation record** contains a map containing the name of the variable / parameter and its current value.

## Current Issues / Looking Forward
- There were many uses of downcasting, which may indicate an OOP design that is not satisfactory enough. This is especially true for the Node AST and working with Symbols.
- Project organization. I'm thinking about breaking up the ```main.cpp``` file in separate files for each group of classes, such as a separate file for AST Nodes and Node visitors, and another for the ```Symbol``` and ```SymbolTable``` classes.
- I still have to check for edge cases and to increase robustness in the program.

Feedback is always appreciated! This is the most complicated project I've done, and I still believe I can dive deeper to how interpreters really work.