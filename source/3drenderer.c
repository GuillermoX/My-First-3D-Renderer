#include "../include/3drenderer.h"

//--------- OBJECT DEFINES ---------------------------------------------------
#define N_MESHES 1

//--------- FRAME DEFINITIONS ------------------------------
#define FPS 40
#define FRAME_TARGET_TIME 1000/FPS

//FPS Control variables
float delta_time;
int last_frame_time;

//Windows and render global variables
SDL_Window* gp_window;
SDL_Renderer* gp_renderer;

//List of boolean state of the keys used
bool gp_keys[NUM_OF_KEYS];

//--------- OBJECTS OF THE SCENE -------------------------------------------

mesh_t meshes[N_MESHES];
float angle;	//Temporary angle to rotate the meshes

//--------- TRANSFORMATION MATRICES ---------------------------------------------

//Projection matrix
mat4x4_t proj_matrix;
//Y-axys rotation matrix
mat4x4_t rot_matrix_z, rot_matrix_x, rot_cam_matrix;

//------ START & STOP SCENE FUNCTIONS ------------------------------------------

void start_scene(scene_t* scene)
{

	//Set the basic initial parameters
	scene->running = true;
	strcpy(scene->name, "3D Renderer");

	//Here you can change the scene initial parameters
	scene->window.height = 700;
	scene->window.width = 1200;

	//Start the camera properties
	scene -> camera.pos.x = 0;	scene->camera.pos.y = 0;	scene->camera.pos.z = 0;
	scene -> camera.look_dir.x = 0;	scene->camera.look_dir.y = 0;	scene->camera.look_dir.z = 1;
	scene->camera.rot_y_angle = 0;

	scene->camera.z_near = 0.1f;
	scene->camera.z_far = 1000.0f;
	scene->camera.fov = 90.0f;
	scene->camera.aspect_ratio = (float)scene->window.height / (float)scene->window.width;

	//Start the simple illumination
	scene->light.x = 0;
	scene->light.y = 0;
	scene->light.z = -1;	
	//Normalize ilumination vector
	float l_lenth = sqrt(scene->light.x*scene->light.x + scene->light.y*scene->light.y + scene->light.z*scene->light.z);
	scene->light.x /= l_lenth; scene->light.y /= l_lenth; scene->light.z /= l_lenth;


	//Initialize meshes
	initialize_meshes(meshes);
	angle = 0;

	//Initialize transformation matrices
	initialize_matrices(scene);
	
	//Initialize keys
	for(int i=0; i<NUM_OF_KEYS; i++)
	{
		gp_keys[i] = false;
	}

	//Create the window
	create_window(scene, scene->name);	
}


bool create_window(scene_t* scene, char window_name[])
{
	bool ini_correct = true;

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		ini_correct = false;	//Error while initializing SDL	
	}
	else
	{
		gp_window = SDL_CreateWindow(scene->name,
							       SDL_WINDOWPOS_CENTERED,
							       SDL_WINDOWPOS_CENTERED,
							       scene->window.width,
							       scene->window.height,
							       0);
		if (!gp_window)
		{
			ini_correct = false;
		}
		else
		{

			gp_renderer = SDL_CreateRenderer(gp_window, -1, 0);
			if(gp_renderer)
			{
				ini_correct = false;
			}
		}
	}

	return ini_correct;
	
}


void stop_scene()
{
	SDL_DestroyRenderer(gp_renderer);
	SDL_DestroyWindow(gp_window);
	SDL_Quit();
}



//-------------- GET INPUT FUNCTIONS -----------------------------------------------

