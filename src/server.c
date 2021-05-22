#include "board.h"
#include "graph.h"
#include "move.h"
#include "opt.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern char* player_1_path;
extern char* player_2_path;
extern int board_size;

bool game_over = false;
size_t position_player_1 = -1;
size_t position_player_2 = -1;
enum color_t active_player = -1;
enum color_t winner = -1;
size_t turn = 0;


enum reasons_t { WIN = 0, INVALID_MOVE = 1 };

void* P1_lib;
void (*P1_initialize)(enum color_t id, struct graph_t* graph, size_t num_walls);
char* (*P1_name)(void);
struct move_t(*P1_play)(struct move_t previous_move);
void (*P1_finalize)();

void* P2_lib;
void (*P2_initialize)(enum color_t id, struct graph_t* graph, size_t num_walls);
char* (*P2_name)(void);
struct move_t(*P2_play)(struct move_t previous_move);
void (*P2_finalize)();

/**
 * @brief Load players' dynamics librairies
 *
 * @details Load the players' dynamics libraries and stores the adresses
 *  of the symbols declared in the client interface into variables
 */
void load_libs(void) {
	P1_lib = dlopen(player_1_path, RTLD_LAZY);
	char* error = dlerror();

	if (error != NULL) {
		fprintf(stderr, "%s\n", error);
		exit(EXIT_FAILURE);
	}

	if (P1_lib == NULL) {
		printf("Path to first player's library is unreachable.\n");
		exit(EXIT_FAILURE);
	}

	P1_initialize = dlsym(P1_lib, "initialize");
	P1_name = dlsym(P1_lib, "get_player_name");
	P1_play = dlsym(P1_lib, "play");
	P1_finalize = dlsym(P1_lib, "finalize");

	P2_lib = dlopen(player_2_path, RTLD_LAZY);
	error = dlerror();

	if (error != NULL) {
		fprintf(stderr, "%s\n", error);
		exit(EXIT_FAILURE);
	}

	if (P2_lib == NULL) {
		printf("Path to second player's library is unreachable.\n");
		exit(EXIT_FAILURE);
	}

	P2_initialize = dlsym(P2_lib, "initialize");
	P2_name = dlsym(P2_lib, "get_player_name");
	P2_play = dlsym(P2_lib, "play");
	P2_finalize = dlsym(P2_lib, "finalize");
}

/**
 * @brief Tell if a player is winning
 *
 * @details Check if the player is winning by checking
 * if he is on a vertex owned by the other player
 *
 * @param board The game board
 * @param active_player The player to check
 * @param position The position of the current player
 *
 * @return True if the player is winning, else false
 */
bool is_winning(struct graph_t* board, enum color_t active_player, size_t position) {
	return gsl_spmatrix_uint_get(board->o, 1 - active_player, position);
}

/**
 * @brief Compute the next player
 *
 * @param player Current player
 */
enum color_t get_next_player(enum color_t player) { return 1 - player; }


void end_game(enum reasons_t reason) {
	winner = reason == WIN ? active_player : get_next_player(active_player);
	game_over = true;
}

/**
 * @brief Check the validity of a displacement
 *
 * @param board The game board
 * @param destination PLayer destination
 * @param player Current player
 *
 * @returns True if the displacement is valid, else false
 */
