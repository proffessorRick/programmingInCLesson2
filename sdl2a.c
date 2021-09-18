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

#define SCREEN_WIDTH				1024
#define SCREEN_HEIGHT				576
#define MOVEMENTLENGTH				251
FILE *fptr;

// Define a player as something drawable @ some x,y-coordinate, while
// being able to register the state of the keyboard keys that represent
// the player's movement in the up, down, left and right directions...
typedef struct _player_ {
  int x;
  int y;
  int up;
  int down;
  int left;
  int right;
  SDL_Texture *txtr;
} player;

typedef enum _keystate_ {
  UP = 0,
  DOWN = 1
} keystate;

void handle_key(SDL_KeyboardEvent *keyevent, keystate updown, player *tha_playa);
void process_input(player *tha_playa);
void update_player(player *tha_playa);
void proper_shutdown(void);
SDL_Texture *load_texture(char *filename);
void blit(SDL_Texture *texture, int x, int y);

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  
  // # Initialization #
  // Spawn Blorp in the middle of the window assuming no keys pressed
  // (all in the UP position). The player texture is set to NULL for 
  // now, since it can only be loaded AFTER IMG_Init has been called
  player blorp = {(SCREEN_WIDTH / 2), (SCREEN_HEIGHT / 2), UP, UP, UP, UP, NULL};
  
  // Begin Init SDL-related stuff
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
	
  // New since last time: initialize the use of images (png).
  // For this to work you need to:
  // a) sudo apt install libsdl2-image-dev
  // b) #include <SDL2/SDL_image.h>
  // c) link against this lib by providing gcc with the correct 
  //    lib description to find it in its include path: 
  //    `gcc (...) -lSDL2_image'
  IMG_Init(IMG_INIT_PNG);
	
  // Now we can load the player texture:	
  blorp.txtr = load_texture("gfx/blorp.png");

  // End Init SDL-related stuff
  // I hope you can see by now that BEFORE the main game loop starts,
  // you should do ALL your initialization: from init SDL, init IMG
  // and `precaching' of textures, to init of map/stage/level, 
  // init of audio system, etc. In fact, when you think about it, you
  // could opt for a separate compilation unit whose single
  // responsibility it is to just init all subsystems (init.c and
  // init.h). Or maybe you want to have compilation units for all
  // your subsystems (video.c/.h, sound.c/.h, input.c/.h, etc.) and
  // have every subsystem define its own init_unit()-function.

  while (1) {
    // By now, you have probably noticed that giving the player the
    // illusion of animation is nothing more than processing and 
    // drawing objects according to the game's rules with a certain
    // clock rate. This is usually driven by the desired frame rate.
    // These are the steps you need to take EACH FRAME:
    // - Preparing the renderer for redrawing, preparing the
    //   sound system for new sounds, etc.;
    // - Recording user input (and other input, say, from the 
    //   network...), i.e. `input handling' (sensor reading);
    // - Translating (mapping) inputs to changes in the game's 
    //   object administration, i.e. `applying game logic';
    // - Redrawing the objects (and their possibly updated 
    //   positions) in the viewport-backbuffer, alongside the 
    //   re-queueing of soundbytes in the soundbuffer, the 
    //   actuation of haptic feedback in a haptic buffer, etc - in
    //   short: actuator output buffering;
    // - Presenting the modified viewport (or `updated scene') to 
    //   the player possibly including the simultaneous 
    //   presentation of sound effects and/or haptic feedback.
    // - Regulating (`capping') the game loop frequency (and with 
    //   it, the framerate) to ensure all users have a similar 
    //   multimodal sensory experience when playing the game.
				
    // # Preparation #
    // Refresh the backbuffer to its original state:
    SDL_SetRenderDrawColor(renderer, 39, 174, 96, 255);
    SDL_RenderClear(renderer);
	
    // # Sensor Reading #
    // In this example, player input is recorded in the player
    // object. When processing input, we need to pass player
    // `blorp' to the function, so that it can record which buttons
    // are pressed at the moment.
    process_input(&blorp);
		
    // # Applying Game Logic #
    // Looking at wPass player `blorp' to this function so that it 
    // may translate input to changes in blorp's position.
    update_player(&blorp);
		
    // # Actuator Output Buffering #
    blit(blorp.txtr, blorp.x, blorp.y);
		
    // # Presentation #
    // Render redrawn scene to front buffer, showing it in the 
    // actual window:
    SDL_RenderPresent(renderer);
		
    // # Game Loop (Frequency) Regulation #
    // Although we're aiming for 60 FPS, rendering takes so much
    // resources, we're probably not making our deadline here
    // any longer -- what we need is a function that computes 
    // the time we already spent,- and subtract that time from
    // our 16 ms... think about it!
    SDL_Delay(16);
  }

  return 0;
}

