/* Dogfight.c                                 */
/* Program:      a dog fighting game          */
/* Author:       Gregory McIntyre             */
/* First edited: 28/08/97                     */

#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>

// game constants
#define SET_W               640                // } resolution
#define SET_H               480                // }

// level constants
#define CLOUD_TRANSPARENCY  180                // 0 <= CLOUD_TRANSPARENCY <= 255

// colours (NB: these are values for "base" colours which are the darkest versions
//              of these colours)
// to brighten a colour, add a number between 1 and 15 to it
// adding 16 will choose the next colour - this number (16) is defined as PAL_DIFFERENCE
#define GREY                 16
#define RED                  32
#define BLUE                 48
#define YELLOW               64
#define PURPLE               80
#define GREEN                96
#define ORANGE              112

typedef struct assetsinfo {
  ALLEGRO_FONT *font;
  ALLEGRO_BITMAP *bg;
  ALLEGRO_BITMAP *explosion[8];
  ALLEGRO_BITMAP *plane[3];
  ALLEGRO_BITMAP *cloud;
  ALLEGRO_BITMAP *parachute;
  ALLEGRO_BITMAP *parafall;
  ALLEGRO_BITMAP *title;
  ALLEGRO_BITMAP *shot;
} assetsinfo;

int load_bitmap(ALLEGRO_BITMAP **bmp, const char *path) {
  ALLEGRO_DISPLAY *display = al_get_current_display();
  char error[512];
  *bmp = al_load_bitmap(path);
  if (!*bmp) {
    snprintf(error, sizeof(error), "Failed to load image: %s", path);
    al_show_native_message_box(display, "Error", "Error", error, NULL, ALLEGRO_MESSAGEBOX_ERROR);
    return -1;
  }
  return 0;
}

// ***** MAIN ***************************************************************************

const float FPS = 60;

int main(int argc, char **argv){

  ALLEGRO_DISPLAY *display = NULL;
  ALLEGRO_EVENT_QUEUE *event_queue = NULL;
  ALLEGRO_TIMER *timer = NULL;
  bool redraw = true;
  float w, h;
  assetsinfo assets;

  if (!al_init()) {
    al_show_native_message_box(display, "Error", "Error", "Failed to initialize allegro.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
    return -1;
  }
  if (!al_init_image_addon()) {
    al_show_native_message_box(display, "Error", "Error", "Failed to initialize image system.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
    return -1;
  }
  if (!al_init_font_addon()) {
    al_show_native_message_box(display, "Error", "Error", "Failed to initialize font system.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
    return -1;
  }
  if (!al_init_ttf_addon()) {
    al_show_native_message_box(display, "Error", "Error", "Failed to initialize Truetype font system.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
    return -1;
  }
  if (!al_init_primitives_addon()) {
    al_show_native_message_box(display, "Error", "Error", "Failed to initialize primitives drawing system.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
    return -1;
  }

  assets.font = al_load_ttf_font("data/font.ttf", 12, 0);
  if (!assets.font) {
    fprintf(stderr, "Could not load font.\n");
    return -1;
  }

  display = al_create_display(640, 480);
  if (!display) {
    fprintf(stderr, "failed to create display!\n");
    al_destroy_timer(timer);
    return -1;
  }

  timer = al_create_timer(1.0 / FPS);
  if (!timer) {
    fprintf(stderr, "failed to create timer!\n");
    return -1;
  }

  int r;
  if ((r = load_bitmap(&assets.title, "data/title.png")) != 0) { return r; }
  if ((r = load_bitmap(&assets.bg, "data/back01.png")) != 0) { return r; }
  if ((r = load_bitmap(&assets.cloud, "data/cloud01.png")) != 0) { return r; }
  if ((r = load_bitmap(&assets.parachute, "data/parachte.png")) != 0) { return r; }
  if ((r = load_bitmap(&assets.parafall, "data/parafall.png")) != 0) { return r; }
  if ((r = load_bitmap(&assets.shot, "data/shot01.png")) != 0) { return r; }
  if ((r = load_bitmap(&assets.explosion[0], "data/explsn01.png")) != 0) { return r; }
  if ((r = load_bitmap(&assets.explosion[1], "data/explsn02.png")) != 0) { return r; }
  if ((r = load_bitmap(&assets.explosion[2], "data/explsn03.png")) != 0) { return r; }
  if ((r = load_bitmap(&assets.explosion[3], "data/explsn04.png")) != 0) { return r; }
  if ((r = load_bitmap(&assets.explosion[4], "data/explsn05.png")) != 0) { return r; }
  if ((r = load_bitmap(&assets.explosion[5], "data/explsn06.png")) != 0) { return r; }
  if ((r = load_bitmap(&assets.explosion[6], "data/explsn07.png")) != 0) { return r; }
  if ((r = load_bitmap(&assets.explosion[7], "data/explsn08.png")) != 0) { return r; }
  if ((r = load_bitmap(&assets.explosion[8], "data/explsn09.png")) != 0) { return r; }

  w = al_get_display_width(display);
  h = al_get_display_height(display);

  event_queue = al_create_event_queue();
  if (!event_queue) {
    fprintf(stderr, "failed to create event_queue!\n");
    al_destroy_display(display);
    al_destroy_timer(timer);
    return -1;
  }

  al_register_event_source(event_queue, al_get_display_event_source(display));
  al_register_event_source(event_queue, al_get_timer_event_source(timer));

  al_clear_to_color(al_map_rgb(0,0,0));
  al_flip_display();

  al_start_timer(timer);

  while (1) {
    ALLEGRO_EVENT ev;
    al_wait_for_event(event_queue, &ev);

    if (ev.type == ALLEGRO_EVENT_TIMER) {
      redraw = true;
    } else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
      break;
    }

    if (redraw && al_is_event_queue_empty(event_queue)) {
      redraw = false;
      al_clear_to_color(al_map_rgb(0, 0, 0));

      al_draw_bitmap(assets.title, 0, 0, 0);
      al_draw_text(assets.font, al_map_rgb(255, 255, 255), w/2, h/2-58, ALLEGRO_ALIGN_CENTRE, "Dogfight by Gregory McIntyre");

      al_flip_display();
    }
  }

  al_destroy_timer(timer);
  al_destroy_display(display);
  al_destroy_event_queue(event_queue);

  // print a message that will be displayed at the DOS prompt on leaving
  printf("Thankyou for playing Dogfight.\n");
  printf("Dogfight was created using the Allegro game library.\n");
  printf("It's free so enjoy and distribute at your leasure.\n\n");

  printf("-------------------------------\n");
  printf("The author can be contacted at:\n\n");
  printf("gregm@pcug.org.au*\n\n");
  printf("-------------------------------\n\n");
  printf("Have a nice day!\n");
  printf("\n");
  printf("* Email no longer valid.\n");
  return 0;
}
