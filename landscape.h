
#include <SDL_opengl.h>

GLfloat landscapeVertexData[] = {
     0.010, 0.000, 0.000,
     -0.310, 0.000, 0.095,
     -0.121, 0.000, 0.000,
     -0.500, 0.000, 0.000,
     0.135, 0.000, 0.062,
     -0.500, 0.000, 0.000,
     0.198, 0.000, 0.031,
     0.385, 0.000, 0.000,
     0.260, 0.000, 0.062,
     0.500, 0.000, 0.057,
     0.500, 0.000, 0.000
};

#define LANDSCAPE_NUM_VERTEX 33
#define LANDSCAPE_VERTEX_DATA_SIZE (LANDSCAPE_NUM_VERTEX * sizeof(GLfloat))

GLuint landscapeEdgeData[] = {
     4, 0,
     1, 2,
     10, 3,
     1, 5,
     4, 6,
     8, 7,
     8, 6,
     9, 7
};

#define LANDSCAPE_NUM_EDGE 16
#define LANDSCAPE_EDGE_DATA_SIZE (LANDSCAPE_NUM_EDGE * sizeof(GLuint))

