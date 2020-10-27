#
#  @brief   Makefile for testing bitvector.h
#

PROGNAME=testbuddy
OBJS=buddy.o

CFLAGS+= -g
CFLAGS+= -DTEST -DDEBUG -DBV_ENABLEMACROS
#CFLAGS+= --save-temps


default: run


run: $(PROGNAME)
	./$(PROGNAME)

build: $(PROGNAME)
	echo "Done"

$(PROGNAME): $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(LFLAGS) $(LIBS)

clean::
	rm -rf *.o $(PROGNAME) html  latex

docs:
	doxygen Doxyfile


buddy.o: bitvector.h

