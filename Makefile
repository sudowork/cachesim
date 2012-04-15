$(CXX) = g++ -g3 -gdwarf2


all: cachesim
.PHONY: cachesim cachesim.o cache.o util.o

debug: CXX += -DDEBUG -g
debug: cachesim

cachesim: cachesim.o cache.o util.o
	$(CXX) -o cachesim cachesim.o cache.o util.o

cachesim.o:
	$(CXX) -c cachesim.cc

cache.o:
	$(CXX) -c cache.cc

util.o:
	$(CXX) -c util.cc

clean:
	rm cachesim *.o
