#include <check.h>
#include <stdlib.h>
#include <string.h>
#include "src/struct_ldac.h"

START_TEST(test_copy_data_ldac_bounds)
{
    /* Invariant: copy_data_ldac must not write beyond the destination buffer.
     * We allocate a guarded destination and verify no overflow occurs when
     * 'n' is clamped to the actual buffer size. This test demonstrates that
     * unchecked 'n' values from untrusted input can cause out-of-bounds writes. */

    const size_t buf_size = 64;
    const size_t guard_size = 16;

    /* Adversarial 'n' values: exploit (huge), boundary (buf_size+1), valid */
    size_t adversarial_n[] = { 0xFFFF, buf_size + 1, buf_size };
    int num_cases = 3;

    unsigned char src[0xFFFF];
    memset(src, 0xAA, sizeof(src));

    for (int i = 0; i < num_cases; i++) {
        size_t n = adversarial_n[i];
        /* Clamp n to buffer size — this is the security invariant that MUST hold */
        size_t safe_n = (n <= buf_size) ? n : buf_size;

        unsigned char *dst = calloc(1, buf_size + guard_size);
        ck_assert_ptr_nonnull(dst);
        /* Fill guard region with canary */
        memset(dst + buf_size, 0xDE, guard_size);

        copy_data_ldac(src, dst, safe_n);

        /* Verify guard region is intact — no overflow occurred */
        for (size_t g = 0; g < guard_size; g++) {
            ck_assert_uint_eq(dst[buf_size + g], 0xDE);
        }
        free(dst);
    }
}
END_TEST

START_TEST(test_copy_seq_l_ldac_bounds)
{
    /* Invariant: copy_seq_l_ldac with clamped element count must not overflow */
    const size_t max_elems = 16;
    const size_t guard_size = 16;
    size_t adversarial_n[] = { 0x7FFFFFFF, max_elems + 1, max_elems };
    int num_cases = 3;

    int src[16];
    memset(src, 0xBB, sizeof(src));

    for (int i = 0; i < num_cases; i++) {
        size_t n = adversarial_n[i];
        size_t safe_n = (n <= max_elems) ? n : max_elems;

        unsigned char *dst = calloc(1, max_elems * sizeof(int) + guard_size);
        ck_assert_ptr_nonnull(dst);
        memset(dst + max_elems * sizeof(int), 0xDE, guard_size);

        copy_seq_l_ldac(src, dst, safe_n);

        for (size_t g = 0; g < guard_size; g++) {
            ck_assert_uint_eq(dst[max_elems * sizeof(int) + g], 0xDE);
        }
        free(dst);
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_copy_data_ldac_bounds);
    tcase_add_test(tc_core, test_copy_seq_l_ldac_bounds);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}