bool is_valid_displacement(struct graph_t* board, size_t destination, enum color_t player) {

	size_t position_player = player == BLACK ? position_player_1 : position_player_2;

	size_t position_opposent = player == BLACK ? position_player_2 : position_player_1;
	// Destination is in the board and destination is not on a player cell
	if (board->num_vertices <= destination || position_player == destination || position_opposent == destination) {
		return false;
	}

	// Check first move
	if (position_player == (long unsigned int) - 1) {
		if (player == BLACK) {
			return destination < (long unsigned int) board_size;
		}
		return (board->num_vertices - board_size <= destination) && (destination < board->num_vertices);
	}

	// Check destination is in the 4 near cells :
	if (is_linked(board, position_player, destination)) {
		return true;
	}

	// Check others destination manually
	// Move 2 on the left
	if (position_player % board_size > 1 &&
		position_player - destination == 2 &&
		is_linked(board, position_player, position_player - 1) &&
		is_linked(board, position_player - 1, destination) &&
		position_opposent == position_player - 1) {
		return true;
	}

	// Move 2 on the right
	if (position_player % board_size < (long unsigned int) board_size - 2 &&
		destination - position_player == 2 &&
		is_linked(board, position_player, position_player + 1) &&
		is_linked(board, position_player + 1, destination) &&
		position_opposent == position_player + 1) {
		return true;
	}

	// Move 2 on the top
	if (position_player / board_size > 1 &&
		destination + 2 * board_size == position_player &&
		is_linked(board, position_player, position_player - board_size) &&
		is_linked(board, position_player - board_size, destination) &&
		position_opposent == position_player - board_size) {
		return true;
	}

	// Move 2 on the bottom
	if (position_player / board_size < (long unsigned int) board_size - 2 &&
		position_player + 2 * board_size == destination &&
		is_linked(board, position_player, position_player + board_size) &&
		is_linked(board, position_player + board_size, destination) &&
		position_opposent == position_player + board_size) {
		return true;
	}

	// Move top + left
	if (position_player / board_size > 0 &&
		position_player % board_size > 0 &&
		position_player - board_size == destination + 1 &&
		(
			(is_linked(board, position_player, position_player - board_size) &&
				is_linked(board, position_player - board_size, destination) &&
				position_opposent == position_player - board_size
				)
			||
			(is_linked(board, position_player, position_player - 1) &&
				is_linked(board, position_player - 1, destination) &&
				position_opposent == position_player - 1)
			)
		) {
		return true;
	}

	// Move top + right
	if (position_player / board_size > 0 &&
		position_player % board_size < (long unsigned int) board_size - 1 &&
		position_player - board_size == destination - 1 &&
		(
			(is_linked(board, position_player, position_player - board_size) &&
				is_linked(board, position_player - board_size, destination) &&
				position_opposent == position_player - board_size
				)
			||
			(is_linked(board, position_player, position_player + 1) &&
				is_linked(board, position_player + 1, destination) &&
				position_opposent == position_player + 1)
			)
		) {
		return true;
	}

	// Move bot + right
	if (position_player / board_size < (long unsigned int) board_size - 1 &&
		position_player % board_size < (long unsigned int) board_size - 1 &&
		position_player + board_size == destination - 1 &&
		(
			(is_linked(board, position_player, position_player + board_size) &&
				is_linked(board, position_player + board_size, destination) &&
				position_opposent == position_player + board_size
				)
			||
			(is_linked(board, position_player, position_player + 1) &&
				is_linked(board, position_player + 1, destination) &&
				position_opposent == position_player + 1)
			)
		) {
		return true;
	}

	// Move bot + left
	if (position_player / board_size < (long unsigned int) board_size - 1 &&
		position_player % board_size > 0 &&
		position_player + board_size == destination + 1 &&
		(
			(is_linked(board, position_player, position_player + board_size) &&
				is_linked(board, position_player + board_size, destination) &&
				position_opposent == position_player + board_size
				)
			||
			(is_linked(board, position_player, position_player - 1) &&
				is_linked(board, position_player - 1, destination) &&
				position_opposent == position_player - 1)
			)
		) {
		return true;
	}
	return false;
}

/**
 * @brief Check the validity of a wall placement
 * 
 * @details Check if the wall :
 * - is in the board
 * - do not cut a existing wall
 * - do not prevents a player to reach the arrival
 * 
 * @param board The game board
 * @param e An array containing two edges representing a wall
 * 
 * @return True if the wall is valid, else False
 */