void process_input(scene_t* scene)
{
	SDL_Event event;
	SDL_PollEvent(&event);
	

	switch (event.type)
	{
		case SDL_QUIT:
			scene->running = false;
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
				case SDLK_ESCAPE:
					gp_keys[KEY_esc] = true;
					break;
				case SDLK_w:
					gp_keys[KEY_w] = true;
					break;
				case SDLK_s:
					gp_keys[KEY_s] = true;
					break;
				case SDLK_a:
					gp_keys[KEY_a] = true;
					break;
				case SDLK_d:
					gp_keys[KEY_d] = true;
					break;
				case SDLK_SPACE:
					gp_keys[KEY_space] = true;
					break;
				case SDLK_LSHIFT:
					gp_keys[KEY_lshift] = true;
					break;
				case SDLK_LEFT:
					gp_keys[KEY_left] = true;
					break;
				case SDLK_RIGHT:
					gp_keys[KEY_right] = true;
					break;
			}
			break;
		case SDL_KEYUP:
			switch (event.key.keysym.sym)
			{
				case SDLK_w:
					gp_keys[KEY_w] = false;
					break;
				case SDLK_s:
					gp_keys[KEY_s] = false;
					break;
				case SDLK_a:
					gp_keys[KEY_a] = false;
					break;
				case SDLK_d:
					gp_keys[KEY_d] = false;
					break;
				case SDLK_SPACE:
					gp_keys[KEY_space] = false;
					break;
				case SDLK_LSHIFT:
					gp_keys[KEY_lshift] = false;
					break;
				case SDLK_LEFT:
					gp_keys[KEY_left] = false;
					break;
				case SDLK_RIGHT:
					gp_keys[KEY_right] = false;
					break;
				
			}
			break;	

	}
	
}

//------------ UPDATE FUNCTIONS --------------------------------------------------

void update_scene(scene_t* scene)
{	
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);
	if(time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) SDL_Delay(time_to_wait); 
	//Update delta time
	delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
	last_frame_time = SDL_GetTicks();

	//Stop running if scape is pressed
	if(gp_keys[KEY_esc]) scene->running = false;

	if(gp_keys[KEY_a]) move_camera(scene, 1, 0, 0);
	if(gp_keys[KEY_d]) move_camera(scene, -1, 0, 0);
	if(gp_keys[KEY_w]) move_camera(scene, 0, 0, 1);
	if(gp_keys[KEY_s]) move_camera(scene, 0, 0, -1);
	if(gp_keys[KEY_space]) move_camera(scene, 0, 1, 0);
	if(gp_keys[KEY_lshift]) move_camera(scene, 0, -1, 0);
	if(gp_keys[KEY_left]) rotate_camera(scene, -1);
	if(gp_keys[KEY_right]) rotate_camera(scene, 1);
	//Increase the rotation angle
	//angle += 80*delta_time;
	update_matrices(scene);


}

void move_camera(scene_t* scene, float x, float y, float z)
{
	scene->camera.pos.x += x*10*delta_time;
	scene->camera.pos.y += y*10*delta_time;
	scene->camera.pos.z += z*10*delta_time;
}

void rotate_camera(scene_t* scene, int dir)
{
	mat4x4_t rot_y;
	rotation_matrix_y(&rot_y, dir*10*delta_time*RAD_CONVERT);

	multiply_vector_matrix(&(scene->camera.look_dir), &rot_y, &(scene->camera.look_dir));
}


//------------ RENDER FUNCTIONS ---------------------------------------------------

void render_scene(scene_t* scene)
{
	
	//Set the background color
	SDL_SetRenderDrawColor(gp_renderer, 0, 0, 0, 255);
	SDL_RenderClear(gp_renderer);
	

	//SDL_SetRenderDrawColor(gp_renderer, 255, 255, 255, 255);
	//Draw a point
	//SDL_RenderDrawPoint(gp_renderer, 100, 100);

	//Draw a line between two points
	//SDL_RenderDrawLine(gp_renderer, 100, 30, 200, 200);

	render_meshes(scene, meshes);

	//Show the final render result
	SDL_RenderPresent(gp_renderer);
}






///////////////////////////////////////////////////////////////////////////////////////////
//		3D MANIPULATION FUNCTIONS						///
///////////////////////////////////////////////////////////////////////////////////////////

