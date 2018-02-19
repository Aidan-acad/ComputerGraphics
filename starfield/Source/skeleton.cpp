#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"
#include <stdint.h>

using namespace std;
using glm::vec3;
using glm::mat3;

#define SCREEN_WIDTH 1370//320
#define SCREEN_HEIGHT 960//256
#define FULLSCREEN_MODE false


/* ----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                            */
int t;
bool goddamn;

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

void Update(vector<vec3>& stars);
void Draw(screen* screen, vector<vec3>& stars, vec3 pastStarPos[][5]);
void Interpolate( float a, float b, vector<float>& result);
void InterpolateVec(vec3 a, vec3 b, vector<vec3>& result);

int main( int argc, char* argv[] )
{
  goddamn = false;
  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );
  t = SDL_GetTicks();	/*Set start value for timer.*/
  /*vector<float> result( 3 );
  Interpolate(5,14,result);
  for(int i = 0;i < result.size();i++){
    cout << result[i] << ' ';
  }
  vector<vec3> result2(4);
  vec3 a(1,4,9.2);
  vec3 b(4,1,9.8);
  InterpolateVec(a,b,result2);
  for(int i = 0;i < result2.size();i++){
    cout << "( " << result2[i].x << ", " << result2[i].y << ", " << result2[i].z << " )";
  }*/
  vector<vec3> stars(2000);
  vec3 pastStarPos[2000][5];
  for(int i=0;i<stars.size();i++){
    stars[i].z = (float) rand()/ (float)RAND_MAX;
    stars[i].x = ((float) rand()/ (float)RAND_MAX)*2-1;
    stars[i].y = ((float) rand()/ (float)RAND_MAX)*2-1;
  }
  while( NoQuitMessageSDL() )
    {
      Draw(screen,stars,pastStarPos);
      Update(stars);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

/*Place your drawing here*/
void Draw(screen* screen, vector<vec3>& stars, vec3 pastStarPos[][5])
{
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));
  /*vec3 topLeft(1,0,0);
  vec3 topRight(0,0,1);
  vec3 bottomRight(0,1,0);
  vec3 bottomLeft(1,1,0);
  vector<vec3> leftSide(SCREEN_HEIGHT);
  vector<vec3> rightSide(SCREEN_HEIGHT);
  vector<vec3> activeRow(SCREEN_WIDTH);
  InterpolateVec(topLeft,bottomLeft,leftSide);
  InterpolateVec(topRight,bottomRight,rightSide);
  
  for(uint32_t ys = 0;ys < SCREEN_HEIGHT;ys++){
    InterpolateVec(leftSide[ys],rightSide[ys],activeRow);
    for(uint32_t xs = 0;xs < SCREEN_WIDTH;xs++){
      PutPixelSDL(screen, xs,ys,activeRow[xs]);
    }
  }*/
  if(t%20 == 0){
    for(int j = 0;j < 2000;j++){
      pastStarPos[j][4] = pastStarPos[j][3];
      pastStarPos[j][3] = pastStarPos[j][2];
      pastStarPos[j][2] = pastStarPos[j][1];
      pastStarPos[j][1] = pastStarPos[j][0];
    }
  }
  vec3 colour((float)0,(float)0,(float)1);
  for(int i = 0;i<stars.size();i++){
    if(stars[i].z <=0)stars[i].z +=1;
    if(stars[i].z > 1)stars[i].z -=1;
    uint32_t x = (SCREEN_HEIGHT/4)*(stars[i].x/stars[i].z)+(SCREEN_WIDTH/2);
    uint32_t y = (SCREEN_HEIGHT/4)*(stars[i].y/stars[i].z)+(SCREEN_HEIGHT/2);
    
    if(goddamn){
      vec3 start(x,y,stars[i].z);
      vector<vec3> results( 30 );
      if(pastStarPos[i][4].z > start.z){
        InterpolateVec(start,pastStarPos[i][4],results);
        for(int k = 0;k < results.size();k++){
          vec3 color = 0.2f * vec3((1/(results[k].z*results[k].z)),1,1*(results[k].z*results[k].z)) / (results[k].z*results[k].z);
          PutPixelSDL(screen,results[k].x,results[k].y,color); 
        }
      }else{
        vec3 color = 0.2f * vec3(1/(stars[i].z*stars[i].z),1,1*(stars[i].z*stars[i].z)) / (stars[i].z*stars[i].z);
        PutPixelSDL(screen,x,y,color);  
      }
    }else{
      vec3 color = 0.2f * vec3(1/(stars[i].z*stars[i].z),1,1*(stars[i].z*stars[i].z)) / (stars[i].z*stars[i].z);
      PutPixelSDL(screen,x,y,color);  
    }
    
    //vec3 color = 0.2f * vec3(0.6,0.8,1) / (stars[i].z*stars[i].z);
    //PutPixelSDL(screen,x,y,color);
    /*color = 0.2f * vec3(0.6,0.8,1) / (pastStarPos[i][1].z*pastStarPos[i][1].z);
    PutPixelSDL(screen,pastStarPos[i][1].x,pastStarPos[i][1].y,color);
    color = 0.2f * vec3(0.6,0.8,1) / (pastStarPos[i][2].z*pastStarPos[i][2].z);
    PutPixelSDL(screen,pastStarPos[i][2].x,pastStarPos[i][2].y,color);
    color = 0.2f * vec3(0.6,0.8,1) / (pastStarPos[i][3].z*pastStarPos[i][3].z);
    PutPixelSDL(screen,pastStarPos[i][3].x,pastStarPos[i][3].y,color);*/
    //color = 0.2f * vec3(0.6,0.8,1) / (pastStarPos[i][4].z*pastStarPos[i][4].z);
    //PutPixelSDL(screen,pastStarPos[i][4].x,pastStarPos[i][4].y,color);
    if(t%20 == 0){
      pastStarPos[i][0] = vec3(x,y,stars[i].z); 
      goddamn = true; 
    }
  }
  /*for(int i=0; i<1000; i++)
    {
      vec3 colour((float) rand()/RAND_MAX,(float) rand()/RAND_MAX,(float) rand()/RAND_MAX);
      uint32_t x = rand() % screen->width;
      uint32_t y = rand() % screen->height;
      PutPixelSDL(screen, x, y, colour);
    }*/
}

