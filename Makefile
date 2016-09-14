all:
	g++ sjparser.cpp test.cpp -lyajl -lgtest -lgtest_main --std=c++14 -Wall -Wextra -Wpedantic -Werror -o test

clean:
	rm test
