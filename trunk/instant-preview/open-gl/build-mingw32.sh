g++ -DSTANDALONE -Wall -Wextra src/main.cc -I./include -L/usr/lib `sdl-config --libs` -lopengl32 -lglu32 -mwindows