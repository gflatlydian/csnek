#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <curses.h>

#include "snek.h"

void delay(int ms)
{
	clock_t start_time = clock();
	while (clock() < start_time + ms);
}

void gotoxy(int x, int y)
{
	printf("%c[%d;%df", 0x1B, y + 1, x + 1);
}

char *make_board(struct s_board b)
{
	int boardsize = (b.w + 2) * (b.h + 2);

	// create the array that will contain the board
	// the function returns this array so it can be used by print_board()
	char *boardptr;
	boardptr = (char *) malloc (boardsize * sizeof(char));

	// populate play area and border with ' ' and '#' respectively
	for (int y = 0; y < b.h + 2; y++)
	{
		for (int x = 0; x < b.w + 2; x++)
		{
			//determine whether border or play area, place the right char
			if (x == 0 || y == 0 || x == b.w + 1 || y == b.h + 1)
			{
				*(boardptr + ((b.w + 2) * y + x)) = 0x23;
			}
			else
			{
				*(boardptr + ((b.w + 2) * y + x)) = 0x20;
			}
		}
	}

	return boardptr;
}

void print_board(char *bp, struct s_board b)
{
	// print the play area to the screen
	for (int y = 0; y < b.h + 2; y++)
	{
		for (int x = 0; x < b.w + 2; x++)
		{
			gotoxy(x, y);
			printf("%c", *(bp + ((b.w + 2) * y + x)));
		}
	}
}

char get_dir(struct s_snek s)
{
	switch (s.head->dir)
	{
	case 'n':
		return '^';
		break;
	case 'e':
		return '>';
		break;
	case 'w':
		return '<';
		break;
	case 's':
		return 'v';
		break;
	default:
		return '@';
		break;
	}
}

struct s_segment newseg(struct s_segment *prevseg)
{
	struct s_segment nextseg;
	nextseg.last = prevseg;
	nextseg.c = 'O';
	nextseg.dir = prevseg->dir;

	switch (nextseg.dir)
	{
	case 'n':
		nextseg.pos_x = prevseg->pos_x;
		nextseg.pos_y = prevseg->pos_y + 1;
		break;
	case 'e':
		nextseg.pos_x = prevseg->pos_x - 1;
		nextseg.pos_y = prevseg->pos_y;
		break;
	case 'w':
		nextseg.pos_x = prevseg->pos_x + 1;
		nextseg.pos_y = prevseg->pos_y;
		break;
	case 's':
		nextseg.pos_x = prevseg->pos_x;
		nextseg.pos_y = prevseg->pos_y - 1;
		break;
	}

	return nextseg;
}


