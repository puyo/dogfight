/* Dogfight.c                                 */
/* Program:      a dog fighting game          */
/* Author:       Gregory McIntyre             */
/* First edited: 28/08/97                     */

#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

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

// ***** MAIN ***************************************************************************

const float FPS = 60;

int main(int argc, char **argv){

  ALLEGRO_DISPLAY *display = NULL;
  ALLEGRO_EVENT_QUEUE *event_queue = NULL;
  ALLEGRO_TIMER *timer = NULL;
  bool redraw = true;
  float w, h;

  if (!al_init()) {
    fprintf(stderr, "failed to initialize allegro!\n");
    return -1;
  }

  al_init_font_addon(); // initialize the font addon
  al_init_ttf_addon();// initialize the ttf (True Type Font) addon

  ALLEGRO_FONT *font = al_load_ttf_font("data/font.ttf", 12, 0);
  if (!font) {
    fprintf(stderr, "Could not load font.\n");
    return -1;
  }

  timer = al_create_timer(1.0 / FPS);
  if (!timer) {
    fprintf(stderr, "failed to create timer!\n");
    return -1;
  }

  display = al_create_display(640, 480);
  if (!display) {
    fprintf(stderr, "failed to create display!\n");
    al_destroy_timer(timer);
    return -1;
  }

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

      al_draw_text(font, al_map_rgb(255, 255, 255), w/2, h/2-58, ALLEGRO_ALIGN_CENTRE, "Dogfight by Gregory McIntyre");

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

/* int main_x(void) */
/* { */
/*  //set_palette(pal); */
/*  //set_color(0, &black); */

/*  // build a colour lookup table for translucent drawing */
/*  // NB: 128 translucency = 50% */
/*  //textout_centre_ex(screen, main_data[FONTSMALL].dat, "Dogfight by Gregory McIntyre", SCREEN_W/2, SCREEN_H/2-58, GREY+15, -1); */
/*  //textout_centre_ex(screen, font, "Loading. Please wait...", SCREEN_W/2, SCREEN_H/2-20, GREY+15, -1); */
/*  callback_count = 0; */
/*  //create_trans_table(&trans_table, main_data[MAINPAL].dat, CLOUD_TRANSPARENCY, CLOUD_TRANSPARENCY, CLOUD_TRANSPARENCY, callback_func); */

/*  // allocate memory for screen buffer */
/*  // scrbuffer = create_bitmap(SCREEN_W, SCREEN_H); clear(scrbuffer); */

/*  // when everything is ready fade out and go to the title screen */
/*  //fade_out(10); */
/*  //title_page(scrbuffer, main_data); */
/* } */
