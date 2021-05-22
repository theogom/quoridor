/**
 * @file board.c
 *
 * @brief Board initialisation and processing
 */

#include "board.h"
#include <gsl/gsl_spmatrix.h>
#include <string.h>
#include <math.h>


//// Graph structure manipulation functions
// TODO Separate graph init and others operations

/**
 * @brief Initialize a square graph
 *
 * @param m An integer setting the size of the square sides
 *
 * @return A graph representing a square board
 */
struct graph_t* square_init(size_t m) {
	struct graph_t* graph = malloc(sizeof(*graph));

	size_t n = m * m;
	graph->num_vertices = n;

	// Initialize the actual board
	graph->t = gsl_spmatrix_uint_alloc(n, n);

	// Fill the whole graph with 0
	gsl_spmatrix_uint_set_zero(graph->t);

	// Fill the strict bottom corner
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = 0; j < i; ++j) {

			if (j % m != m - 1 && i == j + 1) {
				gsl_spmatrix_uint_set(graph->t, i, j, 3);
				gsl_spmatrix_uint_set(graph->t, j, i, 4);
			}

			if (j / m < m - 1 && i == j + m - 1) {
				gsl_spmatrix_uint_set(graph->t, i + 1, j, 1);
				gsl_spmatrix_uint_set(graph->t, j, i + 1, 2);
			}
		}
	}

	// Initialize proprietary matrix
	graph->o = gsl_spmatrix_uint_alloc(2, n);
	for (size_t i = 0; i < m; i++) {
		gsl_spmatrix_uint_set(graph->o, BLACK, i, 1);
		gsl_spmatrix_uint_set(graph->o, WHITE, n - i - 1, 1);
	}

	return graph;
}

/**
 * @brief Initialize a graph representing the game board
 * with the given size and shape
 *
 * @param m An integer setting the size of the board
 * @param shape A shape enum setting the shape of the board
 * between `SQUARE`, `TORIC`, `H` and `SNAKE`
 *
 * @return A graph representing the game board
 */
struct graph_t* graph_init(size_t m, enum shape_t shape) {
	switch (shape) {
	case SQUARE:
		return square_init(m);
	default:
		return square_init(m);
	}
}

/**
 * @brief Free the memory allocated for a graph
 *
 * @param graph The graph to free
 */

void graph_free(struct graph_t* graph) {
	gsl_spmatrix_uint_free(graph->t);
	gsl_spmatrix_uint_free(graph->o);
	free(graph);
}

/**
 * @brief Sort the edges representing a wall in increasing order
 *
 * @details Sorting is performed in increasing order by comparing vertex index
 * Each edge are sorted by comparing their source and destination vertex
 * Then the two edges in the array are sorted by their source vertex
 *
 * @param e An array of two edges representing a wall
 */
void sort_edges(struct edge_t e[2]) {
	for (size_t i = 0; i < 2; ++i) {
		if (e[i].fr > e[i].to) {
			size_t tmp = e[i].fr;
			e[i].fr = e[i].to;
			e[i].to = tmp;
		}
	}

	if (e[0].fr > e[1].fr) {
		struct edge_t tmp = e[0];
		e[0] = e[1];
		e[1] = tmp;
	}
}

/**
 *  @brief Add edges to graph
 *
 * @details Add the wall represented by the two edges in `e` in `graph` \n
 * Change vertices values in the graph matrix
 * by 5 and 6 for the src and dest edge respectively if the wall is vertical
 * or by 7 and 8 respectively if the wall is horizontal \n
 * The wall is supposed to be valid
 *
 * @param graph The graph to update
 * @param e An array of two edges representing a wall
 */
