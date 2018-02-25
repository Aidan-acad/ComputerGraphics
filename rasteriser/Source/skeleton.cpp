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
void ComputePolygonRows(const vector<glm::ivec2>& vertexPixels,vector<glm::ivec2>& leftPixels,vector<glm::ivec2>& rightPixels );
// Global Variables
//camera
float focalLength = SCREEN_HEIGHT;
vec4 cameraPos(0.0, 0.0, -3.0, 1.0);
mat4 R(1.0f);
float yaw = 0.0; //Y
float roll = 0.0; //Z
float pitch = 0.0; //X
vec4 forwards(0,0,1,1);
vec4 rights(1,0,0,1);
mat4 combinedMatrix(1.0f);

//triangles
vector<Triangle> triangles;

int main( int argc, char* argv[] )
{
  //R[3][0] = cameraPos.x;
  //R[3][1] = cameraPos.y;
  //R[3][2] = cameraPos.z;
  //LoadTestModel(triangles);
  //TransformationMatrix();
  //screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );
  vector<glm::ivec2> vertexPixels(3);
  vertexPixels[0] = glm::ivec2(10, 5);
  vertexPixels[1] = glm::ivec2( 5,10);
  vertexPixels[2] = glm::ivec2(15,15);
  int MaxY = -numeric_limits<int>::max();
  int MinY = +numeric_limits<int>::max(); 
  for (int i = 0; i < 3;i++){
    if(vertexPixels[i].y > MaxY){
      MaxY = vertexPixels[i].y;
    }
    if(vertexPixels[i].y < MinY){
      MinY = vertexPixels[i].y;
    }
  }
  int range = MaxY - MinY + 1;
  //cout << range;
  vector<glm::ivec2> leftPixels(range);
  vector<glm::ivec2> rightPixels(range);
  ComputePolygonRows( vertexPixels, leftPixels, rightPixels );
  for( int row=0; row<leftPixels.size(); ++row )
  {
  cout << "Start: ("
  << leftPixels[row].x << ","
  << leftPixels[row].y << "). "
  << "End: ("
  << rightPixels[row].x << ","
  << rightPixels[row].y << "). " << endl;
  }
  //while( NoQuitMessageSDL() )
  //  {
  //    Update();
  //    Draw(screen);
  //    SDL_Renderframe(screen);
  //  }

  //SDL_SaveImage( screen, "screenshot.bmp" ); 

  //KillSDL(screen);
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
      int MaxY = -numeric_limits<int>::max();;
      int MinY = +numeric_limits<int>::max(); 
      for (int i = 0; i < 3;i++){
        if(vertices[i].y > MaxY){
          MaxY = vertices[i].y;
        }else if(vertices[i].y < MinY){
          MinY = vertices[i].y;
        }
      }
      int range = MaxY - MinY + 1;
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
    R[0][0] = cos(yaw)*cos(roll);
    R[0][1] = sin(pitch)*sin(yaw)*cos(roll) + cos(pitch)*sin(roll);
    R[0][2] = -cos(pitch)*sin(yaw)*cos(roll) + sin(pitch)*sin(roll);
    R[1][0] = -cos(yaw)*sin(roll);
    R[1][1] = -sin(pitch) * sin(yaw) * sin(roll) + cos(pitch)*cos(roll);
    R[1][2] = cos(pitch)*sin(yaw)*sin(roll) + sin(pitch)*sin(roll);
    R[2][0] = sin(yaw);
    R[2][1] = -sin(pitch)*cos(yaw);
    R[2][2] = cos(pitch)*cos(yaw);
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
    R[0][0] = cos(yaw)*cos(roll);
    R[0][1] = sin(roll)*sin(yaw)*cos(roll) + cos(roll)*sin(roll);
    R[0][2] = -cos(roll)*sin(yaw)*cos(roll) + sin(roll)*sin(roll);
    R[1][0] = -cos(yaw)*sin(roll);
    R[1][1] = -sin(roll) * sin(yaw) * sin(roll) + cos(roll)*cos(roll);
    R[1][2] = cos(roll)*sin(yaw)*sin(roll) + sin(roll)*sin(roll);
    R[2][0] = sin(yaw);
    R[2][1] = -sin(roll)*cos(yaw);
    R[2][2] = cos(roll)*cos(yaw);
    forwards[0] = R[2][0];
    forwards[1] = R[2][1];
    forwards[2] = R[2][2];
    rights[0] = R[0][0];
    rights[1] = R[0][1];
    rights[2] = R[0][2];
    TransformationMatrix();
  }
  if( keystate[SDL_SCANCODE_RIGHT])
  {

    //y Rotation
    roll -= 0.05;
    R[0][0] = cos(yaw)*cos(roll);
    R[0][1] = sin(pitch)*sin(yaw)*cos(roll) + cos(pitch)*sin(roll);
    R[0][2] = -cos(pitch)*sin(yaw)*cos(roll) + sin(pitch)*sin(roll);
    R[1][0] = -cos(yaw)*sin(roll);
    R[1][1] = -sin(pitch) * sin(yaw) * sin(roll) + cos(pitch)*cos(roll);
    R[1][2] = cos(pitch)*sin(yaw)*sin(roll) + sin(pitch)*sin(roll);
    R[2][0] = sin(yaw);
    R[2][1] = -sin(pitch)*cos(yaw);
    R[2][2] = cos(pitch)*cos(yaw);
    forwards[0] = R[2][0];
    forwards[1] = R[2][1];
    forwards[2] = R[2][2];
    rights[0] = R[0][0];
    rights[1] = R[0][1];
    rights[2] = R[0][2];
    TransformationMatrix();
  }
  if( keystate[SDL_SCANCODE_UP])
  {

    //y Rotation
    pitch += 0.05;
    R[0][0] = cos(yaw)*cos(roll);
    R[0][1] = sin(pitch)*sin(yaw)*cos(roll) + cos(pitch)*sin(roll);
    R[0][2] = -cos(pitch)*sin(yaw)*cos(roll) + sin(pitch)*sin(roll);
    R[1][0] = -cos(yaw)*sin(roll);
    R[1][1] = -sin(pitch) * sin(yaw) * sin(roll) + cos(pitch)*cos(roll);
    R[1][2] = cos(pitch)*sin(yaw)*sin(roll) + sin(pitch)*sin(roll);
    R[2][0] = sin(yaw);
    R[2][1] = -sin(pitch)*cos(yaw);
    R[2][2] = cos(pitch)*cos(yaw);
    forwards[0] = R[2][0];
    forwards[1] = R[2][1];
    forwards[2] = R[2][2];
    rights[0] = R[0][0];
    rights[1] = R[0][1];
    rights[2] = R[0][2];
    TransformationMatrix();
  }
  if( keystate[SDL_SCANCODE_DOWN])
  {

    //y Rotation
    pitch -= 0.05;
    R[0][0] = cos(yaw)*cos(roll);
    R[0][1] = sin(pitch)*sin(yaw)*cos(roll) + cos(pitch)*sin(roll);
    R[0][2] = -cos(pitch)*sin(yaw)*cos(roll) + sin(pitch)*sin(roll);
    R[1][0] = -cos(yaw)*sin(roll);
    R[1][1] = -sin(pitch) * sin(yaw) * sin(roll) + cos(pitch)*cos(roll);
    R[1][2] = cos(pitch)*sin(yaw)*sin(roll) + sin(pitch)*sin(roll);
    R[2][0] = sin(yaw);
    R[2][1] = -sin(pitch)*cos(yaw);
    R[2][2] = cos(pitch)*cos(yaw);
    forwards[0] = R[2][0];
    forwards[1] = R[2][1];
    forwards[2] = R[2][2];
    rights[0] = R[0][0];
    rights[1] = R[0][1];
    rights[2] = R[0][2];
    TransformationMatrix();
  }
  if( keystate[SDL_SCANCODE_LEFT])
  {

    //y Rotation
    roll += 0.05;
    R[0][0] = cos(yaw)*cos(roll);
    R[0][1] = sin(pitch)*sin(yaw)*cos(roll) + cos(pitch)*sin(roll);
    R[0][2] = -cos(pitch)*sin(yaw)*cos(roll) + sin(pitch)*sin(roll);
    R[1][0] = -cos(yaw)*sin(roll);
    R[1][1] = -sin(pitch) * sin(yaw) * sin(roll) + cos(pitch)*cos(roll);
    R[1][2] = cos(pitch)*sin(yaw)*sin(roll) + sin(pitch)*sin(roll);
    R[2][0] = sin(yaw);
    R[2][1] = -sin(pitch)*cos(yaw);
    R[2][2] = cos(pitch)*cos(yaw);
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

  R[3][0] = cameraPos.x;
  R[3][1] = cameraPos.y;
  R[3][2] = cameraPos.z;

  camera_neg[3][0] = -cameraPos.x;
  camera_neg[3][1] = -cameraPos.y;
  camera_neg[3][2] = -cameraPos.z;

  camera_pos[3][0] = cameraPos.x;
  camera_pos[3][1] = cameraPos.y;
  camera_pos[3][2] = cameraPos.z;

  combinedMatrix = camera_neg * R * camera_pos;
}

