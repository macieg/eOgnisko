MFLAGS=-std=c++11 -Wall -lboost_system -lboost_program_options -lpthread
DFLAGS=-std=c++11 -Wall -c
K=klient
S=serwer
C=connection
SC=server
KC=client
M=mixer

O=.o
CPP=.cpp
H=.h

all: $(K) $(S)

$(K): $(K)$(CPP) $(KC)$(O)
	g++ $(K)$(CPP) $(KC)$(O) -o $(K) $(MFLAGS)

$(S): $(S)$(CPP) $(M)$(O) $(C)$(O) $(SC)$(O)
	g++ $(S)$(CPP) $(M)$(O) $(C)$(O) $(SC)$(O) -o $(S) $(MFLAGS)

$(KC)$(O): $(KC)$(CPP) $(KC)$(H)
	g++ $(DFLAGS) $(KC)$(CPP) -o $(KC)$(O)
	
$(SC)$(O): $(SC)$(CPP) $(SC)$(H)
	g++ $(DFLAGS) $(SC)$(CPP) -o $(SC)$(O)
	
$(M)$(O): $(M)$(CPP) $(M)$(H)
	g++ $(DFLAGS) $(M)$(CPP) -o $(M)$(O)

$(C)$(O): $(C)$(CPP) $(C)$(H)
	g++ $(DFLAGS) $(C)$(CPP) -o $(C)$(O)


clean:
	rm $(K) $(S) $(KC)$(O) $(SC)$(O) $(M)$(O) $(C)$(O)
	rm *~
