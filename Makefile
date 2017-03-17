CC=gcc
LD=gcc
CFLAGS = `pkg-config allegro-5 --cflags` -m32 -g
LDLIBS = `pkg-config allegro-5 --libs` -m32
OBJS=game.o main.o menus.o
PROG=dogfight

$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	$(RM) $(OBJS) $(PROG)
