
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define MATH_3D_IMPLEMENTATION
#include "math_3d.h"
#include "tank.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define ASPECT_RATIO ((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)
#define FOV 60
#define FPS 30
#define TICKS_PER_SECOND ((float)1000 / (float)FPS)

#define SW_OVER_TWO (SCREEN_WIDTH / 2)
#define SH_OVER_TWO (SCREEN_HEIGHT / 2)
#define SCREEN_SPACE_X(x) (x * SCREEN_WIDTH + SW_OVER_TWO)
#define SCREEN_SPACE_Y(y) (y * SCREEN_HEIGHT + SH_OVER_TWO)

void init();
void initTank();
void update(float dt);
void render();
void close();
void printProgramLog( GLuint program );
void printShaderLog( GLuint shader );

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

unsigned char *keys;
int quit = 0;

float gTankRotSpeed = 2 * M_PI; // one revolution per second
float gTankSpeed = 5.0f; // 5 units per second

float gPlayerInputY = 0;
float gPlayerInputRot = 0;

vec3_t gTankPosition;
float gTankRotZ;
mat4_t gTankModelMat;

void init()
{
	//Initialize SDL
	SDL_Init( SDL_INIT_VIDEO );
	
    //Create window
    gWindow = SDL_CreateWindow( "Tank Demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );

    gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
    //Initialize renderer color
	SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
}

void initTank()
{
    gTankPosition = vec3(0,0,0);
    gTankRotZ = 0;
}

void update(float dt)
{
    // parse player input
    // https://wiki.libsdl.org/SDL_Scancode
    
    if (keys[SDL_SCANCODE_W])
        gPlayerInputY = 1;
    else if (keys[SDL_SCANCODE_S])
        gPlayerInputY = -1;
    else
        gPlayerInputY = 0;
    
    if (keys[SDL_SCANCODE_A])
        gPlayerInputRot = 1;
    else if (keys[SDL_SCANCODE_D])
        gPlayerInputRot = -1;
    else
        gPlayerInputRot = 0;
    
    int x = 0, y = 0;
    SDL_GetMouseState( &x, &y );
    
    
    // move tank with player input
    // rotation
    gTankRotZ += gPlayerInputRot * gTankRotSpeed * dt;
    gTankRotZ = fmod(gTankRotZ, 2.0f * M_PI);
    
    // get forward vector
    vec3_t velocity = vec3(0.0f, gPlayerInputY * gTankSpeed * dt, 0.0f);
    mat4_t rotation = m4_rotation_z(gTankRotZ);
    velocity = m4_mul_dir(rotation, velocity);
    
    // apply velocity
    gTankPosition = v3_add(gTankPosition, velocity);
    
    
    // update matrices
    // projection matrix
    mat4_t proj = m4_perspective(FOV, ASPECT_RATIO, 0.1f, 100.0f);
    
    // so the coordinate system is:
    // -Y is up
    // -Z is forward
    // X is right
    // ...
    
    // view matrix
    mat4_t view = m4_translation(vec3(0, -1, 5));
    view = m4_mul(view, m4_rotation_x(-M_PI / 2));
    
    // model matrix   
    mat4_t model = m4_translation(gTankPosition);
    model = m4_mul(model, m4_rotation_z(gTankRotZ));
    
    mat4_t mv = m4_mul(proj, view);
    gTankModelMat = m4_mul(mv, model);
}

void render()
{
    SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0xFF );
    SDL_RenderClear( gRenderer );
    
    SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

    for (int e = 0; e < EDGES / 2; ++e)
    {
        int edge = e * 2;
        int v1 = edgeData[edge];
        int v2 = edgeData[edge+1];
        
        float x1, y1, z1;
        int vert1 = v1 * 3;
        x1 = vertexData[vert1];
        y1 = vertexData[vert1+1];
        z1 = vertexData[vert1+2];
        vec3_t vec1 = vec3(x1, y1, z1);
        vec1 = m4_mul_pos(gTankModelMat, vec1);
        
        float x2, y2, z2;
        int vert2 = v2 * 3;
        x2 = vertexData[vert2];
        y2 = vertexData[vert2+1];
        z2 = vertexData[vert2+2];
        vec3_t vec2 = vec3(x2, y2, z2);
        vec2 = m4_mul_pos(gTankModelMat, vec2);
        
        // convert points to screen space and draw
        float p1x = SCREEN_SPACE_X(vec1.x);
        float p1y = SCREEN_SPACE_Y(vec1.y);
        float p2x = SCREEN_SPACE_X(vec2.x);
        float p2y = SCREEN_SPACE_Y(vec2.y);
        
        // draw
        SDL_RenderDrawLine(gRenderer, p1x, p1y, p2x, p2y);
    }
    
    SDL_RenderPresent( gRenderer );
}

void close()
{
    SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
    gRenderer = NULL;

	SDL_Quit();
}

int main(int argc, char *argv[])
{    
	init();
    
    initTank();
    keys = SDL_GetKeyboardState(NULL);
    
    SDL_Event e;
    unsigned int frameTicks = 0;
    float dt = 0;
    
    while( !quit )
    {
        int ticks = SDL_GetTicks();
        dt = ticks - frameTicks;
        frameTicks = ticks;
        
        // print fps
        // printf("%i\n", (int)(1000 / dt));
        
        while(SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
                quit = 1;
        }
        
        update(dt / 1000.0f);
        render();
        
        // sleep
        int end = SDL_GetTicks();
        float delay = TICKS_PER_SECOND - (end - ticks);
        if (delay > 0)
            SDL_Delay( delay );
    }

	close();

	return 0;
}