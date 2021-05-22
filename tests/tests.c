#include "tests.h"
#include <stdlib.h>

int main() {
	(void) pass;
	(void) passed;
	(void) total;

	test_player_main();
	test_server_main();
	return EXIT_SUCCESS;
}
