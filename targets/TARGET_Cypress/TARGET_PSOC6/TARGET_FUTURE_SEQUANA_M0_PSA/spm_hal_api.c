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
#include "cmsis_os2.h"
#include "psa_defs.h"
#include "spm_internal.h"
#include "spm_hal_api.h"

// These implementations are meant to be used only for SPM running on PSoC6 M0+ core.

#define SPM_PSOC6_SECURE_RAM_BASE 0x08000000
#define SPM_PSOC6_SECURE_RAM_LEN  0x10000
#define SPM_PSOC6_SECURE_FLASH_BASE 0x10000000
#define SPM_PSOC6_SECURE_FLASH_LEN  0x80000



bool is_buffer_accessible(const void *ptr, size_t size, spm_partition_t *accessing_partition)
{
    if (NULL == ptr) {
        return false;
    }

    if (size == 0) {
        return true;
    }

    // Check wrap around of ptr + size
    if (((uintptr_t)ptr + size - 1) < (uintptr_t)ptr) {
        return false;
    }

    // Make sure the NSPE is not trying to access the secure ram range
    if (accessing_partition == NULL) {
        if (! ( ( (uintptr_t)ptr >= (uintptr_t)(SPM_PSOC6_SECURE_RAM_BASE + SPM_PSOC6_SECURE_RAM_LEN) ) ||
                ( ((uintptr_t)ptr + size) <= (uintptr_t)SPM_PSOC6_SECURE_RAM_BASE ) )
           ) {
               return false;
           }

        // Make sure the NSPE is not trying to access the secure flash range
        if (! ( ( (uintptr_t)ptr >= (uintptr_t)(SPM_PSOC6_SECURE_FLASH_BASE + SPM_PSOC6_SECURE_FLASH_LEN) ) ||
                ( ((uintptr_t)ptr + size) <= (uintptr_t)SPM_PSOC6_SECURE_FLASH_BASE ) )
           ) {
               return false;
           }
    }

    return true;
}


// TODO - implement memory protection scheme for PSoC 6
void memory_protection_init(const mem_region_t *regions, uint32_t region_count)
{
    PSA_UNUSED(regions);
    PSA_UNUSED(region_count);
}