void initialize_meshes(mesh_t meshes[])
{
	//Open the .obj file
	FILE* f = fopen("./data/object.obj", "r");
	if(f != NULL)
	{	
		//List to store all vertex from .obj (Fix size to make it simpler) TODO: Dinamic storage
		vec3d_t verts[4000];

		//Character of the .obj file that indicates if the information of the line is a vertex or a triangle
		char type_line;

		//Initialize at 1 because .obj file starts counting the first vertex as 1
		int vert_i = 1;

		//Index of the vertex of a triangle
		int i_v1, i_v2, i_v3;

		//Initialize the number of triangles of the mesh to 0
		meshes[0].n_tris = 0;

		//Load all the vertex to a buffer and the faces/triangles to the mesh
		while (!feof(f))
		{
			fscanf(f, "%c ", &type_line);
			switch (type_line)
			{
				case 'v': fscanf(f, "%f %f %f\n", &verts[vert_i].x, &verts[vert_i].y, &verts[vert_i].z);
					  verts[vert_i].w = 1.0f;
					  vert_i ++;
					break;
				case 'f': fscanf(f, "%d %d %d\n", &i_v1, &i_v2, &i_v3);
					  meshes[0].tris[meshes[0].n_tris].vertex[0].x = verts[i_v1].x;
					  meshes[0].tris[meshes[0].n_tris].vertex[0].y = verts[i_v1].y;
					  meshes[0].tris[meshes[0].n_tris].vertex[0].z = verts[i_v1].z;
					  meshes[0].tris[meshes[0].n_tris].vertex[0].w = verts[i_v1].w;

					  meshes[0].tris[meshes[0].n_tris].vertex[1].x = verts[i_v2].x;
					  meshes[0].tris[meshes[0].n_tris].vertex[1].y = verts[i_v2].y;
					  meshes[0].tris[meshes[0].n_tris].vertex[1].z = verts[i_v2].z;
					  meshes[0].tris[meshes[0].n_tris].vertex[1].w = verts[i_v1].w;

					  meshes[0].tris[meshes[0].n_tris].vertex[2].x = verts[i_v3].x;
					  meshes[0].tris[meshes[0].n_tris].vertex[2].y = verts[i_v3].y;
					  meshes[0].tris[meshes[0].n_tris].vertex[2].z = verts[i_v3].z;
					  meshes[0].tris[meshes[0].n_tris].vertex[2].w = verts[i_v1].w;

					  meshes[0].n_tris ++;
					break;

				default: fscanf(f, "\n");
			}
		}
		fclose(f);

	}

}

int compare_depth_tris(const void* tri1_p, const void* tri2_p)
{
	//Return value
	int ret;

	//Comparing depth
	float z1, z2;
	
	//Convert the const void pointer to const triangle_t pointer
	const triangle_t* tri1 = tri1_p;
	const triangle_t* tri2 = tri2_p;

	//Get the average of the three Z of each vertex of a triangle
	z1 = (tri1->vertex[0].z + tri1->vertex[1].z + tri1->vertex[2].z) / 3.0f;
	//Do the same for the second triangle
	z2 = (tri2->vertex[0].z + tri2->vertex[1].z + tri2->vertex[2].z) / 3.0f;

	if(z1 == z2){ ret = 0;}
	else if (z1 > z2){ ret = -1;}
	else if (z1 < z2){ ret = 1;}

	return ret;
}

void render_meshes(scene_t* scene, mesh_t meshes[])
{
	int n_tris;
	
	for(int i=0; i < N_MESHES; i++)
	{
		//Number of triangles in the mesh	
		n_tris = meshes[i].n_tris;

		//Tuple of triangles processed to raster used to sort them by depth	
		triangle_t tris[n_tris];

		for(int j=0; j < n_tris; j++)
		{	
			//Rotate, move, project and add the triangle to the raster tuple
			process_triangle(scene, &(meshes[i].tris[j]), &tris[j]);	
		}

		//Sort the triangles ready to raster depending on the depth they are
		qsort(tris, n_tris, sizeof(triangle_t), compare_depth_tris);

		for (int j=0; j < n_tris; j++)
		{
			raster_triangle(scene, &tris[j]);
		}

	}
}

