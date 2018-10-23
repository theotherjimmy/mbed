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

#ifndef PSA_SECURE_ADD_PARTITION_H
#define PSA_SECURE_ADD_PARTITION_H

#define SECURE_ADD_ID 3

#define SECURE_ADD_ROT_SRV_COUNT (1UL)
#define SECURE_ADD_EXT_ROT_SRV_COUNT (0UL)

/* SECURE_ADD event flags */
#define SECURE_ADD_RESERVED1_POS (1UL)
#define SECURE_ADD_RESERVED1_MSK (1UL << SECURE_ADD_RESERVED1_POS)

#define SECURE_ADD_RESERVED2_POS (2UL)
#define SECURE_ADD_RESERVED2_MSK (1UL << SECURE_ADD_RESERVED2_POS)



#define PSA_SECURE_ADD_MSK_POS (4UL)
#define PSA_SECURE_ADD_MSK (1UL << PSA_SECURE_ADD_MSK_POS)

#define SECURE_ADD_WAIT_ANY_SID_MSK (\
    PSA_SECURE_ADD_MSK)

/*
#define SECURE_ADD_WAIT_ANY_MSK (\
    SECURE_ADD_WAIT_ANY_SID_MSK) | \
    PSA_DOORBELL)
*/


#endif // PSA_SECURE_ADD_PARTITION_H
