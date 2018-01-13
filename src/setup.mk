BUILDDIR = ../bin

# Path to ChibiOS
CHIBIOS = ../ChibiOS_2.6.2
# Path to convex
CONVEX  = ../convex/cortex

# uncomment to use the optional code like the smart motor library
CONVEX_OPT = yes

# User C code files
VEXUSERSRC = vexuser.c $(filter-out main.c vexuser.c, $(wildcard *.c)) $(wildcard autonomous/*.c)

# User CPP code files
VEXUSERCPPSRC = $(wildcard *.cpp)

# Uncomment and add/modify user include files
VEXUSERINC = ../include
