#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include <stdint.h>
#include <math.h>

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;


#define SCREEN_WIDTH 256	
#define SCREEN_HEIGHT 256
#define FULLSCREEN_MODE false

struct Intersection
{
	vec4 position;
	vec4 direction;
	float distance;
	float shadDist;
	float penSize;
	int triangleIndex;
};

struct Camera
{
	vec4 position;
	float focalLength;
	glm::mat4 rotation;
};

struct OmniLight
{
	vec4 position;
	vec3 colour;
	float size;
};

OmniLight dirLight;
float yaw;
float maximumDist;
float denseBuff[SCREEN_HEIGHT][SCREEN_WIDTH];
float distBuff[SCREEN_HEIGHT][SCREEN_WIDTH];
float penBuff[SCREEN_HEIGHT][SCREEN_WIDTH];
float newDenseBuff[SCREEN_HEIGHT][SCREEN_WIDTH];
float freshTouch[SCREEN_HEIGHT][SCREEN_WIDTH];
vec3 fakeScreen[SCREEN_HEIGHT][SCREEN_WIDTH];
vec4 forwards(0,0,1,1);
vec4 rights(1,0,0,1);
vec3 ambientLight = 0.25f*vec3(1,1,1);
/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */
void Update(Camera& myCam);
void Draw(screen* screen, Camera& myCam,const vector<Triangle>& triangles);
vec3 DirectLight( Intersection& i, const vector<Triangle>& triangles, int x, int y);
float getPenumbraSize(float distanceTo,float OccluderDistance);
bool ClosestIntersection(
vec4 s, vec4 d, const vector<Triangle>& triangles, Intersection& closestIntersection );

