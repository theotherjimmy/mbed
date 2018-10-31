/* Copyright (c) 2018 ARM Limited
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


/* -------------------------------------- Includes ----------------------------------- */

#include "spm_hal_addresses.h"


/* -------------------------------------- Definition --------------------------------- */

#define SPM_PSOC6_SHARED_MEM_START_ADDR     0x08010000

#define SPM_PSOC6_SECURE_RAM_BASE           0x08000000
#define SPM_PSOC6_SECURE_RAM_LEN            0x10000
#define SPM_PSOC6_SECURE_FLASH_BASE         0x10000000
#define SPM_PSOC6_SECURE_FLASH_LEN          0x78000

#define SPM_PSOC6_NON_SECURE_RAM_BASE       0x08011000
#define SPM_PSOC6_NON_SECURE_RAM_LEN        0x36800
#define SPM_PSOC6_NON_SECURE_FLASH_BASE     0x10080000
#define SPM_PSOC6_NON_SECURE_FLASH_LEN      0x78000


/* -------------------------------------- HAL API ------------------------------------ */

// Shared memory
// -------------

inline uint32_t spm_hal_get_plat_shared_mem_address(void)
{
    return SPM_PSOC6_SHARED_MEM_START_ADDR;
}


// Secure addresses
// ----------------

inline uint32_t spm_hal_get_plat_sec_ram_base(void)
{
    return SPM_PSOC6_SECURE_RAM_BASE;
}

inline size_t spm_hal_get_plat_sec_ram_len(void)
{
    return SPM_PSOC6_SECURE_RAM_LEN;
}

inline uint32_t spm_hal_get_plat_sec_flash_base(void)
{
    return SPM_PSOC6_SECURE_FLASH_BASE;
}

inline size_t spm_hal_get_plat_sec_flash_len(void)
{
    return SPM_PSOC6_SECURE_FLASH_BASE;
}


// Non-Secure addresses
// --------------------

inline uint32_t spm_hal_get_plat_non_sec_ram_base(void)
{
    return SPM_PSOC6_NON_SECURE_RAM_BASE;
}

inline size_t spm_hal_get_plat_non_sec_ram_len(void)
{
    return SPM_PSOC6_NON_SECURE_RAM_LEN;
}

inline uint32_t spm_hal_get_plat_non_sec_flash_base(void)
{
    return SPM_PSOC6_NON_SECURE_FLASH_BASE;
}

inline size_t spm_hal_get_plat_non_sec_flash_len(void)
{
    return SPM_PSOC6_NON_SECURE_FLASH_LEN;
}
