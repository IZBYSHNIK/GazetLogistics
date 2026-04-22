run : build
	./main &

build : build_ui
	g++ ./libs/tinyxml2.cpp main.cpp -o main `pkg-config --cflags --libs Qt5Widgets`  -Wl,-rpath,'$ORIGIN/so' 
	#ldd ./main | grep "=>" | awk '{print $3}' | xargs -I {} cp {} ./so/

build_ui :
	uic form.ui > form.h
