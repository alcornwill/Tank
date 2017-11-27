
#include <SDL.h>
#include <gl\glew.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>

#define MATH_3D_IMPLEMENTATION
#include "math_3d.h"
#include "tank.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int init();
int initGL();
void initTank();
void update(float dt);
void render();
void close();
void printProgramLog( GLuint program );
void printShaderLog( GLuint shader );

SDL_Window* gWindow = NULL;
SDL_GLContext gContext;

GLuint gProgramID = 0;
GLint gVertexPos3DLocation = -1;
GLint gModelMatrixLocation = -1;
GLuint gVBO = 0;
GLuint gIBO = 0;

unsigned char *keys;
int quit = 0;

float gTankRotSpeed = 2 * M_PI; // one revolution per second
float gTankSpeed = 5.0f; // 5 units per second

float gPlayerInputY = 0;
float gPlayerInputRot = 0;

vec3_t gTankPosition;
vec3_t gTankRotation;
mat4_t gTankModelMat;

int init()
{
	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		return 0;
	}
	
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    //Create window
    gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
    if( gWindow == NULL )
    {
        printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
        return 0;
    }

    //Create context
    gContext = SDL_GL_CreateContext( gWindow );
    if( gContext == NULL )
    {
        printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
        return 0;
    }
    
    // Initialize GLEW
    glewExperimental = GL_TRUE; 
    GLenum glewError = glewInit();
    if( glewError != GLEW_OK )
    {
        printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
    }

    //Initialize OpenGL
    if( !initGL() )
    {
        printf( "Unable to initialize OpenGL!\n" );
        return 0;
    }

	return 1;
}

int initGL()
{
	gProgramID = glCreateProgram();
    
    // vertex shader
	GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
	const GLchar* vertexShaderSource[] =
	{
		"#version 140\nin vec3 LVertexPos3D;\nuniform mat4 modelMatrix;\n void main() { vec4 pos = vec4(LVertexPos3D, 1);\n pos = modelMatrix * pos;\n gl_Position = pos; }"
	};
	glShaderSource( vertexShader, 1, vertexShaderSource, NULL );
	glCompileShader( vertexShader );

	// check for errors
	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &vShaderCompiled );
	if( vShaderCompiled != GL_TRUE )
	{
		printf( "Unable to compile vertex shader %d!\n", vertexShader );
		printShaderLog( vertexShader );
        return 0;
	}
    
    glAttachShader( gProgramID, vertexShader );

    // fragment shader
    GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
    const GLchar* fragmentShaderSource[] =
    {
        "#version 140\nout vec4 LFragment; void main() { LFragment = vec4( 1.0, 1.0, 1.0, 1.0 ); }"
    };
    glShaderSource( fragmentShader, 1, fragmentShaderSource, NULL );
    glCompileShader( fragmentShader );

    // check for errors
    GLint fShaderCompiled = GL_FALSE;
    glGetShaderiv( fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled );
    if( fShaderCompiled != GL_TRUE )
    {
        printf( "Unable to compile fragment shader %d!\n", fragmentShader );
        printShaderLog( fragmentShader );
        return 0;
    }
    
    glAttachShader( gProgramID, fragmentShader );

    //Link program
    glLinkProgram( gProgramID );

    //Check for errors
    GLint programSuccess = GL_TRUE;
    glGetProgramiv( gProgramID, GL_LINK_STATUS, &programSuccess );
    if( programSuccess != GL_TRUE )
    {
        printf( "Error linking program %d!\n", gProgramID );
        printProgramLog( gProgramID );
        return 0;
    }
        
    //Get vertex attribute location
    gVertexPos3DLocation = glGetAttribLocation( gProgramID, "LVertexPos3D" );
    if( gVertexPos3DLocation == -1 )
    {
        printf( "LVertexPos3D is not a valid glsl program variable!\n" );
        return 0;
    }
    
    //Get model matrix location
    gModelMatrixLocation = glGetUniformLocation( gProgramID, "modelMatrix" );
    if( gModelMatrixLocation == -1 )
    {
        printf( "modelMatrix is not a valid glsl program variable!\n" );
        return 0;
    }
            
    //Initialize clear color
    glClearColor( 0.f, 0.f, 0.f, 1.f );

    //Create VBO
    glGenBuffers( 1, &gVBO );
    glBindBuffer( GL_ARRAY_BUFFER, gVBO );
    glBufferData( GL_ARRAY_BUFFER, VERTICES * sizeof(GLfloat), vertexData, GL_STATIC_DRAW );

    //Create IBO
    glGenBuffers( 1, &gIBO );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, EDGES * sizeof(GLuint), edgeData, GL_STATIC_DRAW );
	
	return 1;
}

void initTank()
{
    gTankPosition = vec3(0,0,0);
    gTankRotation = vec3(0,0,0);
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
    gTankRotation.z += gPlayerInputRot * gTankRotSpeed * dt;
    gTankRotation.z = fmod(gTankRotation.z, 2.0f * M_PI);
    // printf("%f", gPlayerInputRot);
    
    // get forward vector
    vec3_t velocity = vec3(0.0f, gPlayerInputY * gTankSpeed * dt, 0.0f);
    mat4_t rotation = m4_rotation_z(gTankRotation.z);
    velocity = m4_mul_dir(rotation, velocity);
    
    // apply velocity
    gTankPosition = v3_add(gTankPosition, velocity);
    
    
    // update model matrix
	mat4_t mat = m4_identity();
    mat4_t translate = m4_translation(gTankPosition);
    mat4_t rotx = m4_rotation_x(gTankRotation.x);
    mat4_t roty = m4_rotation_y(gTankRotation.y);
    mat4_t rotz = m4_rotation_z(gTankRotation.z);
    // fuck scale
    
    mat = m4_mul(mat, translate);
    mat = m4_mul(mat, rotx);
    mat = m4_mul(mat, roty);
    mat = m4_mul(mat, rotz);
    
    gTankModelMat = mat;
}

void render()
{
	glClear( GL_COLOR_BUFFER_BIT );
    
    glUseProgram( gProgramID );
    glEnableVertexAttribArray( gVertexPos3DLocation );
    
    glUniformMatrix4fv(gModelMatrixLocation, 1, GL_FALSE, &gTankModelMat);
    
    glBindBuffer( GL_ARRAY_BUFFER, gVBO );
    glVertexAttribPointer( gVertexPos3DLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), NULL );
    
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
    glDrawElements( GL_LINES, EDGES, GL_UNSIGNED_INT, NULL );

    glDisableVertexAttribArray( gVertexPos3DLocation );

    glUseProgram( 0 );
}

void close()
{
	glDeleteProgram( gProgramID );
    
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
	if( !init() )
    {
		printf( "Failed to initialize!\n" );
        close();
        return 0;
    }
    
    initTank();
    keys = SDL_GetKeyboardState(NULL);
    
    SDL_Event e;
    unsigned int frameTicks = 0;
    float dt = 0;
    
    while( !quit )
    {
        int ticks = SDL_GetTicks();
        dt = (ticks - frameTicks) / 1000.0f;
        frameTicks = ticks;
        
        while(SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
                quit = 1;
        }
        
        update(dt);
        render();
        
        SDL_GL_SwapWindow( gWindow );
        
        // sleep
        SDL_Delay( 16 ); // todo this properly
    }

	close();

	return 0;
}