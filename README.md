
###### Run in Mac
```
$ g++ src/main.cpp src/ShaderProgram.cpp src/Texture2D.cpp src/Camera.cpp src/Mesh.cpp  src/shader.cpp src/texture.cpp  -framework OpenGl -I/usr/local/include -lglfw -lGLEW 

$ ./a.out
```
###### Run in Linux
```
g++ src/main.cpp src/ShaderProgram.cpp src/Texture2D.cpp src/Camera.cpp src/Mesh.cpp  src/shader.cpp src/texture.cpp -std=c++11 -lglfw -lGL -lm -lXrandr -lXi -lX11 -lXxf86vm -lpthread -lGLEW 
```

