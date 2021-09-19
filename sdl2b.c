/*
Copyright (C) 2020
Sander Gieling
Inholland University of Applied Sciences at Alkmaar, the Netherlands

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation, 
Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

// Artwork bibliography
// --------------------
// Reticle taken from here:
// https://flyclipart.com/download-png#printable-crosshair-targets-clip-art-clipart-675613.png
// Blorp taken from here:
// https://opengameart.org/content/animated-top-down-survivor-player
// Desert floor tiling taken from here:
// https://www.flickr.com/photos/maleny_steve/8899498324/in/photostream/

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> // for IMG_Init and IMG_LoadTexture
#include <math.h> // for atan() function

#define SCREEN_WIDTH				1800
#define SCREEN_HEIGHT				1000
#define PI							3.14159265358979323846
// Move this much pixels every frame a move is detected:
#define PLAYER_MAX_SPEED			6.0f
// AFTER a movement-key is released, reduce the movement speed for 
// every consecutive frame by (1.0 - this amount):
#define PLAYER_DECELERATION			0.25f

int moveY = 0;
int moveX = 0;

// A mouse structure holds mousepointer coords & a pointer texture:
typedef struct _mouse_ {
  int x;
  int y;
  SDL_Texture *txtr_reticle;
} mouse;

// Added: speed in both directions and rotation angle:
typedef struct _player_ {
  int x;
  int y;
  float speed_x;
  float speed_y;
  int up;
  int down;
  int left;
  int right;
  float angle;
  SDL_Texture *txtr_player;
} player;

typedef enum _keystate_ {
  UP = 0,
  DOWN = 1
} keystate;

void handle_key(SDL_KeyboardEvent *keyevent, keystate updown, player *tha_playa);
  // This function has changed because mouse movement was added:
  void process_input(player *tha_playa, mouse *tha_mouse);

  // This function has changed because mouse movement was added:
  void update_player(player *tha_playa, mouse *tha_mouse);
  void proper_shutdown(void);
  SDL_Texture *load_texture(char *filename);

  // This function has changed because texture rotation was added,
  // which means drawing a texture centered on a coordinate is easier:
  void blit(SDL_Texture *texture, int x, int y, int center);

  // These two functions are new since sdl2a.c:
  void blit_angled(SDL_Texture *txtr, int x, int y, float angle);
  float get_angle(int x1, int y1, int x2, int y2, SDL_Texture *texture);

  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  player blorp = {(SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2), 0.0f, 0.0f, UP, UP, UP, UP, 0.0, NULL};
	
  // New: Mouse is a type representing a struct containing x and y coords of mouse pointer:
  mouse mousepointer;
	
  unsigned int window_flags = 0;
  unsigned int renderer_flags = SDL_RENDERER_ACCELERATED;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("Couldn't initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }

  window = SDL_CreateWindow("Blorp is going to F U UP!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, window_flags);
	
  if (window == NULL) {
    printf("Failed to create window -- Error: %s\n", SDL_GetError());
    exit(1);	
  }
	
  renderer = SDL_CreateRenderer(window, -1, renderer_flags);

  if (renderer == NULL) {
    printf("Failed to create renderer -- Error: %s\n", SDL_GetError());		
    exit(1);	
  }

  IMG_Init(IMG_INIT_PNG);
  blorp.txtr_player = load_texture("gfx/blorp.png");

  // New: Load mousepointer texture:
  mousepointer.txtr_reticle = load_texture("gfx/reticle.png");

  // New: Turn system mouse cursor off:
  SDL_ShowCursor(0);

  while (1) {
    SDL_SetRenderDrawColor(renderer, 120, 144, 156, 255);
    SDL_RenderClear(renderer);
		
    // # Sensor Reading #
    // Also takes the mouse movement into account:
    process_input(&blorp, &mousepointer);

    // # Applying Game Logic #
    // Also takes the mouse movement into account:
    update_player(&blorp, &mousepointer);

    // # Actuator Output Buffering #
    // Also takes texture rotation into account:
    blit_angled(blorp.txtr_player, blorp.x, blorp.y, blorp.angle);

    // New: Redraw mouse pointer centered on the mouse coordinates:
    blit(mousepointer.txtr_reticle, mousepointer.x, mousepointer.y, 1);
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }

  return 0;
}

void handle_key(SDL_KeyboardEvent *keyevent, keystate updown, player *tha_playa) {
  if (keyevent->repeat == 0) {
    if (keyevent->keysym.scancode == SDL_SCANCODE_W) {
      tha_playa->up = updown;	
    }
    if (keyevent->keysym.scancode == SDL_SCANCODE_S) {			
      tha_playa->down = updown;		
    }
    if (keyevent->keysym.scancode == SDL_SCANCODE_A) {
      tha_playa->left = updown;
    }
    if (keyevent->keysym.scancode == SDL_SCANCODE_D) {
      tha_playa->right = updown;		
    }
  }
}

void process_input(player *tha_playa, mouse *tha_mouse) {	
  SDL_Event event;
	
  while (SDL_PollEvent(&event))	{		
    switch (event.type) {
      case SDL_QUIT:	
        proper_shutdown();
        exit(0);
	break;
      case SDL_KEYDOWN:	
	if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
	  proper_shutdown();					
	  exit(0);
	}
	else {
	  handle_key(&event.key, DOWN, tha_playa);
	}
	break;
      case SDL_KEYUP:
	handle_key(&event.key, UP, tha_playa);
	break;
      default:
	break;		
    }
  }

  // NEW -- Read the mouse position here:
  SDL_GetMouseState(&tha_mouse->x, &tha_mouse->y);
}







// A Lot Has Changed //
void update_player(player *tha_playa, mouse *tha_mouse) {
  // Up And Down //
  if (tha_playa->up) {
    tha_playa->speed_y = (float)PLAYER_MAX_SPEED;
    moveY = 1;

    tha_playa->y -= (int)PLAYER_MAX_SPEED;
  } 
  if (tha_playa->down){		
    tha_playa->speed_y = (float)PLAYER_MAX_SPEED;
    moveY = 2;

    tha_playa->y += (int)PLAYER_MAX_SPEED;	
  }

  // Left And Right //
  if (tha_playa->left) {
    tha_playa->speed_x = (float)PLAYER_MAX_SPEED;
    moveX = 1;	

    tha_playa->x -= (int)PLAYER_MAX_SPEED;
  } 
  if (tha_playa->right) {
    tha_playa->speed_x = (float)PLAYER_MAX_SPEED;
    moveX = 2;

    tha_playa->x += (int)PLAYER_MAX_SPEED;	
  }

  // Make Sure It Slowly Walks Off (Y version) //
  if (tha_playa->speed_y <= 0) {
    moveY = 0;
  } 
  if (moveY != 0) {
    // Step 1: Get The Current Speed Of Blorp 		//
    float currentSpeed = (float)tha_playa->speed_y;

    // Step 2: Remove A Slight Bit Off That Speed 	//
    currentSpeed = (float)currentSpeed - PLAYER_DECELERATION;
    tha_playa->speed_y = (float)currentSpeed - PLAYER_DECELERATION;

    if (moveY == 1) {
      // Step 3: Set It To y Of Blorp 			//
      tha_playa->y = tha_playa->y - (int)currentSpeed;
    } else if (moveY == 2) {
      tha_playa->y = tha_playa->y + (int)currentSpeed;
    }
  }

  // Make Sure It Slowly Walks Off (X version) //
  if (tha_playa->speed_x <= 0) {
    moveX = 0;
  } 
  if (moveX != 0) {
    // Step 1: Get The Current Speed Of Blorp 		//
    float currentSpeed = (float)tha_playa->speed_x;

    // Step 2: Remove A Slight Bit Off That Speed 	//
    currentSpeed = (float)currentSpeed - PLAYER_DECELERATION;
    tha_playa->speed_x = (float)currentSpeed - PLAYER_DECELERATION;

    if (moveX == 1) {
      // Step 3: Set It To y Of Blorp 			//
      tha_playa->x = tha_playa->x - (int)currentSpeed;
    } else if (moveX == 2) {
      tha_playa->x = tha_playa->x + (int)currentSpeed;
    }
  }

  tha_playa->angle = get_angle(tha_playa->x, tha_playa->y, tha_mouse->x, tha_mouse->y, tha_playa->txtr_player);
}








// No Changes Have Been Made //
void proper_shutdown(void) {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

// No Changes Have Been Made //
SDL_Texture *load_texture(char *filename) {
  SDL_Texture *txtr;
  txtr = IMG_LoadTexture(renderer, filename);
  return txtr;
}

// No Changes Have Been Made //
void blit(SDL_Texture *txtr, int x, int y, int center) {
  SDL_Rect dest;
  dest.x = x;
  dest.y = y;
  SDL_QueryTexture(txtr, NULL, NULL, &dest.w, &dest.h);

  // If center != 0, render texture with its center on (x,y), NOT
  // with its top-left corner...
  if (center) {
    dest.x -= dest.w / 2;
    dest.y -= dest.h / 2;
  }

  SDL_RenderCopy(renderer, txtr, NULL, &dest);
}

// No Changes Have Been Made //
void blit_angled(SDL_Texture *txtr, int x, int y, float angle) {
  SDL_Rect dest;
  dest.x = x;
  dest.y = y;
  SDL_QueryTexture(txtr, NULL, NULL, &dest.w, &dest.h);

  // Textures that are rotated MUST ALWAYS be rendered with their
  // center at (x, y) to have a symmetrical center of rotation:
  dest.x -= (dest.w / 2);
  dest.y -= (dest.h / 2);

  // Look up what this function does. What do these rectangles
  // mean? Why is the source rectangle NULL? What are acceptable
  // values for the `angle' parameter?
  SDL_RenderCopyEx(renderer, txtr, NULL, &dest, angle, NULL, SDL_FLIP_NONE);
}

// A Lot Has Changed Here //
float get_angle(int x1, int y1, int x2, int y2, SDL_Texture *txtr) {
  // We Make Sure We Have Our Variables //
  double pythagoras, sinusRule, arcTangus = 0;
  int h = 0;

  // We Want To Know Where Blorp Is //
  SDL_QueryTexture(txtr, NULL, NULL, NULL, &h);

  // Just To Keep Everything A Bit Organized //
  double answerA, answerB;
  
  // We Use Pyhtagaros To Calculate A Diagonal Distance Between Blorp And Mouse //
  answerA = pow((x2 - x1), 2);
  answerB = pow((y2 - y1), 2);
  pythagoras = sqrt(answerA + answerB);

  // After That We Use pythagoras And The Sinus Rule To Calulate Our Degree //
  answerA = h / 3.75;
  answerB = sin(90) / pythagoras;
  sinusRule = asin(answerA * answerB);

  // Now We Calculte It's Opposite //
  answerA = y2 - y1;
  answerB = x2 - x1;
  arcTangus = atan2(answerA, answerB);

  // We Calculate All Our Results And Transform It Into Degrees //
  answerA = arcTangus - sinusRule;
  answerB = 180 / PI;
  return (float)(answerA * answerB);
}
