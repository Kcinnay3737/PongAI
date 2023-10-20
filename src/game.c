#include "game.h"
#include <raylib.h>
#include "raymath.h"
#include <float.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static size_t factor = 60;
typedef struct ball_s {

	float x, y; 
	int w,h; 
	int dx, dy;
} ball_t;

typedef struct paddle {
    float x,y;
	int w,h;
} paddle_t;

static ball_t ball;
#define MAX_BALL_SPEED 150
const int ball_max_speed = MAX_BALL_SPEED;
const float paddle_speed = MAX_BALL_SPEED * 5;
static paddle_t paddle[2];
int score[] = {0,0};

float LastDistanceY = 0.0f;
int MaxGameScore = 30;

static Vector2 paddle_dims = {.x=10,.y=40};

#define REWARD_CLOSE_Y +10
#define REWARD_FAR_Y -10
#define REWARD_SAME_Y 0

#define REWARD_WIN +250
#define REWARD_LOSE -250
#define REWARD_CORNERHIT +100
#define REWARD_MIDHIT +25

int CurrentReward = 0;

int check_collision(ball_t a, paddle_t b) {
	if (a.x > b.x + b.w) {
		return 0;
	}
	if (a.x + a.w < b.x) {
		return 0;
	}
	if (a.y > b.y + b.h) {
		return 0;
	}
	if (a.y + a.h < b.y) {
		return 0;
	}

	return 1;
}

void move_ball(float dt) {
	int w = GetScreenWidth();
    int h = GetScreenHeight();

	/* Move the ball by its motion vector. */
	ball.x += ball.dx * ball_max_speed * dt;
	ball.y += ball.dy * ball_max_speed * dt;
	
	/* Turn the ball around if it hits the edge of the screen. */
	if (ball.x < 0) {
		
		score[1] += 1;
		//Get win reward
		CurrentReward += REWARD_WIN;
		round_restart();
	}

	if (ball.x > w - ball.w) { 
		
		score[0] += 1;
		//Get lose reward
		CurrentReward += REWARD_LOSE;
		round_restart();
	}

	if (ball.y < 0 || ball.y > h - ball.h) {
		
		ball.dy = -ball.dy;
	}

	//check for collision with the paddle
	int i;

	for (i = 0; i < 2; i++) {
		
		int c = check_collision(ball, paddle[i]); 

		//collision detected	
		if (c == 1) {
			int max_Xspeed = 20;
			//ball moving left
			if (ball.dx < 0 && ball.dx > -max_Xspeed) {
					
				ball.dx -= 1;

			//ball moving right
			} else if(ball.dx < max_Xspeed){
					
				ball.dx += 1;
			}
			
			//change ball direction
			ball.dx = -ball.dx;
			
			//change ball angle based on where on the paddle it hit
			int hit_pos = (paddle[i].y + paddle[i].h) - ball.y;

			if(i == 1)
			{
				//Get hit reward
				if(hit_pos >= 0 && hit_pos < 14)
				{
					CurrentReward += REWARD_CORNERHIT;
				}
				else if(hit_pos >= 14 && hit_pos < 46)
				{
					CurrentReward += REWARD_MIDHIT;
				}
				else if(hit_pos >= 46 && hit_pos <= 60)
				{
					CurrentReward += REWARD_CORNERHIT;
				}
			}

			if (hit_pos >= 0 && hit_pos < 7) {
				ball.dy = 4;
			}

			if (hit_pos >= 7 && hit_pos < 14) {
				ball.dy = 3;
			}
			
			if (hit_pos >= 14 && hit_pos < 21) {
				ball.dy = 2;
			}

			if (hit_pos >= 21 && hit_pos < 28) {
				ball.dy = 1;
			}

			if (hit_pos >= 28 && hit_pos < 32) {
				ball.dy = 0;
			}

			if (hit_pos >= 32 && hit_pos < 39) {
				ball.dy = -1;
			}

			if (hit_pos >= 39 && hit_pos < 46) {
				ball.dy = -2;
			}

			if (hit_pos >= 46 && hit_pos < 53) {
				ball.dy = -3;
			}

			if (hit_pos >= 53 && hit_pos <= 60) {
				ball.dy = -4;
			}

            ball.dy += -1 + GetRandomValue(0,1) * 2; 

			//ball moving right
			if (ball.dx > 0) {

				//teleport ball to avoid mutli collision glitch
				if (ball.x < paddle_dims.x * 4) {
				
					ball.x = paddle_dims.x * 4;
				}
				
			//ball moving left
			} else {
				
				//teleport ball to avoid mutli collision glitch
				if (ball.x > w - paddle_dims.x * 4) {
				
					ball.x = w - paddle_dims.x * 4;
				}
			}
		}
	}
}