void place_wall(struct graph_t* graph, struct edge_t e[2]) {
	size_t board_size = (size_t)sqrtl(graph->num_vertices);

	// Copy values so they can be modified, then sort them
	e = (struct edge_t[2]){ e[0], e[1] };
	sort_edges(e);

	// Get nodes
	size_t first_node = e[0].fr;
	size_t second_node = e[0].to;

	if (first_node + 1 == second_node) {
		// Vertical wall
		gsl_spmatrix_uint_set(graph->t, first_node, second_node, 5);
		gsl_spmatrix_uint_set(graph->t, second_node, first_node, 5);

		gsl_spmatrix_uint_set(graph->t, first_node + board_size, second_node + board_size, 6);
		gsl_spmatrix_uint_set(graph->t, second_node + board_size, first_node + board_size, 6);

	}
	else {
		// Horizontal wall
		gsl_spmatrix_uint_set(graph->t, first_node, second_node, 7);
		gsl_spmatrix_uint_set(graph->t, second_node, first_node, 7);

		gsl_spmatrix_uint_set(graph->t, first_node + 1, second_node + 1, 8);
		gsl_spmatrix_uint_set(graph->t, second_node + 1, first_node + 1, 8);
	}
}

/**
 * @brief Remove edges from graph
 *
 * @details Remove the wall represented by the two edges in `e` from `graph` \n
 * Change vertices values in the graph matrix by their corresponding direction
 *
 * @param graph The graph to update
 * @param e An array of two edges representing a wall
 */
void remove_wall(struct graph_t* graph, struct edge_t e[2]) {
	/*
	// get nodes
	size_t first_node = e[0].fr;
	size_t second_node = e[0].to;

	// sort nodes
	if (first_node > second_node) {
		size_t tmp = first_node;
		first_node = second_node;
		second_node = tmp;
	}
	*/
	if (gsl_spmatrix_uint_get(graph->t, e[0].fr, e[0].to) == 5
		|| gsl_spmatrix_uint_get(graph->t, e[0].fr, e[0].to) == 6) {
		gsl_spmatrix_uint_set(graph->t, e[0].fr, e[0].to, EAST);
		gsl_spmatrix_uint_set(graph->t, e[0].to, e[0].fr, WEST);

		gsl_spmatrix_uint_set(graph->t, e[1].fr, e[1].to, EAST);
		gsl_spmatrix_uint_set(graph->t, e[1].to, e[1].fr, WEST);
	}
	else if (gsl_spmatrix_uint_get(graph->t, e[0].fr, e[0].to) == 7
		|| gsl_spmatrix_uint_get(graph->t, e[0].fr, e[0].to) == 8) {
		gsl_spmatrix_uint_set(graph->t, e[0].fr, e[0].to, SOUTH);
		gsl_spmatrix_uint_set(graph->t, e[0].to, e[0].fr, NORTH);

		gsl_spmatrix_uint_set(graph->t, e[1].fr, e[1].to, SOUTH);
		gsl_spmatrix_uint_set(graph->t, e[1].to, e[1].fr, NORTH);
	}
	else if (gsl_spmatrix_uint_get(graph->t, e[0].to, e[0].fr) == 5
		|| gsl_spmatrix_uint_get(graph->t, e[0].to, e[0].fr) == 6) {
		gsl_spmatrix_uint_set(graph->t, e[0].fr, e[0].to, WEST);
		gsl_spmatrix_uint_set(graph->t, e[0].to, e[0].fr, EAST);

		gsl_spmatrix_uint_set(graph->t, e[1].fr, e[1].to, WEST);
		gsl_spmatrix_uint_set(graph->t, e[1].to, e[1].fr, EAST);
	}
	else if (gsl_spmatrix_uint_get(graph->t, e[0].to, e[0].fr) == 7
		|| gsl_spmatrix_uint_get(graph->t, e[0].to, e[0].fr) == 8) {
		gsl_spmatrix_uint_set(graph->t, e[0].fr, e[0].to, NORTH);
		gsl_spmatrix_uint_set(graph->t, e[0].to, e[0].fr, SOUTH);

		gsl_spmatrix_uint_set(graph->t, e[1].fr, e[1].to, NORTH);
		gsl_spmatrix_uint_set(graph->t, e[1].to, e[1].fr, SOUTH);
	}
	else
		printf("ERROR (%u) (%u)\n", gsl_spmatrix_uint_get(graph->t, e[0].fr, e[0].to),
			gsl_spmatrix_uint_get(graph->t, e[0].to, e[0].fr));
}

/**
 * @brief Get the opposite to a direction
 *
 * @param d The direction processed
 *
 * @return The opposite direction of `d`
 */
