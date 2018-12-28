#ifndef __DOGFIGHT_H
#define __DOGFIGHT_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>

#define SCALE 3

typedef struct doginfo {
  ALLEGRO_DISPLAY *display;
  ALLEGRO_EVENT_QUEUE *event_queue;
  ALLEGRO_TIMER *timer;
  ALLEGRO_FONT *font;
  ALLEGRO_BITMAP *bg;
  ALLEGRO_BITMAP *explosion[8];
  ALLEGRO_BITMAP *plane[3];
  ALLEGRO_BITMAP *cloud;
  ALLEGRO_BITMAP *parachute;
  ALLEGRO_BITMAP *parafall;
  ALLEGRO_BITMAP *title;
  ALLEGRO_BITMAP *shot;
  ALLEGRO_BITMAP *cursor;
  ALLEGRO_COLOR black;
  ALLEGRO_COLOR white;
  uint w, h;
} doginfo;

typedef struct optionsinfo {
  unsigned int vehicle;
  unsigned int backdrop;
  unsigned int players;

  float turn_speed;
  float acceleration;
  unsigned int min_speed;
  unsigned int max_speed;
  unsigned int num_of_shots;
  unsigned int shot_lag;
  unsigned int shot_life;
  unsigned int shot_base_speed;
  unsigned int laser_length;

  float gravity;
  unsigned int invincibility_life;
  float explosion_speed;
  unsigned int cloud_type;

  bool coralie;
} optionsinfo;


void game(doginfo *dog, optionsinfo *options);

void title_page(doginfo *dog);

#endif
