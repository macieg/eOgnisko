FLAGS=-std=c++11 -Wall -lboost_system -lboost_program_options -lboost_thread -lpthread
ERR=err
K=klient
S=serwer
M=mixer
CPP=.cpp
H=.h

all: $(K) $(S)

$(K): $(K).cpp $(ERR)$(H) $(ERR)$(CPP)
	g++ $(K)$(CPP) $(ERR)$(CPP) -o $(K) $(FLAGS)

$(S): $(S).cpp $(M)$(H) $(M)$(CPP)
	g++ $(S)$(CPP) $(ERR)$(CPP) $(M)$(CPP) -o $(S) $(FLAGS)

clean:
	rm $(K) $(S)
	rm *~
