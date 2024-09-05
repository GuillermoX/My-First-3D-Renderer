#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <string.h>
#include <stddef.h>

//------- 3D MATH DEFINES -------------------------
#define MAX_TRI_MESH 3000
#define RAD_CONVERT 2*M_PI/360

//------- GPSCENECREATOR DEFINES ------------------
//Number of keys used from keyboard
#define NUM_OF_KEYS 5



//---------- COLOR STRUCT --------------------------------

typedef struct
{
	int r,g,b;
}rgb_t;	


//------- 3D MATH STRUCTURES --------------------------------


typedef struct
{
	int x, y;
} screen_vect_t;

typedef struct
{
	float x, y, z;
} vec3d_t;

typedef struct
{
	vec3d_t vertex[3];
	rgb_t color;
	float brightness;
} triangle_t;

typedef struct
{
	int n_tris;
	triangle_t tris[MAX_TRI_MESH];
} mesh_t;

typedef struct
{
	float m[4][4];
} mat4x4_t;




//---------- SCENE STRUCT -------------------------------

//Enum of the keyboard keys used
typedef enum
{
	KEY_esc,
	KEY_w,
	KEY_s,
	KEY_a,
	KEY_d
} keys_enum;


typedef struct
{
	int height;
	int width;
} window_prop_t;

typedef struct
{
	vec3d_t pos;
	float z_near;
	float z_far;
	float fov;
	float aspect_ratio;
} camera_t;

typedef struct
{
	char name[20];
	bool running;
	window_prop_t window;
	camera_t camera;
	vec3d_t light;
} scene_t;



//------- SCENE CREATOR FUNCTIONS ---------------------------

bool create_window(scene_t* scene, char window_name[]);
void start_scene(scene_t* scene);
void process_input(scene_t* scene);
void update_scene(scene_t* scene);
void render_scene(scene_t* scene);
void stop_scene();


//--------- 3D RENDERER FUNCTIONS ----------------------------

void initialize_meshes(mesh_t meshes[]);
void render_meshes(scene_t* scene, mesh_t meshes[]);
void process_triangle(scene_t* scene, triangle_t* tri, triangle_t* tri_ras);
void raster_triangle(scene_t* scene, triangle_t* tri);
void GPC_paint_triangle(scene_t* scene, screen_vect_t v1, screen_vect_t v2, screen_vect_t v3, rgb_t color);

//Matrix functions
void initialize_matrices(scene_t* scene);
void update_matrices();
void multiply_vector_matrix(vec3d_t* out, vec3d_t* in, mat4x4_t* mat);





