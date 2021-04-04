CC = gcc
CFLAGS = -g -Wall

default: shell

sim_cache: shell.o 
				$(CC) $(CFLAGS) -o shell shell.o -lm
				
alpha.o: shell.c 
		 $(CC) $(CFLAGS) -c shell.c -lm
		 
#################################

# default rule

all: shell
	@echo "my work is done here..."

# type "make clean" to remove all .o files plus the sim_cache binary

clean:
	rm -f *.o shell


# type "make clobber" to remove all .o files (leaves sim_cache binary)

clobber:
	rm -f *.o


