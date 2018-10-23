#ifndef MBED_SPM_INIT_API_H
#define MBED_SPM_INIT_API_H

#if defined(TARGET_PSA_TFM)
#include "spm_internal.h"

bool is_buffer_accessible(const void *ptr, size_t size, spm_partition_t *accessing_partition);
void memory_protection_init(const mem_region_t *regions, uint32_t region_count);

#endif // defined(TARGET_PSA_TFM)

#endif // MBED_SPM_INIT_API_H
