/* Dogfight.c                                 */
/* Program:      a dog fighting game          */
/* Author:       Gregory McIntyre             */
/* First edited: 28/08/97                     */

#include <stdio.h>
#include "dogfight.h"

// resolution
const uint SET_W = 2*640;
const uint SET_H = 2*480;

// frames per second
const float FPS = 10;

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

int main(int argc, char **argv){
  doginfo dog = { 0 };

  if (!al_init()) {
    al_show_native_message_box(dog.display, "Error", "Error", "Failed to initialize allegro.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
    return -1;
  }
  if (!al_install_keyboard()) {
    al_show_native_message_box(dog.display, "Error", "Error", "Failed to initialize keyboard driver.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
    return -1;
  }
  if (!al_init_image_addon()) {
    al_show_native_message_box(dog.display, "Error", "Error", "Failed to initialize image system.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
    return -1;
  }
  if (!al_init_font_addon()) {
    al_show_native_message_box(dog.display, "Error", "Error", "Failed to initialize font system.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
    return -1;
  }
  if (!al_init_ttf_addon()) {
    al_show_native_message_box(dog.display, "Error", "Error", "Failed to initialize Truetype font system.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
    return -1;
  }
  if (!al_init_primitives_addon()) {
    al_show_native_message_box(dog.display, "Error", "Error", "Failed to initialize primitives drawing system.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
    return -1;
  }

  dog.font = al_load_ttf_font("data/font.ttf", SET_W/26, 0);
  if (!dog.font) {
    al_show_native_message_box(dog.display, "Error", "Error", "Failed to load font: data/font.ttf.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
    return -1;
  }

  dog.timer = al_create_timer(1.0 / FPS);
  if (!dog.timer) {
    al_show_native_message_box(dog.display, "Error", "Error", "Failed to create timer.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
    return -1;
  }

  dog.display = al_create_display(SET_W, SET_H);
  if (!dog.display) {
    al_show_native_message_box(dog.display, "Error", "Error", "Failed to create display/window.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
    al_destroy_timer(dog.timer);
    return -1;
  }

  dog.w = al_get_display_width(dog.display);
  dog.h = al_get_display_height(dog.display);

  dog.event_queue = al_create_event_queue();
  if (!dog.event_queue) {
    fprintf(stderr, "failed to create event_queue!\n");
    al_destroy_display(dog.display);
    al_destroy_timer(dog.timer);
    return -1;
  }
  al_register_event_source(dog.event_queue, al_get_display_event_source(dog.display));
  al_register_event_source(dog.event_queue, al_get_timer_event_source(dog.timer));
  al_register_event_source(dog.event_queue, al_get_keyboard_event_source());

  int r;
  if ((r = load_bitmap(&dog.title, "data/title.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.bg, "data/back01.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.cloud, "data/cloud01.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.parachute, "data/parachte.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.parafall, "data/parafall.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.shot, "data/shot01.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.explosion[0], "data/explsn01.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.explosion[1], "data/explsn02.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.explosion[2], "data/explsn03.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.explosion[3], "data/explsn04.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.explosion[4], "data/explsn05.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.explosion[5], "data/explsn06.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.explosion[6], "data/explsn07.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.explosion[7], "data/explsn08.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.explosion[8], "data/explsn09.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.plane[0], "data/plane01.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.plane[1], "data/plane02.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.plane[2], "data/plane03.png")) != 0) { return r; }
  if ((r = load_bitmap(&dog.cursor, "data/cursor.png")) != 0) { return r; }

  dog.black = al_map_rgb(0, 0, 0);
  dog.white = al_map_rgb(255, 255, 255);

  bool redraw = true;
  al_clear_to_color(al_map_rgb(0,0,0));
  al_flip_display();
  al_start_timer(dog.timer);

  title_page(&dog);

  /* while (1) { */
  /*   ALLEGRO_EVENT ev; */
  /*   al_wait_for_event(dog.event_queue, &ev); */

  /*   if (ev.type == ALLEGRO_EVENT_TIMER) { */
  /*     redraw = true; */
  /*   } else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { */
  /*     break; */
  /*   } */

  /*   if (redraw && al_is_event_queue_empty(dog.event_queue)) { */
  /*     redraw = false; */
  /*     al_clear_to_color(dog.black); */
  /*     al_draw_bitmap(dog.title, 0, 0, 0); */
  /*     al_draw_text(dog.font, dog.white, dog.w/2, dog.h/2-58, ALLEGRO_ALIGN_CENTRE, "Dogfight by Gregory McIntyre"); */
  /*     al_flip_display(); */
  /*   } */
  /* } */

  al_destroy_timer(dog.timer);
  al_destroy_display(dog.display);
  al_destroy_event_queue(dog.event_queue);

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
