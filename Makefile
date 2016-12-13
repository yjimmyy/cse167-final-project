CFLAGS = -fsanitize=address -g --std=c++1z -DGL_GLEXT_PROTOTYPES -DGL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED

# glew cpp/linker flags
GLCCFLAGS := `pkg-config --cflags glfw3 glew`
GLLDFLAGS := `pkg-config --libs glfw3 glew`

# imagemagick cpp/linker flags
IMINCFLAGS := `Magick++-config --cppflags`
IMLDFLAGS := `Magick++-config --libs`

# combined flags
INCFLAGS = -I/usr/X11/include $(GLCCFLAGS) $(IMINCFLAGS) -I./tinyobjloader
LDFLAGS = -lGL -lstdc++ $(GLLDFLAGS) $(IMLDFLAGS)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CFLAGS += -DLinux
endif
ifeq ($(UNAME_S),Darwin)
	LDFLAGS += \
		-framework OpenGL \
		-L"/System/Library/Frameworks/OpenGL.framework/Libraries"
	CFLAGS += -DOSX
endif

RM = /bin/rm -f

.PHONY: all
all: glfwStarterProjectBin

.PHONY: release
release: CFLAGS += -O3 -funroll-loops
release: all

all_and_run: glfwStarterProjectBin
	./glfwStarterProjectBin

test_geom: test_geom.o window.o
	$(CXX) $(CFLAGS) -o test_geom $^ $(INCFLAGS) $(LDFLAGS)
test_geom.o: test_geom.cc
	$(CXX) $(CFLAGS) $(INCFLAGS) -c test_geom.cc

OBJS = \
	main.o window.o OBJObject.o shader.o \
	util.o skybox.o tiny_obj_loader.o \
	camera.o scene.o framebuffer.o \
	quad_mesh.o

glfwStarterProjectBin: $(OBJS)
	$(CXX) $(CFLAGS) -o glfwStarterProjectBin $^ $(INCFLAGS) $(LDFLAGS)
main.o: window.o main.cpp gl-inl.h
	$(CXX) $(CFLAGS) $(INCFLAGS) -c main.cpp
window.o: window.cc window.h gl-inl.h
	$(CXX) $(CFLAGS) $(INCFLAGS) -c $<
skybox.o: skybox.cc skybox.h gl-inl.h
	$(CXX) $(CFLAGS) $(INCFLAGS) -c $<
shader.o: shader.cc gl-inl.h
	$(CXX) $(CFLAGS) $(INCFLAGS) -c $<
scene.o: scene.cc gl-inl.h
	$(CXX) $(CFLAGS) $(INCFLAGS) -c $<
OBJObject.o: OBJObject.cpp OBJObject.h gl-inl.h
	$(CXX) $(CFLAGS) $(INCFLAGS) -c $<
util.o: util.cpp util.h
	$(CXX) $(CFLAGS) $(INCFLAGS) -c $<
camera.o: camera.cc camera.h
	$(CXX) $(CFLAGS) $(INCFLAGS) -c $<
framebuffer.o: framebuffer.cc framebuffer.h
	$(CXX) $(CFLAGS) $(INCFLAGS) -c $<
quad_mesh.o: quad_mesh.cc quad_mesh.h
	$(CXX) $(CFLAGS) $(INCFLAGS) -c $<
clean:
	$(RM) *.o glfwStarterProjectBin
tiny_obj_loader.o: tinyobjloader/tiny_obj_loader.cc tinyobjloader/tiny_obj_loader.h
	$(CXX) $(CFLAGS) $(INCFLAGS) -c $<

main.cpp: window.h scene.h quad_mesh.h
OBJObject.cpp: OBJObject.h
OBJObject.h: mesh.h
quad_mesh.h: mesh.h
quad_mesh.cc: quad_mesh.h
window.cc: window.h scene.h framebuffer.h shader.h
framebuffer.cc: framebuffer.h
scene.cc: scene.h
shader.cc: shader.h
util.cpp: util.h
