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

#ifndef __SPM_HAL_ADDRESSES_H__
#define __SPM_HAL_ADDRESSES_H__


/** @addtogroup SPM
 * The SPM (Secure Partition Manager) is responsible for isolating software in 
 * partitions,managing the execution of software within partitions, and 
 * providing IPC between partitions.
 * @{
 */



#include <stdint.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup HAL-Addresses
 * The HAL functions to get platform's specific addresses for 
 * secure/non-secure partitioning.
 * @{
 */


/* --------------------------------- HAL Addresses API ---------------------- */


// Shared memory
// -------------

/**
 * @brief Get platform's start address for the secure/non-secure shared memory.
 *
 * The function must be implemented by target specific code.
 *
 * @note The shared memory address:
 * <ul>
 *   <li> must be aligned with linker script values.
 *   <li> must have RW permissions from secure and non-secure cores.
 *   <li> must be 4 bytes aligned.
 * </ul>
 *
 * @return The 32 bit address of the secure/non-secure shared memory.
 */
uint32_t spm_hal_get_plat_shared_mem_address(void);

/**
 * @brief Get platform's secure/non-secure shared memory size
 *
 * The function must be implemented by target specific code.
 *
 * @note The shared memory size value MUST be aligned with linker script value.
 *
 * @return The platform's secure/non-secure shared memory size.
 */
size_t spm_hal_get_plat_shared_mem_size(void);

// Secure addresses
// ----------------

/**
 * Get platform's base address of the secure RAM.
 *
 * The function must be implemented by target specific code.
 *
 * @note The base address:
 * <ul>
 *   <li> must be aligned with linker script values.
 *   <li> must have RW permissions only from secure core.
 *   <li> must be 4 bytes aligned.
 * </ul>
 *
 * @return The 32 bit base address of the secure RAM.
 */
uint32_t spm_hal_get_plat_sec_ram_base(void);

/**
 * Get platform's length in bytes of the secure RAM.
 *
 * The function must be implemented by target specific code.
 *
 * @note The length must be aligned with linker script values.
 *
 * @return The platform's length in bytes of the secure RAM.
 */
size_t spm_hal_get_plat_sec_ram_len(void);

/**
 * Get platform's base address of the secure FLASH.
 *
 * The function must be implemented by target specific code.
 *
 * @note The base address:
 * <ul>
 *   <li> must be aligned with linker script values.
 *   <li> must have read, write and execute permissions from secure core.
 *   <li> must be 4 bytes aligned.
 * </ul>
 *
 * @return The 32 bit base address of the secure FLASH.
 */
uint32_t spm_hal_get_plat_sec_flash_base(void);

/**
 * Get platform's length in bytes of the secure FLASH.
 *
 * The function must be implemented by target specific code.
 *
 * @note The length must aligned with linker script values.
 *
 * @return The platform's length in bytes of the secure FLASH.
 */
size_t spm_hal_get_plat_sec_flash_len(void);


// Non-Secure addresses
// --------------------

/**
 * Get platform's base address of the non-secure RAM.
 *
 * The function must be implemented by target specific code.
 *
 * @note The base address:
 * <ul>
 *   <li> must be aligned with linker script values.
 *   <li> must have RW permissions from non-secure and secure cores.
 *   <li> must be 4 bytes aligned.
 * </ul>
 *
 * @return The 32 bit base address of the non-secure RAM.
 */
uint32_t spm_hal_get_plat_non_sec_ram_base(void);

/**
 * Get platform's length in bytes of the non-secure RAM.
 *
 * The function must be implemented by target specific code.
 *
 * @note The length must be aligned with linker script values.
 *
 * @return The platform's length in bytes of the non-secure RAM.
 */
size_t spm_hal_get_plat_non_sec_ram_len(void);

/**
 * Get platform's base address of the non-secure FLASH.
 *
 * The function must be implemented by target specific code.
 *
 * @note The base address:
 * <ul>
 *   <li> must be aligned with linker script values.
 *   <li> must have read permissions from non-secure and secure cores.
 *   <li> may have write permissions from non-secure and secure cores.
 *   <li> must have execute permissions from non-secure core.
 *   <li> be 4 bytes aligned.
 * </ul>
 *
 * @return The 32 bit base address of the non-secure FLASH.
 */
uint32_t spm_hal_get_plat_non_sec_flash_base(void);

/**
 * Get platform's length in bytes of the non-secure FLASH.
 *
 * The function must be implemented by target specific code.
 *
 * @note The length must be aligned with linker script values.
 * 
 * @return The platform's length in bytes of the non-secure FLASH.
 */
size_t spm_hal_get_plat_non_sec_flash_len(void);



/** @}*/ // end of HAL-Addresses group

#ifdef __cplusplus
}
#endif

/** @}*/ // end of SPM group

#endif  // __SPM_HAL_ADDRESSES_H__
