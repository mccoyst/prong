#include <iostream>
#include <vector>
#include <SDL2/SDL.h>

struct Ball {
	SDL_Rect r;
	int dx, dy;

	void move();
	void draw(SDL_Renderer *rend, float draw_interp);
};

int move_goals(std::vector<SDL_Rect>*, int);
void draw_goals(SDL_Renderer *rend, std::vector<SDL_Rect> *goals, float draw_interp, int dy);

struct Pad {
	SDL_Rect r;
	int dy;
	SDL_Keycode up, down;

	void control(SDL_Event*);
	void move();
};

inline bool keydown(SDL_Event *e, SDL_Keycode s) {
	return e->type == SDL_KEYDOWN && e->key.keysym.sym == s;
}

inline bool keyup(SDL_Event *e, SDL_Keycode s) {
	return e->type == SDL_KEYUP && e->key.keysym.sym == s;
}

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

	std::vector<SDL_Rect> left_goals = {
		{8, 8, 64, 64},
		{8+8, 8+8, 64-16, 64-16},
		{8+24, 8+24, 64-48, 64-48},
	};
	std::vector<SDL_Rect> right_goals = {
		{width-8-64, height-8-64, 64, 64},
		{width-8-64+8, height-8-64+8, 64-16, 64-16},
		{width-8-64+24, height-8-64+24, 64-48, 64-48},
	};
	auto left_goal_dy = 1, right_goal_dy = -1;
	int flash = 0;
	bool bright = false;

	Pad left_pad = {
		{8+64+8, 8, 32, 128},
		0,
		SDLK_e, SDLK_d,
	};
	Pad right_pad = {
		{width-8-64-32-8, 8, 32, 128},
		0,
		SDLK_UP, SDLK_DOWN,
	};

	Ball ball = {
		{(width-8-64)/2, (height-8-64)/2, 32, 32},
		-1, 0,
	};
	auto original_ball = ball;

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
			left_pad.control(&e);
			right_pad.control(&e);
		}

		while(lag >= frame_ms){
			left_pad.move();
			right_pad.move();

			ball.move();

			left_goal_dy = move_goals(&left_goals, left_goal_dy);
			right_goal_dy = move_goals(&right_goals, right_goal_dy);

			if(SDL_HasIntersection(&left_pad.r, &ball.r)){
				ball.dy += left_pad.dy;
				ball.dx = -ball.dx;
			}
			if(SDL_HasIntersection(&right_pad.r, &ball.r)){
				ball.dy += right_pad.dy;
				ball.dx = -ball.dx;
			}

			if(flash == 0 && SDL_HasIntersection(&left_goals[0], &ball.r)){
				flash = 50;
				ball = original_ball;
			}
			if(flash == 0 && SDL_HasIntersection(&right_goals[0], &ball.r)){
				flash = 50;
				ball = original_ball;
			}

			lag -= frame_ms;
		}
		auto draw_interp = float(lag) / float(frame_ms);

		if(flash > 0){
			if(flash % 5){
				bright = !bright;
			}
			flash--;
		}
		if(bright){
			SDL_SetRenderDrawColor(rend, 64, 64, 64, 255);
		}else{
			SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
		}
		SDL_RenderClear(rend);

		ball.draw(rend, draw_interp);

		draw_goals(rend, &left_goals, draw_interp, left_goal_dy);
		draw_goals(rend, &right_goals, draw_interp, right_goal_dy);

		SDL_SetRenderDrawColor(rend, 128, 255, 64, 255);
		SDL_RenderFillRect(rend, &left_pad.r);
		SDL_RenderFillRect(rend, &right_pad.r);

		SDL_RenderPresent(rend);
	}

	return 0;
}

int move_goals(std::vector<SDL_Rect> *goals, int dy){
	for(auto& r : *goals){
		if(dy < 0 && r.y < 0){
			dy = 1;
		}
		if(dy > 0 && r.y + r.h > height){
			dy = -1;
		}
		r.y += dy;
	}
	return dy;
}

void draw_goals(SDL_Renderer *rend, std::vector<SDL_Rect> *goals, float draw_interp, int dy) {
	auto interp_goals = *goals;
	if(draw_interp != 0){
		move_goals(goals, draw_interp*dy);
	}
	SDL_SetRenderDrawColor(rend, 255, 128, 64, 255);
	SDL_RenderDrawRects(rend, interp_goals.data(), interp_goals.size());
}

void Pad::control(SDL_Event *e) {
	if(keydown(e, this->up) && this->dy == 0){
		this->dy = -2;
	}
	if(keydown(e, this->down) && this->dy == 0){
		this->dy = 2;
	}
	if(keyup(e, this->up) && this->dy == -2){
		this->dy = 0;
	}
	if(keyup(e, this->down) && this->dy == 2){
		this->dy = 0;
	}
}

void Pad::move() {
	this->r.y += dy;
	if(this->r.y < 0){
		this->r.y = 0;
	}
	if(this->r.y+this->r.h > height){
		this->r.y = height - this->r.h;
	}
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
