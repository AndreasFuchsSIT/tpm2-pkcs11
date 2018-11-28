/* SPDX-License-Identifier: BSD-2 */
/***********************************************************************
 * Copyright (c) 2018, Intel Corporation
 *
 * All rights reserved.
 ***********************************************************************/

#include "test.h"

/*
 * C_GetMechanismList is used to obtain a list of mechanism types supported by a token.
 * SlotID is the ID of the token’s slot; pulCount points to the location that receives the number of mechanisms.
 * There are two ways for an application to call C_GetMechanismList:
 * 1. If pMechanismList is NULL_PTR, then all that C_GetMechanismList does is return (in *pulCount) the number of mechanisms,
 *  without actually returning a list of mechanisms.  The contents of *pulCount on entry to C_GetMechanismList has no meaning
 *  in this case, and the call returns the value CKR_OK.
 * 2. If pMechanismList is not NULL_PTR, then *pulCount MUST contain the size (in terms of CK_MECHANISM_TYPE elements) of the
 *  buffer pointed to by pMechanismList.  If that buffer is large enough to hold the list of mechanisms, then the list is returned in it,
 *  and CKR_OK is returned.  If not, then the call to C_GetMechanismList returns the value CKR_BUFFER_TOO_SMALL.
 *  In either case, the value *pulCount is set to hold the number of mechanisms.
 * Because C_GetMechanismList does not allocate any space of its own, an application will often call C_GetMechanismList twice.
 * However, this behavior is by no means required.
 */

struct test_info {
    CK_SLOT_ID slot;
};

static int test_setup(void **state) {

    test_info *info = calloc(1, sizeof(*info));
    assert_non_null(info);

    /* get the slots */
    CK_SLOT_ID slots[32];
    unsigned long count = ARRAY_LEN(slots);
    CK_RV rv = C_GetSlotList(true, slots, &count);
    assert_int_equal(rv, CKR_OK);
    assert_int_not_equal(count, 0);

    /* assign to state */
    info->slot = slots[0];

    *state = info;

    /* success */
    return 0;
}

static int test_teardown(void **state) {

    test_info *ti = test_info_from_state(state);

    free(ti);

    return 0;
}

void test_get_mechanism_list_good(void **state) {

    test_info *ti = test_info_from_state(state);
    CK_SLOT_ID slot_id = ti->slot;

    unsigned long mech_cnt;
    CK_MECHANISM_TYPE mechs[256];
    // Only return the number of mechanisms
    CK_RV rv = C_GetMechanismList(slot_id, NULL, &mech_cnt);
    assert_int_equal(rv, CKR_OK);
    assert_in_range(mech_cnt, 1, ARRAY_LEN(mechs));

    // Return List of mechanisms
    rv = C_GetMechanismList(slot_id, mechs, &mech_cnt);
    assert_int_equal(rv, CKR_OK);
}

void test_get_mechanism_list_bad(void **state) {

    test_info *ti = test_info_from_state(state);
    CK_SLOT_ID slot_id = ti->slot;

    unsigned long mech_cnt;

    // Invalid Slot
    CK_RV rv = C_GetMechanismList((CK_ULONG)-10, NULL, &mech_cnt);
    assert_int_equal(rv, CKR_SLOT_ID_INVALID);

    rv = C_GetMechanismList(slot_id, NULL, NULL);
    assert_int_equal(rv, CKR_ARGUMENTS_BAD);

    CK_MECHANISM_TYPE mechs[1];
    rv = C_GetMechanismList(slot_id, NULL, &mech_cnt);
    assert_int_equal(rv, CKR_OK);
    assert_int_not_equal(mech_cnt, ARRAY_LEN(mechs));

    unsigned long value = 0;
    rv = C_GetMechanismList(slot_id, mechs, &value);
    assert_int_equal(rv, CKR_BUFFER_TOO_SMALL);

    // Low count but buffer present
    value = ARRAY_LEN(mechs);
    rv = C_GetMechanismList(slot_id, mechs, &value);
    assert_int_equal(rv, CKR_BUFFER_TOO_SMALL);
}

/**
 * C_GetMechanismInfo obtains information about a particular mechanism possibly supported by a token.
 * slotID is the ID of the token’s slot; type is the type of mechanism; pInfo points to the location that receives the mechanism
 * information.
 */
void test_get_mechanism_info_good(void **state) {

    test_info *ti = test_info_from_state(state);
    CK_SLOT_ID slot_id = ti->slot;

    CK_MECHANISM_INFO mech_info;

    CK_RV rv = C_GetMechanismInfo(slot_id, CKM_AES_KEY_GEN, &mech_info);
    assert_int_equal(rv, CKR_OK);

    assert_int_equal(mech_info.ulMaxKeySize, 512);
    assert_int_equal(mech_info.ulMinKeySize, 128);
    assert_int_equal(mech_info.flags, 0);
}

void test_get_mechanism_info_bad(void **state) {

    test_info *ti = test_info_from_state(state);
    CK_SLOT_ID slot_id = ti->slot;

    CK_MECHANISM_INFO mech_info;

    // Invalid mechanism
    CK_RV rv = C_GetMechanismInfo(slot_id, (CK_ULONG)-10, &mech_info);
    assert_int_equal(rv, CKR_MECHANISM_INVALID);

    // NULL Arguments
    rv = C_GetMechanismInfo(slot_id, CKM_AES_KEY_GEN, NULL);
    assert_int_equal(rv, CKR_ARGUMENTS_BAD);

    // Invalid slot ID
    rv = C_GetMechanismInfo((CK_ULONG)-10, CKM_AES_KEY_GEN, &mech_info);
    assert_int_equal(rv, CKR_SLOT_ID_INVALID);
}

int main() {

    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_get_mechanism_list_good,
                test_setup, test_teardown),
        cmocka_unit_test_setup_teardown(test_get_mechanism_list_bad,
                test_setup, test_teardown),
        cmocka_unit_test_setup_teardown(test_get_mechanism_info_good,
                test_setup, test_teardown),
        cmocka_unit_test_setup_teardown(test_get_mechanism_info_bad,
                test_setup, test_teardown),
    };

    return cmocka_run_group_tests(tests, group_setup, group_teardown);
}