// the main game set up and loop
int game(int game_width, int game_height, int startlength)
{
	int score;

	struct s_board theboard;
	theboard.w = game_width;
	theboard.h = game_height;

	struct s_segment thehead;

	srand(time(NULL)); //seek rand();

	//initialise a food
	struct s_food food;
	food.c = '@';
	food.pos_x = rand() % (theboard.w - 2);
	food.pos_y = rand() % (theboard.h - 2);

	//initialise the snake
	struct s_snek thesnake;
	thesnake.alive = true;
	thesnake.length = startlength;
	thesnake.head = &thehead;
	thesnake.head->pos_x = theboard.w / 2;
	thesnake.head->pos_y = theboard.h / 2;
	int int_dir;					
	int_dir = rand() % 4;		//start the snake facing a random direction
	switch (int_dir)
	{
		case 0:
			thesnake.head->dir = 'n'; // (n)orth, (e)ast, (s)outh, (w)est
			break;
		case 1:
			thesnake.head->dir = 'e'; // (n)orth, (e)ast, (s)outh, (w)est
			break;
		case 2:
			thesnake.head->dir = 's'; // (n)orth, (e)ast, (s)outh, (w)est
			break;
		case 3:
			thesnake.head->dir = 'w'; // (n)orth, (e)ast, (s)outh, (w)est
			break;
		default:
			return(-2);
			break;
	}
	thesnake.head->c = get_dir(thesnake);

	struct s_segment snake_seg[(theboard.w - 2) * (theboard.h - 2)]; //allocate enough memory for the snake to grow

	//make the head
	snake_seg[0] = *thesnake.head;

	//make the tail
	for (int l = 1; l < thesnake.length; l++)
	{
		snake_seg[l] = newseg(&snake_seg[l - 1]);
	}

	while (thesnake.alive == true)
	{
		//prepare the board
		char *boardarray;
		boardarray = make_board(theboard);

		snake_seg[0].c = get_dir(thesnake);
		for (int l = thesnake.length - 1; l > 0; l--)
		{
			snake_seg[l].c = 'O';
		}

		//put snake on board
		for (int l = 0; l < thesnake.length; l++)
		{
			int x, y;
			x = snake_seg[l].pos_x;
			y = snake_seg[l].pos_y;
			*(boardarray + ((theboard.w + 2) * y + x)) = snake_seg[l].c;
		}

		//put food on board
		*(boardarray + ((theboard.w + 2) * (food.pos_y + 1) + (food.pos_x + 1))) = food.c;

		// print the board
		if (thesnake.alive == true)
		{
			print_board(boardarray, theboard);
			score = thesnake.length - startlength;
			gotoxy(0, theboard.h + 1);
			printf("\nScore: %d\n", score);
		}

		//if you hit a food, increment score and relocate food
		if (snake_seg[0].pos_x == (food.pos_x + 1) && 
			snake_seg[0].pos_y == (food.pos_y + 1)) 
		{
			thesnake.length++;
			food.pos_x = rand() % (game_width - 2);
			food.pos_y = rand() % (game_height - 2);
			snake_seg[thesnake.length] = newseg(&snake_seg[thesnake.length - 1]);
		}

		//if you've hit the wall, game over
		if (snake_seg[0].pos_x == 1 ||
			snake_seg[0].pos_y == 1 ||
			snake_seg[0].pos_x == theboard.w ||
			snake_seg[0].pos_y == theboard.h)
		{
			thesnake.alive = false;
		}

		//if you hit the tail, game over
		for (int i = 1; i < thesnake.length; i++)
		{
			if (snake_seg[i].pos_x == snake_seg[0].pos_x &&
				snake_seg[i].pos_y == snake_seg[0].pos_y) 
			{
				thesnake.alive = false;	
			}
		}

		int user_input = getch();
		switch (user_input)
		{
		case '\e':
			return(-1); //TODO: implement a pause menu
			break;
		case KEY_UP:
			if (thesnake.head->dir == 's') {
				break;
			}
			thesnake.head->dir = 'n';
			break;
		case KEY_RIGHT:
			if (thesnake.head->dir == 'w') {
				break;
			}
			thesnake.head->dir = 'e';
			break;
		case KEY_LEFT:
			if (thesnake.head->dir == 'e') {
				break;
			}
			thesnake.head->dir = 'w';
			break;
		case KEY_DOWN:
			if (thesnake.head->dir == 'n') {
				break;
			}
			thesnake.head->dir = 's';
			break;
		}

		struct s_segment temp_seg_1;
		struct s_segment temp_seg_2;
		temp_seg_1 = snake_seg[0];

		//move the head
		snake_seg[0].dir = thesnake.head->dir;
		snake_seg[0].c = get_dir(thesnake);
		

		switch (snake_seg[0].dir)
		{
		case 'n':
			snake_seg[0].pos_y--;
			break;
		case 'e':
			snake_seg[0].pos_x++;
			break;
		case 'w':
			snake_seg[0].pos_x--;
			break;
		case 's':
			snake_seg[0].pos_y++;
			break;
		}

		//move the tail bits
		for (int l = 1; l < thesnake.length; l++)
		{
			temp_seg_2 = snake_seg[l];
			snake_seg[l].pos_x = temp_seg_1.pos_x;
			snake_seg[l].pos_y = temp_seg_1.pos_y;
			snake_seg[l].dir = temp_seg_1.dir;
			temp_seg_1 = temp_seg_2;
		}

		delay(75000);
	}

	return(score);
}

// put it all together
int main(int argc, char *argv[])
{

	initscr();					//start curses mode
	cbreak();					//enter raw tty mode, but allow interrupts and control keys
	keypad(stdscr, TRUE);		//enable reading of various keys, incl arrows, f-keys 
	noecho();					//disable echoing
	curs_set(0);				//disable the cursor
	timeout(0);

	unsigned int w, h, l;

	if (argv[1]!=NULL && argv[2]!=NULL) 
	{
		w=atoi(argv[1]);
		h=atoi(argv[2]);
	} 
	else 
	{
		w=60;
		h=40;
	}
	
	if (argv[3]!=NULL)
	{
		l=atoi(argv[3]);
	}
	else
	{
		l=6;
	}

	int game_end;
	game_end = game(w, h, l);
	gotoxy(w,h+2);
	timeout(-1);
	if (game_end != -1) {
		printw("Game Over!\n");
		getch();
		curs_set(1);
		endwin();
		printf("Final score: %d\n", game_end);
	} else {
		curs_set(1);
		endwin();
		printf("You quit, or something went wrong!\n");
		//printf("Final score: %d\n", game_end);
	}

	return 0;
}

