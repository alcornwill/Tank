
#include <SDL.h>
#include <gl\glew.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>

#define MATH_3D_IMPLEMENTATION
#include "math_3d.h"
#include "tank.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define ASPECT_RATIO ((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)
#define FOV 60
#define NEAR 0.1f
#define FAR 100.0f
#define FPS 30
#define TICKS_PER_SECOND ((float)1000 / (float)FPS)

#define PI_OVER_180 0.0174532925199432957692369076849f
#define _180_OVER_PI 57.2957795130823208767981548141f

#define RADIANS(x) (x * PI_OVER_180)
#define DEGREES(x) (x * _180_OVER_PI)

void init();
void initGL();
void initTank();
void update(float dt);
void render();
void close();
void printProgramLog( GLuint program );
void printShaderLog( GLuint shader );

SDL_Window* gWindow = NULL;
SDL_GLContext gContext;

unsigned char *keys;
int quit = 0;

float gTankRotSpeed = 2 * M_PI; // one revolution per second
float gTankSpeed = 5.0f; // 5 units per second

float gPlayerInputY = 0;
float gPlayerInputRot = 0;

vec3_t gTankPosition;
float gTankRotZ;

void init()
{
	//Initialize SDL
	SDL_Init( SDL_INIT_VIDEO );
	
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );

    //Create window
    gWindow = SDL_CreateWindow( "Tank Demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );

    //Create context
    gContext = SDL_GL_CreateContext( gWindow );
    
    // Initialize GLEW
    glewExperimental = GL_TRUE; 
    glewInit();

    //Initialize OpenGL
    initGL();
}

void initGL()
{  
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glClearColor( 0.f, 0.f, 0.f, 1.f );

    int w = SCREEN_WIDTH / 2;
    int h = SCREEN_HEIGHT / 2;
    
    float fovy = RADIANS(FOV);
    float tangent = tanf(fovy / 2.0f);
    float height = NEAR * tangent;
    float width = height * ASPECT_RATIO;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-width, width, -height, height, NEAR, FAR);
    
    // view transform
    glTranslatef(0, -1, -5);
    glRotatef(-90, 1.0f, 0.0f, 0.0f);
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
}

void render()
{
	glClear( GL_COLOR_BUFFER_BIT );

    glMatrixMode(GL_MODELVIEW);
    
    // tank
    glLoadIdentity();
    glTranslatef(gTankPosition.x, gTankPosition.y, gTankPosition.z);
    glRotatef(DEGREES(gTankRotZ), 0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
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
        glVertex3f(x1, y1, z1);
        
        float x2, y2, z2;
        int vert2 = v2 * 3;
        x2 = vertexData[vert2];
        y2 = vertexData[vert2+1];
        z2 = vertexData[vert2+2];
        glVertex3f(x2, y2, z2);
    }
    glEnd();
}

void close()
{    
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;

	SDL_Quit();
}

void printProgramLog( GLuint program )
{
	if( glIsProgram( program ) )
	{
		int infoLogLength = 0;
		int maxLength = infoLogLength;
		
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &maxLength );
		
		char* infoLog = malloc(sizeof(char) * maxLength);
		
		glGetProgramInfoLog( program, maxLength, &infoLogLength, infoLog );
		if( infoLogLength > 0 )
			printf( "%s\n", infoLog );
		
		free(infoLog);
	}
	else
		printf( "Name %d is not a program\n", program );
}

void printShaderLog( GLuint shader )
{
	if( glIsShader( shader ) )
	{
		int infoLogLength = 0;
		int maxLength = infoLogLength;
		
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );
		
		char* infoLog = malloc(sizeof(char) * maxLength);
		
		glGetShaderInfoLog( shader, maxLength, &infoLogLength, infoLog );
		if( infoLogLength > 0 )
			printf( "%s\n", infoLog );

		free(infoLog);
	}
	else
		printf( "Name %d is not a shader\n", shader );
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
        
        SDL_GL_SwapWindow( gWindow );
        
        // sleep
        int end = SDL_GetTicks();
        float delay = TICKS_PER_SECOND - (end - ticks);
        if (delay > 0)
            SDL_Delay( delay );
    }

	close();

	return 0;
}