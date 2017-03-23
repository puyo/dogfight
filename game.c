// FUNCTIONS THAT ARE ESSENTIAL TO THE ACTUAL GAME

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include "dogfight.h"


#define PLANE_SIZE_W          32                  // }
#define PLANE_SIZE_H          16                  // } these are set and should not
#define SHOT_SIZE_W            4                  // } be fiddled with by mere mortals
#define SHOT_SIZE_H            4                  // } (unless you know what you're doing)
#define CLOUD_SIZE_W         300                  // }
#define CLOUD_SIZE_H         200                  // }
#define EXPLOSION_SIZE_W     128                  // }
#define EXPLOSION_SIZE_H     128                  // }
#define PARACHUTE_SIZE_W      32                  // }
#define PARACHUTE_SIZE_H      32                  // }

#define MAX_PLAYERS            4

// fixed player constants (shouldn't be changed)
#define P1_START_X           (SCREEN_W * 1/4)     // }
#define P1_START_Y           (SCREEN_H * 1/5)     // } starting positions
#define P2_START_X           (SCREEN_W * 3/4)     // }
#define P2_START_Y           (SCREEN_H * 2/5)     // }
#define P3_START_X           (SCREEN_W * 1/4)     // }
#define P3_START_Y           (SCREEN_H * 3/5)     // }
#define P4_START_X           (SCREEN_W * 3/4)     // }
#define P4_START_Y           (SCREEN_H * 4/5)     // }

#define PLANE_HIT_RADIUS     (PLANE_SIZE_H/2)     // } used with circular hit detection
#define PARACHUTE_HIT_RADIUS (PARACHUTE_SIZE_H/2-5) // }
#define SHOT_HIT_RADIUS      (SHOT_SIZE_W/2)      // }
#define PLANE_HIT_LIFE         7                  // how many frames of explosion before the plane
// is no longer drawn underneath (and is null)
#define SPLAT_LIFE            40

// variable level constants
#define PARA_VELOCITY          6                  // parachute terminal velocity


// colours (NB: these are values for "base" colours which are the darkest versions
//              of these colours)
// to brighten a colour, add a number between 1 and 15 to it
// adding 16 will choose the next colour - this number (16) is defined as PAL_DIFFERENCE
#define GREY                  16
#define RED                   32
#define BLUE                  48
#define YELLOW                64
#define PURPLE                80
#define GREEN                 96
#define ORANGE               112

#define PAL_DIFFERENCE        16                // difference between palette colours
                                                // (ie. RED + PAL_DIFFERENCE = BLUE
                                                // see palette order above)

#define GORE_LEVEL             5                // controls various things such as
                                                // how high/wide blood spurts after
                                                // a splat (<20 but <5 recommended Wil!)
#define BLOOD                (100*GORE_LEVEL)   // amount of blood


// ***** TYPE DEFINITIONS ***************************************************************

enum type_player_status {
  NORMAL,
  GONE
};

enum type_parachute_status {
  IN_COCKPIT,
  HAPPY,
  WORRIED,
  SPLATTED
};

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

// structure contains info required for parachutists
typedef struct parachuteinfo {
  float x, y;
  float speed;
  char status;
  float splatting;
  char mirror;
  //double rocking_angle;
  int blood[6][BLOOD+2]; // stores x, y, x speed, y speed
} parachuteinfo;

// structure contains all information required for one shot
typedef struct shotinfo {
  char life;
  float x, y;
  float speed;
  double heading;
  char laser_length;
} shotinfo;

// structure contains all information required for each player
typedef struct playerinfo {
  float x, y;
  float speed;
  double heading;
  int start_x, start_y;

  char key_up, key_down, key_left, key_right, key_fire;

  int colour;
  char flip;
  char status;
  float exploding;
  char invincible;
  char kills[5];              // room for total kills

  struct shotinfo shot[256];  // room for up to 256 shots
  struct parachuteinfo parachute;

} playerinfo;

// ***** MAIN PROGRAM CODE **************************************************************

void randomize_array(int *array, int end, float rand_min, float rand_max)
{
  // function randomizes (min..max) a 1 dimensional array from 1 to end
  // random numbers are more likely to be around the average of the min and max
  // so that when displayed on screen, random pixels don't appear in a 'block'

  int r, rand_count;
  float rand_mid;

  rand_mid = (rand_max + rand_min)/2;

  for (rand_count = 0; rand_count <= end; rand_count++) {
    // square
    //r = rand_mid + (random() % (int)(rand_max - rand_mid + 1));
    // circular
    r = rand_mid
      + ((random() % (int)(rand_max - rand_mid))
         * cos((float)((random() % 180) + 1) * M_PI/180));

    array[rand_count] = r;
  }
}



void draw_clouds(ALLEGRO_BITMAP *scrbuffer, DATAFILE *object_data, struct optionsinfo options)
{
  // function draws clouds onto the given bitmap
  // (or other objects that can be passed through)

  color_map = &trans_table;
  draw_trans_sprite(scrbuffer, object_data[options.cloud_type].dat,
                    SCREEN_W*1/4 - 100, SCREEN_H*3/4 - 60);
  draw_trans_sprite(scrbuffer, object_data[options.cloud_type].dat,
                    SCREEN_W*3/4 - 200, SCREEN_H*1/4 - 60);
}