int main( int argc, char* argv[] )
{
  yaw = 0;
  dirLight.position = vec4(0,-0.5,-0.7,1.0);
  dirLight.colour = 14.0f * vec3(1,1,1);
  dirLight.size = 15.0f;
  vector<Triangle> triangles;
  Camera myCam;
  myCam.position = vec4(0.0,0.0,-3.0,1.0);
  myCam.focalLength = SCREEN_HEIGHT;
  myCam.rotation = mat4(1.0f);
  LoadTestModel(triangles);
  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );
  
  while( NoQuitMessageSDL() )
    {
      Update(myCam);
      Draw(screen,myCam,triangles);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

bool ClosestIntersection(vec4 s, vec4 d, const vector<Triangle>& triangles, Intersection& closestIntersection ){
	bool result = false;
	float dist = std::numeric_limits<float>::max();
	//float dist = 100000000000;
	Intersection closest;
	for(int i = 0;i < triangles.size();i++){
		Triangle triangle = triangles[i];
		vec4 v0 = triangle.v0;
		vec4 v1 = triangle.v1;
		vec4 v2 = triangle.v2;

		vec3 e1 = vec3(v1.x-v0.x,v1.y-v0.y,v1.z-v0.z);
		vec3 e2 = vec3(v2.x-v0.x,v2.y-v0.y,v2.z-v0.z);
		vec3 b = vec3(s.x-v0.x,s.y-v0.y,s.z-v0.z);

		vec3 d2 = vec3(d.x,d.y,d.z);
		vec3 x;
		mat3 A( -d2, e1, e2 );
		mat3 Ap(b,e1,e2);
		float det1 = determinant(Ap);
		float det2 = determinant(A);
		x.x = det1/det2;
		if(x.x > 0 && x.x < dist){
			Ap = mat3(-d2,b,e2);
			det1 = determinant(Ap);
			det2 = determinant(A);
			x.y = det1/det2;
			Ap = mat3(-d2,e1,b);
			det1 = determinant(Ap);
			det2 = determinant(A);
			x.z = det1/det2;
			if (x.y >= 0 && x.z >= 0 && x.y + x.z <= 1){
				closest.position = s + x.x*d;
				closest.distance = x.x;
				closest.triangleIndex = i;
				closest.direction = d;
				dist = x.x;
				result = true;
			}
		}
		
		//vec3 x = glm::inverse( A ) * b;
	}
	if(result){
		closestIntersection = closest;
	}
	
	return result;
}

vec3 DirectLight( Intersection& i, const vector<Triangle>& triangles, int x, int y){
	float r = glm::distance(dirLight.position, i.position);
	vec4 R = glm::normalize(dirLight.position - i.position);	
	vec4 N = triangles[i.triangleIndex].normal;
	float dot = glm::dot(R,N);
	float max = glm::max(dot,0.0f);
	float denom = (4*M_PI*r*r);
	float E = max/denom;
	vec3 D = dirLight.colour * E;
	i.shadDist = 0;
	i.penSize = 0;
	if(max == 0.0f){
		penBuff[y][x] = 1.0f;
		denseBuff[y][x] = 1.0f;
		//cout << "goddamn ";
	}else{
		Intersection ShadowTheEdgehog;
		bool check = ClosestIntersection(i.position + R*(float)1e-4,R,triangles,ShadowTheEdgehog);
		//D = vec3(0,0,0);
		if(check){
			if(ShadowTheEdgehog.distance < r && ShadowTheEdgehog.distance > 0){
				//distBuff[y][x] = ShadowTheEdgehog.distance;
				//penBuff[y][x] =  (float) getPenumbraSize(r,ShadowTheEdgehog.distance);
				denseBuff[y][x] = 1.0f;
				i.shadDist = ShadowTheEdgehog.distance;
				i.penSize = (float) getPenumbraSize(r,ShadowTheEdgehog.distance);
				//D = vec3(redComp,0,0); 
				//D = vec3(0,0,0);
				//newDenseBuff[y][x] = 1;
			}
		}
	}
	return D;

}

float getPenumbraSize(float distanceTo,float OccluderDistance){
	return (float) dirLight.size * (float)(OccluderDistance/distanceTo);
}

/*Place your drawing here*/
void Draw(screen* screen,Camera& myCam,const vector<Triangle>& triangles)
{
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));
  int y;
  int x;
  maximumDist = 0;
  #pragma omp parallel for private(x,y) shared(distBuff,denseBuff)
  for(y = 0;y < SCREEN_HEIGHT;y++){
  	for(x = 0;x < SCREEN_WIDTH;x++){
  		vec3 colours[4];
  		for(int i=0;i<2;i++){
  			for(int j=0;j<2;j++){
  				vec4 dir;
  				dir.x = (x - 0.5 + i) - SCREEN_WIDTH/2;
  				dir.y = (y - 0.5 + j) - SCREEN_HEIGHT/2;
  				dir.z = myCam.focalLength;
  				dir.w = 1.0f;
  				dir = myCam.rotation * dir;
  				dir = glm::normalize(dir);
  				Intersection bestIntersect;
		  		bool check = ClosestIntersection(myCam.position,dir,triangles,bestIntersect);
		  		if(check){
		  			vec3 direct = DirectLight(bestIntersect,triangles,x,y);
		  			if(denseBuff[y][x] != 1.0f){
		  				distBuff[y][x] = 1;
		  			}else{
		  				float increment = bestIntersect.shadDist/4.0f;
		  				//cout << increment << " ";
		  				distBuff[y][x] += increment;
		  			}
		  			/*if(maximumDist > 3.5){
		  				distBuff[y][x] = 1.0f;
		  				maximumDist = 0;
		  			}else{
		  				maximumDist = 0;
		  			}*/
		  			if(distBuff[y][x] > maximumDist){
		  				maximumDist = distBuff[y][x];
		  			}
		  			penBuff[y][x] += (float) bestIntersect.penSize/4.0f;
		  			vec3 colour = (direct + ambientLight)/*vec3(1,1,1)*/*triangles[bestIntersect.triangleIndex].color;
		  			colours[i*2+j] = colour;
		  		}else{
		  			distBuff[y][x] = 1;
		  			vec3 colour(0,0,0);
		  			colours[i*2+j] = colour;
		  		}
  			}
  		}
  		vec3 colour(0,0,0);
  		for(int k = 0;k<4;k++){
  			colour.x += colours[k].x/4.0f;
  			colour.y += colours[k].y/4.0f;
  			colour.z += colours[k].z/4.0f;
  		}
  		//colour = colour/4.0f;
  		fakeScreen[y][x] = colour;
  		//colour = vec3(distBuff[y][x],0,denseBuff[y][x]);

  		//colour = vec3(0,denseBuff[y][x],0);
  		//PutPixelSDL(screen, x ,y, colour/*vec3(distBuff[y][x],0,0)*/);
  	}
  }
  float maxDense = 0.0f;
  for(y = 0;y < SCREEN_HEIGHT;y++){
  	for(x = 0;x < SCREEN_WIDTH;x++){
  		if(denseBuff[y][x]>0){
  			int radius = penBuff[y][x] + 1;
  			if(radius > 0){
	  			float buildup = 0.0f;
	  			for(int i=-radius;i<radius+1;i++){
	  				if(y+i > 0 && y+i<SCREEN_HEIGHT){
		  				for(int j=-radius;j<radius+1;j++){
		  					if(x+j > 0 && x+j < SCREEN_WIDTH){
		  						buildup += denseBuff[y + i][x + j];
		  					}
		  				}
	  				}
	  			}
	  			buildup = (float) buildup/(float) ((2*radius + 1)*(2*radius + 1));
	  			for(int i=-radius;i<radius+1;i++){
	  				if(y+i > 0 && y+i<SCREEN_HEIGHT){
		  				for(int j=-radius;j<radius+1;j++){
		  					if(x+j > 0 && x+j < SCREEN_WIDTH){
		  						int indeX = j;
		  						int indeY = i;
		  						if(indeY < 0){
		  							indeY = -indeY;
		  						}
		  						if(indeX < 0){
		  							indeX = -indeX;
		  						}
		  						float offRat = (float)(1.1 -  (((float) (indeY + indeX))/(float) (2*radius)))/(float) ((distBuff[y][x] + 1.0f)*(distBuff[y][x] + 1.0f));
		  						newDenseBuff[y + i][x + j] += (float) (buildup)*offRat;
		  						if(newDenseBuff[y + i][x + j] > maxDense){
		  							maxDense = newDenseBuff[y + i][x + j];
		  						}
		  						if (newDenseBuff[y+i][x+j] > 1.0f){newDenseBuff[y+i][x+j] = 1.0f;}
		  						/*if(distBuff[y + i][x + j] == 1.0f){
		  							distBuff[y + i][x + j] = distBuff[y][x];
		  						}*/
		  						/*if(newDenseBuff[y + i][x + j]<0){
		  							newDenseBuff[y + i][x + j] = 0.0f;
		  						}*/
		  					}
		  				}
	  				}
	  			}
	  		}else{
	  			newDenseBuff[y][x] = 1;
	  		}
  		}else{
  			float bestPen = 0;
  			float bestDist = 0.0f;
  			for(int n = 1;n<4;n++){
  				if(penBuff[y+n][x] > bestPen){
  					bestPen = penBuff[y+n][x];
  				}
  				if(penBuff[y-n][x] > bestPen){
  					bestPen = penBuff[y-n][x];
  				}
  				if(penBuff[y][x+n] > bestPen){
  					bestPen = penBuff[y][x+n];
  				}
  				if(penBuff[y][x-n] > bestPen){
  					bestPen = penBuff[y][x-n];
  				}
  				if(distBuff[y+n][x] > bestDist){
  					bestDist = distBuff[y+n][x];
  				}
  				if(distBuff[y-n][x] > bestDist){
  					bestDist = distBuff[y-n][x];
  				}
  				if(distBuff[y][x+n] > bestDist){
  					bestDist = distBuff[y][x+n];
  				}
  				if(distBuff[y][x-n] > bestDist){
  					bestDist = distBuff[y][x-n];
  				}
  			}
  			int radius = bestPen + 1;
  			if(bestPen > 0){
	  			float buildup = 0.0f;
	  			for(int i=-radius;i<radius+1;i++){
	  				if(y+i > 0 && y+i<SCREEN_HEIGHT){
		  				for(int j=-radius;j<radius+1;j++){
		  					if(x+j > 0 && x+j < SCREEN_WIDTH){
		  						buildup += denseBuff[y + i][x + j];
		  					}
		  				}
	  				}
	  			}
	  			buildup = (float) buildup/(float) ((2*radius + 1)*(2*radius + 1));
	  			for(int i=-radius;i<radius+1;i++){
	  				if(y+i > 0 && y+i<SCREEN_HEIGHT){
		  				for(int j=-radius;j<radius+1;j++){
		  					if(x+j > 0 && x+j < SCREEN_WIDTH){
		  						int indeX = j;
		  						int indeY = i;
		  						if(indeY < 0){
		  							indeY = -indeY;
		  						}
		  						if(indeX < 0){
		  							indeX = -indeX;
		  						}
		  						float offRat = (float)(1.1 - (((float) (indeY + indeX))/(float) (2*radius)))/ (float) ((bestDist + 1.0f)*(bestDist + 1.0f));
		  						newDenseBuff[y + i][x + j] += (float) (buildup) * offRat;
		  						if(newDenseBuff[y + i][x + j] > maxDense){
		  							maxDense = newDenseBuff[y + i][x + j];
		  						}
		  						if (newDenseBuff[y+i][x+j] > 1.0f){newDenseBuff[y+i][x+j] = 1.0f;}
		  						distBuff[y + i][x + j] == bestDist;
		  						//cout << bestDist << " ";
		  						/*if(newDenseBuff[y + i][x + j]<0){
		  							newDenseBuff[y + i][x + j] = 0.0f;
		  						}*/
		  					}
		  				}
	  				}
	  			}
	  		}else{
	  			newDenseBuff[y][x] = 0;
	  		}
  		}
  	}
  }
  for(y = 0;y < SCREEN_HEIGHT;y++){
  	for(x = 0;x < SCREEN_WIDTH;x++){
  		/*if(distBuff[y][x] = 1.0f){
  			distBuff[y][x] = maximumDist;
  		}*/
  		//cout << maximumDist << " ";
  		//cout << maxDense << " ";	
  		newDenseBuff[y][x] = (float) ((newDenseBuff[y][x])/(float)maxDense);
  		vec3 colour;
  		if(distBuff[y][x] == 1){
  			distBuff[y][x] = maximumDist;
  		//	cout << distBuff[y][x] << " : ";
  		}
  		float tempDist = (float) distBuff[y][x] / (float) maximumDist;
  		//cout << tempDist << " ";
  		//colour = vec3(0,distBuff[y][x],0);	
  		colour = fakeScreen[y][x]*(1.0f - (newDenseBuff[y][x]) * (1 - tempDist));	
  		PutPixelSDL(screen, x ,y, colour);
  		distBuff[y][x] = 0;
  		denseBuff[y][x] = 0;
  		newDenseBuff[y][x] = 0;
  		penBuff[y][x] = 0;
  		//maximumDist = 0;
  		fakeScreen[y][x] = vec3(0,0,0);
  	}
  }

}