void process_triangle(scene_t* scene, triangle_t* tri, triangle_t* tri_ras)
{	

	triangle_t tri_proj, tri_trans, tri_rotated;
	mat4x4_t mat_world, mat_trans;

	//Create the translation matrix
	translation_matrix(&mat_trans, 0, 0, 100);

	//Do the chain matrix multiplication to get a rotation and translation matrix
	mul_matrices(&rot_matrix_z, &rot_matrix_x, &mat_world);
	mul_matrices(&mat_world, &mat_trans, &mat_world);
	
	multiply_vector_matrix(&(tri->vertex[0]), &mat_world, &(tri_trans.vertex[0]));	
	multiply_vector_matrix(&(tri->vertex[1]), &mat_world, &(tri_trans.vertex[1]));	
	multiply_vector_matrix(&(tri->vertex[2]), &mat_world, &(tri_trans.vertex[2]));

	multiply_vector_matrix(&(tri_trans.vertex[0]), &rot_cam_matrix, &(tri_rotated.vertex[0]));	
	multiply_vector_matrix(&(tri_trans.vertex[1]), &rot_cam_matrix, &(tri_rotated.vertex[1]));	
	multiply_vector_matrix(&(tri_trans.vertex[2]), &rot_cam_matrix, &(tri_rotated.vertex[2]));
	

	
	// ------ Calculate the normal normalized (between 0 and 1) -----
	vec3d_t normal, line1, line2;

	

	//Calculate the vector from v0 -> v1	
	sub_vectors(&tri_trans.vertex[1], &tri_trans.vertex[0], &line1);

	//Calculate the vector from v0 -> v2
	sub_vectors(&tri_trans.vertex[2], &tri_trans.vertex[0], &line2);

	//Do the cross product of the two vectors to get a perpendicular vector of both (the normal)	
	vector_cross_prod (&line1, &line2, &normal);

	//Normalize the normal vector
	normalise_vector(&normal, &normal);

	//Ray used to calculate if the triangle is visible to the camera
	vec3d_t cam_ray;
	sub_vectors(&(tri_trans.vertex[0]), &(scene->camera.pos), &cam_ray);

	//If the dot product of the normal and the camera vector is negative it means the angle between both
	//is greater than 90º, which means it's visible (check page 5 of notes)
	if(vector_dot_prod(&normal, &cam_ray) < 0.0f)
	{	
		//If the triangle is visible to the camera, proceed to calculate the projection of it
		//Do the multiplication of the vector of the vertex with the projection matrix
		//The result is the projection of the vertex at the screen (values between -1 and 1) from the center
		multiply_vector_matrix(&(tri_rotated.vertex[0]), &proj_matrix, &tri_proj.vertex[0]);
		multiply_vector_matrix(&(tri_rotated.vertex[1]), &proj_matrix, &tri_proj.vertex[1]);
		multiply_vector_matrix(&(tri_rotated.vertex[2]), &proj_matrix, &tri_proj.vertex[2]);
	
		//Scale the view (it used to be included in the vector matrix mul, but removed so it has to do this manually)
		vector_div(&(tri_proj.vertex[0]), tri_proj.vertex[0].w, &(tri_proj.vertex[0]));
		vector_div(&(tri_proj.vertex[1]), tri_proj.vertex[1].w, &(tri_proj.vertex[1]));
		vector_div(&(tri_proj.vertex[2]), tri_proj.vertex[2].w, &(tri_proj.vertex[2]));


		//Add 1 so the value is not negative and (0,0) is the top left corner and the range is [0,2]
		vec3d_t v_offset = {1,1,0,0};

		add_vectors(&(tri_proj.vertex[0]), &v_offset, &(tri_proj.vertex[0]));
		add_vectors(&(tri_proj.vertex[1]), &v_offset, &(tri_proj.vertex[1]));
		add_vectors(&(tri_proj.vertex[2]), &v_offset, &(tri_proj.vertex[2]));

		//Get half the hight and width of the screen resolution
		float h_height= scene->window.height/2.0f;
		float h_width = scene->window.width/2.0f;

		//Adjust the range from [0,2] to the width and height
		//Multiply the x value with the width to get the x value in the range of the screen width
		//For the height is the same but proportional to the width
		tri_proj.vertex[0].x *= h_width;	tri_proj.vertex[0].y = h_height*2 - tri_proj.vertex[0].y * h_height;
		tri_proj.vertex[1].x *= h_width;	tri_proj.vertex[1].y = h_height*2 - tri_proj.vertex[1].y * h_height;
		tri_proj.vertex[2].x *= h_width;	tri_proj.vertex[2].y = h_height*2 - tri_proj.vertex[2].y * h_height;

		//Get out the triangle information to raster
		tri_ras->vertex[0].x = tri_proj.vertex[0].x;	tri_ras->vertex[0].y = tri_proj.vertex[0].y;	tri_ras->vertex[0].z = tri_proj.vertex[0].z;
		tri_ras->vertex[1].x = tri_proj.vertex[1].x;	tri_ras->vertex[1].y = tri_proj.vertex[1].y;	tri_ras->vertex[1].z = tri_proj.vertex[1].z;
		tri_ras->vertex[2].x = tri_proj.vertex[2].x;	tri_ras->vertex[2].y = tri_proj.vertex[2].y;	tri_ras->vertex[2].z = tri_proj.vertex[2].z;
		
		//Get the brightness of the triangle
		tri_ras->brightness = vector_dot_prod(&(scene->light), &normal);
		if (tri_ras->brightness < 0) tri_ras->brightness = 0;
	}
	else
	{	
		//In case the triangle is not necessary to raster (is not seen by the camera) set all coords of all vertex to 0	
		tri_ras->vertex[0].x = 0;	tri_ras->vertex[0].y = 0;	tri_ras->vertex[0].z = 0;
		tri_ras->vertex[1].x = 0;	tri_ras->vertex[1].y = 0;	tri_ras->vertex[1].z = 0;
		tri_ras->vertex[2].x = 0;	tri_ras->vertex[2].y = 0;	tri_ras->vertex[2].z = 0;
	}



}

