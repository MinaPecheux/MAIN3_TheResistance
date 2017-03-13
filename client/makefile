# compilation options
CC = gcc
CCFLAGS = -Wall
LIBS = 
LDFLAGS = 

# project files
SRC = main.c server.c fsm.c utils.c
OBJ = $(SRC:.c=.o)
EXEC = main

# initial rule
all: $(EXEC)

# header files dependencies
fsm.o: fsm.h utils.h
utils.o: utils.h
server.o: utils.h fsm.h
main.o: server.h utils.h fsm.h

# compilation rules
%.o: %.c
	$(CC) $(CCFLAGS) -o $@ -c $<
	
# links edition rules
$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LIBSDIR) $(LDFLAGS)

# other rules
clean:
	rm -f *.o
rmproper:
	rm -f $(EXEC) *.o