void draw_stats(ALLEGRO_BITMAP *scrbuffer, struct playerinfo *player, struct optionsinfo options)
{
  // function draws "speed meters" on the given bitmap (screen buffer)
  // (and any other information needed)

  int meter_bar_start, meter_start, meter_len, speed_dif;
  char count, shot_count, kill_count;
  char speed_string[30];
  char kills_string[options.players][30];
  char shot_string[options.players][30];

  speed_dif = options.max_speed - options.min_speed;
  meter_len = SCREEN_W/options.players;
  for (count = 0; count < options.players; count++) {
    // draw speed meters
    meter_bar_start = count * meter_len;
    rect(scrbuffer, meter_bar_start,   7, meter_bar_start   + meter_len-1 , 17,
         PAL_DIFFERENCE*(count+2) + 2);
    rect(scrbuffer, meter_bar_start+1, 8, meter_bar_start+1 + meter_len-2 , 16,
         PAL_DIFFERENCE*(count+2) + 7);

    if (options.min_speed < 0) {
      meter_start = (meter_bar_start+2)-(meter_len-4)*options.min_speed/speed_dif;
      rectfill(scrbuffer, meter_start, 9,
               meter_start + (meter_len-4)*player[count].speed/speed_dif, 16,
               PAL_DIFFERENCE*(count+2) + 12);
    }
    else {
      meter_start = (meter_bar_start+2);
      rectfill(scrbuffer, meter_start, 9,
               meter_start + (meter_len-4)*player[count].speed/options.max_speed, 16,
               PAL_DIFFERENCE*(count+2) + 12);
    }

    // print the actual speed on top of each bar
    sprintf(speed_string, "%2.1f", player[count].speed);
    textout_ex(scrbuffer, font, speed_string, count * SCREEN_W/options.players + 8, 9,
               PAL_DIFFERENCE*(count+2), -1);

    // calculate the total kills
    player[count].kills[options.players] = 0;
    for (kill_count = 0; kill_count < options.players; kill_count++)
      player[count].kills[options.players] += player[count].kills[kill_count];

    // print the kills data
    for (kill_count = 0; kill_count <= options.players; kill_count++) {
      snprintf(kills_string[kill_count], 30, "%d", player[count].kills[kill_count]);
      textout_ex(scrbuffer, font, kills_string[kill_count],
                 count * SCREEN_W/options.players + kill_count*24, 9+8,
                 PAL_DIFFERENCE*(kill_count+2)+15, -1);
    }

    // print status - debugging information
    //if (player[count].status == GONE)
    //  textout(scrbuffer, font, "You're dead", count * SCREEN_W/options.players, 9+16,
    //          GREY+15);
    //else
    //  textout(scrbuffer, font, "You're not dead", count * SCREEN_W/options.players, 9+16,
    //          GREY+15);

    // print blood y coodinate
    //itoa(player[count].parachute.blood[3][0], kills_string[0], 10);
    //textout(scrbuffer, font, kills_string[0], count * SCREEN_W/options.players, 9+32,
    //        GREY+15);

    // print parachute status
    //itoa(player[count].parachute.status, kills_string[0], 10);
    //textout(scrbuffer, font, kills_string[0], count * SCREEN_W/options.players, 9+40,
    //        GREY+15);

    // print shot data
    //for (shot_count = 0; shot_count < options.num_of_shots; shot_count++) {
    //   itoa(player[count].shot[shot_count].life, shot_string[shot_count], 10);
    //   textout(scrbuffer, font, shot_string[shot_count],
    //           count * SCREEN_W/options.players + shot_count*24, 9+16, GREY+15);
    //}

    // print heading data
    //sprintf(speed_string, "%1.4f", player[count].heading);
    //sprintf(kills_string[0], "x vel = %20.4f",
    //        cos(player[count].heading) * player[count].speed);
    //sprintf(kills_string[1], "y vel = %20.4f",
    //        sin(player[count].heading) * player[count].speed);
    //textout(scrbuffer, font, speed_string, count * SCREEN_W/options.players, 9+16,
    //        GREY+15);
    //textout(scrbuffer, font, kills_string[0], count * SCREEN_W/options.players, 9+24,
    //        GREY+15);
    //textout(scrbuffer, font, kills_string[1], count * SCREEN_W/options.players, 9+32,
    //        GREY+15);
  }
}



void paint(ALLEGRO_BITMAP *sprite, int colour_new)
{
  // function checks each pixel of a bitmap and on finding red pixels, changes them
  // to the given colour equivalent

  int x, y, colour_old, colour_equiv;

  for (y = 0; y <= sprite->h; y++) {
    for (x = 0; x <= sprite->w; x++) {
      colour_old = getpixel(sprite, x, y);
      if (colour_old >= RED && colour_old <= RED+15) { // is able to be changed
        colour_equiv = colour_new + (colour_old-32);
        putpixel(sprite, x, y, colour_equiv);
      }
    }
  }

}



void draw_parachute(ALLEGRO_BITMAP *scrbuffer, DATAFILE *main_data, struct playerinfo *player,
                    struct optionsinfo options, char count)
{
  // function draws parachutes and splats

  int blood_count;
  char kills_string[10][50];

