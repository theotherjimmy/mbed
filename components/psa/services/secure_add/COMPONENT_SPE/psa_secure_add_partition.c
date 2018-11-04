/* Copyright (c) 2017 ARM Limited
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

/***********************************************************************************************************************
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * THIS FILE IS AN AUTO-GENERATED FILE - DO NOT MODIFY IT.
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 **********************************************************************************************************************/

#include "cmsis.h"
#include "mbed_toolchain.h" /* For using MBED_ALIGN macro */
#include "rtx_os.h"
#include "spm_panic.h"
#include "spm_internal.h"
#include "psa_secure_add_partition.h"
#include "psa_secure_add_ifs.h"


/* Threads stacks */
MBED_ALIGN(8) uint8_t secure_add_thread_stack[1024] = {0};

/* Threads control blocks */
osRtxThread_t secure_add_thread_cb = {0};

/* Thread attributes - for thread initialization */
osThreadAttr_t secure_add_thread_attr = {
    .name = "secure_add",
    .attr_bits = 0,
    .cb_mem = &secure_add_thread_cb,
    .cb_size = sizeof(secure_add_thread_cb),
    .stack_mem = secure_add_thread_stack,
    .stack_size = 1024,
    .priority = osPriorityNormal,
    .tz_module = 0,
    .reserved = 0
    };

spm_rot_service_t secure_add_rot_services[SECURE_ADD_ROT_SRV_COUNT] = {
    {
        .sid = PSA_SECURE_ADD,
        .mask = PSA_SECURE_ADD_MSK,
        .partition = NULL,
        .min_version = 1,
        .min_version_policy = PSA_MINOR_VERSION_POLICY_RELAXED,
        .allow_nspe = true,
        .queue = {
            .head = NULL,
            .tail = NULL
        }
    },
};


static osRtxMutex_t secure_add_mutex = {0};
static const osMutexAttr_t secure_add_mutex_attr = {
    .name = "secure_add_mutex",
    .attr_bits = osMutexRecursive | osMutexPrioInherit | osMutexRobust,
    .cb_mem = &secure_add_mutex,
    .cb_size = sizeof(secure_add_mutex),
};


extern void secure_add_entry(void *ptr);

void secure_add_init(spm_partition_t *partition)
{
    if (NULL == partition) {
        SPM_PANIC("partition is NULL!\n");
    }

    partition->mutex = osMutexNew(&secure_add_mutex_attr);
    if (NULL == partition->mutex) {
        SPM_PANIC("Failed to create mutex for secure partition secure_add!\n");
    }

    for (uint32_t i = 0; i < SECURE_ADD_ROT_SRV_COUNT; ++i) {
        secure_add_rot_services[i].partition = partition;
    }
    partition->rot_services = secure_add_rot_services;

    partition->thread_id = osThreadNew(secure_add_entry, NULL, &secure_add_thread_attr);
    if (NULL == partition->thread_id) {
        SPM_PANIC("Failed to create start main thread of partition secure_add!\n");
    }
}
