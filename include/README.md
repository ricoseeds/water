
###### Run in Mac
```
$ g++ src/main.cpp src/ShaderProgram.cpp src/Texture2D.cpp src/Camera.cpp src/Mesh.cpp  -framework OpenGl -I/usr/local/include -lglfw -lGLEW 

$ ./a.out
```
###### Run in Linux
```
g++ src/main.cpp src/ShaderProgram.cpp src/Texture2D.cpp src/Camera.cpp src/Mesh.cpp -std=c++11 -lglfw -lGL -lm -lXrandr -lXi -lX11 -lXxf86vm -lpthread -lGLEW 
```
###### GUI
```
g++ src/main.cpp src/ShaderProgram.cpp src/Texture2D.cpp src/Camera.cpp src/Mesh.cpp vendor/im
gui/imgui.cpp vendor/imgui/imgui_demo.cpp vendor/imgui/imgui_draw.cpp vendor/imgui/imgui_widgets.cpp vendor/i
mgui/imgui_impl_glfw.cpp vendor/imgui/imgui_impl_opengl3.cpp -framework OpenGl -I/usr/local/include -lglfw -lGLEW
```