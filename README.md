# My Basic Pascal Interpreter
*By Daniel Chu*  
This is my version of a simple PASCAL Interpreter in progress. It is a self project that reflects studying programming interpreters through this guide here https://ruslanspivak.com/lsbasi-part1/, along with learning the language **C++**.

This has been a large step up from my previous projects and it has taught me countless things about programming.  

Here are the current features of the program:
- The program takes in a ```.txt``` file. Run the ```run``` executable along with the path to your input ```.txt``` file.
- It prints out a representation of the abstract syntax tree architecture in a postorder traversal.
- It prints out a series of symbol tables for each scope of the input program.
- At the end, it prints out the contents of the global scope, containing all the local variable values.

Here are some key features of the source code:
- This interpreter contains a **Token** class, **Lexer** class, a **Parser** class, and an **Interpreter** class.
- It also contains multiple visitors such as the **SemanticAnalyzer**, **PrintVisitor**, and **EvalVisitor**, which are applications of the *Node Visitor Pattern* designed to reduce heavy coupling with the Nodes.
- This program also contains an **Abstract Syntax Tree** data structure with **Nodes** that result from parsing different *formal grammars*.
- Custom error classes that extend ```std::exception``` for custom error handling. Now these errors provide line and column numbers, which provide more information where the error is occurring.

Feedback is always appreciated!