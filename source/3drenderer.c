#include "../include/3drenderer.h"
#include "./texture2.c"

//--------- OBJECT DEFINES ---------------------------------------------------
#define N_MESHES 1
#define TEXTURE_ON 1

//--------- FRAME DEFINITIONS ------------------------------
#define FPS 40
#define FRAME_TARGET_TIME 1000/FPS

#define MOVE_VEL 8
#define ROTATE_VEL 100
#define UP_DOWN_VEL 8

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
	scene->window.height = WINDOW_H;
	scene->window.width = WINDOW_W;

	scene->borders_on = false;

	//Start the camera properties
	scene -> camera.pos.x = 0;	scene->camera.pos.y = 0;	scene->camera.pos.z = 0;
	scene -> camera.look_dir.x = 0;	scene->camera.look_dir.y = 0;	scene->camera.look_dir.z = 1;
	scene->camera.rot_y_angle = 0;
	scene->camera.rot_x_angle = 0;

	scene->camera.z_near = 0.1f;
	scene->camera.z_far = 1000.0f;
	scene->camera.fov = 90.0f;
	scene->camera.aspect_ratio = (float)scene->window.height / (float)scene->window.width;

	//Start the simple illumination
	scene->light.x = -0.3;
	scene->light.y = 0.6;
	scene->light.z = 1;	
	//Normalize ilumination vector
	float l_lenth = sqrt(scene->light.x*scene->light.x + scene->light.y*scene->light.y + scene->light.z*scene->light.z);
	scene->light.x /= l_lenth; scene->light.y /= l_lenth; scene->light.z /= l_lenth;


	//Initialize meshes
	initialize_meshes(scene, meshes);
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
				case SDLK_UP:
					gp_keys[KEY_up] = true;
					break;
				case SDLK_DOWN:
					gp_keys[KEY_down] = true;
					break;
				case SDLK_t:
					gp_keys[KEY_t] = true;
					break;
				case SDLK_b:
					gp_keys[KEY_b] = true;
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
				case SDLK_UP:
					gp_keys[KEY_up] = false;
					break;
				case SDLK_DOWN:
					gp_keys[KEY_down] = false;
					break;	
				case SDLK_t:
					gp_keys[KEY_t] = false;
					break;	
				case SDLK_b:
					gp_keys[KEY_b] = false;
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

	if(gp_keys[KEY_a]) move_camera(scene, 0, 0, -1);
	if(gp_keys[KEY_d]) move_camera(scene, 0, 0, 1);
	if(gp_keys[KEY_w]) move_camera(scene, 1, 0, 0);
	if(gp_keys[KEY_s]) move_camera(scene, -1, 0, 0);
	if(gp_keys[KEY_space]) move_camera(scene, 0, 1, 0);
	if(gp_keys[KEY_lshift]) move_camera(scene, 0, -1, 0);
	if(gp_keys[KEY_left]) rotate_camera(scene, 0, 1);
	if(gp_keys[KEY_right]) rotate_camera(scene, 0, -1);
	if(gp_keys[KEY_up]) rotate_camera(scene, -1, 0);
	if(gp_keys[KEY_down]) rotate_camera(scene, 1, 0);
	if(gp_keys[KEY_t])
	{	//If mode 1 -> mode 2 	/ 	if mode 2 -> mode 1
		scene->texture_mode = scene->texture_mode ^ 0x3;
	}
	if(gp_keys[KEY_b]) scene->borders_on = !scene->borders_on;
	//Increase the rotation angle
	//angle += 30*delta_time;

	update_camera_look_dir(scene);
	update_matrices(scene);



}

void move_camera(scene_t* scene, float forward, float up, float right)
{
	vec3d_t vec_forward, vec_right = {.x = 0, .y = 0, .z = 0};

	if(up != 0) scene->camera.pos.y += up*MOVE_VEL*delta_time;
	if(forward != 0)
	{
		vector_mul(&(scene->camera.look_dir), forward*MOVE_VEL*delta_time, &vec_forward);
		add_vectors(&(scene->camera.pos), &vec_forward, &(scene->camera.pos));
	}
	if(right != 0)
	{
		vector_cross_prod(&((vec3d_t) {0, 1, 0}), &(scene->camera.look_dir), &(vec_right));
		normalise_vector(&vec_right, &vec_right);
		vector_mul(&vec_right, right*MOVE_VEL*delta_time, &vec_right);
		add_vectors(&(scene->camera.pos), &vec_right, &(scene->camera.pos));
	}
	
}

