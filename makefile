
INCLUDES = -I./include

LIBS = -lSDL2 -lm

MAIN = 3drenderer

$(MAIN): ./source/*.c
	gcc -Wall ./source/*.c $(INCLUDE) $(LIBS) -o $(MAIN)

run:
	./$(MAIN)

delete:
	rm ./$(MAIN)