/*Place updates of parameters here*/
void Update(Camera& myCam)
{
  	static int t = SDL_GetTicks();
	int t2 = SDL_GetTicks();
	float dt = float(t2-t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;
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
	if( keystate[SDL_SCANCODE_UP] )
	{
		if(dirLight.position.z + 0.1 > 1){

		}else{
			dirLight.position.z += 0.1;
		}
	// Move camera to the right
	}
	if( keystate[SDL_SCANCODE_DOWN] )
	{
		if(dirLight.position.z - 0.1 < -1){

		}else{
			dirLight.position.z -= 0.1;
		}
	// Move camera to the right
	}
	if( keystate[SDL_SCANCODE_RIGHT] )
	{
		if(dirLight.position.x + 0.1 > 1){

		}else{
			dirLight.position.x += 0.1;
		}
	// Move camera to the right
	}
	if( keystate[SDL_SCANCODE_LEFT] )
	{
		if(dirLight.position.x - 0.1 < -1){

		}else{
			dirLight.position.x -= 0.1;
		}
	// Move camera to the right
	}
	if( keystate[SDL_SCANCODE_KP_MINUS] )
	{
		if(dirLight.position.y + 0.1 > 1){

		}else{
			dirLight.position.y += 0.1;
		}
	// Move camera to the right
	}
	if( keystate[SDL_SCANCODE_KP_PLUS] )
	{
		if(dirLight.position.y - 0.1 < -1){

		}else{
			dirLight.position.y -= 0.1;
		}
	// Move camera to the right
	}
}
