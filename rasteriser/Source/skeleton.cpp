#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include <stdint.h>

using namespace std;
using glm::vec2;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;


#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 256
#define FULLSCREEN_MODE false

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

void Update();
void Draw(screen* screen);
void TransformationMatrix();
void VertexShader( const glm::vec4& v, glm::ivec2& p);
void myInterpolate( glm::ivec2 a, glm::ivec2 b, vector<glm::ivec2>& result );
void DrawLineSDL( screen* screen, glm::ivec2 a, glm::ivec2 b, vec3 color );
void DrawPolygonEdges(screen* screen, const vector<vec4>& vertices );
// Global Variables
//camera
float focalLength = SCREEN_WIDTH;
vec4 cameraPos(0.0, 0.0, -3.0, 1.0);
mat4 R(1.0f);
float yaw = 0.0;
float roll = 0.0;
float pitch = 0.0;
vec4 forwards(0,0,1,1);
vec4 rights(1,0,0,1);
mat4 combinedMatrix(1.0f);

//triangles
vector<Triangle> triangles;

int main( int argc, char* argv[] )
{
  LoadTestModel(triangles);
  TransformationMatrix();
  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );
  
  while( NoQuitMessageSDL() )
    {
      Update();
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
  
  for( uint32_t i=0; i<triangles.size(); ++i )
    {
      vector<vec4> vertices(3);
      vertices[0] = triangles[i].v0;
      vertices[1] = triangles[i].v1;
      vertices[2] = triangles[i].v2;
      for(int v=0; v<3; ++v)
      {
        DrawPolygonEdges(screen,vertices);
      }
    }
}

/*Place updates of parameters here*/
void Update()
{
  static int t = SDL_GetTicks();
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;
  /*Good idea to remove this*/
  std::cout << "Render time: " << dt << " ms." << std::endl;
  /* Update variables*/
  const Uint8* keystate = SDL_GetKeyboardState( 0 );

  if( keystate[SDL_SCANCODE_W] )
  {
    // Move camera forward
    cameraPos.z += 0.1f;
    TransformationMatrix();
  }
  if( keystate[SDL_SCANCODE_S] )
  {
    // Move camera backward
    cameraPos.z -= 0.1f;
    TransformationMatrix();
  }
  if( keystate[SDL_SCANCODE_A] )
  {
    // Move camera to the left
    cameraPos.x -= 0.1f;
    TransformationMatrix();
  }
  if( keystate[SDL_SCANCODE_D] )
  {
    // Move camera to the right
    cameraPos.x += 0.1f;
    TransformationMatrix();
  }
  if( keystate[SDL_SCANCODE_Q])
  {

    //y Rotation
    yaw += 0.05;
    R[0][0] = cos(roll)*cos(yaw);
    R[0][1] = sin(roll)*cos(yaw);
    R[0][2] = -sin(yaw);
    R[1][0] = -sin(roll)*cos(pitch) + cos(roll)*sin(yaw)*sin(pitch);
    R[1][1] = cos(roll)*cos(pitch) + sin(roll)*sin(yaw)*sin(pitch);
    R[1][2] = cos(yaw) * sin(pitch);
    R[2][0] = sin(roll)*sin(pitch) + cos(roll)*sin(yaw)*cos(pitch);
    R[2][1] = -cos(roll)*sin(pitch) + sin(roll)*sin(yaw)*cos(pitch);
    R[2][2] = cos(yaw)*cos(pitch);
    forwards[0] = R[2][0];
    forwards[1] = R[2][1];
    forwards[2] = R[2][2];
    rights[0] = R[0][0];
    rights[1] = R[0][1];
    rights[2] = R[0][2];
    TransformationMatrix();
  }

  if( keystate[SDL_SCANCODE_E])
  {
    yaw -= 0.05;
    R[0][0] = cos(roll)*cos(yaw);
    R[0][1] = sin(roll)*cos(yaw);
    R[0][2] = -sin(yaw);
    R[1][0] = -sin(roll)*cos(pitch) + cos(roll)*sin(yaw)*sin(pitch);
    R[1][1] = cos(roll)*cos(pitch) + sin(roll)*sin(yaw)*sin(pitch);
    R[1][2] = cos(yaw) * sin(pitch);
    R[2][0] = sin(roll)*sin(pitch) + cos(roll)*sin(yaw)*cos(pitch);
    R[2][1] = -cos(roll)*sin(pitch) + sin(roll)*sin(yaw)*cos(pitch);
    R[2][2] = cos(yaw)*cos(pitch);
    forwards[0] = R[2][0];
    forwards[1] = R[2][1];
    forwards[2] = R[2][2];
    rights[0] = R[0][0];
    rights[1] = R[0][1];
    rights[2] = R[0][2];
    TransformationMatrix();
  }
}

void TransformationMatrix()
{
  glm::mat4 camera_neg = mat4(1.0f);
  glm::mat4 camera_pos = mat4(1.0f);

  camera_neg[3][0] = -cameraPos.x;
  camera_neg[3][1] = -cameraPos.y;
  camera_neg[3][2] = -cameraPos.z;

  camera_pos[3][0] = cameraPos.x;
  camera_pos[3][1] = cameraPos.y;
  camera_pos[3][2] = cameraPos.z;

  combinedMatrix = camera_pos * R * camera_neg;
}

void VertexShader( const glm::vec4& v, glm::ivec2& p)
{
  vec4 transformed = combinedMatrix * v;
  //vec4 transformed = v - cameraPos;
  p.x = (focalLength * (transformed.x / transformed.z)) + (SCREEN_WIDTH / 2);
  p.y = (focalLength * (transformed.y / transformed.z)) + (SCREEN_HEIGHT / 2);
}

void myInterpolate( glm::ivec2 a, glm::ivec2 b, vector<glm::ivec2>& result )
{
  int N = result.size();
  vec2 step = vec2(b-a) / float(max(N-1,1));
  vec2 current( a );
  for( int i=0; i<N; ++i )
  {
  result[i] = current;
  current += step;
  }
}

void DrawLineSDL( screen* screen, glm::ivec2 a, glm::ivec2 b, vec3 color )
{
  glm::ivec2 delta = glm::abs( a - b );
  int pixels = glm::max( delta.x, delta.y ) + 1;
  vector<glm::ivec2> line( pixels );
  myInterpolate( a, b, line );
  //vec3 color(1,1,1);
  for(int k = 0;k < line.size();k++){
    PutPixelSDL( screen, line[k].x, line[k].y, color);
  }
}

void DrawPolygonEdges(screen* screen, const vector<vec4>& vertices )
{
  int V = vertices.size();
  // Transform each vertex from 3D world position to 2D image position:
  vector<glm::ivec2> projectedVertices( V );
  for( int i=0; i<V; ++i )
  {
    VertexShader( vertices[i], projectedVertices[i] );
  }
  // Loop over all vertices and draw the edge from it to the next vertex:
  for( int i=0; i<V; ++i )
  {
    int j = (i+1)%V; // The next vertex
    vec3 color( 1, 1, 1 );
    DrawLineSDL( screen, projectedVertices[i], projectedVertices[j], color );
  }
}