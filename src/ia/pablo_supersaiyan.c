/**
 * @file pablo_supersaiyan.c
 *
 * @brief Implementation of the player Pablo Supersaiyan. This player intelligence is based principally on the Dijkstra algorithm
 */


#include "ia.h"
#include "ia_utils.h"
#include "board.h"
#include "move.h"


#define MAX_POSSIBLE_WALLS 500
#define IMPOSSIBLE_ID 1234500
#define MAX_MOVE_PLACES 7

char *name = "Pablo Super Saiyan";



/**
 * @brief Gather all the emplacements that can receive a wall
 * @param walls A 2-dimension array that will be filled by walls
 * @param game To have necessary information on the graph
 * @returns The number of possible walls
 */ 
size_t get_possible_walls(struct game_state_t game, struct edge_t walls[MAX_POSSIBLE_WALLS][2]) {
	size_t nb_wall = 0;
	for (size_t i = 0; i < game.graph->num_vertices; i++) {
		// To verify if a wall can be put on the south of the vertex i and the vertex on right of it
			if (vertex_from_direction(game.graph, i, EAST) != no_vertex()) {
				if (vertex_from_direction(game.graph, i, SOUTH) != no_vertex() &&
					vertex_from_direction(game.graph, vertex_from_direction(game.graph, i, EAST), SOUTH) !=
					no_vertex()) {//Add a wall to the list if the place is free
					walls[nb_wall][0].fr = i;
					walls[nb_wall][0].to = vertex_from_direction(game.graph, i, SOUTH);
					walls[nb_wall][1].fr = vertex_from_direction(game.graph, i, EAST);
					walls[nb_wall][1].to = vertex_from_direction(game.graph, vertex_from_direction(game.graph, i, EAST), SOUTH);
					nb_wall++;
				}
			}
			// To verify if a wall can be put on the EAST of the vertex i and the vertex below
			if (vertex_from_direction(game.graph, i, SOUTH) != no_vertex()) {
				if (vertex_from_direction(game.graph, i, EAST) != no_vertex() &&
					vertex_from_direction(game.graph, vertex_from_direction(game.graph, i, SOUTH), EAST) !=
					no_vertex()) {//Add a wall to the list uf the place is free
					walls[nb_wall][0].fr = i;
					walls[nb_wall][0].to = vertex_from_direction(game.graph, i, EAST);
					walls[nb_wall][1].fr = vertex_from_direction(game.graph, i, SOUTH);
					walls[nb_wall][1].to = vertex_from_direction(game.graph, vertex_from_direction(game.graph, i, SOUTH), EAST);
					nb_wall++;
			}
		}
	}
	return nb_wall;
}

/**
 * @brief Put a wall on the graph in order to know if it is a good play or not
 */ 
void put_wall_opti(struct graph_t *graph, struct edge_t wall[2]) {
	gsl_spmatrix_uint_set(graph->t, wall[0].fr, wall[0].to, 0);
	gsl_spmatrix_uint_set(graph->t, wall[0].to, wall[0].fr, 0);
	gsl_spmatrix_uint_set(graph->t, wall[1].fr, wall[1].to, 0);
	gsl_spmatrix_uint_set(graph->t, wall[1].to, wall[1].fr, 0);
}

/**
 * @brief Remove a wall placed by put_wall
 * @param dir The direction of the second vertex relative to the first, in order to restore correctly the graph
 */ 
void remove_wall_opti(struct graph_t *graph, struct edge_t wall[2], enum direction_t dir) {
	gsl_spmatrix_uint_set(graph->t, wall[0].fr, wall[0].to, dir);
	gsl_spmatrix_uint_set(graph->t, wall[0].to, wall[0].fr, opposite(dir));
	gsl_spmatrix_uint_set(graph->t, wall[1].fr, wall[1].to, dir);
	gsl_spmatrix_uint_set(graph->t, wall[1].to, wall[1].fr, opposite(dir));
}

/**
 * @brief Get the better wall basing on Dijkstra algorithm
 * @returns The index of the wall in the array posswall, if there is no good wall, returns IMPOSSIBLE_ID
 */ 
