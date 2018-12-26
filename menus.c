// FUNCTIONS USED TO DISPLAY THE TITLE SCREEN AND MENUS

#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "dogfight.h"

char *strlwr(char *str)
{
  unsigned char *p = (unsigned char *)str;
  while (*p) {
    *p = tolower((unsigned char)*p);
    p++;
  }
  return str;
}

#define NUM_OF_VEHICLES       3

#define TITLE_ROW             6

// ***** TYPE DEFINITIONS ***************************************************************

// structure contains info for the menu on the title page & sub menus
typedef struct menuinfo {
  // the current highlighted option
  char highlight;
  // the option selected
  char selection;
  // whether the selection should increment or decrement
  char selectupdn;
  char num_of_options;
  int row;
  int col;
  char fontsize;
  // room for max of 15 options with 50 characters each
  char option[20][50];
  // these are for the options menu, where something different happens for each option
  double optionval[20];
  char optionval_str[20][8];
} menuinfo;


// ***** MAIN PROGRAM CODE **************************************************************

void fixed_str(char *string, int num, int places)
{
  // function converts a number (int) to a string with a fixed number of places
  // filling in leading space with zeros

  char temp[places];
  int count;

  strcpy(string, "");
  strcpy(temp, "");

  // copy the number to the temporary string
  sprintf(temp, "%d", num);

  // add required zeros to the real string first
  for (count = 0; count < (places - strlen(temp)); count++)
    strcat(string, "0");

  // copy the temp string (the number) onto the end of any zeros
  strcat(string, temp);
}



double read_value(FILE *fp, char headchar_spef, char *heading, char *string)
{
  // function reads the given line of the cfg file, returning the value
  // assumes file is already open for reading and that the line represents
  // a value (not a string) - if a string is found it will return 0
  // if the line being looked for is not found, the function returns a NULL pointer

  char headchar;            // first character of the line
  char check_str[256];      // each whole line read from file
  char value_str[256];      // the string containing the value read
  int equals;               // position of the assignment operator '='
  char c;                   // used to copy parts of the strings
  char *end;                // pointer to the last character of the string
  // read that is not used in the value
  int count;
  double value;

  rewind(fp);

  // read every line of the file until the heading is found
  do {
    // read the next line from the file
    fgets(check_str, 256, fp);
    strcpy(check_str, strlwr(check_str));
    headchar = check_str[0];
  } while (strstr(check_str, heading) == NULL
           && (!feof(fp) || headchar != headchar_spef)
           && !feof(fp));

  if (strstr(check_str, heading) != NULL) {
    // read every line of the file until the string is matched
    do {
      // read the next line from the file
      fgets(check_str, 256, fp);
      strcpy(check_str, (char *)strlwr(check_str));
      headchar = check_str[0];
    } while (strstr(check_str, string) == NULL && headchar != headchar_spef && !feof(fp));

    if (strstr(check_str, string) != NULL) {

      // find '=' in the string
      equals = 0;
      while (check_str[equals] != '=') {
        equals++;
      };

      // copy the rest of the string to the value string
      count = 0;
      c = ' ';
      strcpy(value_str, "0\n");
      do {
        count++;
        c = check_str[equals+count];
        value_str[count-1] = c;
      } while (check_str[equals+count+1] != '\n');
      value_str[count] = '\n';

      // convert the string after the '=' to a value and return it
      value = strtod(value_str, &end);
      return value;
    }
  }
  return 0.0;
}



void reset_type(FILE *cfgfile, menuinfo *stats_menu, int vehicle)
{
  // function loads the options for that type of plane

  char vehicle_str[15];       // for storing the vehicle heading (in the cfg file)
  char vehicle_number_str[3]; // for temporarily storing the vehicle number

  // open the config file
  cfgfile = fopen("dogfight.cfg", "r");

  // generate the vehicle heading required
  strcpy(vehicle_str, "vehicle ");
  fixed_str(vehicle_number_str, vehicle, 2);
  strcat(vehicle_str, vehicle_number_str);

  stats_menu->optionval[0] = read_value(cfgfile, '@', vehicle_str, "vehicle");
  stats_menu->optionval[1] = read_value(cfgfile, '@', vehicle_str, "turn_speed");
  stats_menu->optionval[2] = read_value(cfgfile, '@', vehicle_str, "acceleration");
  stats_menu->optionval[3] = read_value(cfgfile, '@', vehicle_str, "min_speed");
  stats_menu->optionval[4] = read_value(cfgfile, '@', vehicle_str, "max_speed");
  stats_menu->optionval[5] = read_value(cfgfile, '@', vehicle_str, "num_of_shots");
  stats_menu->optionval[6] = read_value(cfgfile, '@', vehicle_str, "shot_life");
  stats_menu->optionval[7] = read_value(cfgfile, '@', vehicle_str, "shot_lag");
  stats_menu->optionval[8] = read_value(cfgfile, '@', vehicle_str, "shot_base_speed");
  stats_menu->optionval[9] = read_value(cfgfile, '@', vehicle_str, "laser_length");
  fclose(cfgfile);
}



