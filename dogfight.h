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
  char vehicle;
  char backdrop;
  char players;

  float turn_speed;
  float acceleration;
  char min_speed;
  char max_speed;
  char num_of_shots;
  char shot_lag;
  char shot_life;
  char shot_base_speed;
  char laser_length;

  float gravity;
  char invincibility_life;
  float explosion_speed;
  char cloud_type;

  char coralie;
} optionsinfo;


void game(doginfo *dog, optionsinfo *options);

void title_page(doginfo *dog);

#endif
