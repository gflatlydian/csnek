#include <stdbool.h>

#ifndef SNEK_H
#define SNEK_H

typedef struct s_board
{
	int w;
	int h;
} s_board;

typedef struct s_segment
{
	char c;
	int pos_x;
	int pos_y;
	char dir;
	struct s_segment *last;
} s_segment;

typedef struct s_snek
{
	bool alive;
	int length;
	int score;
	struct s_segment *head;
} s_snek;

typedef struct s_food
{
	int pos_x;
	int pos_y;
	char c;
} s_food;

void delay(int s);
void gotoxy(int x, int y);

char *make_board(struct s_board b);
void print_board(char *bp, struct s_board b);

char get_dir(struct s_snek s);
struct s_segment newseg(struct s_segment *prevseg);

int game(int game_width, int game_height, int startlength);


#endif