void rotate_camera(scene_t* scene, int dir_x, int dir_y)
{
	/*mat4x4_t rot;
	rotation_matrix_x(&rot, dir_x*ROTATE_VEL*delta_time*RAD_CONVERT);
	multiply_vector_matrix(&(scene->camera.look_dir), &rot, &(scene->camera.look_dir));

	rotation_matrix_y(&rot, dir_y*ROTATE_VEL*delta_time*RAD_CONVERT);
	multiply_vector_matrix(&(scene->camera.look_dir), &rot, &(scene->camera.look_dir));*/
	scene->camera.rot_x_angle += dir_x*ROTATE_VEL*delta_time;
	if(scene->camera.rot_x_angle >= 89)
	{
		scene->camera.rot_x_angle = 89;
	}
	else if(scene->camera.rot_x_angle <= -89)
	{
		scene->camera.rot_x_angle = -89;
	}
	

	scene->camera.rot_y_angle += dir_y*ROTATE_VEL*delta_time;
	if(scene->camera.rot_y_angle >= 360 || scene->camera.rot_y_angle <= -360){
		scene->camera.rot_y_angle = 0 + dir_x*ROTATE_VEL*delta_time;
	}


}

void update_camera_look_dir(scene_t* scene)
{
	mat4x4_t rot;
	
	rotation_matrix_x(&rot, scene->camera.rot_x_angle*RAD_CONVERT);
	multiply_vector_matrix(&((vec3d_t){.x=0, .y=0, .z=1, .w=0}), &rot, &(scene->camera.look_dir));

	rotation_matrix_y(&rot, scene->camera.rot_y_angle*RAD_CONVERT);
	multiply_vector_matrix(&(scene->camera.look_dir), &rot, &(scene->camera.look_dir));

	//normalise_vector(&(scene->camera.look_dir), &(scene->camera.look_dir));
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

	//Crosshair
	int half_h = (scene->window.height/2);
	int half_w = (scene->window.width/2);
	SDL_SetRenderDrawColor(gp_renderer, 255, 255, 255, 255);
	SDL_RenderDrawLine(gp_renderer, half_w, half_h - 3, half_w, half_h + 3);
	SDL_RenderDrawLine(gp_renderer, half_w - 3, half_h, half_w + 3, half_h);

	//Show the final render result
	SDL_RenderPresent(gp_renderer);
}






///////////////////////////////////////////////////////////////////////////////////////////
//		3D MANIPULATION FUNCTIONS						///
///////////////////////////////////////////////////////////////////////////////////////////

void initialize_meshes(scene_t* scene, mesh_t meshes[])
{
	//Open the .obj file
	FILE* f = fopen(scene->model_path, "r");
	if(f != NULL)
	{	
		//List to store all vertex from .obj (Fix size to make it simpler) TODO: Dinamic storage
		vec3d_t verts[10000];
		vec2d_t text_v[10000];

		//Character of the .obj file that indicates if the information of the line is a vertex or a triangle
		char type_line;

		//Initialize at 1 because .obj file starts counting the first vertex as 1
		int vert_i = 1;
		int vert_text_i = 1;

		//Index of the vertex of a triangle
		int i_v1, i_v2, i_v3;
		int i_vt1, i_vt2, i_vt3;

		//Initialize the number of triangles of the mesh to 0
		meshes[0].n_tris = 0;

		int n_lines_read = 0;
			
		char line[256];
		//Load all the vertex to a buffer and the faces/triangles to the mesh
		while (fgets(line, sizeof(line), f))
		{
			sscanf(line, "%c", &type_line);
			switch (type_line)
			{
				case 'v': 
					if(line[1] == 't')
					{	
						sscanf(line, "vt %f %f", &text_v[vert_text_i].u, &text_v[vert_text_i].v);
						//printf("vertex text - u: %f  v: %f\n", text_v[vert_text_i].u ,text_v[vert_text_i].v);
						vert_text_i++;
					}
					else
					{	
						sscanf(line, "v %f %f %f", &verts[vert_i].x, &verts[vert_i].y, &verts[vert_i].z);
						//printf("vertex - x: %f  y: %f  z: %f\n", verts[vert_i].x, verts[vert_i].y, verts[vert_i].z);
						verts[vert_i].w = 1.0f;
						verts[vert_i].x *= -1;		//Invert the X coord because some 3d editors export with inverted X (Blender in my case)
						vert_i ++;
					}
					break;

				case 'f': 
					if(scene->texture_mode == TEXT_UV_MODEL_ON || scene->texture_mode == TEXT_UV_MODEL_OFF)
					{
						sscanf(line, "f %d/%d %d/%d %d/%d", &i_v1, &i_vt1, &i_v2, &i_vt2, &i_v3, &i_vt3);
						//printf("face - vertex: %d %d %d - vertex text: %d %d %d\n", i_v1, i_v2, i_v3, i_vt1, i_vt2, i_vt3);

						meshes[0].tris[meshes[0].n_tris].t[0].u = text_v[i_vt1].u;
						meshes[0].tris[meshes[0].n_tris].t[0].v = 1-text_v[i_vt1].v;
						meshes[0].tris[meshes[0].n_tris].t[0].w = 1.0f;

						meshes[0].tris[meshes[0].n_tris].t[1].u = text_v[i_vt3].u;
						meshes[0].tris[meshes[0].n_tris].t[1].v = 1-text_v[i_vt3].v;
						meshes[0].tris[meshes[0].n_tris].t[1].w = 1.0f;

						meshes[0].tris[meshes[0].n_tris].t[2].u = text_v[i_vt2].u;
						meshes[0].tris[meshes[0].n_tris].t[2].v = 1-text_v[i_vt2].v;
						meshes[0].tris[meshes[0].n_tris].t[2].w = 1.0f;

					}
					else
					{
						sscanf(line, "f %d %d %d", &i_v1, &i_v2, &i_v3);
						printf("face - vertex: %d %d %d\n", i_v1, i_v2, i_v3);
					}

					meshes[0].tris[meshes[0].n_tris].vertex[0].x = verts[i_v1].x;
					meshes[0].tris[meshes[0].n_tris].vertex[0].y = verts[i_v1].y;
					meshes[0].tris[meshes[0].n_tris].vertex[0].z = verts[i_v1].z;
					meshes[0].tris[meshes[0].n_tris].vertex[0].w = verts[i_v1].w;

					meshes[0].tris[meshes[0].n_tris].vertex[1].x = verts[i_v3].x;
					meshes[0].tris[meshes[0].n_tris].vertex[1].y = verts[i_v3].y;
					meshes[0].tris[meshes[0].n_tris].vertex[1].z = verts[i_v3].z;
					meshes[0].tris[meshes[0].n_tris].vertex[1].w = verts[i_v3].w;

					meshes[0].tris[meshes[0].n_tris].vertex[2].x = verts[i_v2].x;
					meshes[0].tris[meshes[0].n_tris].vertex[2].y = verts[i_v2].y;
					meshes[0].tris[meshes[0].n_tris].vertex[2].z = verts[i_v2].z;
					meshes[0].tris[meshes[0].n_tris].vertex[2].w = verts[i_v2].w;

					meshes[0].n_tris ++;
					break;
				
				default: printf("Tipo no reconocido: %s", line);
			}

			n_lines_read++;
			//printf("Linea: %d\n", n_lines_read);
		}
		fclose(f);

	}
	else
	{
		printf("\nError: Unable to open the file");
		scene->running = false;
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

		//Queue of triangles to raster
		triangle_queue_t tri_q;
		triangle_t array_raster_q[n_tris*2];
		ini_triangle_queue(&tri_q, array_raster_q, n_tris*2);

		for(int j=0; j < n_tris; j++)
		{	
			//Rotate, move, project and add the triangle to the raster queue
			process_triangle(scene, &(meshes[i].tris[j]), &tri_q);	
		}

		//Sort the triangles ready to raster depending on the depth they are
		qsort(tri_q.q, tri_q.n_tris, sizeof(triangle_t), compare_depth_tris);

		while(tri_q.n_tris > 0)
		{
			triangle_t tri_ras;
			next_triangle(&tri_q, &tri_ras);
			raster_triangle(scene, &tri_ras);
		}

	}
}

