$(CXX) = g++ -g3 -gdwarf2
CXX += -std=c++0x


all: cachesim
.PHONY: cachesim cache.o util.o cachesim.o

debug: CXX += -DDEBUG -g
debug: cachesim

cachesim: util.o cache.o cachesim.o
	$(CXX) -o cachesim cache.o util.o cachesim.o 

cachesim.o:
	$(CXX) -c cachesim.cc

cache.o:
	$(CXX) -c cache.cc

util.o:
	$(CXX) -c util.cc

clean:
	rm cachesim *.o
