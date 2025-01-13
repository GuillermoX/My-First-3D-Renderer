#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <string.h>
#include <stddef.h>

//------- 3D MATH DEFINES -------------------------
#define MAX_TRI_MESH 7000
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
	float x, y, z, w;
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

//-------- VECTOR AND MATRIX FUNCTIONS ------------------------------------

//Initialize matrices
void initialize_matrices(scene_t* scene);
void init_matrix (mat4x4_t* matrix);
void identity_matrix (mat4x4_t* matrix);
void rotation_matrix_x (mat4x4_t* matrix, float angle_rad);
void rotation_matrix_y (mat4x4_t* matrix, float angle_rad);
void rotation_matrix_z (mat4x4_t* matrix, float angle_rad);
void translation_matrix (mat4x4_t* matrix, float x, float y, float z);
void projection_matrix (mat4x4_t* matrix, float fov, float aspect_r, float z_far, float z_near);

void update_matrices();

//Vector & Matrix operations
void multiply_vector_matrix(vec3d_t* in, mat4x4_t* mat, vec3d_t* out);
void add_vectors(vec3d_t* v1, vec3d_t* v2, vec3d_t* vr);
void sub_vectors(vec3d_t* v1, vec3d_t* v2, vec3d_t* vr);
void vector_mul(vec3d_t* v, float k, vec3d_t* vr);
void vector_div(vec3d_t* v, float k, vec3d_t* vr);
float vector_dot_prod(vec3d_t* v1, vec3d_t* v2);
float vector_length(vec3d_t* v);
void normalise_vector(vec3d_t* v, vec3d_t* vn);
void vector_cross_prod(vec3d_t* v1, vec3d_t* v2, vec3d_t* vr);
void mul_matrices (mat4x4_t* m1, mat4x4_t* m2, mat4x4_t* mr);