enum direction_t opposite(enum direction_t d) {
	switch (d) {
	case NORTH:
		return SOUTH;
	case SOUTH:
		return NORTH;
	case WEST:
		return EAST;
	case EAST:
		return WEST;
	default:
		return NO_DIRECTION;
	}
}

/**
 * @brief Check if vertex src is linked to vertex dest (i.e there is an edge between them)
 *
 * @param graph The graph processed
 * @param src The source vertex
 * @param dest The destination vertex
 */
bool is_linked(const struct graph_t* graph, size_t src, size_t dest) {
	return gsl_spmatrix_uint_get(graph->t, src, dest) > 0 && gsl_spmatrix_uint_get(graph->t, src, dest) < 5;
}

/**
 * @brief Get the adjacent vertex of a vertex in a given direction
 * 
 * @details Check if there is an adjacent vertex of v (i.e a vertex linked by an edge) in
 * the direction d
 * 
 * @param graph The graph processed
 * @param v The vertex processed
 * @param d The direction to search
 * 
 * @return The adjacent vertex of `v` in direction `d` vertex if it exists, if not returns no_vertex()
 */
size_t vertex_from_direction(const struct graph_t* graph, size_t v, enum direction_t d) {
	// TODO : this function can be optimised

	if (v >= graph->num_vertices)
		return no_vertex();

	for (size_t i = 0; i < graph->num_vertices; i++) {
		if (gsl_spmatrix_uint_get(graph->t, v, i) == d)
			return i;
	}

	return no_vertex();
}

/**
 * @brief Get all the linked vertices from the vertex v
 *
 * @details Fill an array with the linked vertices where indices are directions,
 * put no_vertex() if there is no vertex in the direction
 *
 * @param graph The graph processed
 * @param v The vertex processed
 * @param vertices An array of `size_t` of size at least `MAX_DIRECTION`
 */
size_t get_linked(const struct graph_t* graph, size_t v, size_t vertices[]) {
	size_t linked = 0;
	vertices[NO_DIRECTION] = v; // First one is himself

	for (enum direction_t i = NO_DIRECTION + 1; i < MAX_DIRECTION; i++) {
		vertices[i] = vertex_from_direction(graph, v, i);
		linked += !is_no_vertex(vertices[i]);
	}
	return linked;
}

/**
 * @brief Display the adjacent matrix of a board
 * 
 * @param board The board processed
 * @param board_size The size of the board
 */
void display_adj_matrix(struct graph_t* board, size_t board_size) {
	size_t nbCells = board_size * board_size;
	for (size_t i = 0; i < nbCells; ++i) {
		for (size_t j = 0; j < nbCells; ++j) {
			printf("% d ", gsl_spmatrix_uint_get(board->t, i, j));
		}
		printf("\n");
	}
}

/**
 * @brief Display a board
 *
 * @details Display a board by printing vertices, edges, wall, and players
 *
 * @param board The board processed
 * @param board_size The size of the board
 * @param position_player_1 The position of the first player
 * @param position_player_2 The position of the second player
 */
void display_board(struct graph_t* board, size_t board_size, size_t position_player_1, size_t position_player_2) {
	/*
	 5 = wall on east + next line
	 6 = wall on east

	 7 = wall on south + south right
	 8 = wall on south
	*/

	// Care about out of tab
	char next_line[24 * board_size + 1];
	next_line[0] = '\0';

	for (size_t i = 0; i < board_size * board_size; ++i) {

		if (i % board_size == 0) {
			printf("\n");
			printf("%s", next_line);
			printf("\n");
			strcpy(next_line, "");
		}

		printf("%s", i == position_player_1 ? "\033[31m1\033[m" :
			i == position_player_2 ? "\033[34m1\033[m" :
			"0");

		int end_pattern = 0;

		unsigned int matrix_state_1 = 0;
		if (i + 1 < board_size * board_size) {
			matrix_state_1 = gsl_spmatrix_uint_get(board->t, i, i + 1);
		}

		unsigned int matrix_state_2 = 0;
		if (i + board_size < board_size * board_size) {
			matrix_state_2 = gsl_spmatrix_uint_get(board->t, i, i + board_size);
		}

		// South connection
		if (matrix_state_2 == 2) {
			strcat(next_line, "| ");
		}
		else if (matrix_state_2 == 7) {
			strcat(next_line, "\033[33m──\033[m");
			end_pattern = 1;
		}
		else if (matrix_state_2 == 8) {
			strcat(next_line, "\033[33m─\033[m ");
		}
		else {
			strcat(next_line, "  ");
		}

		// east connection
		if (matrix_state_1 == 4) {
			printf(" - ");
		}
		else if (matrix_state_1 == 5) {
			printf(" \033[33m│\033[m ");
			strcat(next_line, "\033[33m│\033[m ");
			end_pattern = 2;
		}
		else if (matrix_state_1 == 6) {
			printf(" \033[33m│\033[m ");
		}
		else {
			printf("   ");
		}

		if (end_pattern == 0) {
			strcat(next_line, "  ");
		}
		else if (end_pattern == 1) {
			strcat(next_line, "\033[33m──\033[m");
		}
	}
	printf("\n");
}





