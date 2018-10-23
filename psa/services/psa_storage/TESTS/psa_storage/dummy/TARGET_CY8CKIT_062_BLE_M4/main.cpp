/*
* Copyright (c) 2018 ARM Limited. All rights reserved.
* SPDX-License-Identifier: Apache-2.0
* Licensed under the Apache License, Version 2.0 (the License); you may
* not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an AS IS BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/



#include "greentea-client/test_env.h"
#include "unity/unity.h"
#include "utest/utest.h"
#include "mbed_psa_storage.h"

using namespace utest::v1;

#define TEST_BUFF_SIZE 16

static void psa_storage_test()
{
    psa_error_t status = PSA_SUCCESS;
    uint8_t write_buff[TEST_BUFF_SIZE] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    uint8_t read_buff[TEST_BUFF_SIZE] = {0};
    bool exists = true;
    size_t read_size = 0;
    memset(read_buff, 0, TEST_BUFF_SIZE);

    status = mbed_psa_st_file_exists(5, &exists);
    TEST_ASSERT_EQUAL(PSA_SUCCESS, status);
    TEST_ASSERT_EQUAL(exists, false);

    status = mbed_psa_st_set(5, write_buff, TEST_BUFF_SIZE);
    TEST_ASSERT_EQUAL(PSA_SUCCESS, status);

    status = mbed_psa_st_file_exists(5, &exists);
    TEST_ASSERT_EQUAL(PSA_SUCCESS, status);
    TEST_ASSERT_EQUAL(exists, true);

    status = mbed_psa_st_get_size(5, &read_size);
    TEST_ASSERT_EQUAL(PSA_SUCCESS, status);
    TEST_ASSERT_EQUAL(read_size, TEST_BUFF_SIZE);

    read_size = 0;
    status = mbed_psa_st_get(5, read_buff, TEST_BUFF_SIZE, &read_size);
    TEST_ASSERT_EQUAL(PSA_SUCCESS, status);
    TEST_ASSERT_EQUAL(read_size, TEST_BUFF_SIZE);
    TEST_ASSERT_EQUAL_MEMORY(write_buff, read_buff, TEST_BUFF_SIZE);

    status = mbed_psa_st_erase(5);
    TEST_ASSERT_EQUAL(PSA_SUCCESS, status);

    status = mbed_psa_st_file_exists(5, &exists);
    TEST_ASSERT_EQUAL(PSA_SUCCESS, status);
    TEST_ASSERT_EQUAL(exists, false);
}


Case cases[] = {
    Case("PSA_Storage - Basic",  psa_storage_test),
};

utest::v1::status_t greentea_test_setup(const size_t number_of_cases)
{
    GREENTEA_SETUP(120, "default_auto");
    return greentea_test_setup_handler(number_of_cases);
}

Specification specification(greentea_test_setup, cases, greentea_test_teardown_handler);

int main()
{
    return !Harness::run(specification);
}
