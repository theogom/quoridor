#ifndef _QUOR_TESTS_H_
#define _QUOR_TESTS_H_

#include <stdio.h>
#include <stdbool.h>

static bool pass;
static int passed = 0;
static int total = 0;

// Test macros
#define TEST(f) pass = true, setup(), f(), teardown(), printf("\t%s\n", pass ? "OK" : "FAIL"), passed += pass, total++
#define FAIL(msg) fprintf(stderr, "ERROR: %s\n", msg), pass = false
#define SUMMARY() printf("%s(%d/%d) --> %s\n\n", __func__, passed, total, passed == total ? "PASSED" : "FAILED")

void test_player_main(void);
void test_server_main(void);

#endif // _QUOR_TESTS_H_
