package main

/*
#include <GL/glew.h>
#include <SDL2/SDL.h>

#cgo darwin LDFLAGS: -framework SDL2 -framework OpenGL -lglew
#cgo linux LDFLAGS: -L/usr/local/lib -lSDL2

static Uint32 event_type(SDL_Event *e){
	return e->type;
}

static void GenVertexArrays(GLsizei n, GLuint *arrays){ glGenVertexArrays(n, arrays); }
static void BindVertexArray(GLuint array){ glBindVertexArray(array); }
static void GenBuffers(GLsizei n, GLuint * buffers){ glGenBuffers(n, buffers); }
static void BindBuffer(GLenum target, GLuint buffer){ glBindBuffer(target, buffer); }
static void BufferData(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage){ glBufferData(target, size, data, usage); }
static GLuint GetAttribLocation(GLuint program, const GLchar *name){ return glGetAttribLocation(program, name); }
static void EnableVertexAttribArray(GLuint index){ glEnableVertexAttribArray(index); }
static void VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer){ glVertexAttribPointer(index, size, type, normalized, stride, pointer); }
static GLuint GetUniformLocation(GLuint program, const GLchar *name){ return glGetUniformLocation(program, name); }
static void Uniform2f(GLint location, GLfloat v0, GLfloat v1){ glUniform2f(location, v0, v1); }

static GLuint init_shaders(){
	const GLchar* vsrc =
		"#version 150\n"
		"in vec2 position;"
		"uniform vec2 offset;"
		"void main() {"
		"	gl_Position = vec4(position+offset, 0.0, 1.0);"
		"}";
	const GLchar* fsrc =
		"#version 150\n"
		"out vec4 outColor;"
		"void main() {"
		"	outColor = vec4(1.0, 1.0, 1.0, 1.0);"
		"}";

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vsrc, NULL);
	glCompileShader(vs);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fsrc, NULL);
	glCompileShader(fs);

	GLuint p = glCreateProgram();
	glAttachShader(p, vs);
	glAttachShader(p, fs);
	glBindFragDataLocation(p, 0, "outColor");
	glLinkProgram(p);
	GLint ok = 0;
	glGetProgramiv(p, GL_LINK_STATUS, &ok);
	if(ok){
		glUseProgram(p);
		return p;
	}
	return 0;
}
*/
import "C"

import (
	"fmt"
	"os"
	"unsafe"
)

const (
	width = 640
	height = 480
)

func main() {
	if C.SDL_Init(C.SDL_INIT_VIDEO) < 0 {
		fmt.Println("SDL_Init: ", C.GoString(C.SDL_GetError()))
		os.Exit(1)
	}

	C.SDL_GL_SetAttribute(C.SDL_GL_CONTEXT_MAJOR_VERSION, 3)
	C.SDL_GL_SetAttribute(C.SDL_GL_CONTEXT_MINOR_VERSION, 2)
	C.SDL_GL_SetAttribute(C.SDL_GL_CONTEXT_PROFILE_MASK, C.SDL_GL_CONTEXT_PROFILE_CORE)
	C.SDL_GL_SetAttribute(C.SDL_GL_ACCELERATED_VISUAL, 1)
	C.SDL_GL_SetAttribute(C.SDL_GL_DOUBLEBUFFER, 1)
	C.SDL_GL_SetAttribute(C.SDL_GL_DEPTH_SIZE, 24)

	if C.SDL_GL_SetSwapInterval(-1) < 0 {
		// Late-swap tearing isn't possible, so use good old vsync:
		C.SDL_GL_SetSwapInterval(1)
	}

	wintitle := C.CString("Go Square")
	win := C.SDL_CreateWindow(wintitle,
		C.SDL_WINDOWPOS_CENTERED,
		C.SDL_WINDOWPOS_CENTERED,
		width,
		height,
		C.SDL_WINDOW_OPENGL|C.SDL_WINDOW_ALLOW_HIGHDPI)
	if win == nil {
		fmt.Println("SDL_CreateWindow: ", C.GoString(C.SDL_GetError()))
		os.Exit(1)
	}

	C.SDL_GL_CreateContext(win)
	C.glewExperimental = C.GL_TRUE
	err := C.glewInit();
	if err != C.GLEW_OK {
		fmt.Println("glewinit: ", C.glewGetErrorString(err))
		os.Exit(1)
	}

	fmt.Println(C.glGetString(C.GL_VERSION))
	fmt.Println(C.glGetString(C.GL_SHADING_LANGUAGE_VERSION))
	for {
		err := C.glGetError()
		if err == C.GL_NO_ERROR {
			break
		}
	}

	C.glEnable(C.GL_BLEND)
	C.glBlendFunc(C.GL_SRC_ALPHA, C.GL_ONE_MINUS_SRC_ALPHA)

	verts := []C.GLfloat{
		-0.04, 0.04,
		0.04, 0.04,
		0.04, -0.04,
		0.04, -0.04,
		-0.04, 0.04,
		-0.04, -0.04,
	}

	var vao C.GLuint
	C.GenVertexArrays(1, &vao);
	C.BindVertexArray(vao);
	var vbo C.GLuint
	C.GenBuffers(1, &vbo);
	C.BindBuffer(C.GL_ARRAY_BUFFER, vbo);
	C.BufferData(C.GL_ARRAY_BUFFER, C.GLsizeiptr(8*len(verts)), unsafe.Pointer(&verts[0]), C.GL_STATIC_DRAW)

	p := C.init_shaders()
	pos := C.GetAttribLocation(p, (*C.GLchar)(C.CString("position")))
	C.EnableVertexAttribArray(pos)
	C.VertexAttribPointer(pos, 2, C.GL_FLOAT, C.GL_FALSE, 0, nil)
	off := C.GetUniformLocation(p, (*C.GLchar)(C.CString("offset")))

	//var last_time C.Uint32
	//Uint32 frame_ms = 16, lag = 0;
	dx := 1/C.GLfloat(width)

	//last_time = C.SDL_GetTicks()
	for {
		//auto current_time = C.SDL_GetTicks();
		//auto dt = current_time - last_time;
		//last_time = current_time;
		//lag += dt;

		var e C.SDL_Event
		for C.SDL_PollEvent(&e) != 0 {
			if C.event_type(&e) == C.SDL_QUIT {
				return
			}
		}

		//while(lag >= frame_ms){
		//	lag -= frame_ms;
		//}
		//auto draw_interp = float(lag) / float(frame_ms);

		C.glClearColor(0,0,0,1)
		C.glClear(C.GL_COLOR_BUFFER_BIT)
		C.Uniform2f(C.GLint(off), dx, 0)
		C.glDrawArrays(C.GL_TRIANGLES, 0, 6)
		C.SDL_GL_SwapWindow(win)
		dx += 3/C.GLfloat(width)
		if(dx > 1.0){
			dx = -1
		}
	}
}