void process_triangle(scene_t* scene, triangle_t* tri, triangle_queue_t* tri_q)
{	

	triangle_t tri_proj, tri_trans, tri_rotated;
	mat4x4_t mat_world, mat_trans;

	//Create the translation matrix
	translation_matrix(&mat_trans, 0, 0, 10);

	//Do the chain matrix multiplication to get a rotation and translation matrix
	mul_matrices(&rot_matrix_z, &rot_matrix_x, &mat_world);
	mul_matrices(&mat_world, &mat_trans, &mat_world);
	
	multiply_vector_matrix(&(tri->vertex[0]), &mat_world, &(tri_trans.vertex[0]));	
	multiply_vector_matrix(&(tri->vertex[1]), &mat_world, &(tri_trans.vertex[1]));	
	multiply_vector_matrix(&(tri->vertex[2]), &mat_world, &(tri_trans.vertex[2]));

	

	
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

		//Rotate the triangle to the camera view space
		multiply_vector_matrix(&(tri_trans.vertex[0]), &rot_cam_matrix, &(tri_rotated.vertex[0]));	
		multiply_vector_matrix(&(tri_trans.vertex[1]), &rot_cam_matrix, &(tri_rotated.vertex[1]));	
		multiply_vector_matrix(&(tri_trans.vertex[2]), &rot_cam_matrix, &(tri_rotated.vertex[2]));

		triangle_t tris_clip[2];	int n_tris_clip = 0;
		vec3d_t plane_p = {.x = 0, .y = 0, .z = scene->camera.z_near};
		vec3d_t plane_n = {.x = 0, .y = 0, .z = 1};

		tri_rotated.color = (rgb_t){200, 200, 200};

		tri_rotated.t[0] = tri->t[0];
		tri_rotated.t[1] = tri->t[1];
		tri_rotated.t[2] = tri->t[2];
		n_tris_clip = triangle_clip_against_plane(&plane_p, &plane_n, &tri_rotated, &tris_clip[0], &tris_clip[1]);


		for(int i = 0; i < n_tris_clip; i++)
		{	


			//If the triangle is visible to the camera, proceed to calculate the projection of it
			//Do the multiplication of the vector of the vertex with the projection matrix
			//The result is the projection of the vertex at the screen (values between -1 and 1) from the center
			multiply_vector_matrix(&(tris_clip[i].vertex[0]), &proj_matrix, &tri_proj.vertex[0]);
			multiply_vector_matrix(&(tris_clip[i].vertex[1]), &proj_matrix, &tri_proj.vertex[1]);
			multiply_vector_matrix(&(tris_clip[i].vertex[2]), &proj_matrix, &tri_proj.vertex[2]);

			if(scene->texture_mode == TEXT_UV_MODEL_ON)
			{
				tri_proj.t[0] = tris_clip[i].t[0];
				tri_proj.t[1] = tris_clip[i].t[1];
				tri_proj.t[2] = tris_clip[i].t[2];

				tri_proj.t[0].u /= tri_proj.vertex[0].w;
				tri_proj.t[1].u /= tri_proj.vertex[1].w;
				tri_proj.t[2].u /= tri_proj.vertex[2].w;

				tri_proj.t[0].v /= tri_proj.vertex[0].w;
				tri_proj.t[1].v /= tri_proj.vertex[1].w;
				tri_proj.t[2].v /= tri_proj.vertex[2].w;

				tri_proj.t[0].w = 1.0f / tri_proj.vertex[0].w;
				tri_proj.t[1].w = 1.0f / tri_proj.vertex[1].w;
				tri_proj.t[2].w = 1.0f / tri_proj.vertex[2].w;
			}	

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

			tri_proj.color = tris_clip[i].color;




			//Get the brightness of the triangle
			tri_proj.brightness = vector_dot_prod(&(scene->light), &normal);
			if (tri_proj.brightness < 0.05) tri_proj.brightness = 0.05;

			tri_proj.vertex[0].z = tris_clip[i].vertex[0].z;
			tri_proj.vertex[1].z = tris_clip[i].vertex[1].z;
			tri_proj.vertex[2].z = tris_clip[i].vertex[2].z;

			//Add the triangle to raster to the queue
			add_triangle(tri_q, &tri_proj);
		}

	}

}

