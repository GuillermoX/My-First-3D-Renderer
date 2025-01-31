
INCLUDES = -I./include

LIBS = -lSDL2 -lpng -lm

MAIN = ./bin/3drenderer

MODEL = spyroDragon
TEXTURE = -t

$(MAIN): ./source/*.c ./include/*.h
	gcc -Wall ./source/*.c $(INCLUDE) $(LIBS) -o $(MAIN)

run:
	./$(MAIN) $(MODEL) $(TEXTURE)

debug:
	gcc -std=99 -Wall -g ./source/*.c $(INCLUDE) $(LIBS) -o $(MAIN)
	gdb ./$(MAIN)

clean:
	rm ./$(MAIN)
