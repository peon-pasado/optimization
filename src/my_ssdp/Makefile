# Compilador
CXX = g++

# Flags de compilación acordados
CXXFLAGS = -std=c++20 -Wall -Wextra -Wshadow -O3 -fomit-frame-pointer -ffast-math

# Archivo fuente y nombre del ejecutable
SRC = main.cpp
OUT = main.exe

# Recompilar siempre que se ejecute 'make'
all:
	@echo "Compilando $(SRC) → $(OUT)..."
	$(CXX) $(CXXFLAGS) -o $(OUT) $(SRC)

# Eliminar el ejecutable
clean:
	-del /Q $(OUT) 2>nul || exit 0

.PHONY: all clean
