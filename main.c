/* Dogfight.c                                 */
/* Program:      a dog fighting game          */
/* Author:       Gregory McIntyre             */
/* First edited: 28/08/97                     */
/* Last edited:  11/23/97                     */

#include <stdio.h>
#include <allegro.h>
#include "dogdata.h"

// game constants
#define SET_W               640                // } resolution
#define SET_H               480                // }

#define CALLBACKS          (256*1)             // if 2 tables need to be set up then this
                                               // will be 256 * 2

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



// ***** TYPE DEFINITIONS ***************************************************************

// translucency tables
COLOR_MAP trans_table;

// global count for call back functions' progress meters
int callback_count;
// global palette
PALETTE pal;


// ***** MAIN PROGRAM CODE **************************************************************

// progress indicator for the color table calculations
void callback_func()
{
 // function is called 256 times during the translucency table setup

 callback_count++;

 // draw the progress bar's frame
 rect(screen, SCREEN_W/4, SCREEN_H/2-11, SCREEN_W*3/4, SCREEN_H/2+11, GREY+15);

 // add to the progress bar
 rectfill(screen, (callback_count+1)*((SCREEN_W/2)-1)/(CALLBACKS+1)+SCREEN_W/4, SCREEN_H/2-10, ((callback_count+1)*((SCREEN_W/2)-1)/(CALLBACKS+1))+(((SCREEN_W/2)-1)/CALLBACKS)+SCREEN_W/4, SCREEN_H/2+10, (callback_count+1)*(GREY+15)/(CALLBACKS+1));
}



// ***** MAIN ***************************************************************************

int main(void)
{
 BITMAP *scrbuffer;
 DATAFILE *main_data;
 int vmode_error;

 RGB black = { 0,  0,  0 };

 // initialise allegro
 allegro_init();
 install_keyboard();
 install_timer();
 //install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL);

 // load main data file into memory
 main_data = load_datafile("dogfight.dat");

 // use the palette stored in the datafile and
 // set the "clear" colour from a working hot pink to an invisible black
 memcpy(&pal, main_data[MAINPAL].dat, sizeof(pal));

 // enter graphics mode -- don't debug from here --
 vmode_error = set_gfx_mode(GFX_AUTODETECT, SET_W, SET_H, 0, 0);
 if (vmode_error) {
   printf("Error setting graphics mode...\n%s\n\n", allegro_error);
   allegro_exit();
   exit(1);
 }

 set_palette(pal);
 set_color(0, &black);

 // build a colour lookup table for translucent drawing
 // NB: 128 translucency = 50%
 textout_centre_ex(screen, main_data[FONTSMALL].dat, "Dogfight by Gregory McIntyre", SCREEN_W/2, SCREEN_H/2-58, GREY+15, -1);
 textout_centre_ex(screen, font, "Loading. Please wait...", SCREEN_W/2, SCREEN_H/2-20, GREY+15, -1);
 callback_count = 0;
 create_trans_table(&trans_table, main_data[MAINPAL].dat, CLOUD_TRANSPARENCY, CLOUD_TRANSPARENCY, CLOUD_TRANSPARENCY, callback_func);

 // allocate memory for screen buffer
 scrbuffer = create_bitmap(SCREEN_W, SCREEN_H); clear(scrbuffer);

 // when everything is ready fade out and go to the title screen
 fade_out(10);
 title_page(scrbuffer, main_data);

 // free allocated memory and exit allegro
 destroy_bitmap(scrbuffer);

 unload_datafile(main_data);
 allegro_exit();

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
END_OF_MAIN();
