#ifndef MBED_PSA_STORAGE_H
#define MBED_PSA_STORAGE_H

#include <stddef.h>
#include <inttypes.h>
#include <stdbool.h>
#include "psa_defs.h"

#ifdef   __cplusplus
extern "C"
{
#endif // __cplusplus

#define MBED_PSA_ST_SUCCESS            PSA_SUCCESS
#define MBED_PSA_ST_ERR_NOT_FOUND      ((psa_error_t)(-1))
#define MBED_PSA_ST_ERR_NOT_ALLOWED    ((psa_error_t)(-2))
#define MBED_PSA_ST_ERR_BUFF_TOO_SMALL ((psa_error_t)(-3))
#define MBED_PSA_ST_ERR_BAD_PARAMS     ((psa_error_t)(-4))
#define MBED_PSA_ST_ERR_INTERNAL_ERROR ((psa_error_t)(-10))

psa_error_t mbed_psa_st_set(uint32_t file_id, const uint8_t *p_data, size_t data_length);

psa_error_t mbed_psa_st_get(uint32_t file_id, uint8_t *p_data, size_t data_size, size_t *p_data_length);

psa_error_t mbed_psa_st_get_size(uint32_t file_id, size_t *p_size);

psa_error_t mbed_psa_st_file_exists(uint32_t file_id, bool *p_exists);

psa_error_t mbed_psa_st_erase(uint32_t file_id);

#ifdef   __cplusplus
}
#endif // __cplusplus

#endif // MBED_PSA_STORAGE_H