void move_paddle(int dir,int paddle_id){
	paddle[paddle_id].y += dir * paddle_speed * GetFrameTime();
    if(paddle[paddle_id].y > GetScreenHeight() - paddle[paddle_id].h){
        paddle[paddle_id].y = GetScreenHeight() - paddle[paddle_id].h;
    }
    if(paddle[paddle_id].y < 0){
        paddle[paddle_id].y = 0;
    }
}

void move_paddle_ai(float dt, int paddle_id) {
	int center = paddle[paddle_id].y + paddle[paddle_id].h * 0.5f;
	int screen_center = GetScreenHeight() - paddle[paddle_id].h * 0.5f;
    SetRandomSeed(time(0));
    int r_dir = GetRandomValue(1,1);
    int dir = ball.dy > 0 ? r_dir  : -r_dir;
    if(dir < 0 && paddle[paddle_id].y < ball.y){
        dir = r_dir;
    }

    if(dir > 0 && paddle[paddle_id].y > ball.y){
        dir = -r_dir;
    }

	move_paddle(dir,paddle_id);
}


void game_init(void){
    
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(360);
    InitWindow(factor*16, factor*9, "Pong");
    game_restart();

}
void game_deinit(void){
    CloseAudioDevice();
    CloseWindow();
}
static size_t gridsize = 64; 
static double lastTime = 0.0;
#define NUM_GRID_SQUARES 8
static int isPlayer = 0;

void game_draw(void)
{
    float dt = GetTime() - lastTime;
    lastTime = GetTime();

	if(IsKeyReleased(KEY_P))
	{
		isPlayer = !isPlayer;
	}
	if(isPlayer)
	{
		if(IsKeyDown(KEY_UP))
		{
			move_paddle(-1,0);
		}
		if(IsKeyDown(KEY_DOWN))
		{
			move_paddle(1,0);
		}
	}
	else 
	{
		move_paddle_ai(dt,0);
	}
    //move_paddle_ai(dt,1);
    move_ball(dt);

    ClearBackground(GetColor(0x000000FF));
    BeginDrawing();
    int w = GetRenderWidth();
    int h = GetRenderHeight();
    for(int i = 0; i < 2;++i)
	{
        DrawRectangle(paddle[i].x,paddle[i].y,paddle[i].w,paddle[i].h,GetColor(0xFFFFFFFF));
    }

	// Draw lines
	float ww = 5;
	Rectangle rect = {w * 0.5f - ww * 0.5f,paddle_dims.y * 0.25f,ww,paddle_dims.y - paddle_dims.y * 0.3f};
	for(int i = 0;rect.y < h;rect.y = i * (rect.height * 2))
	{
		DrawRectangleRec(rect,WHITE);
		++i;
	}

    DrawRectangle(ball.x,ball.y,ball.w,ball.h,GetColor(0xFFFFFFFF));
    char s_text[64] = {0};
    snprintf(s_text,64,"%d",score[0]);
    int fontSize = 24;
    DrawText(s_text,w* 0.5f - 50,0,fontSize,GetColor(0xFF0000FF));
	snprintf(s_text,64,"%d",score[1]);
	DrawText(s_text,w* 0.5f + 50 - ww*2,0,fontSize,GetColor(0xFF0000FF));
    
    EndDrawing();
}

