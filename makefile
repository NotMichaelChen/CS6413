#http://blog.jgc.org/2011/07/gnu-make-recursive-wildcard-function.html
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

CC=gcc
CPC=g++
CPFLAGS+=-I. -Wall -std=c++14
LIBS =
VPATH = %.cpp %.o
DEPS = $(call rwildcard,, *.hpp)
SRC = $(call rwildcard,, *.cpp)
CSRC = tok.c
OBJ = $(SRC:.cpp=.o)
COBJ = $(CSRC:.c=.o)
DEBUG = -g -O0
OPTIMIZE = -O3

LEX = flex
# what flags to pass to it
LFLAGS	= -i

%.c: %l
	$(LEX) $(LFLAGS) $< > $@

%.o: %.cpp $(DEPS)
	$(CPC) -c -o $@ $< $(CPFLAGS) $(LIBS)

%.o: %.c
	$(CPC) -c -o $@ $< $(CPFLAGS) $(LIBS)

out: $(OBJ) $(COBJ)
	mkdir -p exe
	$(CPC) -o ./exe/$@ $^ $(CPFLAGS) $(LIBS)

.PHONY: clean
clean:
	find . -type f -name '*.o' -delete
	rm -f tok.c

.PHONY: debug
debug: CPFLAGS+=$(DEBUG)
debug: out

.PHONY: optimize
optimize: CPFLAGS+=$(OPTIMIZE)
optimize: out
