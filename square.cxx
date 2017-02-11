#include <iostream>
#include <vector>
#include <SDL2/SDL.h>

struct Ball {
	SDL_Rect r;
	int dx, dy;

	void move();
	void draw(SDL_Renderer *rend, float draw_interp);
};

int width = 640, height = 480;

int main(int argc, char *argv[]){
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		std::cerr << "SDL_Init: " << SDL_GetError() << '\n';
		return 1;
	}

	auto win = SDL_CreateWindow("Prong",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width,
		height,
		SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
	if(!win){
		std::cerr << "SDL_CreateWindow: " << SDL_GetError() << '\n';
		return 1;
	}

	auto rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
	SDL_RenderSetLogicalSize(rend, width, height);

	Ball ball = {
		{(width-8-64)/2, (height-8-64)/2, 32, 32},
		-1, -1,
	};

	Uint32 last_time = 0;
	Uint32 frame_ms = 8, lag = 0;

	last_time = SDL_GetTicks();
	for(;;){
		auto current_time = SDL_GetTicks();
		auto dt = current_time - last_time;
		last_time = current_time;
		lag += dt;

		SDL_Event e;
		while(SDL_PollEvent(&e)){
			if(e.type == SDL_QUIT){
				exit(0);
			}
		}

		while(lag >= frame_ms){
			ball.move();
			lag -= frame_ms;
		}
		auto draw_interp = float(lag) / float(frame_ms);

		SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
		SDL_RenderClear(rend);
		ball.draw(rend, draw_interp);
		SDL_RenderPresent(rend);
	}

	return 0;
}

void Ball::draw(SDL_Renderer *rend, float draw_interp) {
	auto interp_ball = this->r;
	if(draw_interp != 0){
		interp_ball.x += draw_interp*this->dx;
		interp_ball.y += draw_interp*this->dy;
	}
	SDL_SetRenderDrawColor(rend, 64, 128, 255, 255);
	SDL_RenderFillRect(rend, &interp_ball);
}

void Ball::move() {
	this->r.x += this->dx;
	this->r.y += this->dy;
	if(this->r.x < -this->r.w){
		this->r.x = width;
	}
	if(this->r.x > width){
		this->r.x = 0;
	}
	if(this->r.y < 0){
		this->dy = -this->dy;
	}
	if(this->r.y > height-this->r.h){
		this->dy = -this->dy;
	}
}