bool is_valid_wall(struct graph_t* board, struct edge_t e[]) {

	// Sort verticies
	size_t e0fr_tmp = e[0].fr < e[0].to ? e[0].fr : e[0].to;
	size_t e0to_tmp = e[0].fr > e[0].to ? e[0].fr : e[0].to;
	size_t e1fr_tmp = e[1].fr < e[1].to ? e[1].fr : e[1].to;
	size_t e1to_tmp = e[1].fr > e[1].to ? e[1].fr : e[1].to;

	size_t e0fr = e0fr_tmp < e1fr_tmp ? e0fr_tmp : e1fr_tmp;
	size_t e1fr = e0fr_tmp > e1fr_tmp ? e0fr_tmp : e1fr_tmp;
	size_t e0to = e0to_tmp < e1to_tmp ? e0to_tmp : e1to_tmp;
	size_t e1to = e0to_tmp > e1to_tmp ? e0to_tmp : e1to_tmp;

	// Check if the wall cut valid edges
	if (!(is_linked(board, e0fr, e0to) && is_linked(board, e1fr, e1to))) {
		return false;
	}

	// Check if vertices form a valid square
	if ((e0fr % board_size) + 1 == e0to % board_size && (e1fr % board_size) + 1 == e1to % board_size && e0fr + board_size == e1fr) { // vertical wall
		// Check if the wall cut another wall
		size_t e2 = gsl_spmatrix_uint_get(board->t, e0fr, e1fr);
		if (e2 == 7) {
			return false;
		}
	}
	else if (e0fr + board_size == e0to && e1fr + board_size == e1to && (e0fr % board_size) + 1 == e1fr % board_size) { // horizontal wall
		// Check if the wall cut another wall
		size_t e2 = gsl_spmatrix_uint_get(board->t, e0fr, e1fr);
		if (e2 == 5) {
			return false;
		}
	}
	else {
		return false;
	}

	// Ckeck if nobody is locked in
	struct edge_t edge[2];
	edge[0].fr = e0fr;
	edge[0].to = e0to;
	edge[1].fr = e1fr;
	edge[1].to = e1to;
	place_wall(board, edge);
	if (dijkstra(board, position_player_1, BLACK) == IMPOSSIBLE_DISTANCE || dijkstra(board, position_player_2, WHITE) == IMPOSSIBLE_DISTANCE) {
		remove_wall(board, edge);
		return false;
	}
	remove_wall(board, edge);
	return true;
}

/**
 * @brief Check the validity of a move
 * 
 * @details Check if the move :
 * - has a valid type
 * - has a valid color id
 * - if it is a displacement, check if it respects the displacement rules, i.e only by one vertex
 * - if it is a wall placement, check if the player has walls remaining and if the wall is valid
 * 
 * @param mv A move
 * @param board The game board
 * @param player The current player
 * 
 * @return True if the move is valid, else False
 */
bool move_is_valid(struct move_t* mv, struct graph_t* board, enum color_t player) {

	// Check type
	if (!(mv->t == WALL || mv->t == MOVE)) {
		fprintf(stderr, "Error from %s: incorrect move type (%u)\n", player == BLACK ? P1_name() : P2_name(), mv->t);
		end_game(INVALID_MOVE);
		return false;
	}

	// Check color
	if (mv->c != player) {
		fprintf(stderr, "Error from %s: incorrect color (%u)\n", player == BLACK ? P1_name() : P2_name(), mv->c);
		end_game(INVALID_MOVE);
		return false;
	}

	// Check if move respect rules
	if (mv->t == MOVE) {
		if (!is_valid_displacement(board, mv->m, player)) {
			fprintf(stderr, "Error from %s: bad displacement %zu --> %zu\n", player == BLACK ? P1_name() : P2_name(), player == BLACK ? position_player_1 : position_player_2, mv->m);
			end_game(INVALID_MOVE);
			return false;
		}
		return true;
	}

	// Check if edges create a wall
	if (mv->t == WALL) {
		if (!is_valid_wall(board, mv->e)) {
			fprintf(stderr, "Error from %s: bad wall (%zu, %zu), (%zu, %zu)\n", player == BLACK ? P1_name() : P2_name(), mv->e[0].fr, mv->e[0].to, mv->e[1].fr, mv->e[1].to);
			end_game(INVALID_MOVE);
			return false;
		}
	}

	// TODO : check valid moves
	return true;
}

