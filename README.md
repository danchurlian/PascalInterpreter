# My Basic Pascal Interpreter
*By Daniel Chu*  
This is my version of a simple PASCAL Interpreter in progress. It is a personal project that reflects studying programming interpreters through this guide here https://ruslanspivak.com/lsbasi-part1/, along with learning the language **C++**.

I started working on this project in August 2025 and have been working on this extensively during my fall semester of my freshman year at UMD. This has been a large step up from my previous projects and it has taught me countless things about programming as a whole, such as string manipulation, building trees, traversing through trees, error handling, and careful awareness of memory usage.  

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

## What Went Well: The Node Visitor Pattern
- When I first wrote the Interpreter class, I wrote the interpreter to traverse through the whole AST in one large whole method. To determine the behavior of the Node the program was visiting, it would check its type and downcast appropriately. This was a code smell, a sign that I could use polymorphism better with the AST. To address this problem, I researched and learned about the Node Visitor Pattern. 
- Writing the visitor pattern made thinking about the behavior of each Node so much easier for me. I was able to program the node behaviors without having to think about type-checking and downcasting (which was pretty verbose). And if I want to have another visitor, I could easily make another one. At first I only used the visitor pattern to print and evaluate the Pascal Code. Then I used it again for the Semantic Analyzer.

## Lesson Learned: Learning Smart Pointers
- When I first started out programming and building the Abstract Syntax Tree using the Parser, I used raw pointers to hold everything together. This meant that I would use the ```new``` keyword every single time I would create a new Node. 
- Sooner or later I realized that I had to address the issue of memory leaks in order to make the program more robust and less prone to errors. When I attempted using the ```delete``` keyword to free up the memory space of the AST, I used Valgrind to check for memory leaks from the AST. And every single time, the program wasn't able to free everything up and there was always some sort of memory leak somewhere. 
- At that point I had to look for other alternatives, which was using smart pointers. I had a bit of a learning curve using them for the first time. I first had the idea to use ```std::unique_ptr``` for AST nodes, since each node should have unique ownership by its parent. As a consequence, I had to replace the majority of raw pointers in the program to ```std::unique_ptr```'s. This was brutal because I also faced a lot of compilation errors as a result of not understanding move semantics.

## Another Lesson Learned: Destructors and Polymorphism
- Another issue that I ran into that took a long time to realize was the awareness of Destructors in polymorphism. I came across the issue of always finding some memory leak despite having replaced almost everything with smart pointers. Although the executable ran fine, I wanted to resolve the memory errors. 
- The day finally came when I realized that the Valgrind errors were arising from the Destructor methods of some AST node classes. I learned the major lesson that without the ```virtual``` keyword, the Base Node class would destruct itself first before the fields of the child classes were freed. It turned that I had to make the Base Node class destructor ```virtual```, such as ```virtual ~Node() {}``` which solved the problem. 

## Current Issues / Looking Forward
- There are still many uses of downcasting, which may indicate an OOP design that is still not satisfactory enough. This is especially true for the Node AST and working with Symbols.
- Project organization. I'm thinking about breaking up the ```main.cpp``` file in separate files for each group of classes, such as a separate file for AST Nodes and Node visitors, and another for the ```Symbol``` and ```SymbolTable``` classes.
- I still have to check for edge cases and to increase robustness in the program.

Feedback is always appreciated! This is the most complicated project I've done, and I still believe I can dive deeper to how interpreters really work.