void move_menu_cursor(menuinfo *menu, int keycode)
{
  // function detects keystrokes and moves cursor/selects something

  switch (keycode) {
  case ALLEGRO_KEY_UP    : menu->highlight--; break;
  case ALLEGRO_KEY_DOWN  : menu->highlight++; break;
  case ALLEGRO_KEY_LEFT  : menu->selection = menu->highlight;
    menu->selectupdn = -1;
    break;
  case ALLEGRO_KEY_RIGHT :
  case ALLEGRO_KEY_ENTER : menu->selection = menu->highlight;
    menu->selectupdn = 1;
    break;
  case ALLEGRO_KEY_ESCAPE: exit(0); break;
  }

  if (menu->highlight < 0)
    menu->highlight = menu->num_of_options - 1;
  else if (menu->highlight > menu->num_of_options - 1)
    menu->highlight = 0;
}



void draw_menu(doginfo *dog, menuinfo *menu)
{
  // function paints a given menu onto the given bitmap

  char count;
  int row = menu->row;
  int col = menu->col;

  al_draw_bitmap(dog->title, 0, 0, 0);

  if (menu->fontsize == 2) {
    for (count = 0; count <= menu->num_of_options-1; count++) {
      al_draw_text(dog->font, dog->white, col, row + count * 32, 0, menu->option[count]);
    }
    al_draw_bitmap(dog->cursor, col - 32, row + menu->highlight*32 + 4, 0);
  } else if (menu->fontsize == 1) {
    for (count = 0; count <= menu->num_of_options-1; count++) {
      al_draw_text(dog->font, dog->white, col, row + count * 16, 0, menu->option[count]);
    }
    al_draw_bitmap(dog->cursor, col - 32, row + menu->highlight*16, 0);
  }
}



