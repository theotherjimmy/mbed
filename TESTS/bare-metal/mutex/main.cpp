/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "greentea-client/test_env.h"
#include "mbed.h"

#ifdef MBED_CONF_RTOS_PRESENT
#error [NOT_SUPPORTED] Bare metal tests do not test the RTOS
#endif

int main(void)
{
    GREENTEA_SETUP(10, "default_auto");
    Mutex test_mut = Mutex();
    test_mut.lock();
    bool result = true;
    test_mut.unlock();
    GREENTEA_TESTSUITE_RESULT(result);
}
