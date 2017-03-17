CC=gcc
LD=gcc
CFLAGS = `pkg-config allegro-5 --cflags`
LDLIBS = `pkg-config allegro-5 --libs` -lallegro_main -lallegro_font -lallegro_ttf
#OBJS=game.o main.o menus.o
OBJS=main.o
PROG=dogfight

$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	$(RM) $(OBJS) $(PROG)
