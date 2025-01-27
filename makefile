
INCLUDES = -I./include

LIBS = -lSDL2 -lm

MAIN = ./bin/3drenderer

MODEL = object

$(MAIN): ./source/*.c ./include/*.h
	gcc -Wall ./source/*.c $(INCLUDE) $(LIBS) -o $(MAIN)

run:
	./$(MAIN) $(MODEL) $(TEXTURE)

debug:
	gcc -Wall -g ./source/*.c $(INCLUDE) $(LIBS) -o $(MAIN)
	gdb ./$(MAIN)

clean:
	rm ./$(MAIN)