void goto_stats_menu(doginfo *dog, optionsinfo *options, menuinfo *stats_menu)
{
  // function controls the stats options menu and optionally writes options to a file

  FILE *cfgfile;
  char count;
  float increment;
  char option_str[128] = "";

  stats_menu->highlight = 0;
  // reset the selection
  stats_menu->selection = -1;

  do {
    ALLEGRO_EVENT ev;

    al_wait_for_event(dog->event_queue, &ev);

    if (ev.type == ALLEGRO_EVENT_TIMER) {
      // draw the options menu
      draw_menu(dog, stats_menu);
      al_draw_text(dog->font, dog->white, dog->w/2, TITLE_ROW*8, ALLEGRO_ALIGN_CENTRE, "Player Statistics");
      // draw the current data next to each option
      al_draw_bitmap(dog->plane[((int)stats_menu->optionval[0])], stats_menu->col+18*8, stats_menu->row-2, 0);
      for (count = 1; count <= 9; count++) {
        sprintf(stats_menu->optionval_str[count], "%2.2f", stats_menu->optionval[count]);
        al_draw_text(dog->font, dog->white, stats_menu->col+18*8, stats_menu->row+count*16, 0, stats_menu->optionval_str[count]);
      }
      al_flip_display();

    } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
      move_menu_cursor(stats_menu, ev.keyboard.keycode);

      // add or minus a set amount depending on which button was pressed
      increment = 0.05;
      if (stats_menu->selectupdn == -1)
        increment *= -1;
      switch (stats_menu->selection) {
      case 1 : stats_menu->optionval[1] += increment; break; // turn
      case 2 : stats_menu->optionval[2] += increment; break; // acceleration
      };

      // add or minus a set amount depending on which button was pressed
      increment = 1;
      if (stats_menu->selectupdn == -1)
        increment *= -1;
      switch (stats_menu->selection) {
      case 0 : stats_menu->optionval[0] += increment; break; // plane
      case 3 : stats_menu->optionval[3] += increment; break; // min speed
      case 4 : stats_menu->optionval[4] += increment; break; // max speed
      case 5 : stats_menu->optionval[5] += increment; break; // num of shots
      case 6 : stats_menu->optionval[6] += increment; break; // shot life
      case 7 : stats_menu->optionval[7] += increment; break; // shot lag
      case 8 : stats_menu->optionval[8] += increment; break; // shot base speed
      case 9 : stats_menu->optionval[9] += increment; break; // laser len
      case 10: reset_type(cfgfile, stats_menu, stats_menu->optionval[0]); break;
      };

      // make sure options are within boundaries (boundaries will be different for
      // each option)
      // vehicle
      if (stats_menu->optionval[0] > NUM_OF_VEHICLES-1)
        stats_menu->optionval[0] = 0;
      if (stats_menu->optionval[0] < 0)
        stats_menu->optionval[0] = NUM_OF_VEHICLES-1;
      // turn
      if (stats_menu->optionval[1] > 3.10)
        stats_menu->optionval[1] = 0;
      if (stats_menu->optionval[1] < 0)
        stats_menu->optionval[1] = 3.10;
      // acceleration
      if (stats_menu->optionval[2] > stats_menu->optionval[4])
        stats_menu->optionval[2] = 0.05;
      if (stats_menu->optionval[2] < 0)
        stats_menu->optionval[2] = stats_menu->optionval[4];
      // min speed
      if (stats_menu->optionval[3] > stats_menu->optionval[4])
        stats_menu->optionval[3] = -1*stats_menu->optionval[4];
      if (stats_menu->optionval[3] < -1*stats_menu->optionval[4])
        stats_menu->optionval[3] = stats_menu->optionval[4];
      // max speed
      if (stats_menu->optionval[4] > 127)
        stats_menu->optionval[4] = 0;
      if (stats_menu->optionval[4] < 0)
        stats_menu->optionval[4] = 127;
      // num of shots
      if (stats_menu->optionval[5] > 255)
        stats_menu->optionval[5] = 1;
      if (stats_menu->optionval[5] < 1)
        stats_menu->optionval[5] = 255;
      // shot life
      if (stats_menu->optionval[6] > 255)
        stats_menu->optionval[6] = 5;
      if (stats_menu->optionval[6] < 0)
        stats_menu->optionval[6] = 255;
      // shot lag
      if (stats_menu->optionval[7] > 255)
        stats_menu->optionval[7] = 0;
      if (stats_menu->optionval[7] < 0)
        stats_menu->optionval[7] = 255;
      // shot base speed
      if (stats_menu->optionval[8] > 255)
        stats_menu->optionval[8] = 0;
      if (stats_menu->optionval[8] < 0)
        stats_menu->optionval[8] = 255;
      // laser len
      if (stats_menu->optionval[9] > 255)
        stats_menu->optionval[9] = 0;
      if (stats_menu->optionval[9] < 0)
        stats_menu->optionval[9] = 255;
      // reset the selection
      stats_menu->selection = -1;
    }


  } while (stats_menu->selection != stats_menu->num_of_options-1);

  // copy the values back into the options structure
  options->vehicle         = stats_menu->optionval[0];
  options->turn_speed      = stats_menu->optionval[1];
  options->acceleration    = stats_menu->optionval[2];
  options->min_speed       = stats_menu->optionval[3];
  options->max_speed       = stats_menu->optionval[4];
  options->num_of_shots    = stats_menu->optionval[5];
  options->shot_life       = stats_menu->optionval[6];
  options->shot_lag        = stats_menu->optionval[7];
  options->shot_base_speed = stats_menu->optionval[8];
  options->laser_length    = stats_menu->optionval[9];
}



void goto_options_menu(doginfo *dog, optionsinfo *options, menuinfo *options_menu, menuinfo *stats_menu)
{
  // function controls the options menu

  options_menu->highlight = 0;
  do {
    ALLEGRO_EVENT ev;

    // reset the selection
    options_menu->selection = -1;

    // draw the options screen
    draw_menu(dog, options_menu);
    al_draw_text(dog->font, dog->white, dog->w/2, TITLE_ROW*8, ALLEGRO_ALIGN_CENTRE, "Options");
    al_flip_display();

    al_wait_for_event(dog->event_queue, &ev);

    if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
      move_menu_cursor(options_menu, ev.keyboard.keycode);
      if (options_menu->selection == 0) {
        // go to the stats menu
        goto_stats_menu(dog, options, stats_menu);
        options_menu->highlight = 0;
      }
    }

  } while (options_menu->selection != options_menu->num_of_options - 1);
}



void draw_base_title_page(doginfo *dog, menuinfo *title_menu)
{
  // function draws the base options list (ie - the title page)

  draw_menu(dog, title_menu);
  al_draw_text(dog->font, dog->white, dog->w/2, TITLE_ROW*8, ALLEGRO_ALIGN_CENTRE, "Dogfight");
}


