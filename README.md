# bot_manager
## Bot for game "Manager"

My study project from 4-th semester of CMC MSU

The Bot can connect to "Manager" server via sockets and play with other players or bots by compliting the program assigned to him.

It reads program from file given to it in the command line arguments. 

The Bot : 
- checks lexical (lexer.cpp, using Finite-state machine) 
- and syntaсtic (syntax.cpp, using Recursive descent parser) correctness (Chomsky's theory of formal grammars) 
- makes Reverse Polish Notation (rpn.cpp) 
- and executes it
- in case of lexical or syntactic mistake Bot explains type of error and what it expets on input. 

There are 2 examples of program for Bot: simple.txt and smart.txt. In simple.txt Bot just does similar actions on every turn. In smart.txt Bot tries to extrapolate results of previous turns on the following.
