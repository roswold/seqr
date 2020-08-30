CFLAGS=		$(shell pkg-config --cflags libxml-2.0 portaudio-2.0 ncurses libxml-2.0) -Wfatal-errors
LDFLAGS=	$(shell pkg-config --libs portaudio-2.0 ncurses libxml-2.0) -lm -lpthread -pthread
OBJS= main.o seqr.o ui.o list.o song.o wav.o

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
