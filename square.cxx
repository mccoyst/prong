#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <SDL2/SDL.h>

GLint init_shaders();

int width = 640, height = 480;

int main(int argc, char *argv[]){
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		std::cerr << "SDL_Init: " << SDL_GetError() << '\n';
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	if(SDL_GL_SetSwapInterval(-1) < 0){
		// Late-swap tearing isn't possible, so use good old vsync:
		SDL_GL_SetSwapInterval(1);
	}

	auto win = SDL_CreateWindow("Square",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width,
		height,
		SDL_WINDOW_OPENGL|SDL_WINDOW_ALLOW_HIGHDPI);
	if(!win){
		std::cerr << "SDL_CreateWindow: " << SDL_GetError() << '\n';
		return 1;
	}

	SDL_GL_CreateContext(win);
	glewExperimental = GL_TRUE; 
	auto err = glewInit();
	if(err != GLEW_OK){
		std::cerr << "glewinit: " << glewGetErrorString(err) << '\n';
		return 1;
	}

	std::cerr << glGetString(GL_VERSION) << '\n';
	std::cerr << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';
	for(;;){
		auto err = glGetError();
		if(err == GL_NO_ERROR){
			break;
		}
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLfloat verts[] = {
		-0.04f, 0.04f,
		0.04f, 0.04f,
		0.04f, -0.04f,
		0.04f, -0.04f,
		-0.04f, 0.04f,
		-0.04f, -0.04f,
	};

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	auto p = init_shaders();
	GLint pos = glGetAttribLocation(p, "position");
	glEnableVertexAttribArray(pos);
	glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 0, 0);
	GLint off = glGetUniformLocation(p, "offset");

	Uint32 last_time = 0;
	//Uint32 frame_ms = 16, lag = 0;
	GLfloat dx = 1/float(width);

	last_time = SDL_GetTicks();
	for(;;){
		//auto current_time = SDL_GetTicks();
		//auto dt = current_time - last_time;
		//last_time = current_time;
		//lag += dt;

		SDL_Event e;
		while(SDL_PollEvent(&e)){
			if(e.type == SDL_QUIT){
				exit(0);
			}
		}

		//while(lag >= frame_ms){
		//	lag -= frame_ms;
		//}
		//auto draw_interp = float(lag) / float(frame_ms);

		glClearColor(0,0,0,1);
		glClear(GL_COLOR_BUFFER_BIT);
		glUniform2f(off, dx, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		SDL_GL_SwapWindow(win);
		dx += 3/float(width);
		if(dx > 1.0){
			dx = -1;
		}
	}

	return 0;
}