void raster_triangle(scene_t* scene, triangle_t* tri)
{

	triangle_queue_t raster_q;
	triangle_t raster_array[300];
	ini_triangle_queue(&raster_q, raster_array, 300);
	add_triangle(&raster_q, tri);

	triangle_t clipped[2];
	int n_new_tris = 1;

	float screen_h = scene->window.height - 1;
	float screen_w = scene->window.width - 1;

	for(int p = 0; p < 4; p++)
	{
		int n_tris_to_add = 0;
		while(n_new_tris > 0)
		{
			triangle_t tri_to_clip;
			next_triangle(&raster_q, &tri_to_clip);
			n_new_tris--;

			switch (p)
			{
				case 0: n_tris_to_add = triangle_clip_against_plane(&(vec3d_t){0,0,0}, &(vec3d_t){0,1,0}, &tri_to_clip, &clipped[0], &clipped[1]);
						break;
				case 1: n_tris_to_add = triangle_clip_against_plane(&(vec3d_t){0,screen_h,0}, &(vec3d_t){0,-1,0}, &tri_to_clip, &clipped[0], &clipped[1]);
						break;
				case 2: n_tris_to_add = triangle_clip_against_plane(&(vec3d_t){0,0,0}, &(vec3d_t){1,0,0}, &tri_to_clip, &clipped[0], &clipped[1]);
						break;
				case 3: n_tris_to_add = triangle_clip_against_plane(&(vec3d_t){screen_w,0,0}, &(vec3d_t){-1,0,0}, &tri_to_clip, &clipped[0], &clipped[1]);
						break;
			}

			for(int i = 0; i < n_tris_to_add; i++)
			{
				add_triangle(&raster_q, &clipped[i]);
			}

		}

		n_new_tris = raster_q.n_tris;
	
	}

	while(raster_q.n_tris > 0)
	{


		//Create a screen vector (2D) for each triangle vertex projection
		triangle_t tri_raster;
		next_triangle(&raster_q, &tri_raster);


		screen_vect_t v1, v2, v3;
		vec2d_t t1 = tri_raster.t[0];
		vec2d_t t2 = tri_raster.t[1];
		vec2d_t t3 = tri_raster.t[2];

			

		v1.x = tri_raster.vertex[0].x; v1.y = tri_raster.vertex[0].y;
		v2.x = tri_raster.vertex[1].x; v2.y = tri_raster.vertex[1].y;
		v3.x = tri_raster.vertex[2].x; v3.y = tri_raster.vertex[2].y;	

		paint_triangle(scene, v1, v2, v3, t1, t2, t3, tri_raster.brightness);

		//Decoment if wanted all triangles borders to be drawn

		if(scene->borders_on)
		{
			SDL_SetRenderDrawColor(gp_renderer, 255, 255, 255, 255);
			SDL_RenderDrawLine(gp_renderer, v1.x, v1.y, v2.x, v2.y);
			SDL_RenderDrawLine(gp_renderer, v1.x, v1.y, v3.x, v3.y);
			SDL_RenderDrawLine(gp_renderer, v2.x, v2.y, v3.x, v3.y);
		}
	}
		
}


