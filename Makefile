STANDART =g++ -Wall -g bot.cpp syntax.o lexer.o lexeme.o report.o rpn.o -lm -o bot

all: bot 

bot : bot.cpp syntax.o rpn.o lexer.o lexeme.o report.o lexer.hpp lexeme.hpp rpn.hpp syntax.hpp report.hpp 
	$(STANDART)

syntax.o: rpn.hpp syntax.cpp lexeme.hpp lexer.hpp report.hpp syntax.hpp
	g++ -Wall -c syntax.cpp

rpn.o: rpn.cpp rpn.hpp
	g++ -Wall -c rpn.cpp
	
lexer.o: lexer.cpp lexer.hpp lexeme.hpp
	g++ -Wall -c lexer.cpp
	
lexeme.o: lexeme.cpp lexeme.hpp
	g++ -Wall -c lexeme.cpp
	
report.o: report.cpp report.hpp lexeme.hpp
	g++ -Wall -c report.cpp