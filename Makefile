# Compiler
NVCC = nvcc

# Include directories
INCLUDES = -I../Common -I../Common/UtilNPP

# Libraries
LIBS = -lnppc -lnppif -lnppisu -lfreeimage

# Source and executable
SRC = filter.cpp
TARGET = filter.exe

# Default target
all: build

# Build target
build:
	$(NVCC) $(INCLUDES) $(SRC) $(LIBS) -o $(TARGET)

# Run the program
run: build
	./$(TARGET) $(ARGS)

# Remove generated files
clean:
	rm -f $(TARGET)