int triangle_clip_against_plane(vec3d_t* plane_p, vec3d_t* plane_n, triangle_t* tri_in, triangle_t* tri_out1, triangle_t* tri_out2)
{

	vec3d_t plane_n_n;
	normalise_vector(plane_n, &plane_n_n);

	//Calculate the distance between each vertex of triangle and the plane
	float dist[3];
	dist[0] = dist_point_plane(plane_p, plane_n, &(tri_in->vertex[0]));
	dist[1] = dist_point_plane(plane_p, plane_n, &(tri_in->vertex[1]));
	dist[2] = dist_point_plane(plane_p, plane_n, &(tri_in->vertex[2]));

	//Arrays of vec3d_t pointers to store each vertex depending if it's in the border or out
	vec3d_t* inside_points[3];		int n_inside_points = 0;
	vec3d_t* outside_points[3]; 	int n_outside_points = 0;
	vec2d_t* inside_text[3]; 		int n_inside_text = 0;
	vec2d_t* outside_text[3]; 		int n_outside_text = 0;

	//Add the vertex to the corresponding array and increment the counter
	if(dist[0] >= 0)
	{ 
		inside_points[n_inside_points++] = &(tri_in->vertex[0]);
		inside_text[n_inside_text++] = &(tri_in->t[0]);
	}
	else
	{ 
		outside_points[n_outside_points++] = &(tri_in->vertex[0]);
		outside_text[n_outside_text++] = &(tri_in->t[0]);
	}

	if(dist[1] >= 0)
	{ 
		inside_points[n_inside_points++] = &(tri_in->vertex[1]);
		inside_text[n_inside_text++] = &(tri_in->t[1]);
	}
	else
	{ 
		outside_points[n_outside_points++] = &(tri_in->vertex[1]);
		outside_text[n_outside_text++] = &(tri_in->t[1]);
	}

	if(dist[2] >= 0)
	{ 
		inside_points[n_inside_points++] = &(tri_in->vertex[2]);
		inside_text[n_inside_text++] = &(tri_in->t[2]);
	}
	else
	{ 
		outside_points[n_outside_points++] = &(tri_in->vertex[2]);
		outside_text[n_outside_text++] = &(tri_in->t[2]);
	}

	//Return the values depending on the case of clipping we got

	if(n_inside_points == 0){ return 0; }


	if(n_inside_points == 3)
	{
		*tri_out1 = *tri_in;
		return 1;
	}

	if(n_inside_points == 1 && n_outside_points == 2)
	{
		//Copy the color and brightness of original triangle to the clipped
		tri_out1->color = tri_in->color;
		tri_out1->brightness = tri_in->brightness;

		tri_out1->vertex[0] = *inside_points[0];
		tri_out1->t[0] = *inside_text[0];

		float t;
		//Add as vertex of clipped triangle the intersection points between point_inside - plane - points_outside
		vector_intersect_plane(plane_p, &plane_n_n, inside_points[0], outside_points[0], &(tri_out1->vertex[1]), &t);
		tri_out1->t[1].u = t*(outside_text[0]->u - inside_text[0]->u) + inside_text[0]->u;
		tri_out1->t[1].v = t*(outside_text[0]->v - inside_text[0]->v) + inside_text[0]->v;
		tri_out1->t[1].w = t*(outside_text[0]->w - inside_text[0]->w) + inside_text[0]->w;

		vector_intersect_plane(plane_p, &plane_n_n, inside_points[0], outside_points[1], &(tri_out1->vertex[2]), &t);	
		tri_out1->t[2].u = t*(outside_text[1]->u - inside_text[0]->u) + inside_text[0]->u;
		tri_out1->t[2].v = t*(outside_text[1]->v - inside_text[0]->v) + inside_text[0]->v;
		tri_out1->t[2].w = t*(outside_text[1]->w - inside_text[0]->w) + inside_text[0]->w;


		
		return 1;
	}

	if(n_inside_points == 2 && n_outside_points == 1)
	{
		//Copy the color and brightness of original triangle to the clipped
		tri_out1->color = tri_in->color;
		tri_out1->brightness = tri_in->brightness;

		tri_out2->color = tri_in->color;
		tri_out2->brightness = tri_in->brightness;

		float t;

		//Get the first triangle output
		tri_out1->vertex[0] = *inside_points[0];
		tri_out1->vertex[1] = *inside_points[1];
		tri_out1->t[0] = *inside_text[0];
		tri_out1->t[1] = *inside_text[1];
		vector_intersect_plane(plane_p, &plane_n_n, inside_points[0], outside_points[0], &(tri_out1->vertex[2]), &t);
		tri_out1->t[2].u = t*(outside_text[0]->u - inside_text[0]->u) + inside_text[0]->u;
		tri_out1->t[2].v = t*(outside_text[0]->v - inside_text[0]->v) + inside_text[0]->v;
		tri_out1->t[2].w = t*(outside_text[0]->w - inside_text[0]->w) + inside_text[0]->w;

		//Get the second triangle output
		tri_out2->vertex[0] = *inside_points[1];
		tri_out2->t[0] = *inside_text[1];
		tri_out2->vertex[1] = tri_out1->vertex[2];
		tri_out2->t[1] = tri_out1->t[2];
		vector_intersect_plane(plane_p, &plane_n_n, inside_points[1], outside_points[0], &(tri_out2->vertex[2]), &t);
		tri_out2->t[2].u = t*(outside_text[0]->u - inside_text[1]->u) + inside_text[1]->u;
		tri_out2->t[2].v = t*(outside_text[0]->v - inside_text[1]->v) + inside_text[1]->v;
		tri_out2->t[2].w = t*(outside_text[0]->w - inside_text[1]->w) + inside_text[1]->w;


		return 2;
	}

	return 0;

}


