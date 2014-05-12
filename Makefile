FLAGS=-std=c++11 -Wall -lboost_program_options
ERR=err
K=klient
S=serwer
CPP=.cpp

all: $(K) $(S)

$(K): $(K).cpp $(ERR).h $(ERR).cpp
	g++ $(K)$(CPP) $(ERR)$(CPP) -o $(K) $(FLAGS)

$(S): $(S).cpp
	g++ $(S)$(CPP) $(ERR)$(CPP) -o $(S) $(FLAGS)

clean:
	rm $(K) $(S)
	rm *~
