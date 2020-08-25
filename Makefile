CFLAGS= -Wfatal-errors
LDFLAGS= -lportaudio -lm -lpthread -pthread
OBJS= main.o seqr.o ui.o list.o

# OS Specific
ifeq ($(OS),Windows_NT)
# CYGWIN flags
CFLAGS += -I/mingw64/include
LDFLAGS += -L/mingw64/lib -lpdcurses

else
# Linux
CFLAGS +=
LDFLAGS += -lncurses
endif

all: seqr
seqr: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)
%:%.c %.h
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)
clean:
	$(RM) *.o $(OBJS) seqr
