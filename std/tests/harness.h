#ifndef TESTS_HARNESS_H
#define TESTS_HARNESS_H

#include <stdio.h>
#include <stdlib.h>

#include "types.h"

typedef VoidFunc* test_fn;

typedef struct TestEntry {
        ConstC_String     name;
        test_fn           fn;
        struct TestEntry* next;
} TestEntry;

void tests_register(ConstC_String name, test_fn fn);
int  tests_run_all(void);

#define AssertTrue(x)                                              \
        do {                                                       \
                if (!(x)) {                                        \
                        fprintf(stderr,                            \
                                "[%s:%d] AssertTrue failed: %s\n", \
                                __FILE__,                          \
                                __LINE__,                          \
                                #x);                               \
                        exit(1);                                   \
                }                                                  \
        } while (0)

#define AssertEq(a, b)                                                 \
        do {                                                           \
                if (!((a) == (b))) {                                   \
                        fprintf(stderr,                                \
                                "[%s:%d] AssertEq failed: %s != %s\n", \
                                __FILE__,                              \
                                __LINE__,                              \
                                #a,                                    \
                                #b);                                   \
                        exit(1);                                       \
                }                                                      \
        } while (0)

#define AssertEqF(a, b, fmt)                                       \
        do {                                                       \
                if (!((a) == (b))) {                               \
                        fprintf(stderr,                            \
                                "[%s:%d] AssertEq failed: %s(" fmt \
                                ") != %s(" fmt ")\n",              \
                                __FILE__,                          \
                                __LINE__,                          \
                                #a,                                \
                                a,                                 \
                                #b,                                \
                                b);                                \
                        exit(1);                                   \
                }                                                  \
        } while (0)

#define RegisterTest(name)                                                \
        static void test_##name(void);                                    \
        static void __register_##name(void) __attribute__((constructor)); \
        static void __register_##name(void) {                             \
                tests_register(#name, test_##name);                       \
        }                                                                 \
        static void test_##name(void)

#ifdef BSTD_IMPL
#        define HARNESS_IMPL
#endif
#ifdef HARNESS_IMPL
static TestEntry* tests_head = null;

void tests_register(ConstC_String name, test_fn fn) {
        TestEntry* e = (TestEntry*)malloc(sizeof(TestEntry));
        if (!e) {
                fprintf(stderr, "OOM registering test %s\n", name);
                exit(2);
        }
        e->name    = name;
        e->fn      = fn;
        e->next    = tests_head;
        tests_head = e;
}

int tests_run_all(void) {
        int total  = 0;
        int failed = 0;
        for (TestEntry* e = tests_head; e; e = e->next) {
                ++total;
                printf("[ RUN ] %s\n", e->name);
                fflush(stdout);
                /* run test; if it calls exit(1) or crashes, the runner will
                 * stop */
                e->fn();
                printf("[  OK ] %s\n", e->name);
        }
        printf("Ran %d tests. %d failed.\n", total, failed);
        return failed;
}

/* test runner main */
int main_tests_runner(void) {
        int r = tests_run_all();
        return r == 0 ? 0 : 1;
}
#endif /* HARNESS_IMPL */

#endif /* TESTS_HARNESS_H */
