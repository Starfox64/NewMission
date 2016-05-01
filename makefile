all: plugin.c
	gcc -c plugin.c
	gcc -shared -o new_mission.dll plugin.o
