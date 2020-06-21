#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <termios.h>
#include <string.h>

#include "snek.h"

void delay(int ms)
{
	clock_t start_time = clock();
	while (clock() < start_time + ms)
		;
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
	system("clear");
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

struct s_segment newseg(struct s_segment *prevseg, struct s_snek s)
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

int read_input(void)
{
	int character;
	struct termios orig_term_attr;
	struct termios new_term_attr;

	/* set the terminal to raw mode */
	tcgetattr(fileno(stdin), &orig_term_attr);
	memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
	new_term_attr.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL);
	new_term_attr.c_cc[VTIME] = 0;
	new_term_attr.c_cc[VMIN] = 0;
	tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

	/* read a character from the stdin stream without blocking */
	/*   returns EOF (-1) if no character is available */
	character = fgetc(stdin);

	/* restore the original terminal attributes */
	tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

	return character;
}




void game(void)
{
	int startlength;
	int score;
	startlength = 5;

	struct s_board theboard;
	theboard.w = 60;
	theboard.h = 30;

	struct s_segment thehead;

	//initialise the snake
	struct s_snek thesnake;
	thesnake.alive = true;
	thesnake.length = startlength;
	thesnake.head = &thehead;
	thesnake.head->pos_x = theboard.w / 2;
	thesnake.head->pos_y = theboard.h / 2;
	thesnake.head->dir = 'e'; // (n)orth, (e)ast, (s)outh, (w)est
	thesnake.head->c = get_dir(thesnake);

	struct s_segment snake_seg[thesnake.length];

	//make the head
	snake_seg[0] = *thesnake.head;

	//make the tail
	for (int l = 1; l < thesnake.length; l++)
	{
		snake_seg[l] = newseg(&snake_seg[l - 1], thesnake);
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

		// print the board
		if (thesnake.alive == true)
		{
			print_board(boardarray, theboard);
			score = thesnake.length - startlength;
			gotoxy(0, theboard.h + 1);
			printf("\nScore: %d\n", score);
		}

		//if you've hit the wall, game over
		if (snake_seg[0].pos_x == 1 ||
			snake_seg[0].pos_y == 1 ||
			snake_seg[0].pos_x == theboard.w ||
			snake_seg[0].pos_y == theboard.h)
		{
			thesnake.alive = false;
		}

		int user_input = read_input();
		switch (user_input)
		{
		case 'w':
			thesnake.head->dir = 'n';
			break;
		case 'd':
			thesnake.head->dir = 'e';
			break;
		case 'a':
			thesnake.head->dir = 'w';
			break;
		case 's':
			thesnake.head->dir = 's';
			break;
		case EOF:
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

		delay(50000);
	}
}

int main(int argc, char *argv[])
{
	system("clear");
	system("stty -echo");
	system("/bin/stty raw");

	game();

	system("/bin/stty cooked");
	system("stty echo");

	printf("\nGame Over!\n");

	return 0;
}