//------------- TRIANGLE QUEUE FUNCTIONS --------------------------------

void ini_triangle_queue(triangle_queue_t* q, triangle_t tri_array[], int max)
{
	q->n_tris = 0;
	q->q = tri_array;
	q->max = max;
}

void add_triangle(triangle_queue_t* q, triangle_t* tri)
{
	if(q->n_tris != q->max)
	{
		q->q[q->n_tris] = *tri;
		q->n_tris++;
	}
}

void next_triangle(triangle_queue_t* q, triangle_t* next_tri)
{
	triangle_t tri_out = q->q[0];
	int n = q->n_tris;

	//Move forward one position in queue
	for(int i = 1; i < n; i++)
	{
		q->q[i-1] = q->q[i];
	}

	q->n_tris --;

	*next_tri = tri_out;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// 		SCREEN DRAWING FUNCTIONS 		/////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void paint_triangle(scene_t* scene, screen_vect_t v1, screen_vect_t v2, screen_vect_t v3, vec2d_t t1, vec2d_t t2, vec2d_t t3, float bright)
{
	bool text_on = scene->texture_mode == TEXT_UV_MODEL_ON;
	//Sort the vertex from smaller Y to bigger
	if(v2.y < v1.y)
	{
		screen_vect_t aux_v = v1;	vec2d_t aux_t = t1;
		v1 = v2;
		t1 = t2;
		v2 = aux_v;
		t2 = aux_t;
	}

	if(v3.y < v1.y)
	{
		screen_vect_t aux_v = v1;	vec2d_t aux_t = t1;
		v1 = v3;
		t1 = t3;
		v3 = aux_v;
		t3 = aux_t;
	}

	if(v3.y < v2.y)
	{
		screen_vect_t aux_v = v2;	vec2d_t aux_t = t2;
		v2 = v3;
		t2 = t3;
		v3 = aux_v;
		t3 = aux_t;
	}




	//Difference of coordinates in A line (screen and texture)
	int dy1 = v2.y - v1.y;
	int dx1 = v2.x - v1.x;
	float du1, dv1, dw1;
	if(text_on)
	{
		du1 = t2.u - t1.u;
		dv1 = t2.v - t1.v;
		dw1 = t2.w - t1.w;
	}
	//Difference of coords in B lines (screen and texture)
	int dy2 = v3.y - v1.y;
	int dx2 = v3.x - v1.x;
	float du2, dv2, dw2;
	if(text_on)
	{
		du2 = t3.u - t1.u;
		dv2 = t3.v - t1.v;
		dw2 = t3.w - t1.w;
	}

	//Step values of screen lines A and B (dx/dy)
	float dax_step = 0, dbx_step = 0;
	
	//Step values of texture lines A and B (dxt/dyv and dyt/dyv)
	float du1_step = 0, du2_step = 0;
	float dv1_step = 0, dv2_step = 0;		//In texture space we need a "Y" step value
											//that represents the value we have to move in Y
											//in texture lines for each step of Y we do in screen line.
	float dw1_step = 0, dw2_step = 0;

	//Calculate the values of steps preventing a division by 0
	if(dy1 != 0)
	{	
		dax_step = dx1 / (float)abs(dy1);
		if(text_on)
		{	
			du1_step = du1 / (float)abs(dy1);
			dv1_step = dv1 / (float)abs(dy1);
			dw1_step = dw1 / (float)abs(dy1);
		}
	}
	if(dy2 != 0)
	{
		dbx_step = dx2 / (float)abs(dy2);
		if(text_on)
		{
			du2_step = du2 / (float)abs(dy2);
			dv2_step = dv2 / (float)abs(dy2);
			dw2_step = dw2 / (float)abs(dy2);
		}
	}

	//If the top part of the triangle is not flat
	if(dy1 != 0)
	{
		int ax;
		int bx;
		//For each "Y" between v1 and v2
		for(int i = v1.y; i <= v2.y; i++)
		{
			//Get the A and B screen lines "X" position on this "Y"
			ax = v1.x + (float)(i - v1.y)*dax_step;
			bx = v1.x + (float)(i - v1.y)*dbx_step;

			float tex_su, tex_sv, tex_sw, tex_eu, tex_ev, tex_ew;
			if(text_on)
			{	
				//Get the A texture line position on this "Y"
				tex_su = t1.u + (float)(i - v1.y)*du1_step;	//"su" -> Starting U
				tex_sv = t1.v + (float)(i - v1.y)*dv1_step;
				tex_sw = t1.w + (float)(i - v1.y)*dw1_step;

				//Get the B texture line position on this "Y"
				tex_eu = t1.u + (float)(i - v1.y)*du2_step;	//"eu" -> Ending U
				tex_ev = t1.v + (float)(i - v1.y)*dv2_step;
				tex_ew = t1.w + (float)(i - v1.y)*dw2_step;
			}

			//Line A "x" has to be smaller than line B "x" to draw from left to right
			if(ax > bx)
			{
				int aux_i; float aux_f;
				//Swap the screen "x" 
				aux_i = bx;
				bx = ax;
				ax = aux_i;
				//Swap the starting and ending texture U and V
				if(text_on)
				{
					aux_f = tex_su;
					tex_su = tex_eu;
					tex_eu = aux_f;

					aux_f = tex_sv;
					tex_sv = tex_ev;
					tex_ev = aux_f;

					aux_f = tex_sw;
					tex_sw = tex_ew;
					tex_ew = aux_f;
				}
			}


			if(text_on)
			{
				//This is the final point of the texture to get the color from
				float tex_u = tex_su;
				float tex_v = tex_sv;
				float tex_w = tex_sw;

				//Get the range of a step between the "X"s from lines A and B 
				float tstep = 1.0f / (float)(bx - ax);
				float t = 0.0f;		//Acumulate steps


				for(int j = ax; j < bx; j++)
				{
					tex_u = (1.0f - t)*tex_su + t*tex_eu;
					tex_v = (1.0f - t)*tex_sv + t*tex_ev;
					tex_w = (1.0f - t)*tex_sw + t*tex_ew;
					rgb_t color;


					get_color_texture(texture, tex_u / tex_w, tex_v / tex_w, TEXTURE_H, TEXTURE_W, &color);
					SDL_SetRenderDrawColor(gp_renderer, color.r*bright, color.g*bright, color.b*bright, 255);

					SDL_RenderDrawPoint(gp_renderer, j, i);
					t += tstep;
				}
			}
			else
			{	
				SDL_SetRenderDrawColor(gp_renderer, 140*bright, 140*bright, 140*bright, 255);
				SDL_RenderDrawLine(gp_renderer, ax, i, bx, i);
			}

		}

	}

	dy1 = v3.y - v2.y;
	dx1 = v3.x - v2.x;

	if(text_on)
	{
		du1 = t3.u - t2.u;
		dv1 = t3.v - t2.v;
		dw1 = t3.w - t2.w;

		du1_step = 0;	dv1_step = 0;
	}

	//Calculate the values of steps preventing a division by 0
	if(dy1 != 0)
	{	
		dax_step = dx1 / (float)abs(dy1);
		if(text_on)
		{	
			du1_step = du1 / (float)abs(dy1);
			dv1_step = dv1 / (float)abs(dy1);
			dw1_step = dw1 / (float)abs(dy1);
		}
	}
	if(dy2 != 0)
	{
		dbx_step = dx2 / (float)abs(dy2);
	}

	if(dy1 != 0)
	{
		//For each "Y" between v1 and v2
		for(int i = v2.y; i <= v3.y; i++)
		{
			//Get the A and B screen lines "X" position on this "Y"
			int ax = v2.x + (float)(i - v2.y)*dax_step;
			int bx = v1.x + (float)(i - v1.y)*dbx_step;

			float tex_su, tex_sv, tex_sw, tex_eu, tex_ev, tex_ew;
			if(text_on)
			{	
				//Get the A texture line position on this "Y"
				tex_su = t2.u + (float)(i - v2.y)*du1_step;	//"su" -> Starting U
				tex_sv = t2.v + (float)(i - v2.y)*dv1_step;
				tex_sw = t2.w + (float)(i - v2.y)*dw1_step;

				//Get the B texture line position on this "Y"
				tex_eu = t1.u + (float)(i - v1.y)*du2_step;	//"eu" -> Ending U
				tex_ev = t1.v + (float)(i - v1.y)*dv2_step;
				tex_ew = t1.w + (float)(i - v1.y)*dw2_step;
			}

			//Line A "x" has to be smaller than line B "x" to draw from left to right
			if(ax > bx)
			{
				int aux_i; float aux_f;
				//Swap the screen "x" 
				aux_i = bx;
				bx = ax;
				ax = aux_i;

				if(text_on)
				{	
					//Swap the starting and ending texture U and V
					aux_f = tex_su;
					tex_su = tex_eu;
					tex_eu = aux_f;

					aux_f = tex_sv;
					tex_sv = tex_ev;
					tex_ev = aux_f;

					aux_f = tex_sw;
					tex_sw = tex_ew;
					tex_ew = aux_f;
				}
			}

			//This is the final point of the texture to get the color from
			float tex_u = tex_su;
			float tex_v = tex_sv;
			float tex_w = tex_sw;

			//Get the range of a step between the "X"s from lines A and B 
			float tstep = 1.0f / (float)(bx - ax);
			float t = 0.0f;		//Acumulate steps

			if(text_on)
			{	
				for(int j = ax; j < bx; j++)
				{
					tex_u = (1.0f - t)*tex_su + t*tex_eu;
					tex_v = (1.0f - t)*tex_sv + t*tex_ev;
					tex_w = (1.0f - t)*tex_sw + t*tex_ew;
					rgb_t color;

					/*if(tex_u/tex_w > 1) printf("\n U/W ABAJO > 1 --- W: %f", tex_w);
					if(tex_v/tex_w > 1) printf("\n V/W ABAJO > 1 --- W: %f", tex_w);
					else printf("\n --- W: %f", tex_w);*/

					get_color_texture(texture, tex_u / tex_w, tex_v / tex_w, TEXTURE_H, TEXTURE_W, &color);
					SDL_SetRenderDrawColor(gp_renderer, color.r*bright, color.g*bright, color.b*bright, 255);
					SDL_RenderDrawPoint(gp_renderer, j, i);
					t += tstep;
				}
			}
			else
			{	
				SDL_SetRenderDrawColor(gp_renderer, 140*bright, 140*bright, 140*bright, 255);
				SDL_RenderDrawLine(gp_renderer, ax, i, bx, i);
			}




		}
	}


	



}


void get_color_texture(uint32_t texture[][TEXTURE_W*TEXTURE_H], float u, float v, int height, int width, rgb_t* color)
{
	int col = u * width + 0.5;
	int fil = v * height + 0.5;


	uint32_t full_color = texture[0][fil*TEXTURE_W+col];
	color->r = full_color & 0x000000FF;
	color->g = (full_color & 0x0000FF00) >> 8;
	color->b = (full_color & 0x00FF0000) >> 16;

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
	vec3d_t up = {.x = 0, .y =1, .z = 0, .w = 1};
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
	vector_mul(&forward_cam, vector_dot_prod(up, &forward_cam), &b);
	sub_vectors(up, &b, &up_cam);
	normalise_vector(&up_cam, &up_cam);

	//Get the right vector
	vector_cross_prod(&up_cam, &forward_cam, &right_cam);



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
	matrix->m[3][0] = -(vector_dot_prod(pos, &right_cam));		
	matrix->m[3][1] = -(vector_dot_prod(pos, &up_cam));		
	matrix->m[3][2] = -(vector_dot_prod(pos, &forward_cam));		
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
	vec3d_t out_vec;

	out_vec.x = in->x * mat->m[0][0] + in->y * mat->m[1][0] + in->z * mat->m[2][0] + in->w * mat->m[3][0];
	out_vec.y = in->x * mat->m[0][1] + in->y * mat->m[1][1] + in->z * mat->m[2][1] + in->w * mat->m[3][1];
	out_vec.z = in->x * mat->m[0][2] + in->y * mat->m[1][2] + in->z * mat->m[2][2] + in->w * mat->m[3][2];
	out_vec.w = in->x * mat->m[0][3] + in->y * mat->m[1][3] + in->z * mat->m[2][3] + in->w * mat->m[3][3];

	*out = out_vec;
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


/*void vector_intersect_plane(vec3d_t* plane_p, vec3d_t* plane_n, vec3d_t* line_start, vec3d_t* line_end, vec3d_t* inter_p)
{
	vec3d_t line_vect;
	//Get the directional vector of the line
	sub_vectors(line_end, line_start, &line_vect);

	//Get the sum of plane point and line starting point
	vec3d_t sub_p_plane_line;
	sub_vectors(plane_p, line_start, &sub_p_plane_line);

	//Get the dot product of N*(P+r0) and negate it
	float a = vector_dot_prod(plane_n, &sub_p_plane_line);

	//Get the dot product of N*vr
	float b = vector_dot_prod(plane_n, &line_vect);

	float t = a/b;

	//Get the vector that points to the interset point
	vector_mul(&line_vect, t, &line_vect);
	//Add the vector to the starting point of line to get the intersect point
	add_vectors(line_start, &line_vect, inter_p);

}*/

void vector_intersect_plane(vec3d_t* plane_p, vec3d_t* plane_n, vec3d_t* line_start, vec3d_t* line_end, vec3d_t* inter_p, float* t)
{
	vec3d_t plane_nn;
	normalise_vector(plane_n, &plane_nn);

	float plane_d = -vector_dot_prod(&plane_nn, plane_p);
	float ad = vector_dot_prod(line_start, &plane_nn);
	float bd = vector_dot_prod(line_end, &plane_nn);
	*t = (-plane_d - ad) / (bd - ad);
	vec3d_t line_start_to_end;
	sub_vectors(line_end, line_start, &line_start_to_end);
	vec3d_t line_to_intersect;
	vector_mul(&line_start_to_end, *t, &line_to_intersect);
	vec3d_t inter_point;
	add_vectors(line_start, &line_to_intersect, &inter_point);
	*inter_p = inter_point;

}


float dist_point_plane(vec3d_t* plane_p, vec3d_t* plane_n, vec3d_t* point)
{	
	//Assure that the plane normal is normalised
	vec3d_t plane_n_n;
	normalise_vector(plane_n, &plane_n_n);

	return (plane_n_n.x*point->x + plane_n_n.y*point->y + plane_n_n.z*point->z - vector_dot_prod(&plane_n_n, plane_p));
}










