mkdir -p build
g++ -I./glad main.cpp -I./stb -o build/Audio $(sdl2-config --cflags --libs)