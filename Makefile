#
#  @brief   Makefile for testing bitvector.h
#

PROGNAME=testbuddy
OBJS=buddy.o testbuddy.o

CFLAGS+= -g
CFLAGS+= -DTEST -DBV_ENABLEMACROS
CFLAGS+= -DDEBUG
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


buddy.o: bitvector.h buddy.h
testbuddy.o: bitvector.h buddy.h

