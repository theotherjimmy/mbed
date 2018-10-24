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
#include "secure_add.h"

using namespace utest::v1;

static void secure_add_test()
{
    TEST_ASSERT_EQUAL(2, secure_add(1, 1));
}


Case cases[] = {
    Case("Secure Add - Stupid",  secure_add_test),
};

utest::v1::status_t greentea_test_setup(const size_t number_of_cases)
{
#ifndef NO_GREENTEA
    GREENTEA_SETUP(20, "default_auto");
#endif
    return greentea_test_setup_handler(number_of_cases);
}

Specification specification(greentea_test_setup, cases, greentea_test_teardown_handler);

int main()
{
    return !Harness::run(specification);
}
