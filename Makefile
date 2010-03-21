CC=gcc
LD=gcc
CFLAGS = `allegro-config --cflags` -m32 -g
LDLIBS = `allegro-config --libs` -m32
OBJS=game.o main.o menus.o
PROG=dogfight

$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	$(RM) $(OBJS) $(PROG)