void VertexShader( const glm::vec4& v, glm::ivec2& p)
{
  vec4 transformed = v - cameraPos;
  transformed = combinedMatrix * transformed;
  p.x = (focalLength * (transformed.x / transformed.z)) + (SCREEN_WIDTH / 2);
  p.y = (focalLength * (transformed.y / transformed.z)) + (SCREEN_HEIGHT / 2);
}

void myInterpolate( glm::ivec2 a, glm::ivec2 b, vector<glm::ivec2>& result )
{
  int N = result.size();
  vec2 step = vec2(b-a) / float(max(N-1,1));
  vec2 current( a );
  //cout << "Beginning interpolate";
  for( int i=0; i<N; ++i )
  {
    result[i] = current;
    //cout << result[i].x << " " << result[i].y << ":";
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

void ComputePolygonRows(
const vector<glm::ivec2>& vertexPixels,
vector<glm::ivec2>& leftPixels,
vector<glm::ivec2>& rightPixels ){
  for( int i=0; i<leftPixels.size(); ++i )
  {
    leftPixels[i].x = +numeric_limits<int>::max();
    rightPixels[i].x = -numeric_limits<int>::max();
  }
  vector<glm::ivec2> outPix(leftPixels.size());
  for (int i = 0;i < 3;i++){
    myInterpolate(vertexPixels[i],vertexPixels[(i+1)%3],outPix);
    int prevY = numeric_limits<int>::max();
    for(int j = 0;j < outPix.size();j++){
        if(leftPixels[j].x > outPix[j].x){
          leftPixels[j].x = outPix[j].x;
        }
        if(rightPixels[j].x < outPix[j].x){
          rightPixels[j].x = outPix[j].x;
        }
        leftPixels[j].y = outPix[j].y;
        rightPixels[j].y = outPix[j].y;
    }
  }
  // 1. Find max and min y-value of the polygon
  //and compute the number of rows it occupies.
  // 2. Resize leftPixels and rightPixels
  //so that they have an element for each row.
  // 3. Initialize the x-coordinates in leftPixels
  //to some really large value and the x-coordinates
  //in rightPixels to some really small value.
  // 4. Loop through all edges of the polygon and use
  //linear interpolation to find the x-coordinate for
  //each row it occupies. Update the corresponding
  //values in rightPixels and leftPixels.
}