  if (!player[count].parachute.splatting) {
    // parachutes
    ALLEGRO_BITMAP *parabuffer;

    parabuffer = create_bitmap(PARACHUTE_SIZE_W, PARACHUTE_SIZE_H);
    clear(parabuffer);

    // copy the parachute bitmap onto a buffer
    if (player[count].parachute.status == HAPPY)
      draw_sprite(parabuffer, main_data[PARACHUTE01].dat, 0, 0);
    else
      draw_sprite(parabuffer, main_data[PARACHUTE02].dat, 0, 0);

    // paint the parachute
    paint(parabuffer, PAL_DIFFERENCE*(count+2));

    // draw the parachute
    if (player[count].parachute.mirror <= 2)
      draw_sprite_h_flip(scrbuffer, parabuffer, player[count].parachute.x,
                         player[count].parachute.y);
    else if (player[count].parachute.mirror > 2)
      draw_sprite(scrbuffer, parabuffer, player[count].parachute.x,
                  player[count].parachute.y);

    destroy_bitmap(parabuffer);
  }
  else if (player[count].parachute.splatting) {
    // splats
    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
    for (blood_count = 0; blood_count < BLOOD; blood_count++)
      circlefill(scrbuffer, player[count].parachute.blood[0][blood_count],
                 player[count].parachute.blood[1][blood_count], 1,
                 player[count].parachute.blood[4][blood_count]);
    drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);
  }

}




void draw_shots(ALLEGRO_BITMAP *scrbuffer, struct playerinfo *player, struct optionsinfo options,
                char count)
{
  char shot_count;
  int x, y;

  for (shot_count = 0; shot_count < options.num_of_shots; shot_count++) {
    if (player[count].shot[shot_count].life > 0) {
      if (options.vehicle == 0 || options.vehicle == 2) {
        x = player[count].shot[shot_count].x+SHOT_SIZE_W/2;
        y = player[count].shot[shot_count].y+SHOT_SIZE_H/2;
        circlefill(scrbuffer, x, y, SHOT_HIT_RADIUS,
                   ORANGE - (player[count].shot[shot_count].life * (ORANGE-RED)/options.shot_life));
        circle(scrbuffer, x, y, SHOT_HIT_RADIUS,
               RED + (player[count].shot[shot_count].life * RED/options.shot_life));
      }
      else if (options.vehicle == 1) {
        // here x and y store the second point of the 'laser line'
        x = player[count].shot[shot_count].x
          + cos(player[count].shot[shot_count].heading)
          * player[count].shot[shot_count].laser_length;
        y = player[count].shot[shot_count].y
          + sin(player[count].shot[shot_count].heading)
          * player[count].shot[shot_count].laser_length;
        line(scrbuffer, player[count].shot[shot_count].x,
             player[count].shot[shot_count].y, x, y,
             ORANGE - (player[count].shot[shot_count].life * (ORANGE-RED)/options.shot_life));
      }
    }
  }
}



void draw_screen(ALLEGRO_BITMAP *scrbuffer, DATAFILE *main_data, DATAFILE *plane_data,
                 DATAFILE *explosion_data, DATAFILE *object_data,
                 struct playerinfo *player, struct optionsinfo options)
{
  // function draws the game screen

  ALLEGRO_BITMAP *spritebuffer;
  double angle256;
  char explosion;
  int count;
  float explosion_x, explosion_y;

  // allocate memory for sprite buffer
  spritebuffer = create_bitmap(PLANE_SIZE_W, PLANE_SIZE_H);

