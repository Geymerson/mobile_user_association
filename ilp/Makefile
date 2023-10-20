#CPLEXFLAGS=-O3 -m64 -O -fPIC -fexceptions -DNDEBUG -DIL_STD -I/opt/cplex/cplex/include -I/opt/cplex/concert/include  -L/opt/cplex/cplex/lib/x86-64_sles10_4.1/static_pic -lilocplex -lcplex -L/opt/cplex/concert/lib/x86-64_sles10_4.1/static_pic -lconcert -lm -pthread -std=c++0x 

CPLEXFLAGS=-O3 -m64 -O -fPIC -fexceptions -DNDEBUG -DIL_STD -I/opt/ibm/ILOG/CPLEX_Studio1210/cplex/include/ -I/opt/ibm/ILOG/CPLEX_Studio1210/concert/include  -L/opt/ibm/ILOG/CPLEX_Studio1210/cplex/lib/x86-64_linux/static_pic -lilocplex -lcplex -L/opt/ibm/ILOG/CPLEX_Studio1210/concert/lib/x86-64_linux/static_pic -lconcert -lm -pthread -std=c++0x -ldl

CFLAGS=-std=c++11 -static-libstdc++ -static-libgcc -Wall

all:handover.o
	g++ handover.o -o handover.run $(CPLEXFLAGS) $(CFLAGS)


%.o: %.cpp %.hpp
	g++ -c $< -o $@ $(CFLAGS)

handover.o: handover.cpp
	g++ -c -o handover.o handover.cpp $(CPLEXFLAGS) $(CFLAGS)

clean:
	rm -f *.o

