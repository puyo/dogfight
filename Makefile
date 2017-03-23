CC=gcc
LD=gcc
CFLAGS = `pkg-config allegro-5 --cflags` -ferror-limit=2
LDLIBS = `pkg-config allegro-5 --libs` -lallegro_main -lallegro_font -lallegro_ttf -lallegro_image -lallegro_dialog -lallegro_primitives
OBJS=game.o main.o menus.o
PROG=dogfight

$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	$(RM) $(OBJS) $(PROG)
