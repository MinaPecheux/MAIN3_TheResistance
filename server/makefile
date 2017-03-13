# compilation options
CC = gcc
CCFLAGS = -Wall
LIBS = -lpthread
LDFLAGS = 

# project files
SRC = main.c server.c fsm.c ../common/utils.c
OBJ = $(SRC:.c=.o)
EXEC = main

# initial rule
all: $(EXEC)

# header files dependencies
fsm.o: fsm.h ../common/utils.h
utils.o: ../common/utils.h
server.o: ../common/utils.h fsm.h
main.o: server.h ../common/utils.h fsm.h

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