void handle_key(SDL_KeyboardEvent *keyevent, keystate updown, player *tha_playa) {
  // Open The Editor //
  fptr = fopen("movement.txt", "a");

  // This function can be called multiple times during a
  // `frame handle event', because multiple different keys may have
  // been pressed / released during this cycle.
  // If the keyevent is a keyboard repeat event, i.e. a key is pressed
  // repeatedly until we respond to a certain number of presses, then
  // ignore the event - just look at the scancodes of keys that
  // are registered to have undergone KEYDOWN or KEYUP events (see
  // the process_input function):
  if (keyevent->repeat == 0) {
    // Use a separate if-statement for EVERY key, so you can
    // seemingly handle multiple keys pressed at once.
    // NOTE: doing it like this means prioritizing UP (W) over DOWN (S) and LEFT (A) over RIGHT (D) ...

    // `updown' can only take the values 0 (UP) or 1 (DOWN)

    if (keyevent->keysym.scancode == SDL_SCANCODE_W) {
      tha_playa->up = updown;
      fprintf(fptr, "W");
    } else if (keyevent->keysym.scancode == SDL_SCANCODE_S) {
      tha_playa->down = updown;	
      fprintf(fptr, "S");
    }

    if (keyevent->keysym.scancode == SDL_SCANCODE_A) {
      tha_playa->left = updown;
      fprintf(fptr, "A");
    } else if (keyevent->keysym.scancode == SDL_SCANCODE_D) {
      tha_playa->right = updown;
      fprintf(fptr, "D");
    }
  }

  fclose(fptr);
}

void process_input(player *tha_playa) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
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
	  // pass the SDL_KeyboardEvent to a separate handler
	  handle_key(&event.key, DOWN, tha_playa);	
	}
	break;
      case SDL_KEYUP:
	// pass the SDL_KeyboardEvent to a separate handler
	handle_key(&event.key, UP, tha_playa);
	break;
      default:
	break;
    }
  }
}

void update_player(player *tha_playa) {
  // All keys should respond independently. The hardcoded +/-4 pixels
  // is obviously not a great idea. How would you solve it?
  if (tha_playa->up) {
    tha_playa->y -= 4;
  }

  if (tha_playa->down) {
    tha_playa->y += 4;
  }

  if (tha_playa->left) {
    tha_playa->x -= 4;
  }

  if (tha_playa->right) {
    tha_playa->x += 4;
  }
}

//void writeToFile(int movementData[]) {
//}

void proper_shutdown(void) {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

SDL_Texture *load_texture(char *filename) {
  SDL_Texture *txtr;
  txtr = IMG_LoadTexture(renderer, filename);
  return txtr;
}

void blit(SDL_Texture *txtr, int x, int y) {
  SDL_Rect dest;
  dest.x = x;
  dest.y = y;
  SDL_QueryTexture(txtr, NULL, NULL, &dest.w, &dest.h);
  SDL_RenderCopy(renderer, txtr, NULL, &dest);
}