void raster_triangle(scene_t* scene, triangle_t* tri)
{
	//Calculate the color depending on the brightness	
	tri->color.r = 255*tri->brightness;
	tri->color.g = 0*tri->brightness;
	tri->color.b = 255/2*tri->brightness;

	//Create a screen vector (2D) for each triangle vertex projection
	screen_vect_t v1, v2, v3;
					
	v1.x = tri->vertex[0].x; v1.y = tri->vertex[0].y;
	v2.x = tri->vertex[1].x; v2.y = tri->vertex[1].y;
	v3.x = tri->vertex[2].x; v3.y = tri->vertex[2].y;	

	printf("v1.x = %d   //   v1.y = %d\n", v1.x, v1.y);
	printf("v2.x = %d   //   v2.y = %d\n", v2.x, v2.y);
	printf("v3.x = %d   //   v3.y = %d\n\n", v3.x, v3.y);
	GPC_paint_triangle(scene, v1, v2, v3, tri->color);

	//Decoment if wanted all triangles borders to be drawn
	/*	
	SDL_SetRenderDrawColor(gp_renderer, 255, 0, 255/2, 255);
	SDL_RenderDrawLine(gp_renderer, v1.x, v1.y, v2.x, v2.y);
	SDL_RenderDrawLine(gp_renderer, v1.x, v1.y, v3.x, v3.y);
	SDL_RenderDrawLine(gp_renderer, v2.x, v2.y, v3.x, v3.y);
	*/
		
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// 		SCREEN DRAWING FUNCTIONS 		/////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------- FILLED TRIANGLE DRAW FUNCTION --------------------------------------------------

void find_order(screen_vect_t v1, screen_vect_t v2, screen_vect_t v3, screen_vect_t* v_max, screen_vect_t* v_mid, screen_vect_t* v_min)
{
	
	screen_vect_t v_tuple[3];

	v_tuple[0].x = v1.x;
	v_tuple[0].y = v1.y;
	v_tuple[1].x = v2.x;
	v_tuple[1].y = v2.y;
	v_tuple[2].x = v3.x;
	v_tuple[2].y = v3.y;

	for (int i = 0; i < 2; i++)
	{
		int min = i;
		for (int j = i+1; j < 3; j++)
		{
			if(v_tuple[j].y < v_tuple[min].y) min = j;	
		}
		screen_vect_t v;
		v.x = v_tuple[i].x;
		v.y = v_tuple[i].y;
		v_tuple[i].x = v_tuple[min].x;
		v_tuple[i].y = v_tuple[min].y;
		v_tuple[min].x = v.x;
		v_tuple[min].y = v.y;

	}

	v_min->x = v_tuple[0].x;
	v_min->y = v_tuple[0].y;
	v_mid->x = v_tuple[1].x;
	v_mid->y = v_tuple[1].y;
	v_max->x = v_tuple[2].x;
	v_max->y = v_tuple[2].y;

}


void GPC_paint_triangle(scene_t* scene, screen_vect_t v1, screen_vect_t v2, screen_vect_t v3, rgb_t color)
{	
	screen_vect_t v_max, v_mid, v_min;
	
	SDL_SetRenderDrawColor(gp_renderer, color.r, color.g, color.b, 255);

	//finding the height order of the vertex is possible to divide the triangle at the diag in top triangle and bottom triangle
	find_order(v1, v2, v3, &v_max, &v_mid, &v_min);	
	
	screen_vect_t v_line1, v_line2;
	float k1, k2;
	int x1, x2;
	//if the higher and the mid vertex have the same "y" there is no bottom triangle to draw, if they are different the triangle is painted	
	if(v_max.y != v_mid.y)
	{
		//We get the vectors from (vmax->vmid) and (vmax->vmin)
		// vmax->vmid
		v_line1.x = v_mid.x - v_max.x;
		v_line1.y = v_mid.y - v_max.y;
		// vmax->vmid
		v_line2.x = v_min.x - v_max.x;
		v_line2.y = v_min.y - v_max.y;
		
		//Get the constant of the line formula for each line
		k1 = (float)v_line1.x/(float)v_line1.y;
		k2 = (float)v_line2.x/(float)v_line2.y;

		//For each y we get the x of each line, starting from the y of the vmax
		for (int y = v_max.y; y >= v_mid.y; y --)
		{
			x1 = k1*(y-v_max.y) + v_max.x + 0.5f;
			x2 = k2*(y-v_max.y) + v_max.x + 0.5f;
			SDL_RenderDrawLine(gp_renderer, x1, y, x2, y); 
		}
		
	}

	//If the lower and the mid vertex are at the same "y" we don't draw the top triangle
	if(v_min.y != v_mid.y)
	{
		//We get the vectors from (vmax->vmid) and (vmax->vmin)
		// vmax->vmid
		v_line1.x = v_min.x - v_mid.x;
		v_line1.y = v_min.y - v_mid.y;
		
		v_line2.x = v_min.x - v_max.x;
		v_line2.y = v_min.y - v_max.y;

		//Get the constant of the line formula for each line
		k1 = (float)v_line1.x/(float)v_line1.y;
		k2 = (float)v_line2.x/(float)v_line2.y;

		//For each y we get the x of each line, starting from the y of the vmax
		for (int y = v_mid.y; y >= v_min.y; y --)
		{
			x1 = k1*(y-v_mid.y) + v_mid.x + 0.5f;
			x2 = k2*(y-v_max.y) + v_max.x + 0.5f;
			SDL_RenderDrawLine(gp_renderer, x1, y, x2, y); 
		}
		
	}
	
	
}


// -------------------------------- MATH VECTOR AND MATRIX FUNCTIONS ----------------------------------------------------------------

void initialize_matrices(scene_t* scene)
{

	projection_matrix (&proj_matrix, scene->camera.fov, scene->camera.aspect_ratio, scene->camera.z_far, scene->camera.z_near);

	rotation_matrix_z (&rot_matrix_z, angle*RAD_CONVERT);	
	rotation_matrix_x (&rot_matrix_x, angle*RAD_CONVERT);
	
}

void update_matrices(scene_t* scene)
{	
	rotation_matrix_z (&rot_matrix_z, angle*RAD_CONVERT/2);	
	rotation_matrix_x (&rot_matrix_x, angle*RAD_CONVERT/2);	

	vec3d_t target;
	add_vectors(&(scene->camera.pos), &(scene->camera.look_dir), &target);
	vec3d_t up;
	up.x = 0;	up.y = 1;	up.z = 0;	up.w = 1;
	rotation_direction_matrix_inv(&rot_cam_matrix, &(scene->camera.pos), &target, &up);
}

void init_matrix (mat4x4_t* matrix)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			matrix->m[i][j] = 0;
		}
	}
}