size_t get_the_better_wall_id(struct game_state_t game, struct edge_t posswall[MAX_POSSIBLE_WALLS][2], size_t nb_wall) {
	size_t opp_dist = dijkstra(game.graph, game.opponent.pos, game.opponent.color);
	size_t self_dist = dijkstra(game.graph, game.self.pos, game.self.color);
	long long int diff = self_dist - opp_dist;
	size_t wall_id = IMPOSSIBLE_ID;
	for (size_t i = 0; i < nb_wall; i++) {
		size_t dir = gsl_spmatrix_uint_get(game.graph->t, posswall[i][0].fr, posswall[i][0].to);
		put_wall_opti(game.graph, posswall[i]);
		size_t new_opp_dist = dijkstra(game.graph, game.opponent.pos, game.opponent.color);
		size_t new_self_dist = dijkstra(game.graph, game.self.pos, game.self.color);
		long long int new_diff = new_self_dist - new_opp_dist;
		if (new_opp_dist < IMPOSSIBLE_DISTANCE && new_self_dist < IMPOSSIBLE_DISTANCE){
			if (new_diff < diff) {
				opp_dist = new_opp_dist;
				self_dist = new_self_dist;
				diff = new_diff;
				wall_id = i;
			}
		}
		remove_wall_opti(game.graph, posswall[i], dir);
	}
	return wall_id;
}

/**
 * @brief Add to an array the vertex where Pablo can eventually move
 * @param self_pos The position of Pablo
 * @param opp_pos The position of the opponent 
 * @param linked An array that contains the vertices in contact : at north, south, east and west. On this array will be added valid positions 
 * @returns The number of possible places 
 */ 
size_t get_linked_for_move(struct graph_t *graph, size_t self_pos, size_t opp_pos, size_t linked[]){
	size_t num = get_linked(graph, self_pos, linked);
	int ind = MAX_DIRECTION;
	linked[ind] = no_vertex();
	linked[ind + 1] = no_vertex();
	for (int i = 1; i < MAX_DIRECTION; i++){
		if (linked[i] == opp_pos){
			if (vertex_from_direction(graph, linked[i], i) != no_vertex()){
				linked[i] = vertex_from_direction(graph, linked[i], i);
				return num;
			}
			else {
				for (int j = 1; j < MAX_DIRECTION; j++)
					if (vertex_from_direction(graph, linked[i], j) != no_vertex() && vertex_from_direction(graph, linked[i], j) != self_pos){
						linked[ind] = vertex_from_direction(graph, linked[i], j);
						ind++;
						num++;
					}
				linked[i] = no_vertex();

			}
		}
	}
	return num;
}

/**
 * @brief Get the best edge where Pablo can move to get as close to the finish line as possible. 
 * @param game Gather all the info we need to get the best move
 * @returns The id of a vertex
 */ 

size_t move_forward(struct game_state_t game) {
	size_t linked[MAX_MOVE_PLACES];
	size_t num = get_linked_for_move(game.graph, game.self.pos, game.opponent.pos, linked);
	int dir = NO_DIRECTION;
	if (num == 0) {
		fprintf(stderr, "ERROR: Player is blocked\n");
	}
	size_t shortest = 2 * (game.graph->num_vertices);
	for (int i = 1; i < MAX_MOVE_PLACES; i++) {
		if (!is_no_vertex(linked[i])) {
			size_t dist_tmp = dijkstra(game.graph, linked[i], game.self.color);
			if (dist_tmp < shortest) {
				shortest = dist_tmp;
				dir = i;
				
			}
		}
	}
	return linked[dir];
}

struct move_t make_first_move(struct game_state_t game) {
	return make_default_first_move(game);
}


/**
 * @brief Get the best move : If Pablo is closer to the arrival than his opponent, he will advance. Else he will try to put the wall that increase the more the distance of the opponnent without penalizing himself
 * @param game Gather all the info we need to get the best move
 * @returns A struct move_t containing the move of Pablo Super Saiyan
 */ 

struct move_t make_move(struct game_state_t game) {
	struct move_t move;
	struct edge_t poss_walls[MAX_POSSIBLE_WALLS][2];
	size_t self_dist = dijkstra(game.graph, game.self.pos, game.self.color);
	size_t opp_dist = dijkstra(game.graph, game.opponent.pos, game.opponent.color);
	if (self_dist <= opp_dist || self_dist == 1 || game.self.num_walls == 0){
		move.m = move_forward(game);
		move.t = MOVE;
	}
	else {
		size_t nb_of_walls = get_possible_walls(game, poss_walls);
		size_t id_wall = get_the_better_wall_id(game, poss_walls, nb_of_walls);

		if (id_wall != IMPOSSIBLE_ID) {
			move.m = game.self.pos;
			move.e[0].fr = poss_walls[id_wall][0].fr;
			move.e[0].to = poss_walls[id_wall][0].to;
			move.e[1].fr = poss_walls[id_wall][1].fr;
			move.e[1].to = poss_walls[id_wall][1].to;
			move.t = WALL;
		} else {
			move.m = move_forward(game);
			move.t = MOVE;
		}
		}
	move.c = game.self.color;
	return move;
}

void finalize_ia() {
	// do nothing
}
