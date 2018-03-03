CFLAGS=-Wall -Werror -pedantic -Iinclude

_OBJ = game.o piece.o cemented-block-array.o game-box.o piece-generator.o
OBJ = $(patsubst %,obj/%,$(_OBJ))

bin/main: $(OBJ)
	g++ -o $@ main.cpp $^ $(CFLAGS) $(LIBS)

bin/testsuite: $(OBJ) test/testsuite.cpp
	g++ -o $@ $^ $(CFLAGS) $(LIBS) -Ivendor

obj/%.o: src/%.cpp include/%.hpp include/api.hpp
	g++ -c -o $@ $< $(CFLAGS)

dirs:
	mkdir -p obj bin vendor obj/test

vendor/catch.hpp:
	cd vendor && wget https://raw.githubusercontent.com/CatchOrg/Catch2/master/single_include/catch.hpp

setup: dirs vendor/catch.hpp

test: bin/testsuite
	./bin/testsuite

.PHONY: clean

clean:
	rm -f obj/*.o bin/*
