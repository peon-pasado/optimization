HOMEDIR  = .

CC       = g++
CC_WARN  = -Wall #-Wextra -Wpedantic -Wshadow -Weffc++
CC_ARGS  = -std=c++17 -O3 -march=native
CC_LIB   = -lm -lpthread -ldl

#================= GUROBI =====================================================
#GUROBI_DIR = /opt/gurobi1101/armlinux64
#ifeq ($(SYSTEM),Darwin)
GUROBI_DIR = /Library/gurobi1101/macos_universal2
#endif
FLAGVERSION := 110

GUROBI_INC = -I $(GUROBI_DIR)/include # used isystem to ignore warnings
GUROBI_LIB = -L $(GUROBI_DIR)/lib -lgurobi_c++ -lgurobi$(FLAGVERSION)

# define includes and libraries
INC = $(GUROBI_INC)
LIB = $(CC_LIB) $(GUROBI_LIB)

_OBJ = ${HOMEDIR}/main.cpp

main: $(_OBJ)
	$(CC) $(CC_ARGS) $(CC_WARN) $^ -o $(HOMEDIR)/$@ $(LIB) $(INC)


 #-I /Library/gurobi1101/macos_universal2/include 