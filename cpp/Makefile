CFLAGS=-Wall -Werror -pedantic -Iinclude -std=c++11

_OBJ = game.o piece.o cemented-block-array.o game-box.o piece-generator.o
OBJ = $(patsubst %,obj/%,$(_OBJ))
JS_OBJ = $(patsubst %,obj/js/%,$(_OBJ))

#bin/main: $(OBJ)
#	g++ -o $@ main.cpp $^ $(CFLAGS) $(LIBS)

bin/js/game.js: $(JS_OBJ) js-api/js-api.cpp
	emcc --bind -o $@ $^ $(CFLAGS)

bin/testsuite: $(OBJ) test/testsuite.cpp
	g++ -o $@ $^ $(CFLAGS) $(LIBS) -Ivendor

obj/%.o: src/%.cpp include/%.hpp include/api.hpp
	g++ -c -o $@ $< $(CFLAGS)

obj/js/%.o: src/%.cpp include/%.hpp include/api.hpp
	emcc -c -o $@ $< $(CFLAGS)

dirs:
	mkdir -p obj bin vendor obj/test obj/js bin/js

vendor/catch.hpp:
	cd vendor && wget https://raw.githubusercontent.com/CatchOrg/Catch2/master/single_include/catch.hpp

setup: dirs vendor/catch.hpp

test: bin/testsuite
	./bin/testsuite

.PHONY: clean

clean:
	rm -r obj/* bin/* && make dirs