/*Place updates of parameters here*/
void Update(vector<vec3>& stars)
{
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;
  /*Good idea to remove this*/
  //std::cout << "Render time: " << dt << " ms." << std::endl;
  /* Update variables*/

  for(int i = 0; i < stars.size();i++){
    stars[i].z -= (0.0005*(1.2-stars[i].z))*dt;
  }
}

void Interpolate( float a, float b, vector<float>& result){
  if(result.size() == 1){
    result[0] = a;
  }else{
    result[0] = a;
    result[result.size()-1] = b;
    float spacing = ((b)-(a))/(result.size()-1);
    //cout << spacing << "  <<<<  ";
    for(int i = 1;i < result.size();i++){
      a += spacing;
      result[i] = a;
    }
  }
}

void InterpolateVec(vec3 a, vec3 b, vector<vec3>& result){
  if(result.size() == 1){
    result[0] = a;
  }else{
    result[0] = a;
    result[result.size()-1] = b;
    float xspacing = ((b.x)-(a.x))/(result.size()-1);
    float yspacing = ((b.y)-(a.y))/(result.size()-1);
    float zspacing = ((b.z)-(a.z))/(result.size()-1);
    //cout << spacing << "  <<<<  ";
    for(int i = 1;i < result.size();i++){
      a.x += xspacing;
      a.y += yspacing;
      a.z += zspacing;
      result[i] = a;  
    }
  }
}