  // clear screen buffer to the background
  if (options.vehicle == 2)
    clear_to_color(scrbuffer, GREEN);
  else
    blit(main_data[options.backdrop].dat, scrbuffer, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

  // draw planes and plane related objects (eg. shots, parachutes) on buffer
  for (count = 0; count < options.players; count++) {

    // draw the planes
    if (player[count].exploding < PLANE_HIT_LIFE && player[count].status != GONE) {
      // clear the sprite buffer (this is a loop - other planes could be drawn in it)
      clear(spritebuffer);

      // copy the plane bitmap onto a sprite buffer, flipping it if necessary
      if (player[count].flip)
        draw_sprite_v_flip(spritebuffer, plane_data[options.vehicle].dat, 0, 0);
      else
        draw_sprite(spritebuffer, plane_data[options.vehicle].dat, 0, 0);

      // paint the planes different colours
      if (player[count].invincible)
        paint(spritebuffer, RED + (player[count].invincible
                                   * ORANGE/options.invincibility_life) * (count+1)/(options.players+1));
      else
        paint(spritebuffer, PAL_DIFFERENCE*(count+2));

      // calculate the angle used by rotate_sprite (0 - 256)
      angle256 = itofix(player[count].heading * 256/(2*M_PI));

      // debugging information
      // CIRCULAR HIT DETECTION CIRCLE
      //circle(scrbuffer, player[count].x+PLANE_SIZE_W/2, player[count].y+PLANE_SIZE_H/2,
      //       PLANE_HIT_RADIUS, GREEN+15);

      // draw the plane
      rotate_sprite(scrbuffer, spritebuffer, player[count].x, player[count].y, angle256);

      // debugging information
      // cross hair marking centre of each plane
      //line(scrbuffer, player[count].x+PLANE_SIZE_W/2, player[count].y+PLANE_SIZE_H/2-5,
      //     player[count].x+PLANE_SIZE_W/2, player[count].y+PLANE_SIZE_H/2+5, GREY+15);
      //line(scrbuffer, player[count].x+PLANE_SIZE_W/2-5, player[count].y+PLANE_SIZE_H/2,
      //     player[count].x+PLANE_SIZE_W/2+5, player[count].y+PLANE_SIZE_H/2, GREY+15);

      // cross hair marking front of each plane
      //line(scrbuffer, player[count].x+PLANE_SIZE_W/2, player[count].y+PLANE_SIZE_H/2-5,
      //     player[count].x+PLANE_SIZE_W/2, player[count].y+PLANE_SIZE_H/2+5, GREY+15);
      //line(scrbuffer, player[count].x+PLANE_SIZE_W/2-5, player[count].y+PLANE_SIZE_H/2,
      //     player[count].x+PLANE_SIZE_W/2+5, player[count].y+PLANE_SIZE_H/2, GREY+15);

      // optionally draw an explosion on top
      if (player[count].exploding && player[count].exploding < 10) {
        explosion = player[count].exploding-1;
        explosion_x = (player[count].x + PLANE_SIZE_W/2) - EXPLOSION_SIZE_W/2;
        explosion_y = (player[count].y + PLANE_SIZE_H/2) - EXPLOSION_SIZE_H/2;

        rotate_sprite(scrbuffer, explosion_data[explosion].dat, explosion_x, explosion_y,
                      angle256);
      }
      // draw any shots
      draw_shots(scrbuffer, &player[0], options, count);

    }

    if (player[count].parachute.status == HAPPY
        || player[count].parachute.status == WORRIED) {
      // draw a nice, safe, pleasant little parachute

      // debugging information
      // CIRCULAR HIT DETECTION CIRCLE
      //circle(scrbuffer, player[count].parachute.x+PARACHUTE_SIZE_W/2,
      //       player[count].parachute.y+PARACHUTE_SIZE_H/2, PARACHUTE_HIT_RADIUS,
      //       GREEN+15);
      draw_parachute(scrbuffer, main_data, &player[0], options, count);
    }

    if (player[count].exploding >= PLANE_HIT_LIFE) {
      // draw an explosion *instead* of the plane
      // goes last to 'cover' parachutes etc
      explosion = player[count].exploding-1;
      explosion_x = (player[count].x + PLANE_SIZE_W/2) - EXPLOSION_SIZE_W/2;
      explosion_y = (player[count].y + PLANE_SIZE_H/2) - EXPLOSION_SIZE_H/2;
      angle256 = itofix(player[count].heading * 128/M_PI);
      rotate_sprite(scrbuffer, explosion_data[explosion].dat, explosion_x, explosion_y,
                    angle256);
    }

  }

  // draw everything else
  if (options.vehicle != 2)
    draw_clouds(scrbuffer, object_data, options);

  draw_stats(scrbuffer, &player[0], options);

  blit(scrbuffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

  destroy_bitmap(spritebuffer);
}



void explode_player(struct playerinfo *player, char player_count)
{
  // function blows the sucker up!
  // looks meek, but certain other things used to be done here and
  // i've left it 'in case'

  player[player_count].exploding = 1;
}



// this troublesome function was meant to be a callback function so that
// every point along the laser line could be checked for collisions
// it most certainly does not work, because you can't pass the information
// required to do the collision detection to the function
// why not? allegro does not support more than those arguments it sees as
// necessary
//void line_check(ALLEGRO_BITMAP *scrbuffer, int x, int y, int n)//, struct playerinfo *player,
//                char plane_count, char check_count)
//{
// function is a callback function used to check each pixel point on the 'laser lines'

// if ( sqrt( pow((x - (player[check_count].x+PLANE_SIZE_W/2)), 2)
//          + pow((y - (player[check_count].y+PLANE_SIZE_H/2)), 2) )
//  <= PLANE_HIT_RADIUS)
//   {
//explode_player(&player[0], plane_count, check_count);
//    rest(2000);
//   }
//}



void detect_collisions(ALLEGRO_BITMAP *scrbuffer, struct playerinfo *player,
                       struct optionsinfo options)
{
  // function checks to see if any planes or shots overlap to a certain extent

  char plane_count, check_count, shot_count;
  int shot_x2, shot_y2, n;

  for (plane_count = 0; plane_count < options.players; plane_count++) {

    if (player[plane_count].status != GONE) {
      for (check_count = 0; check_count < options.players; check_count++) {

        if (check_count != plane_count
            && player[plane_count].status != GONE
            && player[check_count].status != GONE
            && !player[plane_count].exploding) {

          // CIRCULAR HIT DETECTION METHOD - working a lot better :)
          // PLANES
          // if plane hits plane
          if ( sqrt( pow(((player[plane_count].x + PLANE_SIZE_W/2)
                          - (player[check_count].x + PLANE_SIZE_W/2)), 2)
                     + pow(((player[plane_count].y + PLANE_SIZE_H/2)
                            - (player[check_count].y + PLANE_SIZE_H/2)), 2) )
               <= PLANE_HIT_RADIUS * 2 ) // because we're talking about 2 planes,
            // not a point and a plane
            {
              if (!player[check_count].invincible && !player[check_count].exploding)
                explode_player(&player[0], check_count);
              if (!player[plane_count].invincible && !player[plane_count].exploding)
                explode_player(&player[0], plane_count);
            }

          // SHOTS
          //           if (options.vehicle == 1) {
          //             // laser collision detection
          //             shot_x2 = player[plane_count].shot[shot_count].x
          //                       + cos(player[plane_count].shot[shot_count].heading)
          //                       * player[plane_count].shot[shot_count].laser_length;
          //             shot_y2 = player[plane_count].shot[shot_count].y
          //                       + sin(player[plane_count].shot[shot_count].heading)
          //                       * player[plane_count].shot[shot_count].laser_length;

          //             do_line(scrbuffer, player[plane_count].shot[shot_count].x,
          //                     player[plane_count].shot[shot_count].y, shot_x2, shot_y2, n,
          //                     line_check);
          //           }
          //           else {
          // normal shot collision detection
          for (shot_count = 0; shot_count < options.num_of_shots; shot_count++) {
            if (player[plane_count].shot[shot_count].life
                && !player[check_count].invincible
                && !player[check_count].exploding) {

              // hit detection lines - very interesting to turn on and see!!!
              //line(scrbuffer, player[plane_count].shot_x+SHOT_SIZE_W/2,
              //     player[plane_count].shot_y+SHOT_SIZE_H/2,
              //     player[check_count].x+PLANE_SIZE_W/2,
              //     player[check_count].y+PLANE_SIZE_H/2, YELLOW+15);

              // if shot hits plane
              if ( sqrt( pow(((player[plane_count].shot[shot_count].x+SHOT_SIZE_W/2)
                              - (player[check_count].x+PLANE_SIZE_W/2)), 2)
                         + pow(((player[plane_count].shot[shot_count].y+SHOT_SIZE_H/2)
                                - (player[check_count].y+PLANE_SIZE_H/2)), 2) )
                   <= PLANE_HIT_RADIUS + SHOT_HIT_RADIUS )
                {
                  explode_player(&player[0], check_count);
                  player[plane_count].kills[check_count]++;
                }
            }
          }
          //           }
        } // end if not own plane (so planes can't shoot themselves)

      } // end check_count
    } // end if not dead

    // PARACHUTES
    if (player[plane_count].parachute.status == HAPPY
        && player[plane_count].parachute.speed > 0
        && !options.coralie) {
      for (check_count = 0; check_count < options.players; check_count++) {
        if (check_count != plane_count && player[check_count].status != GONE) {
          // if plane hits parachute
          if ( sqrt( pow(((player[plane_count].parachute.x+PARACHUTE_SIZE_W/2)
                          - (player[check_count].x+PLANE_SIZE_W/2)), 2)
                     + pow(((player[plane_count].parachute.y+PARACHUTE_SIZE_H/2)
                            - (player[check_count].y+PLANE_SIZE_H/2)), 2) )
               <= PARACHUTE_HIT_RADIUS + PLANE_HIT_RADIUS )
            {
              player[plane_count].parachute.status = WORRIED;
            }
  
          for (shot_count = 0; shot_count < options.num_of_shots; shot_count++) {
            if (player[check_count].shot[shot_count].life) {
              if ( sqrt( pow(((player[plane_count].parachute.x+PARACHUTE_SIZE_W/2)
                              - (player[check_count].shot[shot_count].x+SHOT_SIZE_W/2)), 2)
                         + pow(((player[plane_count].parachute.y+PARACHUTE_SIZE_H/2)
                                - (player[check_count].shot[shot_count].y+SHOT_SIZE_H/2)), 2) )
                   <= PARACHUTE_HIT_RADIUS + SHOT_HIT_RADIUS )
                {
                  player[plane_count].parachute.status = WORRIED;
                }
            } // end if shot alive
          } // end shot count
        } // end if not same plane
      } // end check count
    } // end if parachute happy

  } // end plane_count

}



void reset_plane(struct playerinfo *player, char count, struct optionsinfo options)
{
  char shot_count;

  player[count].status = NORMAL;
  if (options.min_speed < 0)
    player[count].speed = 0;
  else
    player[count].speed = options.min_speed;

  for (shot_count = 0; shot_count < options.num_of_shots; shot_count++)
    player[count].shot[shot_count].life = 0;

  player[count].x = player[count].start_x;
  player[count].y = player[count].start_y;

  player[count].invincible = options.invincibility_life;
  player[count].exploding = 0;

  // reset parachute
  player[count].parachute.status  = IN_COCKPIT;
  player[count].parachute.mirror  = 0;
  player[count].parachute.x       = -1;
  player[count].parachute.y       = -1;

  if (!player[count].flip)
    player[count].heading = 0;
  else
    player[count].heading = M_PI;
}



void initiate_variables(struct playerinfo *player, struct optionsinfo options)
{
  char count, sub_count;

  // function sets options defaults (these override options read from the cfg file)
  // therefore what they should do is *use* the options stucture so that
  // options read from the cfg still have an effect

  player[0].start_x     = P1_START_X;
  player[0].start_y     = P1_START_Y;
  player[0].heading     = 0;
  player[0].key_up      = KEY_UP;
  player[0].key_down    = KEY_DOWN;
  player[0].key_left    = KEY_LEFT;
  player[0].key_right   = KEY_RIGHT;
  player[0].key_fire    = KEY_SLASH;
  player[0].flip        = 0;

  player[1].start_x     = P2_START_X;
  player[1].start_y     = P2_START_Y;
  player[1].heading     = M_PI;
  player[1].key_up      = KEY_W;
  player[1].key_down    = KEY_S;
  player[1].key_left    = KEY_A;
  player[1].key_right   = KEY_D;
  player[1].key_fire    = KEY_Q;
  player[1].flip        = 1;

  player[2].start_x     = P3_START_X;
  player[2].start_y     = P3_START_Y;
  player[2].heading     = 0;
  player[2].key_up      = KEY_Y;
  player[2].key_down    = KEY_H;
  player[2].key_left    = KEY_G;
  player[2].key_right   = KEY_J;
  player[2].key_fire    = KEY_T;
  player[2].flip        = 0;

  player[3].start_x     = P4_START_X;
  player[3].start_y     = P4_START_Y;
  player[3].heading     = M_PI;
  player[3].key_up      = KEY_P;
  player[3].key_down    = KEY_COLON;
  player[3].key_left    = KEY_L;
  player[3].key_right   = KEY_QUOTE;
  player[3].key_fire    = KEY_O;
  player[3].flip        = 1;

  for (count = 0; count < MAX_PLAYERS; count++) {
    player[count].x                   = player[count].start_x;
    player[count].y                   = player[count].start_y;
    player[count].status              = NORMAL;
    player[count].exploding           = 0;

    if (options.min_speed < 0)
      player[count].speed = 0;
    else
      player[count].speed = options.min_speed;

    player[count].invincible          = options.invincibility_life;
    player[count].parachute.status    = IN_COCKPIT;
    player[count].parachute.mirror    = 0;
    player[count].parachute.x         = -1;
    player[count].parachute.y         = -1;
    player[count].parachute.splatting = 0;

    for (sub_count = 0; sub_count < options.num_of_shots; sub_count++)
      player[count].shot[sub_count].laser_length = options.laser_length;

    for (sub_count = 0; sub_count < options.num_of_shots; sub_count++)
      player[count].shot[sub_count].life = 0;

    for (sub_count = 0; sub_count < MAX_PLAYERS; sub_count++)
      player[count].kills[sub_count] = 0;
  }
}



char move_planes(struct playerinfo *player, struct optionsinfo options)
{
  // function checks to see what keys are being pressed and moves
  // the planes appropriatly

  char count, shot_count, shot_check, last_shot;
  int c, count_x, count_y, blood_count;
  float rand_min, rand_max;

  for (count = 0; count < options.players; count++) {

    if (player[count].status != GONE) {
      // up
      if (key[player[count].key_up]/* && player[count].speed < options.max_speed*/)
        player[count].speed += options.acceleration;
      // down
      else if (key[player[count].key_down]/* && player[count].speed > options.min_speed*/)
        player[count].speed -= options.acceleration;
      if (player[count].speed > options.max_speed)
        player[count].speed = options.max_speed;
      else if (player[count].speed < options.min_speed)
        player[count].speed = options.min_speed;

      // natural decceleration - has not been implemented, because
      // although for 1 player this is quite good, when 4 players have
      // to hold 'up' to maintain their velocity, the keyboard freezes up
      //else if (!key[player[count].key_up] && player[count].speed > options.min_speed)
      //  player[count].speed -= options.acceleration;

      // right
      if (key[player[count].key_right])
        player[count].heading += options.turn_speed;

      // left
      if (key[player[count].key_left])
        player[count].heading -= options.turn_speed;

      // free any finished shot elements
      shot_count = 0;
      for (shot_count = 0; shot_count < options.num_of_shots; shot_count++) {
        if (player[count].shot[shot_count].life > options.shot_life)
          player[count].shot[shot_count].life = 0;
      }
      // fire
      if (key[player[count].key_fire]) {
        // find a free "slot" - a free array element to store more shot info in
        shot_count = 0;
        while (shot_count < options.num_of_shots
               && player[count].shot[shot_count].life) {
          shot_count++;
        };

        if (!player[count].shot[shot_count].life) {
          // find the last shot - to tell if it's been long enough since last firing
          // find the earliest shot
          last_shot = 0;
          for (shot_check = 0; shot_check < options.num_of_shots; shot_check++) {
            if (player[count].shot[shot_check].life > last_shot
                && player[count].shot[shot_check].life)
              last_shot = player[count].shot[shot_check].life;
          }
          if (last_shot == 0 || last_shot < options.shot_life - options.shot_lag) {
            // fire!! - set up shot info
            player[count].shot[shot_count].life    = options.shot_life;
            player[count].shot[shot_count].x       = (player[count].x+PLANE_SIZE_W/2)
              + (cos(player[count].heading));
            player[count].shot[shot_count].y       = (player[count].y+PLANE_SIZE_H/2)
              + (sin(player[count].heading));
            player[count].shot[shot_count].speed   = options.shot_base_speed
              + player[count].speed;
            player[count].shot[shot_count].heading = player[count].heading;
          }
        }
      }

      // 0 <= heading <= 2pi (or 360 degrees)
      if (player[count].heading >= 2*M_PI)
        player[count].heading -= 2*M_PI;
      else if (player[count].heading < 0)
        player[count].heading += 2*M_PI;

      // flip the planes if they go upsidedown (so they don't!)
      /*
        if (player[count].heading >= M_PI/2 && player[count].heading <= M_PI*3/2)
        player[count].flip = 1;
        else if ((player[count].heading >= 0
        && player[count].heading < M_PI/2)
        || (player[count].heading > M_PI*3/2
        && player[count].heading <= 2*M_PI))
        player[count].flip = 0;
      */

      // calculate the new coodinates of planes
      player[count].x += cos(player[count].heading) * player[count].speed;
      player[count].y += sin(player[count].heading) * player[count].speed;

      // make sure no planes go off the screen (loop the screen)
      if (player[count].x > SCREEN_W)
        player[count].x -= SCREEN_W;
      else if (player[count].x < 0)
        player[count].x += SCREEN_W;

      if (player[count].y > SCREEN_H)
        player[count].y -= SCREEN_H;
      else if (player[count].y < 0)
        player[count].y += SCREEN_H;

      // calculate the new coodinates of shots
      for (shot_count = 0; shot_count < options.num_of_shots; shot_count++) {

        if (player[count].shot[shot_count].life > 0) {
          player[count].shot[shot_count].x += cos(player[count].shot[shot_count].heading)
            * player[count].shot[shot_count].speed;
          player[count].shot[shot_count].y += sin(player[count].shot[shot_count].heading)
            * player[count].shot[shot_count].speed;

          // make sure no shots go off the screen (loop the screen)
          if (player[count].shot[shot_count].x > SCREEN_W)
            player[count].shot[shot_count].x -= SCREEN_W;
          else if (player[count].shot[shot_count].x < 0)
            player[count].shot[shot_count].x += SCREEN_W;

          if (player[count].shot[shot_count].y > SCREEN_H)
            player[count].shot[shot_count].y -= SCREEN_H;
          else if (player[count].shot[shot_count].y < 0)
            player[count].shot[shot_count].y += SCREEN_H;

          // shots deteriorate
          player[count].shot[shot_count].life--;
        }
      }

      // invincibility wears off
      if (player[count].invincible)
        player[count].invincible--;

    } // if player not dead
    else if (player[count].status == GONE
             && (player[count].parachute.status == SPLATTED
                 || player[count].parachute.status == IN_COCKPIT)) {
      if (key[player[count].key_fire])
        reset_plane(&player[0], count, options);
    }

    // explosions "wear off"
    if (player[count].exploding)
      player[count].exploding += options.explosion_speed;
    if (player[count].exploding >= 10) {
      player[count].exploding = 0;
      player[count].status = GONE;
    }

    // splats "wear off"
    if (player[count].parachute.splatting)
      player[count].parachute.splatting++;
    if (player[count].parachute.status == WORRIED
        && player[count].parachute.splatting >= SPLAT_LIFE) {
      player[count].parachute.splatting = 0;
      player[count].parachute.status = SPLATTED;
    }

    // create parachutes
    if (player[count].exploding >= PLANE_HIT_LIFE && options.vehicle == 0) {
      player[count].parachute.status = HAPPY;
      player[count].parachute.x      = player[count].x;
      player[count].parachute.y      = player[count].y;
      player[count].parachute.speed  = -4;
    }

    // calculate the new coodinates of parachutes
    if (player[count].parachute.status != IN_COCKPIT
        && player[count].parachute.y < SCREEN_H) {
      if (player[count].parachute.status == HAPPY) {

        if (player[count].parachute.speed < PARA_VELOCITY)
          player[count].parachute.speed += options.gravity/4;
        else if (player[count].parachute.speed > PARA_VELOCITY)
          player[count].parachute.speed -= options.gravity;
      }
      else if (player[count].parachute.status == WORRIED) {

        player[count].parachute.speed += options.gravity;
        // draw a mirrored version of the parachute every 2nd frame, so it looks
        // like the parachute is 'flailing'
        player[count].parachute.mirror++;
        if (player[count].parachute.mirror >= 5)
          player[count].parachute.mirror = 0;
      }
      player[count].parachute.y += (player[count].parachute.speed);
    }

    if (player[count].parachute.y >= SCREEN_H
        && player[count].parachute.status == WORRIED
        && !player[count].parachute.splatting) {
      // fill the array with appropriate random numbers (coodinates of blood spots, etc)
      // x
      rand_min = player[count].parachute.x + PARACHUTE_SIZE_W * 1/3;
      rand_max = player[count].parachute.x + PARACHUTE_SIZE_W * 2/3;
      randomize_array(&player[count].parachute.blood[0][0], BLOOD, rand_min, rand_max);
      // y
      rand_min = player[count].parachute.y + PARACHUTE_SIZE_H * 1/3;
      rand_max = player[count].parachute.y + PARACHUTE_SIZE_H * 2/3;
      randomize_array(&player[count].parachute.blood[1][0], BLOOD, rand_min, rand_max);
      // x vel
      rand_min = player[count].parachute.speed * GORE_LEVEL * -0.2;
      rand_max = player[count].parachute.speed * GORE_LEVEL *  0.2;
      randomize_array(&player[count].parachute.blood[2][0], BLOOD, rand_min, rand_max);
      // y vel
      rand_min = player[count].parachute.speed * GORE_LEVEL * -0.5;
      rand_max = player[count].parachute.speed * GORE_LEVEL * -0.25;
      randomize_array(&player[count].parachute.blood[3][0], BLOOD, rand_min, rand_max);
      // colour
      rand_min = RED;
      rand_max = RED+15;
      //srandom(player[count].parachute.speed+2);
      randomize_array(&player[count].parachute.blood[4][0], BLOOD, rand_min, rand_max);

      player[count].parachute.splatting = 1;
    }
    else if (player[count].parachute.y >= SCREEN_H
             && player[count].parachute.status == HAPPY)
      player[count].parachute.status = SPLATTED;

    // calculate new coodinates, etc of blood spots
    if (player[count].parachute.splatting) {
      for (blood_count = 0; blood_count < BLOOD; blood_count++) {
        // x, y
        player[count].parachute.blood[0][blood_count]
          += player[count].parachute.blood[2][blood_count];
        player[count].parachute.blood[1][blood_count]
          += player[count].parachute.blood[3][blood_count];
        // y vel
        player[count].parachute.blood[3][blood_count] += options.gravity*3;
      }
    }

  } // end for count

  // screen capture
  if (key[KEY_F12]) {
    save_bitmap("dogscrn.pcx", screen, pal);
    textout_centre_ex(screen, font, "SCREEN SAVED TO FILE \"DOGSCRN.PCX\"", SCREEN_W/2, SCREEN_H/2-8*3, GREY+15, -1);
    textout_centre_ex(screen, font, "Press the spacebar to continue...",
                      SCREEN_W/2, SCREEN_H/2+8, GREY+15, -1);
    do {} while (!key[KEY_SPACE]);
  }

  // help - space to continue
  if (key[KEY_F1]) {
    textout_centre_ex(screen, font, "HELP", SCREEN_W/2, SCREEN_H/2-8*3, GREY+15, -1);
    textout_centre_ex(screen, font, "UP - accelerate, DOWN - decelerate, LEFT/RIGHT - steer",
                      SCREEN_W/2, SCREEN_H/2-8, GREY+15, -1);
    textout_centre_ex(screen, font, "Press the spacebar to continue...",
                      SCREEN_W/2, SCREEN_H/2+8, GREY+15, -1);
    do {} while (!key[KEY_SPACE]);
  }

  // escape
  if (key[KEY_ESC]) {
    // ask if they really want to quit
    textout_centre_ex(screen, font, "Are you sure you want to quit? (Y/N)",
                      SCREEN_W/2, SCREEN_H/2, GREY+15, -1);
    clear_keybuf();
    do {
      c = readkey();
    } while ((c >> 8) != KEY_Y && (c >> 8) != KEY_N);
    if ((c >> 8) == KEY_Y)
      return(1);
  }

  return(0);
}



struct optionsinfo change_options(struct optionsinfo options)
{
  // function changes aspects of the options structure that must be passed
  // between main game functions (as the options structure is not returned
  // from the move planes structure
  // coralie mode
  if (key[KEY_F2]) {
    textout_centre_ex(screen, font, "CORALIE MODE", SCREEN_W/2, SCREEN_H/2-8*3, GREY+15, -1);
    if (!options.coralie) {
      options.coralie = 1;
      textout_centre_ex(screen, font, "Coralie Mode has been turned on",
                        SCREEN_W/2, SCREEN_H/2-8, GREY+15, -1);
    }
    else {
      options.coralie = 0;
      textout_centre_ex(screen, font, "Coralie Mode has been turned off",
                        SCREEN_W/2, SCREEN_H/2-8, GREY+15, -1);
    }

    textout_centre_ex(screen, font, "Press the spacebar to continue...",
                      SCREEN_W/2, SCREEN_H/2+8, GREY+15, -1);
    do {} while (!key[KEY_SPACE]);
  }

  return(options);
}


void game(doginfo *dog, struct optionsinfo options, char players)
{
  // set room for four players because defaults are set for
  // them regardless of how many players participate
  struct playerinfo player[4];

  char count, countdown, end;
  char countdown_string[3];

  options.players = players;

  // reset game variables that may have been changed from previous games
  initiate_variables(&player[0], options);

  // make players their proper colours for the countdown
  for (count = 0; count < options.players; count++) {
    player[count].invincible = 0;
  }

  // do a countdown to build suspense!
  for (countdown = 3; countdown >= 1; countdown--) {
    snprintf(countdown_string, 3, "%d", countdown);
    textout_centre_ex(screen, main_data[FONTLARGE].dat, countdown_string,
                      SCREEN_W/2, SCREEN_H/2, GREY+15, -1);
    rest(1000);
    draw_screen(scrbuffer, main_data, plane_data, explosion_data, object_data, &player[0], options);
  }
  textout_centre_ex(screen, main_data[FONTLARGE].dat, "GO", SCREEN_W/2, SCREEN_H/2, GREEN+15, -1);
  rest(200);

  // reset players to invincible once the countdown has ended
  for (count = 0; count < options.players; count++) {
    player[count].invincible = options.invincibility_life;
  }

  char end = 0;

  while (!end) {
    ALLEGRO_EVENT ev;
    al_wait_for_event(dog->event_queue, &ev);

    if (ev.type == ALLEGRO_EVENT_TIMER) {
      redraw = true;
    } else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
      break;
    }

    if (redraw && al_is_event_queue_empty(event_queue)) {
      redraw = false;

      end = move_planes(&player[0], options);

      // variables not passed back from move_planes such as coralie mode must
      // be changed outside of the function
      options = change_options(options);

      //detect_collisions(scrbuffer, &player[0], options);
      //draw_screen(display, assets, &player[0], options);

      al_flip_display();
    }
  }
}
