#include "../include/3drenderer.h"


int main()
{
	scene_t scene;
	start_scene(&scene);
	while(scene.running)
	{
		process_input(&scene);
		update_scene(&scene);
		render_scene(&scene);
	}
	stop_scene();

	return 0;
}
