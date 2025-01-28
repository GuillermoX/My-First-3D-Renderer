#include "../include/3drenderer.h"
#define MODELS_PATH "./data/"
#define TEXTURES_PATH "./data/textures/"


int main(int argc, char *argv[])
{
	scene_t scene;
	if(argc == 1)
	{
		printf("Especify the name of the model");
	}
	else
	{
		bool start = true;

		char path[30];
		strcpy(path, MODELS_PATH);
		strcat(path, argv[1]);
		strcat(path, ".obj");
		strcpy(scene.model_path, path);
		printf("\nModel: %s", scene.model_path);
		if(argc == 2) scene.texture_mode = 0;
		else if(argc == 3 && strcmp(argv[2], "-t") == 0) scene.texture_mode = TEXT_UV_MODEL_OFF;
		else if(argc == 4 && strcmp(argv[2], "-t") == 0)
		{
			scene.texture_mode = TEXT_UV_MODEL_ON;

			strcpy(path, TEXTURES_PATH);
			strcat(path, argv[3]);
			strcat(path, ".c");
			strcpy(scene.text_path, path);
			printf("\tTexture: %s", scene.text_path);
		}
		else
		{
			start = false;
			printf("\nInvalid arguments");
		}

		if(start)
		{
			start_scene(&scene);
			while(scene.running)
			{
				process_input(&scene);
				update_scene(&scene);
				render_scene(&scene);
			}
			stop_scene();
		}
	}

	return 0;
}