/**
 * @brief Restart game.
 *
 */
void game_restart(void)
{
	round_restart();
	score[0] = 0;
	score[1] = 0;
}

void round_restart(void)
{
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    ball.x = w * 0.5f - paddle_dims.x;
    ball.y = h * 0.5f - paddle_dims.x;
    ball.w = paddle_dims.x;
	ball.h = paddle_dims.x;
    ball.dx = -1 * 4;
	ball.dy = -1 + GetRandomValue(0,1) * 4;
	
	paddle[0].x = 0;
	paddle[0].y = h * 0.5f - paddle_dims.y;
	paddle[0].w = paddle_dims.x;
	paddle[0].h = paddle_dims.y;

	paddle[1].x = w - paddle_dims.x;
	paddle[1].y = h * 0.5f - paddle_dims.y;
	paddle[1].w = paddle_dims.x;
	paddle[1].h = paddle_dims.y;

	//Init the last distance between the ball and the paddle
	float PaddleY = paddle[1].y + paddle[1].h * 0.5f;
	float BallY = ball.y + ball.h * 0.5f;
	LastDistanceY = fabs(PaddleY - BallY);
}

/**
 * @brief Move paddle.
 *
 * @param move Player move: IDLE, UP, DOWN, LEFT, or RIGHT.
 *
 */
void game_apply_move(uint8_t move)
{
	switch(move)
	{
	case IDLE:
		break;
	case UP:
		move_paddle(-1, 1);
		break;
	case DOWN:
		move_paddle(1, 1);
		break;
	case LEFT:
		break;
	case RIGHT:
		break;
	}
}

/**
 * @brief Check if the game ended (i.e., player on end).
 *
 * @return true
 * @return false
 */
bool game_is_ended(void)
{
	//Check if a player has the score
	if(score[0] >= MaxGameScore || score[1] >= MaxGameScore) return true;
    return false;
}

/**
 * @brief Get 8-bit state representation from game instance.
 *
 * @return uint16_t
 */
uint16_t game_get_state(void)
{
	int16_t state = 0;
    
    float PaddleCenterY = paddle[1].y + paddle[1].h * 0.5f;
    float BallCenterY = ball.y + ball.h * 0.5f;

	//State on top with the ball
    state |= (BallCenterY > PaddleCenterY) << 0;
	//State align with the ball
	state |= (BallCenterY == PaddleCenterY) << 1;
	//State on bottom with the ball
	state |= (BallCenterY < PaddleCenterY) << 2;

    return state;
}

/**
 * @brief Get reward for making last move.
 *
 * @return int16_t
 */
int16_t game_get_reward(void)
{
	//Get the current reward and reset it
	int TempCurrentReward = CurrentReward;
	CurrentReward = 0;

	//Get the current distance between the ball and the paddle
	float PaddleY = paddle[1].y + paddle[1].h * 0.5f;
	float BallY = ball.y + ball.h * 0.5f;
	float CurrDistanceY = fabs(PaddleY - BallY);

	//Check if the paddle is closer then the last frame
	if(CurrDistanceY < LastDistanceY)
	{
		TempCurrentReward += REWARD_CLOSE_Y;
	}
	//Check if the paddle is farther then the last frame
	else if(CurrDistanceY > LastDistanceY)
	{
		TempCurrentReward += REWARD_FAR_Y;
	}
	//Check if the paddle is the same distance then the last frame
	else if(CurrDistanceY == LastDistanceY)
	{
		TempCurrentReward += REWARD_SAME_Y;
	}

	LastDistanceY = CurrDistanceY;
	return TempCurrentReward;
}