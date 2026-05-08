CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g
SRCDIR = src
TARGET = cpu_emulator

# Find all .cpp files
SOURCES = main.cpp

# Find all header files
HEADERS = $(shell find $(SRCDIR) -name "*.hpp")

.PHONY: all clean run help debug run-hello run-fibonacci run-timer

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

help:
	@echo "Available targets:"
	@echo "  make               - Build the emulator"
	@echo "  make run           - Run emulator (interactive)"
	@echo "  make run-hello     - Run hello program"
	@echo "  make run-fibonacci - Run fibonacci program"
	@echo "  make run-timer     - Run timer program"
	@echo "  make clean         - Remove binary"
	@echo "  make debug         - Build with debug flags"

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

run-hello: $(TARGET)
	./$(TARGET) programs/hello.asm run

run-fibonacci: $(TARGET)
	./$(TARGET) programs/fibonacci.asm run

run-timer: $(TARGET)
	./$(TARGET) programs/timer.asm run

debug: CXXFLAGS += -DDEBUG -g3
debug: clean $(TARGET)