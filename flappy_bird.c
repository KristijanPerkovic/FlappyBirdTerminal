#define _CRT_SECURE_NO_WARNINGS
#define MAX_LEN 31
#define ROWS 15
#define PIPE_GAP 4
#define PIPE_SPACING 11
#define GRAVITY_SPEED 2.5
#define BIRD_POSITION MAX_LEN / 5
#define MAX_PIPES MAX_LEN/PIPE_SPACING + 1
#define GAME_SPEED 100
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <windows.h>
#include <conio.h>

int gravity_timer = 0;
float gravity_speed = GRAVITY_SPEED;
const float gravity_acceleration = 0.75;

int global_pipe_index = 0;
int top_pipe_length = 0;
int bottom_pipe_length = 0;

int score = 0;

int animation_frame_timer = 0;
const int animation_frame_time = 3;
int animation_frame = 0;

char animation_frames[5] = "@*@o";

bool jump = false, falling = false;

typedef struct _pipe
{
	int top_length;
	int bottom_length;
	int position;
} Pipe;

Pipe pipes[MAX_PIPES];
int num_pipes = 0;

typedef struct _row* position;
typedef struct _row
{
	char row[MAX_LEN];
	int index;
	position next;
}Row;

int create_row(position head, char* row, int index);

int draw_screen(position head);

int move_bird(position head);

int gravity(position head);

int create_pipe(position head);

bool collision(position head);

bool death(position head);

bool bird_visible(position head);

int restart_game(position head);

int get_score(position head);

int animation();

int free_memory(position head);

int main()
{
	srand(time(NULL));
	bool game_over = false, restart = true, bird_is_visible = true;
	Row head = { .next = NULL, .row = "", .index = 0 };
	char row[MAX_LEN] = "";
	for (int i = 0; i < MAX_LEN - 1; i++) strcat(row, ".");
	for (int i = 0; i < ROWS; i++) create_row(&head, row, i + 1);

	do
	{
		while (!_kbhit() || _getch() != ' ')
		{
			animation();
			draw_screen(&head);
			Sleep(GAME_SPEED);
			printf("SCORE: %d\n", score);
		}
		do
		{
			animation();
			draw_screen(&head);
			if (_kbhit() && _getch() == ' ') move_bird(&head);
			if (!jump) gravity(&head);
			create_pipe(&head);
			get_score(&head);
			game_over = collision(&head);
			bird_is_visible = bird_visible(&head);
			if (!game_over) game_over = death(&head);
			Sleep(GAME_SPEED);
			jump = false;
			printf("SCORE: %d\n", score);
		} while (!game_over && bird_is_visible);

		falling = true;

		while (falling && bird_is_visible)
		{
			draw_screen(&head);
			gravity(&head);
			Sleep(GAME_SPEED);
			printf("SCORE: %d\n", score);
		}

		printf("Restart (Y/N)?\n");
		char input;

		while (true)
		{
			input = _getch();

			if (input == 'y' || input == 'Y')
			{
				restart = true;
				break;
			}
			else if (input == 'n' || input == 'N')
			{
				restart = false;
				break;
			}
		}

		if (restart)
		{
			game_over = false;
			restart_game(&head);
		}

	} while (restart);

	free_memory(&head);

	return EXIT_SUCCESS;
}

int create_row(position head, char* row, int index)
{
	position new = malloc(sizeof(Row));
	if (!new)
	{
		printf("Malloc error in create_row!\n");
		return EXIT_FAILURE;
	}

	position temp = head;
	while (temp->next) temp = temp->next;

	strcpy(new->row, row);
	new->next = temp->next;
	new->index = index;
	temp->next = new;

	if (index == ROWS / 2 + 1) new->row[BIRD_POSITION] = '@';

	return EXIT_SUCCESS;
}

int draw_screen(position head)
{
	system("cls");
	printf("\n\n\n\n\n\n\n");
	position temp = head->next;
	while (temp)
	{
		if (strchr(temp->row, '@') || strchr(temp->row, '*') || strchr(temp->row, 'o'))
			temp->row[BIRD_POSITION] = animation_frames[animation_frame];
		printf("%s\n", temp->row);
		temp = temp->next;
	}
	for (int i = 0; i < MAX_LEN - 1; i++) printf("~");
	printf("\n\n\n\n\n\n\n");

	return EXIT_SUCCESS;
}

int move_bird(position head)
{
	position prev = head;
	position temp = head->next;
	while (temp)
	{
		if ((strchr(temp->row, '@') || strchr(temp->row, '*') || strchr(temp->row, 'o')) && prev != head)
		{
			temp->row[BIRD_POSITION] = '.';
			prev->row[BIRD_POSITION] = animation_frames[animation_frame];
			jump = true;
			gravity_timer = 0;
			gravity_speed = GRAVITY_SPEED;
			break;
		}
		prev = temp;
		temp = temp->next;
	}
}

