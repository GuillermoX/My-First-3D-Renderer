
INCLUDES = -I./include

LIBS = -lSDL2 -lm

MAIN = ./bin/3drenderer

$(MAIN): ./source/*.c ./include/*.h
	gcc -Wall ./source/*.c $(INCLUDE) $(LIBS) -o $(MAIN)

run:
	./$(MAIN)

clean:
	rm ./$(MAIN)
