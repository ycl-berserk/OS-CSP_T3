CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2

test_framework: test_framework.cpp
	$(CXX) $(CXXFLAGS) test_framework.cpp -o test_framework

clean:
	rm -f test_framework temp_executable

.PHONY: clean