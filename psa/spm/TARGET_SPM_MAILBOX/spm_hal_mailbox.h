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

#ifndef __SPM_HAL_MAILBOX_H__
#define __SPM_HAL_MAILBOX_H__


/** @addtogroup SPM
 * The SPM (Secure Partition Manager) is responsible for isolating software in partitions,
 * managing the execution of software within partitions, and providing IPC between partitions.
 * @{
 */


#include <stdint.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup HAL-Mailbox
 *  The HAL functions to mailbox configuration.
 * @{
 */


/* -------------------------------------- ARM API --------------------------- */


/**
 * @brief Wakeup mailbox dispatcher thread
 * 
 * This function is implemented by ARM and expected to be called by target
 * specific Inter-Processor-Communication logic on mailbox interrupt handler.
 * 
 */
void spm_mailbox_irq_callback(void);



/* ---------------------------------- HAL-Mailbox API ----------------------- */


/**
 * @brief Notify the peer processor about a general event occurrence.
 * 
 * Wakeup the peer processor waiting on the mailbox driver event.
 * 
 * @note The functions below should be implemented by target specific code.
 */
void spm_hal_mailbox_notify(void);



/** @}*/ // end of HAL-Mailbox group

#ifdef __cplusplus
}
#endif

/** @}*/ // end of SPM group

#endif  // __SPM_HAL_MAILBOX_H__
