#include <iostream>
#include <vector>
#include <SDL2/SDL.h>

int move_goals(std::vector<SDL_Rect>*, int);
void draw_goals(SDL_Renderer *rend, std::vector<SDL_Rect> *goals, float draw_interp, int dy);
void move_pad(SDL_Rect*, int);
int move_ball(SDL_Rect*, int, int);
void draw_ball(SDL_Renderer *rend, SDL_Rect *ball, float draw_interp, int dx, int dy);

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

	SDL_Rect left_pad = {8+64+8, 8, 32, 128 };
	int left_dy = 0;
	SDL_Rect right_pad = {width-8-64-32, 8, 32, 128 };
	int right_dy = 0;

	SDL_Rect ball = {(width-8-64)/2, (height-8-64)/2, 32, 32};
	int ball_dx = -1, ball_dy = 0;

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
			if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_e && left_dy == 0){
				left_dy = -2;
			}
			if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_d && left_dy == 0){
				left_dy = 2;
			}
			if(e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_e && left_dy == -2){
				left_dy = 0;
			}
			if(e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_d && left_dy == 2){
				left_dy = 0;
			}
			if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_UP && right_dy == 0){
				right_dy = -2;
			}
			if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_DOWN && right_dy == 0){
				right_dy = 2;
			}
			if(e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_UP && right_dy == -2){
				right_dy = 0;
			}
			if(e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_DOWN && right_dy == 2){
				right_dy = 0;
			}
		}

		while(lag >= frame_ms){
			ball.x += ball_dx;
			ball.y += ball_dy;
			if(ball.x < -ball.w){
				ball.x = width;
			}
			if(ball.x > width){
				ball.x = 0;
			}
			if(ball.y < 0){
				ball_dy = -ball_dy;
			}
			if(ball.y > height-ball.h){
				ball_dy = -ball_dy;
			}

			move_pad(&left_pad, left_dy);
			move_pad(&right_pad, right_dy);

			left_goal_dy = move_goals(&left_goals, left_goal_dy);
			right_goal_dy = move_goals(&right_goals, right_goal_dy);

			if(SDL_HasIntersection(&left_pad, &ball)){
				ball_dy += left_dy;
				ball_dx = -ball_dx;
			}
			if(SDL_HasIntersection(&right_pad, &ball)){
				ball_dy += right_dy;
				ball_dx = -ball_dx;
			}

			if(flash == 0 && SDL_HasIntersection(&left_goals[0], &ball)){
				flash = 50;
			}
			if(flash == 0 && SDL_HasIntersection(&right_goals[0], &ball)){
				flash = 50;
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

		draw_ball(rend, &ball, draw_interp, ball_dx, ball_dy);

		draw_goals(rend, &left_goals, draw_interp, left_goal_dy);
		draw_goals(rend, &right_goals, draw_interp, right_goal_dy);

		SDL_SetRenderDrawColor(rend, 128, 255, 64, 255);
		SDL_RenderDrawRect(rend, &left_pad);
		SDL_RenderDrawRect(rend, &right_pad);

		SDL_RenderPresent(rend);
	}

	return 0;
}

int move_goals(std::vector<SDL_Rect> *goals, int dy){
	for(auto& r : *goals){
		if(r.y < 0){
			dy = 1;
		}
		if(r.y + r.h > height){
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

void move_pad(SDL_Rect *pad, int dy) {
	pad->y += dy;
	if(pad->y < 0){
		pad->y = 0;
	}
	if(pad->y+pad->h > height){
		pad->y = height - pad->h;
	}
}

void draw_ball(SDL_Renderer *rend, SDL_Rect *ball, float draw_interp, int dx, int dy) {
	auto interp_ball = *ball;
	if(draw_interp != 0){
		interp_ball.x += draw_interp*dx;
		interp_ball.y += draw_interp*dy;
	}
	SDL_SetRenderDrawColor(rend, 64, 128, 255, 255);
	SDL_RenderDrawRect(rend, &interp_ball);
}

