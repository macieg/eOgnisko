FLAGS=-std=c++11 -Wall -lboost_system -lboost_program_options -lboost_thread -lpthread
ERR=err
K=klient
S=serwer
C=connection
M=mixer
U=utils
CPP=.cpp
H=.h

all: $(K) $(S)

$(K): $(K).cpp $(ERR)$(CPP)
	g++ $(K)$(CPP) $(ERR)$(CPP) -o $(K) $(FLAGS)

$(S): $(S)$(CPP) $(M)$(CPP) $(C)$(CPP) $(U)$(CPP)
	g++ $(S)$(CPP) $(ERR)$(CPP) $(M)$(CPP) $(C)$(CPP) $(U)$(CPP) -o $(S) $(FLAGS)

clean:
	rm $(K) $(S) $(C)
	rm *~
