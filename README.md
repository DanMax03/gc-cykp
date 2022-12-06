# gc-cykp - Grammar Converter and CYK Parser
## Description
The program works with context-free grammars and has 2 modes:
 - Grammar convertation. Takes a context-free grammar in any form and converts it into the Chomsky form.
 - Text recognition. Takes a context-free grammar and a text, converts the grammar to Chomsky form and executes Cocke-Younger-Kasami algorithm.

## Build
```
mkdir build
cd build
cmake ..
make
```

## Usage
The program waits a grammar to be in some sort of Backus-Naur form. More precisely, the basic statements are:
 - Comment in a grammar file starts after the first '#' symbol and goes to the end of a line
 - Word is a sequence of any literals without spaces
 - Every word which is not inside of "" quotes is recognized as a nonterminal
 - Every string in "" quotes is recognized as a terminal. The c++ escape sequences do work
 - Every rule ends with a semicolon ;

## Explanation
To be going soon.