void identity_matrix (mat4x4_t* matrix)
{
	init_matrix (matrix);
	matrix->m[0][0] = 1.0f;
	matrix->m[1][1] = 1.0f;
	matrix->m[2][2] = 1.0f;
	matrix->m[3][3] = 1.0f;
}

void rotation_matrix_x (mat4x4_t* matrix, float angle_rad)
{
	
	matrix->m[0][0] = 1;
	matrix->m[0][1] = 0;
	matrix->m[0][2] = 0;
	matrix->m[0][3] = 0;

	matrix->m[1][0] = 0;
	matrix->m[1][1] = cos(angle_rad);
	matrix->m[1][2] = sin(angle_rad);
	matrix->m[1][3] = 0;

	matrix->m[2][0] = 0;
	matrix->m[2][1] = -sin(angle_rad);
	matrix->m[2][2] = cos(angle_rad);
	matrix->m[2][3] = 0;

	matrix->m[3][0] = 0;
	matrix->m[3][1] = 0;
	matrix->m[3][2] = 0;
	matrix->m[3][3] = 1;
}

void rotation_matrix_y (mat4x4_t* matrix, float angle_rad)
{
	
	matrix->m[0][0] = cos(angle_rad);
	matrix->m[0][1] = 0;
	matrix->m[0][2] = sin(angle_rad);
	matrix->m[0][3] = 0;

	matrix->m[1][0] = 0;
	matrix->m[1][1] = 1;
	matrix->m[1][2] = 0;
	matrix->m[1][3] = 0;

	matrix->m[2][0] = -sin(angle_rad);
	matrix->m[2][1] = 0;
	matrix->m[2][2] = cos(angle_rad);
	matrix->m[2][3] = 0;

	matrix->m[3][0] = 0;
	matrix->m[3][1] = 0;
	matrix->m[3][2] = 0;
	matrix->m[3][3] = 1;
}

