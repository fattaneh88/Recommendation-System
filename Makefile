# ***********************************************
#                    JustGarble
# ***********************************************

SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
TESTDIR   = test
OBJECTFULL = obj/*.o

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

IDIR = /home/software/include/
LFLAGS = -L/home/software/lib/
CCG=gcc 
CFLAGS= -O3 -lm -lrt -lpthread -maes -msse4 -L/home/software/lib  -lmsgpack -lssl -march=native -I$(IDIR) 


T1 = RS-Swap

all: T1

T1: $(OBJECTS) $(TESTDIR)/$(T1).c
	$(CC) $(OBJECTFULL) $(TESTDIR)/$(T1).c -o $(BINDIR)/$(T1).out $(LIBS) $(LFLAGS)  $(CFLAGS) 

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC)  -c  $< -o $@ $(LIBS) $(LFLAGS) $(CFLAGS) 

.PHONEY: clean
clean:
	@$(rm) $(OBJECTS)
	@$(rm) $(BINDIR)/$(T1)