////////////////////////////// FOR DIJKSTRA ALGORITHM ////////////////////////////////
/**
 * @brief Init the array d of the distance and v of the vertices to be able to apply Dijkstra algorithm
 * @param d An array that will contain the distance of each vertex to the source. When the Dijkstra algorithm will be apllied, the distance between the source and the vertex i will be d[i]
 * @param v An array that will contain the vertices remaining to be traversed
 * @param length The number of vertices
 */
void dijkstra_init(size_t d[], size_t v[], size_t pos, size_t length){
		for (size_t i = 0; i < length; i++) {
		d[i] = IMPOSSIBLE_DISTANCE;
		v[i] = i;
	}
	d[pos] = 0;
	
}

/**
 * @brief Used to know what vertex is the next to be release the Dijkstra algorithm context. This vextex is removed from the array v : it is replaced by the last element of v and the size of v decreases by one. 
 * @param d The array containing the distances between vertices and the source.
 * @param v The array containing the remaining vertex
 * @param length The number of remaining params
 * @returns The index of a vertex
 */ 
size_t extract_min(size_t d[], size_t v[],size_t length){
	size_t ind = length+1;
	size_t min = length*4;
	for(size_t i = 0; i < length; i++)
		if(d[v[i]] < min){
			min = d[v[i]];
			ind = i;
		}
	size_t vertex = v[ind];
	v[ind] = v[length - 1];
	return vertex;
}

/**
 * @brief Used to know if an element is in an array
 * 
 *
 */ 
bool belong(size_t elem, size_t arr[], size_t length){
	for (size_t i = 0; i < length; i++)
		if (elem == arr[i])
			return true;
	return false;
}

/**
 * @brief Release the edge between vertex u and vertex v in the Dijkstra algorithm context
 * @param d The array that contains the distances
 *
 */ 

void release_dijk(size_t u, size_t v, size_t d[]){
	if (d[v] > d[u] + 1)
		d[v] = d[u] + 1;
}


/**
 * @brief Get the shortest distance between the source and a vertex among the target line vertices
 * @param color The color of the player that we want to know his distance to reach to the arrival line.
 *
 */ 
//Returns the shortest distance in array d under condition that the vertes is a target vertex.
size_t get_shortest(size_t d[], struct graph_t *graph, enum color_t color){
	size_t min = IMPOSSIBLE_DISTANCE;
	for (size_t i = 0; i < graph->num_vertices; i++)
		if (d[i] < min && gsl_spmatrix_uint_get(graph->o, 1 - color, i)){
			min = d[i];
		}
	return min;
}
/**
 * @brief Dijkstra algorithm to get the closest path
 * @returns The distance between the position pos and the target line for the right player using the Dijkstra algorithm
 * 
 */
size_t dijkstra(struct graph_t *graph, size_t pos, enum color_t color){
	size_t nb_v = graph->num_vertices;
	size_t d[nb_v];
	size_t v[nb_v];
	dijkstra_init(d, v, pos, nb_v);
	while (nb_v){
		size_t u = extract_min(d, v, nb_v);
		nb_v--;
		size_t succ[MAX_DIRECTION];
		get_linked(graph, u, succ);
		for (size_t i = 0; i < MAX_DIRECTION; i++)
			if (belong(succ[i], v, nb_v))
				release_dijk(u, succ[i], d);
	}
	return get_shortest(d, graph, color);
}