void rotation_matrix_z (mat4x4_t* matrix, float angle_rad)
{
	
	matrix->m[0][0] = cos(angle_rad);
	matrix->m[0][1] = sin(angle_rad);
	matrix->m[0][2] = 0;
	matrix->m[0][3] = 0;

	matrix->m[1][0] = -sin(angle_rad);
	matrix->m[1][1] = cos(angle_rad);
	matrix->m[1][2] = 0;
	matrix->m[1][3] = 0;

	matrix->m[2][0] = 0;
	matrix->m[2][1] = 0;
	matrix->m[2][2] = 1;
	matrix->m[2][3] = 0;

	matrix->m[3][0] = 0;
	matrix->m[3][1] = 0;
	matrix->m[3][2] = 0;
	matrix->m[3][3] = 1;
}

void translation_matrix (mat4x4_t* matrix, float x, float y, float z)
{
	init_matrix(matrix);
	matrix->m[0][0] = 1;
	matrix->m[1][1] = 1;
	matrix->m[2][2] = 1;
	matrix->m[3][0] = x;
	matrix->m[3][1] = y;
	matrix->m[3][2] = z;
	matrix->m[3][3] = 1;
}

void rotation_direction_matrix_inv(mat4x4_t* matrix, vec3d_t* pos, vec3d_t* target, vec3d_t* up)
{
	vec3d_t forward_cam, right_cam, up_cam;

	//Get the foward vector
	sub_vectors(target, pos, &forward_cam);
	normalise_vector(&forward_cam, &forward_cam);

	//Get the up vector
	vec3d_t b;
	vector_mul(&forward_cam, vector_dot_prod(&forward_cam, up), &b);
	sub_vectors(up, &b, &up_cam);

	//Get the right vector
	vector_cross_prod(&forward_cam, &up_cam, &right_cam);



	//First row
	matrix->m[0][0] = right_cam.x;		
	matrix->m[0][1] = up_cam.x;		
	matrix->m[0][2] = forward_cam.x;		

	//Second row		
	matrix->m[1][0] = right_cam.y;		
	matrix->m[1][1] = up_cam.y;		
	matrix->m[1][2] = forward_cam.y;		

	//Third row
	matrix->m[2][0] = right_cam.z;		
	matrix->m[2][1] = up_cam.z;		
	matrix->m[2][2] = forward_cam.z;		

	//Fourth row
	matrix->m[3][0] = -vector_dot_prod(pos, &right_cam);		
	matrix->m[3][1] = -vector_dot_prod(pos, &up_cam);		
	matrix->m[3][2] = -vector_dot_prod(pos, &forward_cam);		
	matrix->m[3][3] = 1;
	

}

