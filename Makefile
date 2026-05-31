CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wno-unused-function
TARGET   = scheme2python

all: $(TARGET)

# 1. Bison gera parser.tab.c e parser.tab.h
parser.tab.cpp parser.tab.h: parser.y
	bison -d -o parser.tab.cpp parser.y

# 2. Flex gera scanner.cpp
scanner.cpp: scanner.l parser.tab.h
	flex -o scanner.cpp scanner.l

# 3. Compila tudo junto
$(TARGET): parser.tab.cpp scanner.cpp ast.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) parser.tab.cpp scanner.cpp ast.cpp

clean:
	rm -f $(TARGET) parser.tab.cpp parser.tab.h scanner.cpp *.o

# Testa todos os exemplos
test: $(TARGET)
	@echo "=== Testando exemplos ==="
	@for f in ./*.scm; do \
		[ -f "$$f" ] || continue; \
		echo "\n--- $$f ---"; \
		./$(TARGET) $$f && python3 $${f%.scm}.py 2>/dev/null || true; \
	done

.PHONY: all clean test
