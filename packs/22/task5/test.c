#include "priority_queue.h"

#include <cmocka.h>
#include <stdint.h>
#include <stdlib.h>

static void test_create_destroy(void **state)
{
    priority_queue_t *queue = pq_create(4);
    (void) state;

    assert_non_null(queue);
    assert_true(pq_is_empty(queue));
    assert_false(pq_is_full(queue));
    assert_int_equal(pq_size(queue), 0);
    pq_destroy(queue, NULL);
}

static void test_create_unlimited(void **state)
{
    priority_queue_t *queue = pq_create(0);
    (void) state;

    assert_non_null(queue);
    assert_false(pq_is_full(queue));
    pq_destroy(queue, NULL);
}

static void test_create_capacity_one(void **state)
{
    priority_queue_t *queue = pq_create(1);
    assert_non_null(queue);
    assert_true(pq_push(queue, (void *) (intptr_t) 10, 5));
    assert_true(pq_is_full(queue));
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_push(void **state)
{
    priority_queue_t *queue = pq_create(4);
    assert_true(pq_push(queue, (void *) (intptr_t) 1, 7));
    assert_int_equal(pq_size(queue), 1);
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_push_pop_priority(void **state)
{
    priority_queue_t *queue = pq_create(8);
    void *data = NULL;
    int priority = 0;

    pq_push(queue, (void *) (intptr_t) 1, 10);
    pq_push(queue, (void *) (intptr_t) 2, 30);
    pq_push(queue, (void *) (intptr_t) 3, 20);

    assert_true(pq_pop(queue, &data, &priority));
    assert_int_equal((int) (intptr_t) data, 2);
    assert_int_equal(priority, 30);

    assert_true(pq_pop(queue, &data, &priority));
    assert_int_equal((int) (intptr_t) data, 3);
    assert_int_equal(priority, 20);

    assert_true(pq_pop(queue, &data, &priority));
    assert_int_equal((int) (intptr_t) data, 1);
    assert_int_equal(priority, 10);

    pq_destroy(queue, NULL);
    (void) state;
}

static void test_peek(void **state)
{
    priority_queue_t *queue = pq_create(2);
    void *data = NULL;
    int priority = 0;

    pq_push(queue, (void *) (intptr_t) 5, 8);
    pq_push(queue, (void *) (intptr_t) 3, 1);
    assert_true(pq_peek(queue, &data, &priority));
    assert_int_equal((int) (intptr_t) data, 5);
    assert_int_equal(priority, 8);
    assert_int_equal(pq_size(queue), 2);
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_overflow(void **state)
{
    priority_queue_t *queue = pq_create(1);
    pq_push(queue, (void *) (intptr_t) 1, 1);
    assert_false(pq_push(queue, (void *) (intptr_t) 2, 2));
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_clear(void **state)
{
    priority_queue_t *queue = pq_create(4);
    int *item = malloc(sizeof(*item));
    int *another = malloc(sizeof(*another));

    *item = 7;
    *another = 9;
    pq_push(queue, item, 1);
    pq_push(queue, another, 2);
    pq_clear(queue, free);

    assert_true(pq_is_empty(queue));
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_empty_operations(void **state)
{
    priority_queue_t *queue = pq_create(0);
    assert_false(pq_pop(queue, NULL, NULL));
    assert_false(pq_peek(queue, NULL, NULL));
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_equal_priorities(void **state)
{
    priority_queue_t *queue = pq_create(8);
    void *data = NULL;

    pq_push(queue, (void *) (intptr_t) 11, 3);
    pq_push(queue, (void *) (intptr_t) 22, 3);
    pq_push(queue, (void *) (intptr_t) 33, 3);

    pq_pop(queue, &data, NULL);
    assert_int_equal((int) (intptr_t) data, 11);
    pq_pop(queue, &data, NULL);
    assert_int_equal((int) (intptr_t) data, 22);
    pq_pop(queue, &data, NULL);
    assert_int_equal((int) (intptr_t) data, 33);

    pq_destroy(queue, NULL);
    (void) state;
}

static void test_unlimited_queue(void **state)
{
    priority_queue_t *queue = pq_create(0);
    size_t i;

    for (i = 0; i < 128; ++i) {
        assert_true(pq_push(queue, (void *) (intptr_t) i, (int) i));
    }
    assert_int_equal(pq_size(queue), 128);
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_null_queue(void **state)
{
    assert_false(pq_push(NULL, NULL, 0));
    assert_false(pq_pop(NULL, NULL, NULL));
    assert_false(pq_peek(NULL, NULL, NULL));
    assert_int_equal(pq_size(NULL), 0);
    assert_false(pq_is_full(NULL));
    assert_true(pq_is_empty(NULL));
    pq_clear(NULL, NULL);
    pq_destroy(NULL, NULL);
    (void) state;
}

static void test_clear_no_free(void **state)
{
    priority_queue_t *queue = pq_create(4);
    int value = 5;
    pq_push(queue, &value, 1);
    pq_clear(queue, NULL);
    assert_true(pq_is_empty(queue));
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_pop_peek_null_params(void **state)
{
    priority_queue_t *queue = pq_create(4);
    pq_push(queue, (void *) (intptr_t) 4, 9);
    assert_true(pq_peek(queue, NULL, NULL));
    assert_true(pq_pop(queue, NULL, NULL));
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_pop_only_data(void **state)
{
    priority_queue_t *queue = pq_create(4);
    void *data = NULL;
    pq_push(queue, (void *) (intptr_t) 8, 1);
    assert_true(pq_pop(queue, &data, NULL));
    assert_int_equal((int) (intptr_t) data, 8);
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_pop_only_priority(void **state)
{
    priority_queue_t *queue = pq_create(4);
    int priority = 0;
    pq_push(queue, (void *) (intptr_t) 8, 12);
    assert_true(pq_pop(queue, NULL, &priority));
    assert_int_equal(priority, 12);
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_peek_only_data(void **state)
{
    priority_queue_t *queue = pq_create(4);
    void *data = NULL;
    pq_push(queue, (void *) (intptr_t) 9, 2);
    assert_true(pq_peek(queue, &data, NULL));
    assert_int_equal((int) (intptr_t) data, 9);
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_peek_only_priority(void **state)
{
    priority_queue_t *queue = pq_create(4);
    int priority = 0;
    pq_push(queue, (void *) (intptr_t) 9, 2);
    assert_true(pq_peek(queue, NULL, &priority));
    assert_int_equal(priority, 2);
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_heapify_up_single_element(void **state)
{
    priority_queue_t *queue = pq_create(4);
    pq_push(queue, (void *) (intptr_t) 1, 1);
    assert_int_equal(pq_size(queue), 1);
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_heapify_down_single_element(void **state)
{
    priority_queue_t *queue = pq_create(4);
    void *data = NULL;

    pq_push(queue, (void *) (intptr_t) 1, 1);
    assert_true(pq_pop(queue, &data, NULL));
    assert_int_equal((int) (intptr_t) data, 1);
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_destroy_no_free(void **state)
{
    priority_queue_t *queue = pq_create(4);
    int value = 1;
    pq_push(queue, &value, 1);
    pq_destroy(queue, NULL);
    (void) state;
}

static int free_counter = 0;

static void count_free(void *data)
{
    free(data);
    free_counter++;
}

static void test_destroy_frees_queue(void **state)
{
    priority_queue_t *queue = pq_create(4);
    int *first = malloc(sizeof(*first));
    int *second = malloc(sizeof(*second));

    *first = 1;
    *second = 2;
    free_counter = 0;
    pq_push(queue, first, 1);
    pq_push(queue, second, 2);
    pq_destroy(queue, count_free);
    assert_int_equal(free_counter, 2);
    (void) state;
}

static void test_clear_resets_capacity(void **state)
{
    priority_queue_t *queue = pq_create(0);
    pq_push(queue, (void *) (intptr_t) 1, 1);
    pq_push(queue, (void *) (intptr_t) 2, 2);
    pq_clear(queue, NULL);
    assert_true(pq_is_empty(queue));
    assert_false(pq_is_full(queue));
    assert_true(pq_push(queue, (void *) (intptr_t) 3, 3));
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_pop_clears_last_element(void **state)
{
    priority_queue_t *queue = pq_create(2);
    pq_push(queue, (void *) (intptr_t) 1, 1);
    assert_true(pq_pop(queue, NULL, NULL));
    assert_true(pq_is_empty(queue));
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_heap_stability_up(void **state)
{
    priority_queue_t *queue = pq_create(8);
    void *data = NULL;

    pq_push(queue, (void *) (intptr_t) 1, 5);
    pq_push(queue, (void *) (intptr_t) 2, 5);
    pq_push(queue, (void *) (intptr_t) 3, 6);
    pq_pop(queue, &data, NULL);
    assert_int_equal((int) (intptr_t) data, 3);
    pq_pop(queue, &data, NULL);
    assert_int_equal((int) (intptr_t) data, 1);
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_heap_stability_down(void **state)
{
    priority_queue_t *queue = pq_create(8);
    void *data = NULL;

    pq_push(queue, (void *) (intptr_t) 10, 9);
    pq_push(queue, (void *) (intptr_t) 20, 7);
    pq_push(queue, (void *) (intptr_t) 30, 7);
    pq_push(queue, (void *) (intptr_t) 40, 7);
    pq_pop(queue, &data, NULL);
    assert_int_equal((int) (intptr_t) data, 10);
    pq_pop(queue, &data, NULL);
    assert_int_equal((int) (intptr_t) data, 20);
    pq_pop(queue, &data, NULL);
    assert_int_equal((int) (intptr_t) data, 30);
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_unlimited_queue_realloc(void **state)
{
    priority_queue_t *queue = pq_create(0);
    size_t i;
    void *data = NULL;
    int priority = 0;

    for (i = 0; i < 1000; ++i) {
        assert_true(pq_push(queue, (void *) (intptr_t) i, (int) i));
    }
    assert_true(pq_pop(queue, &data, &priority));
    assert_int_equal((int) (intptr_t) data, 999);
    assert_int_equal(priority, 999);
    pq_destroy(queue, NULL);
    (void) state;
}

static void test_push_checks_overflow(void **state)
{
    priority_queue_t *queue = pq_create(1);
    assert_true(pq_push(queue, (void *) (intptr_t) 1, INT_MAX));
    assert_false(pq_push(queue, (void *) (intptr_t) 2, INT_MIN));
    pq_destroy(queue, NULL);
    (void) state;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_create_destroy),
        cmocka_unit_test(test_create_unlimited),
        cmocka_unit_test(test_create_capacity_one),
        cmocka_unit_test(test_push),
        cmocka_unit_test(test_push_pop_priority),
        cmocka_unit_test(test_peek),
        cmocka_unit_test(test_overflow),
        cmocka_unit_test(test_clear),
        cmocka_unit_test(test_empty_operations),
        cmocka_unit_test(test_equal_priorities),
        cmocka_unit_test(test_unlimited_queue),
        cmocka_unit_test(test_null_queue),
        cmocka_unit_test(test_clear_no_free),
        cmocka_unit_test(test_pop_peek_null_params),
        cmocka_unit_test(test_pop_only_data),
        cmocka_unit_test(test_pop_only_priority),
        cmocka_unit_test(test_peek_only_data),
        cmocka_unit_test(test_peek_only_priority),
        cmocka_unit_test(test_heapify_up_single_element),
        cmocka_unit_test(test_heapify_down_single_element),
        cmocka_unit_test(test_destroy_no_free),
        cmocka_unit_test(test_destroy_frees_queue),
        cmocka_unit_test(test_clear_resets_capacity),
        cmocka_unit_test(test_pop_clears_last_element),
        cmocka_unit_test(test_heap_stability_up),
        cmocka_unit_test(test_heap_stability_down),
        cmocka_unit_test(test_unlimited_queue_realloc),
        cmocka_unit_test(test_push_checks_overflow),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
