FLAGS=-std=c++11 -Wall
K=klient
S=serwer

all: $(K) $(S)

$(K): $(K).cpp
	g++ $(FLAGS) $(K).cpp -o $(K)

$(S): $(S).cpp
	g++ $(FLAGS) $(S).cpp -o $(S)

clean:
	rm $(K) $(S)
	rm *~