void title_page(doginfo *dog)
{
  // function sets up the title screen and allows access to the actual game

  menuinfo title_menu, options_menu, stats_menu;
  optionsinfo options;
  FILE *cfgfile;

  title_menu.highlight = 0;

  strcpy(title_menu.option[0], "2 Player Game");
  strcpy(title_menu.option[1], "3 Player Game");
  strcpy(title_menu.option[2], "4 Player Game");
  strcpy(title_menu.option[3], "Options");
  strcpy(title_menu.option[4], "Credits");
  strcpy(title_menu.option[5], "Exit");
  title_menu.num_of_options = 6;
  title_menu.row = 25*8;
  title_menu.col = dog->w/2-78;
  title_menu.fontsize = 2;

  strcpy(options_menu.option[0], "Player Statistics");
  strcpy(options_menu.option[1], "Player Controls");
  strcpy(options_menu.option[2], "Level Options");
  strcpy(options_menu.option[3], "Other Options");
  strcpy(options_menu.option[4], "Back");
  options_menu.num_of_options = 5;
  options_menu.row = 25*8;
  options_menu.col = dog->w/2-92;
  options_menu.fontsize = 2;

  strcpy(stats_menu.option[0],  "Vehicle         :");
  strcpy(stats_menu.option[1],  "Turning Speed   :");
  strcpy(stats_menu.option[2],  "Acceleration    :");
  strcpy(stats_menu.option[3],  "Minimum Speed   :");
  strcpy(stats_menu.option[4],  "Maximum Speed   :");
  strcpy(stats_menu.option[5],  "Number of Shots :");
  strcpy(stats_menu.option[6],  "Shot Life       :");
  strcpy(stats_menu.option[7],  "Shot Lag        :");
  strcpy(stats_menu.option[8],  "Shot Base Speed :");
  strcpy(stats_menu.option[9],  "Laser Length    :");
  strcpy(stats_menu.option[10], "Reset to Vehicle Type");
  strcpy(stats_menu.option[11], "Save Vehicle to Configuration File");
  strcpy(stats_menu.option[12], "Return to Options Menu Without Saving");
  stats_menu.num_of_options = 13;
  stats_menu.row = 22*8;
  stats_menu.col = dog->w/3;
  stats_menu.fontsize = 1;

  // load the game's various options from the config file
  cfgfile = fopen("dogfight.cfg", "r");

  /* options.vehicle      = stats_menu.optionval[0] = read_value(cfgfile, '@', "user vehicle", "vehicle"); */
  /* options.turn_speed   = stats_menu.optionval[1] = read_value(cfgfile, '@', "user vehicle", "turn_speed"); */
  /* options.acceleration = stats_menu.optionval[2] = read_value(cfgfile, '@', "user vehicle", "acceleration"); */
  /* options.min_speed    = stats_menu.optionval[3] = read_value(cfgfile, '@', "user vehicle", "min_speed"); */
  /* options.max_speed    = stats_menu.optionval[4] = read_value(cfgfile, '@', "user vehicle", "max_speed"); */
  /* options.num_of_shots = stats_menu.optionval[5] = read_value(cfgfile, '@', "user vehicle", "num_of_shots"); */
  /* options.shot_life    = stats_menu.optionval[6] = read_value(cfgfile, '@', "user vehicle", "shot_life"); */
  /* options.shot_lag     = stats_menu.optionval[7] = read_value(cfgfile, '@', "user vehicle", "shot_lag"); */
  /* options.shot_base_speed = stats_menu.optionval[8] = read_value(cfgfile, '@', "user vehicle", "shot_base_speed"); */
  /* options.laser_length    = stats_menu.optionval[9] = read_value(cfgfile, '@', "user vehicle", "laser_length"); */

  /* options.gravity            = read_value(cfgfile, '@', "user level", "gravity"); */
  /* options.backdrop           = read_value(cfgfile, '@', "user level", "backdrop"); */
  /* options.cloud_type         = read_value(cfgfile, '@', "user level", "cloud_type"); */
  /* options.invincibility_life = read_value(cfgfile, '@', "options",    "invincibility_life"); */
  /* options.explosion_speed    = read_value(cfgfile, '@', "options",    "explosion_speed"); */
  /* options.coralie            = read_value(cfgfile, '@', "options",    "coralie_mode"); */

  fclose(cfgfile);

  bool redraw = true;
  do {
    ALLEGRO_EVENT ev;

    al_wait_for_event(dog->event_queue, &ev);

    // reset the selection in case a game has just been exited
    title_menu.selection = -1;

    if (ev.type == ALLEGRO_EVENT_TIMER) {
      redraw = true;

    } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {

      move_menu_cursor(&title_menu, ev.keyboard.keycode);

      // start a game with the appropriate number of players
      if (title_menu.selection >= 0 && title_menu.selection <= 2) {
        options.players = title_menu.selection+2;
        game(dog, &options);

        // fix up the title screen before fading in to it
        title_menu.highlight = 0;
        draw_base_title_page(dog, &title_menu);

      } else if (title_menu.selection == 3) {
        // go to the options sub menu
        goto_options_menu(dog, &options, &options_menu, &stats_menu);
        title_menu.highlight = 0;
      }

    } else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
      break;
    }

    if (redraw && al_is_event_queue_empty(dog->event_queue)) {
      // draw the title page
      draw_base_title_page(dog, &title_menu);
      al_flip_display();
    }

  } while (title_menu.selection != title_menu.num_of_options-1);
}
