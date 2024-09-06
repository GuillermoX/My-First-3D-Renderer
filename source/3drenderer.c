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
mat4x4_t rot_matrix_z, rot_matrix_x;

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
				case SDLK_d:
					gp_keys[KEY_a] = false;
					break;
				case SDLK_a:
					gp_keys[KEY_d] = false;
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

	//Increase the rotation angle
	angle += 80*delta_time;
	update_matrices();

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
	FILE* f = fopen("./object.obj", "r");
	if(f != NULL)
	{	
		//List to store all vertex from .obj (Fix size to make it simpler) TODO: Dinamic storage
		vec3d_t verts[2000];

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
					  vert_i ++;
					break;
				case 'f': fscanf(f, "%d %d %d\n", &i_v1, &i_v2, &i_v3);
					  meshes[0].tris[meshes[0].n_tris].vertex[0].x = verts[i_v1].x;
					  meshes[0].tris[meshes[0].n_tris].vertex[0].y = verts[i_v1].y;
					  meshes[0].tris[meshes[0].n_tris].vertex[0].z = verts[i_v1].z;

					  meshes[0].tris[meshes[0].n_tris].vertex[1].x = verts[i_v2].x;
					  meshes[0].tris[meshes[0].n_tris].vertex[1].y = verts[i_v2].y;
					  meshes[0].tris[meshes[0].n_tris].vertex[1].z = verts[i_v2].z;

					  meshes[0].tris[meshes[0].n_tris].vertex[2].x = verts[i_v3].x;
					  meshes[0].tris[meshes[0].n_tris].vertex[2].y = verts[i_v3].y;
					  meshes[0].tris[meshes[0].n_tris].vertex[2].z = verts[i_v3].z;

					  meshes[0].n_tris ++;
					break;

				default: fscanf(f, "\n");
			}
		}
		fclose(f);

	}


	//Initialize block mesh

	/*
	meshes[0].n_tris = 12;

	//SOUTH
	meshes[0].tris[0].vertex[0].x = 0;	meshes[0].tris[0].vertex[0].y = 0;	meshes[0].tris[0].vertex[0].z = 0;
	meshes[0].tris[0].vertex[1].x = 0;	meshes[0].tris[0].vertex[1].y = 1;	meshes[0].tris[0].vertex[1].z = 0;
	meshes[0].tris[0].vertex[2].x = 1;	meshes[0].tris[0].vertex[2].y = 1;	meshes[0].tris[0].vertex[2].z = 0;
		
	meshes[0].tris[1].vertex[0].x = 0;	meshes[0].tris[1].vertex[0].y = 0;	meshes[0].tris[1].vertex[0].z = 0;
	meshes[0].tris[1].vertex[1].x = 1;	meshes[0].tris[1].vertex[1].y = 1;	meshes[0].tris[1].vertex[1].z = 0;
	meshes[0].tris[1].vertex[2].x = 1;	meshes[0].tris[1].vertex[2].y = 0;	meshes[0].tris[1].vertex[2].z = 0;
	

	//EAST
	meshes[0].tris[2].vertex[0].x = 1;	meshes[0].tris[2].vertex[0].y = 0;	meshes[0].tris[2].vertex[0].z = 0;
	meshes[0].tris[2].vertex[1].x = 1;	meshes[0].tris[2].vertex[1].y = 1;	meshes[0].tris[2].vertex[1].z = 0;
	meshes[0].tris[2].vertex[2].x = 1;	meshes[0].tris[2].vertex[2].y = 1;	meshes[0].tris[2].vertex[2].z = 1;
		
	meshes[0].tris[3].vertex[0].x = 1;	meshes[0].tris[3].vertex[0].y = 0;	meshes[0].tris[3].vertex[0].z = 0;
	meshes[0].tris[3].vertex[1].x = 1;	meshes[0].tris[3].vertex[1].y = 1;	meshes[0].tris[3].vertex[1].z = 1;
	meshes[0].tris[3].vertex[2].x = 1;	meshes[0].tris[3].vertex[2].y = 0;	meshes[0].tris[3].vertex[2].z = 1;


	//NORTH	
	meshes[0].tris[4].vertex[0].x = 1;	meshes[0].tris[4].vertex[0].y = 0;	meshes[0].tris[4].vertex[0].z = 1;
	meshes[0].tris[4].vertex[1].x = 1;	meshes[0].tris[4].vertex[1].y = 1;	meshes[0].tris[4].vertex[1].z = 1;
	meshes[0].tris[4].vertex[2].x = 0;	meshes[0].tris[4].vertex[2].y = 1;	meshes[0].tris[4].vertex[2].z = 1;
		
	meshes[0].tris[5].vertex[0].x = 1;	meshes[0].tris[5].vertex[0].y = 0;	meshes[0].tris[5].vertex[0].z = 1;
	meshes[0].tris[5].vertex[1].x = 0;	meshes[0].tris[5].vertex[1].y = 1;	meshes[0].tris[5].vertex[1].z = 1;
	meshes[0].tris[5].vertex[2].x = 0;	meshes[0].tris[5].vertex[2].y = 0;	meshes[0].tris[5].vertex[2].z = 1;


	//WEST	
	meshes[0].tris[6].vertex[0].x = 0;	meshes[0].tris[6].vertex[0].y = 0;	meshes[0].tris[6].vertex[0].z = 1;
	meshes[0].tris[6].vertex[1].x = 0;	meshes[0].tris[6].vertex[1].y = 1;	meshes[0].tris[6].vertex[1].z = 1;
	meshes[0].tris[6].vertex[2].x = 0;	meshes[0].tris[6].vertex[2].y = 1;	meshes[0].tris[6].vertex[2].z = 0;
		
	meshes[0].tris[7].vertex[0].x = 0;	meshes[0].tris[7].vertex[0].y = 0;	meshes[0].tris[7].vertex[0].z = 1;
	meshes[0].tris[7].vertex[1].x = 0;	meshes[0].tris[7].vertex[1].y = 1;	meshes[0].tris[7].vertex[1].z = 0;
	meshes[0].tris[7].vertex[2].x = 0;	meshes[0].tris[7].vertex[2].y = 0;	meshes[0].tris[7].vertex[2].z = 0;

	
	//TOP	
	meshes[0].tris[8].vertex[0].x = 0;	meshes[0].tris[8].vertex[0].y = 1;	meshes[0].tris[8].vertex[0].z = 0;
	meshes[0].tris[8].vertex[1].x = 0;	meshes[0].tris[8].vertex[1].y = 1;	meshes[0].tris[8].vertex[1].z = 1;
	meshes[0].tris[8].vertex[2].x = 1;	meshes[0].tris[8].vertex[2].y = 1;	meshes[0].tris[8].vertex[2].z = 1;
		
	meshes[0].tris[9].vertex[0].x = 0;	meshes[0].tris[9].vertex[0].y = 1;	meshes[0].tris[9].vertex[0].z = 0;
	meshes[0].tris[9].vertex[1].x = 1;	meshes[0].tris[9].vertex[1].y = 1;	meshes[0].tris[9].vertex[1].z = 1;
	meshes[0].tris[9].vertex[2].x = 1;	meshes[0].tris[9].vertex[2].y = 1;	meshes[0].tris[9].vertex[2].z = 0;
	
	//BOTTOM	
	meshes[0].tris[10].vertex[0].x = 1;	meshes[0].tris[10].vertex[0].y = 0;	meshes[0].tris[10].vertex[0].z = 1;
	meshes[0].tris[10].vertex[1].x = 0;	meshes[0].tris[10].vertex[1].y = 0;	meshes[0].tris[10].vertex[1].z = 1;
	meshes[0].tris[10].vertex[2].x = 0;	meshes[0].tris[10].vertex[2].y = 0;	meshes[0].tris[10].vertex[2].z = 0;
		
	meshes[0].tris[11].vertex[0].x = 1;	meshes[0].tris[11].vertex[0].y = 0;	meshes[0].tris[11].vertex[0].z = 1;
	meshes[0].tris[11].vertex[1].x = 0;	meshes[0].tris[11].vertex[1].y = 0;	meshes[0].tris[11].vertex[1].z = 0;
	meshes[0].tris[11].vertex[2].x = 1;	meshes[0].tris[11].vertex[2].y = 0;	meshes[0].tris[11].vertex[2].z = 0;
	*/
	

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

	triangle_t tri_proj, tri_rotZ, tri_rotZX, tri_trans;
	
	//Rotate the triangle on Z axys
	multiply_vector_matrix(&tri_rotZ.vertex[0], &(tri->vertex[0]), &rot_matrix_z);
	multiply_vector_matrix(&tri_rotZ.vertex[1], &(tri->vertex[1]), &rot_matrix_z);
	multiply_vector_matrix(&tri_rotZ.vertex[2], &(tri->vertex[2]), &rot_matrix_z);

	//Rotate the triangle on X axys	
	multiply_vector_matrix(&tri_rotZX.vertex[0], &tri_rotZ.vertex[0], &rot_matrix_x);
	multiply_vector_matrix(&tri_rotZX.vertex[1], &tri_rotZ.vertex[1], &rot_matrix_x);
	multiply_vector_matrix(&tri_rotZX.vertex[2], &tri_rotZ.vertex[2], &rot_matrix_x);
	
	//Copy the rotated triangle and translate it
	tri_trans.vertex[0].x = tri_rotZX.vertex[0].x - 0.0;
	tri_trans.vertex[1].x = tri_rotZX.vertex[1].x - 0.0;
	tri_trans.vertex[2].x = tri_rotZX.vertex[2].x - 0.0;
	
	tri_trans.vertex[0].y = tri_rotZX.vertex[0].y + 0.0;
	tri_trans.vertex[1].y = tri_rotZX.vertex[1].y + 0.0;
	tri_trans.vertex[2].y = tri_rotZX.vertex[2].y + 0.0;
	
	tri_trans.vertex[0].z = tri_rotZX.vertex[0].z + 20;
	tri_trans.vertex[1].z = tri_rotZX.vertex[1].z + 20;
	tri_trans.vertex[2].z = tri_rotZX.vertex[2].z + 20;
	
	// ------ Calculate the normal normalized (between 0 and 1) -----
	vec3d_t normal, line1, line2;

	//Calculate the vector from v0 -> v1
	line1.x = tri_trans.vertex[1].x - tri_trans.vertex[0].x;
	line1.y = tri_trans.vertex[1].y - tri_trans.vertex[0].y;
	line1.z = tri_trans.vertex[1].z - tri_trans.vertex[0].z;

	//Calculate the vector from v0 -> v2
	line2.x = tri_trans.vertex[2].x - tri_trans.vertex[0].x;
	line2.y = tri_trans.vertex[2].y - tri_trans.vertex[0].y;
	line2.z = tri_trans.vertex[2].z - tri_trans.vertex[0].z;
	
	//Do the cross product of the two vectors to get a perpendicular vector of both (the normal)
	normal.x = line1.y*line2.z - line2.y*line1.z;	
	normal.y = line2.x*line1.z - line1.x*line2.z;	
	normal.z = line1.x*line2.y - line2.x*line1.y;

	//Normalize the normal vector (
	float n_lenth = sqrt(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
	normal.x /= n_lenth; normal.y /= n_lenth; normal.z /= n_lenth;

	//If the dot product of the normal and the camera vector is negative it means the angle between both
	//is greater than 90º, which means it's visible (check page 5 of notes)
	if((normal.x*(tri_trans.vertex[0].x - scene->camera.pos.x) + 
	    normal.y*(tri_trans.vertex[0].y - scene->camera.pos.y) + 
	    normal.z*(tri_trans.vertex[0].z - scene->camera.pos.z)) < 0.0f)
	{	
		//If the triangle is visible to the camera, proceed to calculate the projection of it
		//Do the multiplication of the vector of the vertex with the projection matrix
		//The result is the projection of the vertex at the screen (values between -1 and 1) from the center
		multiply_vector_matrix(&tri_proj.vertex[0], &(tri_trans.vertex[0]), &proj_matrix);
		multiply_vector_matrix(&tri_proj.vertex[1], &(tri_trans.vertex[1]), &proj_matrix);
		multiply_vector_matrix(&tri_proj.vertex[2], &(tri_trans.vertex[2]), &proj_matrix);
		
		//Add 1 so the value is not negative and (0,0) is the top left corner and the range is [0,2]
		tri_proj.vertex[0].x += 1.0f;	tri_proj.vertex[0].y += 1.0f;
		tri_proj.vertex[1].x += 1.0f;	tri_proj.vertex[1].y += 1.0f;
		tri_proj.vertex[2].x += 1.0f;	tri_proj.vertex[2].y += 1.0f;
		
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
		tri_ras->brightness = scene->light.x*normal.x + scene->light.y*normal.y + scene->light.z*normal.z;
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


// -------------------------------- MATH MATRIX FUNCTIONS ----------------------------------------------------------------

void initialize_matrices(scene_t* scene)
{

	//PROJECTION MATRIX
	
	proj_matrix.m[0][0] = scene->camera.aspect_ratio * (1.0f/tan((scene->camera.fov)*RAD_CONVERT/2.0f));
	proj_matrix.m[0][1] = 0;
	proj_matrix.m[0][2] = 0;
	proj_matrix.m[0][3] = 0;

	proj_matrix.m[1][0] = 0;
	proj_matrix.m[1][1] = 1.0f/tan((scene->camera.fov)*RAD_CONVERT/2.0f);
	proj_matrix.m[1][2] = 0;
	proj_matrix.m[1][3] = 0;

	proj_matrix.m[2][0] = 0;
	proj_matrix.m[2][1] = 0;
	proj_matrix.m[2][2] = (float)(scene->camera.z_far)/(float)((scene->camera.z_far)-(scene->camera.z_near));
	proj_matrix.m[2][3] = 1;

	proj_matrix.m[3][0] = 0;
	proj_matrix.m[3][1] = 0;
	proj_matrix.m[3][2] = -(float)((scene->camera.z_far)*(scene->camera.z_near))/(float)((scene->camera.z_far)-(scene->camera.z_near));
	proj_matrix.m[3][3] = 0;	


	//Z AXYS ROTATION MATRIX
	
	rot_matrix_z.m[0][0] = cos(angle*RAD_CONVERT);
	rot_matrix_z.m[0][1] = sin(angle*RAD_CONVERT);
	rot_matrix_z.m[0][2] = 0;
	rot_matrix_z.m[0][3] = 0;

	rot_matrix_z.m[1][0] = -sin(angle*RAD_CONVERT);
	rot_matrix_z.m[1][1] = cos(angle*RAD_CONVERT);
	rot_matrix_z.m[1][2] = 0;
	rot_matrix_z.m[1][3] = 0;

	rot_matrix_z.m[2][0] = 0;
	rot_matrix_z.m[2][1] = 0;
	rot_matrix_z.m[2][2] = 1;
	rot_matrix_z.m[2][3] = 0;

	rot_matrix_z.m[3][0] = 0;
	rot_matrix_z.m[3][1] = 0;
	rot_matrix_z.m[3][2] = 0;
	rot_matrix_z.m[3][3] = 1;


	//X AXYS ROTATION MATRIX

	rot_matrix_x.m[0][0] = 1;
	rot_matrix_x.m[0][1] = 0;
	rot_matrix_x.m[0][2] = 0;
	rot_matrix_x.m[0][3] = 0;

	rot_matrix_x.m[1][0] = 0;
	rot_matrix_x.m[1][1] = cos(angle*RAD_CONVERT);
	rot_matrix_x.m[1][2] = sin(angle*RAD_CONVERT);
	rot_matrix_x.m[1][3] = 0;

	rot_matrix_x.m[2][0] = 0;
	rot_matrix_x.m[2][1] = -sin(angle*RAD_CONVERT);
	rot_matrix_x.m[2][2] = cos(angle*RAD_CONVERT);
	rot_matrix_x.m[2][3] = 0;

	rot_matrix_x.m[3][0] = 0;
	rot_matrix_x.m[3][1] = 0;
	rot_matrix_x.m[3][2] = 0;
	rot_matrix_x.m[3][3] = 1;

}

void update_matrices()
{
	//Z AXYS ROTATION
		
	rot_matrix_z.m[0][0] = cos(angle*RAD_CONVERT);
	rot_matrix_z.m[0][1] = sin(angle*RAD_CONVERT);
	rot_matrix_z.m[0][2] = 0;
	rot_matrix_z.m[0][3] = 0;

	rot_matrix_z.m[1][0] = -sin(angle*RAD_CONVERT);
	rot_matrix_z.m[1][1] = cos(angle*RAD_CONVERT);
	rot_matrix_z.m[1][2] = 0;
	rot_matrix_z.m[1][3] = 0;

	rot_matrix_z.m[2][0] = 0;
	rot_matrix_z.m[2][1] = 0;
	rot_matrix_z.m[2][2] = 1;
	rot_matrix_z.m[2][3] = 0;

	rot_matrix_z.m[3][0] = 0;
	rot_matrix_z.m[3][1] = 0;
	rot_matrix_z.m[3][2] = 0;
	rot_matrix_z.m[3][3] = 1;


	//X AXYS ROTATION MATRIX

	rot_matrix_x.m[0][0] = 1;
	rot_matrix_x.m[0][1] = 0;
	rot_matrix_x.m[0][2] = 0;
	rot_matrix_x.m[0][3] = 0;

	rot_matrix_x.m[1][0] = 0;
	rot_matrix_x.m[1][1] = cos(angle*RAD_CONVERT*0.5);
	rot_matrix_x.m[1][2] = sin(angle*RAD_CONVERT*0.5);
	rot_matrix_x.m[1][3] = 0;

	rot_matrix_x.m[2][0] = 0;
	rot_matrix_x.m[2][1] = -sin(angle*RAD_CONVERT*0.5);
	rot_matrix_x.m[2][2] = cos(angle*RAD_CONVERT*0.5);
	rot_matrix_x.m[2][3] = 0;

	rot_matrix_x.m[3][0] = 0;
	rot_matrix_x.m[3][1] = 0;
	rot_matrix_x.m[3][2] = 0;
	rot_matrix_x.m[3][3] = 1;
}


void multiply_vector_matrix(vec3d_t* out, vec3d_t* in, mat4x4_t* mat)
{
	out->x = in->x * mat->m[0][0] + in->y * mat->m[1][0] + in->z * mat->m[2][0] + mat->m[3][0];
	out->y = in->x * mat->m[0][1] + in->y * mat->m[1][1] + in->z * mat->m[2][1] + mat->m[3][1];
	out->z = in->x * mat->m[0][2] + in->y * mat->m[1][2] + in->z * mat->m[2][2] + mat->m[3][2];
	float w = in->x * mat->m[0][3] + in->y * mat->m[1][3] + in->z * mat->m[2][3] + mat->m[3][3];
	if(w != 0.0f)
	{
		out->x /= w;
		out->y /= w;
		out->z /= w;
	}	
}



















