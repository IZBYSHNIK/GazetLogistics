run : build
	./main &

build : build_ui
	g++  ./libs/tinyxml2.cpp main.cpp -o main `pkg-config --cflags --libs Qt5Widgets`

build_ui :
	uic form.ui > form.h
