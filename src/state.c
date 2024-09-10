#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_state_t *state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t *state, unsigned int snum);
static char next_square(game_state_t *state, unsigned int snum);
static void update_tail(game_state_t *state, unsigned int snum);
static void update_head(game_state_t *state, unsigned int snum);

/* Task 1 */
game_state_t *create_default_state() {
  // TODO: Implement this function.
  game_state_t* state = malloc(sizeof(game_state_t));
  state->num_rows = 18;
  state->num_snakes = 1;

  // initialize snakes
  state->snakes = malloc(sizeof(snake_t));
  state->snakes[0].head_row = 2;
  state->snakes[0].head_col = 4;
  state->snakes[0].tail_row = 2;
  state->snakes[0].tail_col = 2;
  state->snakes[0].live = true;

  //initialize board
  state->board = malloc(18 * sizeof(char*));
  for (int i = 0;  i < 18; i++){
    state->board[i] = malloc(22 * sizeof(char));
    for (int j = 0; j < 20; j++){
      state->board[i][j] = ' ';
    }
    state->board[i][20] = '\n';
    state->board[i][21] = '\0';
  }
  for (int i = 0;  i < 20; i++){
    state->board[0][i] = '#';
    state->board[17][i] = '#';
  }
  for (int i = 1;  i < 18; i++){
    state->board[i][0] = '#';
    state->board[i][19] = '#';
  }

  //snake and food on board
  state->board[2][2] = 'd';
  state->board[2][3] = '>';
  state->board[2][4] = 'D';
  state->board[2][9] = '*';

  return state;
}

/* Task 2 */
void free_state(game_state_t *state) {
  // TODO: Implement this function.
  for (int i = 0;  i < state->num_rows; i++){
      free(state->board[i]);
  }
  free(state->board);
  free(state->snakes);
  free(state);
  return;
}

/* Task 3 */
void print_board(game_state_t *state, FILE *fp) {
  // TODO: Implement this function.
  for (int i = 0;  i < state->num_rows; i++){
    fprintf(fp, "%s", state->board[i]);
  }
  return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t *state, char *filename) {
  FILE *f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t *state, unsigned int row, unsigned int col) { return state->board[row][col]; }

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t *state, unsigned int row, unsigned int col, char ch) {
  state->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  // TODO: Implement this function.
  return c=='w' || c=='a' || c=='s' || c=='d';
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  // TODO: Implement this function.
  return c=='W' || c=='A' || c=='S' || c=='D' || c=='x';
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
  // TODO: Implement this function.
  return is_tail(c) || is_head(c) || c=='^' || c=='<' || c=='v' || c=='>';
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  // TODO: Implement this function.
  switch (c){
    case '^': return 'w';
    case 'v': return 's';
    case '<': return 'a';
    case '>': return 'd';
  }
  return '?';
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
  // TODO: Implement this function.
  switch(c){
    case 'W': return '^';
    case 'S': return 'v';
    case 'D': return '>';
    case 'A': return '<';
  }
  return '?';
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
  // TODO: Implement this function.
  if (c=='v' || c=='s' || c=='S'){
    return cur_row + 1;
  }else if (c=='^' || c=='w' || c=='W'){
    return cur_row - 1;
  }
  return cur_row;
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
  // TODO: Implement this function.
  if (c=='>' || c=='d' || c=='D'){
    return cur_col + 1;
  }else if (c=='<' || c=='a' || c=='A'){
    return cur_col - 1;
  }
  return cur_col;
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  unsigned int row = get_next_row(state->snakes[snum].head_row, state->board[state->snakes[snum].head_row][state->snakes[snum].head_col]);
  unsigned int col = get_next_col(state->snakes[snum].head_col, state->board[state->snakes[snum].head_row][state->snakes[snum].head_col]);
  char c = get_board_at(state, row, col);
  return c;
}

/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  snake_t* snake = &state->snakes[snum];
  char current_head = get_board_at(state, snake->head_row, snake->head_col);
  char new_body = head_to_body(current_head);
  set_board_at(state, snake->head_row, snake->head_col, new_body);
  unsigned int new_row = get_next_row(snake->head_row, current_head);
  unsigned int new_col = get_next_col(snake->head_col, current_head);
  
  //update new snake head
  snake->head_row = new_row;
  snake->head_col = new_col;
  set_board_at(state, new_row, new_col, current_head);

  return;
}

/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  return;
}

/* Task 4.5 */
void update_state(game_state_t *state, int (*add_food)(game_state_t *state)) {
  // TODO: Implement this function.
  return;
}

/* Task 5.1 */
char *read_line(FILE *fp) {
  // TODO: Implement this function.
  return NULL;
}

/* Task 5.2 */
game_state_t *load_board(FILE *fp) {
  // TODO: Implement this function.
  return NULL;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_state_t *state, unsigned int snum) {
  // TODO: Implement this function.
  return;
}

/* Task 6.2 */
game_state_t *initialize_snakes(game_state_t *state) {
  // TODO: Implement this function.
  return NULL;
}
