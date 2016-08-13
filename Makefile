all:
	g++ sjparser.cpp test.cpp -lyajl -lgtest -lgtest_main --std=c++14 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter -o parser -g3

clean:
	rm parser