/**
 * @brief Update the player board
 * 
 * @details Add the last move in the server graph
 * 
 * @param board The game board
 * @param last_move The last played move to add
 */
void update_board(struct graph_t* board, struct move_t* last_move) {

	// Update players positions if they move
	if (last_move->t == MOVE) {
		if (active_player == BLACK) {
			position_player_1 = last_move->m;
		}
		else {
			position_player_2 = last_move->m;
		}
	}
	else {
		// Update the board if a player has put a wall
		place_wall(board, last_move->e);
	}
}

/**
 * @brief Free all allocated memory during the game
 * 
 * @details Finalize the two players, free the board and then close the dynamic librairies
 * 
 * @param board The game board
 */
void close_server(struct graph_t* board) {
	P1_finalize();
	P2_finalize();
	graph_free(board);
	dlclose(P1_lib);
	dlclose(P2_lib);
}

/**
 * @brief Do a game
 * 
 * @details Compute a game by doing the following steps :
 * - Parse the command line arguments
 * - Load the players' librairies
 * - Initialize the board
 * - Do the game loop
 * - Finalize the game by freeing the allocated memory
 * 
 * @param argc Number of command line arguments
 * @param argv An array of strings containing the command line arguments
 * 
 * @returns The exit code of the game
 */
int play_game(int argc, char* argv[]) {

	// Parse arguments
	parse_args(argc, argv);

	// Initialize random generator
	time_t seed = time(NULL);
	srand(seed);
	printf("Seed: %ld\n", seed);

	// Load players
	load_libs();
	printf("Libs loaded\n");

	// Initialize a new board of size m and shape t
	size_t m = board_size;
	struct graph_t* board = graph_init(m, SQUARE);
	struct graph_t* boardCopy1 = graph_init(m, SQUARE);
	struct graph_t* boardCopy2 = graph_init(m, SQUARE);
	printf("Board created\n");

	int edges = 2 * board_size * (board_size - 1);
	int num_walls = ceil(edges / 15.0);

	// Initialize random starting player
	active_player = rand() % 2;

	// Initialize players
	P1_initialize(BLACK, boardCopy1, num_walls);
	P2_initialize(WHITE, boardCopy2, num_walls);
	printf("Players initialized\n");
	printf("\n");
	printf("%s vs %s\n", P1_name(), P2_name());
	printf("%s begins\n", active_player == BLACK ? P1_name() : P2_name());

	// Initialize the first move as a move to the initial place
	struct move_t last_move = (struct move_t){
			.m = SIZE_MAX,
			.c = active_player,
			.e = {no_edge(), no_edge()},
			.t = NO_TYPE
	};

	// Game loop
	while (!game_over) {
		turn++;
		// Plays the active player
		last_move = active_player == BLACK ? P1_play(last_move) : P2_play(last_move);

		// Check move validity
		if (!move_is_valid(&last_move, board, active_player)) {
			break;
		}

		update_board(board, &last_move);

		printf("\n\n%s:\n", active_player == BLACK ? P1_name() : P2_name());
		display_board(board, m, position_player_1, position_player_2);

		//printf("wall : %ld:%ld - %ld:%ld \n", last_move.e[0].fr,  last_move.e[0].to,  last_move.e[1].fr,  last_move.e[1].to);

		// Check if a player has won
		if (is_winning(board, active_player, active_player == BLACK ? position_player_1 : position_player_2)) {
			end_game(WIN);
			break;
		}

		active_player = get_next_player(active_player);
	}

	printf("GAME OVER\n");
	printf("%s won\n", winner == BLACK ? P1_name() : P2_name());
	printf("Finish after %zu turns\n", turn);

	close_server(board);

	return EXIT_SUCCESS;
}
