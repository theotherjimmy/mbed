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

#ifndef PSA_ITS_PARTITION_H
#define PSA_ITS_PARTITION_H

#define ITS_ID 10

#define ITS_ROT_SRV_COUNT (4UL)
#define ITS_EXT_ROT_SRV_COUNT (0UL)

/* ITS event flags */
#define ITS_RESERVED1_POS (1UL)
#define ITS_RESERVED1_MSK (1UL << ITS_RESERVED1_POS)

#define ITS_RESERVED2_POS (2UL)
#define ITS_RESERVED2_MSK (1UL << ITS_RESERVED2_POS)



#define PSA_ITS_GET_MSK_POS (4UL)
#define PSA_ITS_GET_MSK (1UL << PSA_ITS_GET_MSK_POS)
#define PSA_ITS_SET_MSK_POS (5UL)
#define PSA_ITS_SET_MSK (1UL << PSA_ITS_SET_MSK_POS)
#define PSA_ITS_INFO_MSK_POS (6UL)
#define PSA_ITS_INFO_MSK (1UL << PSA_ITS_INFO_MSK_POS)
#define PSA_ITS_REMOVE_MSK_POS (7UL)
#define PSA_ITS_REMOVE_MSK (1UL << PSA_ITS_REMOVE_MSK_POS)

#define ITS_WAIT_ANY_SID_MSK (\
    PSA_ITS_GET_MSK | \
    PSA_ITS_SET_MSK | \
    PSA_ITS_INFO_MSK | \
    PSA_ITS_REMOVE_MSK)

/*
#define ITS_WAIT_ANY_MSK (\
    ITS_WAIT_ANY_SID_MSK) | \
    PSA_DOORBELL)
*/


#endif // PSA_ITS_PARTITION_H
