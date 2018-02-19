#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include <stdint.h>

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;


#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 256
#define FULLSCREEN_MODE false

struct Camera
{
  vec4 position;
  float focalLength;
  glm::mat4 rotation;
};

float yaw;
vec4 forwards(0,0,1,1);
vec4 rights(1,0,0,1);
/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

void Update(Camera& myCam);
void Draw(screen* screen);

int main( int argc, char* argv[] )
{
  Camera myCam;
  myCam.position = vec4(0.0,0.0,-3.0,1.0);
  myCam.focalLength = SCREEN_HEIGHT;
  myCam.rotation = mat4(1.0f);
  yaw = 0;
  
  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );
  
  while( NoQuitMessageSDL() )
    {
      Update(myCam);
      Draw(screen);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

/*Place your drawing here*/
void Draw(screen* screen)
{
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));
  
  vec3 colour(1.0,0.0,0.0);
  for(int i=0; i<1000; i++)
    {
      uint32_t x = rand() % screen->width;
      uint32_t y = rand() % screen->height;
      PutPixelSDL(screen, x, y, colour);
    }
}

/*Place updates of parameters here*/
void Update(Camera& myCam)
{
  static int t = SDL_GetTicks();
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;
  /*Good idea to remove this*/
  std::cout << "Render time: " << dt << " ms." << std::endl;
  /* Update variables*/
  const uint8_t* keystate = SDL_GetKeyboardState( 0 );
  if( keystate[SDL_SCANCODE_W] )
  {
    myCam.position.z += 0.1;
  // Move camera forward
  }
  if( keystate[SDL_SCANCODE_S] )
  {
    myCam.position.z -= 0.1;
  // Move camera backward
  }
  if( keystate[SDL_SCANCODE_D] )
  {
    myCam.position.x += 0.1;
  // Move camera forward
  }
  if( keystate[SDL_SCANCODE_A] )
  {
    myCam.position.x -= 0.1;
  // Move camera forward
  }
  if( keystate[SDL_SCANCODE_E] )
  {
    //myCam.position.x -= 0.1;
    yaw += 0.05;
      myCam.rotation[0][0] = cos(yaw);
      myCam.rotation[2][0] = sin(yaw);
      myCam.rotation[1][1] = 1.0f;
      myCam.rotation[0][2] = -sin(yaw);
      myCam.rotation[2][2] = cos(yaw);
      forwards[0] = myCam.rotation[2][0];
      forwards[1] = myCam.rotation[2][1];
      forwards[2] = myCam.rotation[2][2];
      rights[0] = myCam.rotation[0][0];
      rights[1] = myCam.rotation[0][1];
      rights[2] = myCam.rotation[0][2];
    //yaw -= 5;
  // Move camera to the left
  }
  if( keystate[SDL_SCANCODE_Q] )
  {
    //myCam.position.x += 0.1;
    yaw -= 0.05;
      myCam.rotation[0][0] = cos(yaw);
      myCam.rotation[2][0] = sin(yaw);
      myCam.rotation[1][1] = 1.0f;
      myCam.rotation[0][2] = -sin(yaw);
      myCam.rotation[2][2] = cos(yaw);
      forwards[0] = myCam.rotation[2][0];
      forwards[1] = myCam.rotation[2][1];
      forwards[2] = myCam.rotation[2][2];
      rights[0] = myCam.rotation[0][0];
      rights[1] = myCam.rotation[0][1];
      rights[2] = myCam.rotation[0][2];
    //yaw += 5;
  // Move camera to the right
  }
}