int gravity(position head)
{

	gravity_timer++;

	if (gravity_timer < gravity_speed) return EXIT_SUCCESS;

	gravity_timer = 0;

	if (gravity_speed > GRAVITY_SPEED / 3)
		gravity_speed -= gravity_acceleration;

	position temp = head->next;
	while (temp)
	{
		if (strchr(temp->row, '@') || strchr(temp->row, '*') || strchr(temp->row, 'o'))
		{
			if (temp->next && temp->next->row[BIRD_POSITION] != '#')
			{
				temp->row[BIRD_POSITION] = '.';
				temp->next->row[BIRD_POSITION] = animation_frames[animation_frame];
			}
			else if (!temp->next || temp->next->row[BIRD_POSITION] == '#') falling = false;
			break;
		}
		temp = temp->next;
	}

	return EXIT_SUCCESS;
}

int create_pipe(position head)
{

	for (int i = 0; i < num_pipes; i++) pipes[i].position++;

	int write_index = 0;
	for (int i = 0; i < num_pipes; i++)
		if (pipes[i].position <= MAX_LEN - 1)
			pipes[write_index++] = pipes[i];

	num_pipes = write_index;

	if (global_pipe_index % PIPE_SPACING == 0)
	{
		if (num_pipes < MAX_PIPES)
		{
			top_pipe_length = rand() % (ROWS - PIPE_GAP - 1) + 1;
			bottom_pipe_length = ROWS - PIPE_GAP - top_pipe_length;
			pipes[num_pipes].top_length = top_pipe_length;
			pipes[num_pipes].bottom_length = bottom_pipe_length;
			pipes[num_pipes].position = 0;
			num_pipes++;
		}
		global_pipe_index = 0;
	}

	global_pipe_index++;

	position temp = head->next;

	while (temp)
	{
		for (int i = 0; i < num_pipes; i++)
		{
			if ((temp->index >= 1 && temp->index <= pipes[i].top_length) ||
				(temp->index > ROWS - pipes[i].bottom_length && temp->index <= ROWS))
			{
				if (pipes[i].position > 0)
					temp->row[MAX_LEN - pipes[i].position - 1] = '.';

				if (pipes[i].position < MAX_LEN - 1)
					temp->row[MAX_LEN - pipes[i].position - 2] = '#';
			}
		}

		temp = temp->next;
	}

	return EXIT_SUCCESS;
}

bool collision(position head)
{
	position temp = head->next;
	while (temp)
	{
		if (strchr(temp->row, '@') || strchr(temp->row, '*') || strchr(temp->row, 'o'))
		{
			if (temp->row[BIRD_POSITION + 1] == '#') return true;

			if (jump)
			{
				position above = head;
				while (above->next && above->next != temp)
					above = above->next;

				if (above != head && above->row[BIRD_POSITION] == '#') return true;
			}
			else if (!jump)
			{
				position below = temp->next;
				while (below && below->index < temp->index + (int)gravity_speed)
					below = below->next;

				if (below && below->row[BIRD_POSITION] == '#') return true;
			}
		}

		temp = temp->next;
	}

	return false;
}

bool death(position head)
{
	position temp = head;
	while (temp->next) temp = temp->next;
	if (strchr(temp->row, animation_frames[animation_frame])) return true;
	return false;
}

bool bird_visible(position head)
{
	position temp = head->next;
	while (temp)
	{
		if (strchr(temp->row, '@') || strchr(temp->row, '*') || strchr(temp->row, 'o')) return true;
		temp = temp->next;
	}
	return false;
}

int get_score(position head)
{
	position temp = head->next->next;
	while (temp->next->next)
	{
		if ((strchr(temp->row, '@') || strchr(temp->row, '*') || strchr(temp->row, 'o')) && head->next->row[BIRD_POSITION] == '#')
		{
			score++;
			Beep(1000, 50);
		}
		temp = temp->next;
	}

	return EXIT_SUCCESS;
}

int restart_game(position head)
{
	gravity_timer = 0;
	gravity_speed = GRAVITY_SPEED;

	global_pipe_index = 0;
	top_pipe_length = 0;
	bottom_pipe_length = 0;
	num_pipes = 0;

	animation_frame_timer = 0;
	animation_frame = 0;

	score = 0;

	for (int i = 0; i < MAX_PIPES; i++)
	{
		pipes[i].top_length = 0;
		pipes[i].bottom_length = 0;
		pipes[i].position = 0;
	}

	position temp = head->next;
	while (temp)
	{
		for (int i = 0; i < MAX_LEN - 1; i++) if (temp->row[i] == '#') temp->row[i] = '.';

		if (temp->index == ROWS / 2 + 1) temp->row[BIRD_POSITION] = animation_frames[animation_frame];

		else if (temp->index != ROWS / 2 + 1 && (strchr(temp->row, '@') || strchr(temp->row, '*') || strchr(temp->row, 'o'))) temp->row[BIRD_POSITION] = '.';

		temp = temp->next;
	}

	return EXIT_SUCCESS;
}

int animation()
{
	animation_frame_timer++;

	if (animation_frame_timer < animation_frame_time) return EXIT_FAILURE;

	animation_frame_timer = 0;

	animation_frame = (animation_frame + 1) % 4;

	return EXIT_SUCCESS;
}

int free_memory(position head)
{
	position temp = NULL;
	while (head->next)
	{
		temp = head->next;
		head->next = head->next->next;
		free(temp);
	}

	return EXIT_SUCCESS;
}