void projection_matrix (mat4x4_t* matrix, float fov, float aspect_r, float z_far, float z_near)
{
	
	matrix->m[0][0] = aspect_r * (1.0f/tan(fov*RAD_CONVERT/2.0f));
	matrix->m[0][1] = 0;
	matrix->m[0][2] = 0;
	matrix->m[0][3] = 0;

	matrix->m[1][0] = 0;
	matrix->m[1][1] = 1.0f/tan(fov*RAD_CONVERT/2.0f);
	matrix->m[1][2] = 0;
	matrix->m[1][3] = 0;

	matrix->m[2][0] = 0;
	matrix->m[2][1] = 0;
	matrix->m[2][2] = z_far/(z_far-z_near);
	matrix->m[2][3] = 1;

	matrix->m[3][0] = 0;
	matrix->m[3][1] = 0;
	matrix->m[3][2] = ((-z_far) * z_near) / (z_far - z_near);
	matrix->m[3][3] = 0;	


}

void multiply_vector_matrix(vec3d_t* in, mat4x4_t* mat, vec3d_t* out)
{
	out->x = in->x * mat->m[0][0] + in->y * mat->m[1][0] + in->z * mat->m[2][0] + in->w * mat->m[3][0];
	out->y = in->x * mat->m[0][1] + in->y * mat->m[1][1] + in->z * mat->m[2][1] + in->w * mat->m[3][1];
	out->z = in->x * mat->m[0][2] + in->y * mat->m[1][2] + in->z * mat->m[2][2] + in->w * mat->m[3][2];
	out->w = in->x * mat->m[0][3] + in->y * mat->m[1][3] + in->z * mat->m[2][3] + in->w * mat->m[3][3];
}

void add_vectors(vec3d_t* v1, vec3d_t* v2, vec3d_t* vr)
{
	vr->x = v1->x + v2->x;
	vr->y = v1->y + v2->y;
	vr->z = v1->z + v2->z;
}

void sub_vectors(vec3d_t* v1, vec3d_t* v2, vec3d_t* vr)
{
	vr->x = v1->x - v2->x;
	vr->y = v1->y - v2->y;
	vr->z = v1->z - v2->z;
}

void vector_mul(vec3d_t* v, float k, vec3d_t* vr)
{
	vr->x = v->x*k;
	vr->y = v->y*k;
	vr->z = v->z*k;
}


void vector_div(vec3d_t* v, float k, vec3d_t* vr)
{
	vr->x = v->x/k;
	vr->y = v->y/k;
	vr->z = v->z/k;
}

float vector_dot_prod(vec3d_t* v1, vec3d_t* v2)
{
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z);
}

float vector_length(vec3d_t* v)
{
	return (sqrt(vector_dot_prod(v, v)));
}

void normalise_vector(vec3d_t* v, vec3d_t* vn)
{
	float l = vector_length(v);
	vector_div(v, l, vn);
}

void vector_cross_prod(vec3d_t* v1, vec3d_t* v2, vec3d_t* vr)
{
	vr->x = v1->y*v2->z - v1->z*v2->y;
	vr->y = v1->z*v2->x - v1->x*v2->z;
	vr->z = v1->x*v2->y - v1->y*v2->x;	
}

void mul_matrices (mat4x4_t* m1, mat4x4_t* m2, mat4x4_t* mr)
{
	mat4x4_t mc;
	for (int i=0; i < 4; i++)
	{
		for (int j=0; j < 4; j++)
		{
			mc.m[i][j] = m1->m[i][0] * m2->m[0][j] +
				      m1->m[i][1] * m2->m[1][j] +
				      m1->m[i][2] * m2->m[2][j] +
				      m1->m[i][3] * m2->m[3][j]; 
		}
	}


	for (int i=0; i < 4; i++)
	{
		for (int j=0; j < 4; j++)
		{
			mr->m[i][j] = mc.m[i][j]; 
		}
	}

	
}










