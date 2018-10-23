#include "mbed_psa_storage.h"

psa_error_t mbed_psa_st_set(uint32_t file_id, const uint8_t *p_data, size_t data_length)
{
    return mbed_psa_st_set_impl(file_id, p_data, data_length);
}

psa_error_t mbed_psa_st_get(uint32_t file_id, uint8_t *p_data, size_t data_size, size_t *p_data_length)
{
    return mbed_psa_st_get_impl(file_id, p_data, data_size, p_data_length);
}

psa_error_t mbed_psa_st_get_size(uint32_t file_id, size_t *p_size)
{
    return mbed_psa_st_get_size_impl(file_id, p_size);
}

psa_error_t mbed_psa_st_file_exists(uint32_t file_id, bool *p_exists)
{
    return mbed_psa_st_file_exists_impl(file_id, p_exists);
}

psa_error_t mbed_psa_st_erase(uint32_t file_id)
{
    return mbed_psa_st_erase_impl(file_id);
}
