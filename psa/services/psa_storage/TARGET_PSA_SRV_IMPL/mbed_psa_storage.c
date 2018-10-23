#include "mbed_psa_storage.h"
#include "nvstore.h"

static bool validate_file_id(uint32_t file_id)
{
    return ((file_id & ((uint32_t)0xFFFF0000)) == 0);
}

psa_error_t mbed_psa_st_set_impl(uint32_t file_id, const uint8_t *p_data, size_t data_length)
{
    NVStore &nvstore = NVStore::get_instance();
    psa_error_t status = MBED_PSA_ST_SUCCESS;
    if (!validate_file_id(file_id)) {
        return MBED_PSA_ST_ERR_BAD_PARAMS;
    }

    if (nvstore.set((uint16_t)file_id, (uint16_t)data_length, data) == NVSTORE_SUCCESS) {
        status = MBED_PSA_ST_SUCCESS;
    } else {
        status = MBED_PSA_ST_ERR_INTERNAL_ERROR;
    }

    return status;
}

psa_error_t mbed_psa_st_get_impl(uint32_t file_id, uint8_t *p_data, size_t data_size, size_t *p_data_length)
{
    NVStore &nvstore = NVStore::get_instance();
    uint16_t actual_size = 0;
    psa_error_t status = PSA_SUCCESS;

    if (!validate_file_id(file_id)) {
        return MBED_PSA_ST_ERR_BAD_PARAMS;
    }

    int nvstore_status = nvstore.get((uint16_t)file_id, (uint16_t)data_size, p_data, &actual_size);
    switch(nvstore_status) {
        case NVSTORE_SUCCESS:
        {
            status = MBED_PSA_ST_SUCCESS;
            break;
        }
        case NVSTORE_NOT_FOUND:
        {
            status = MBED_PSA_ST_ERR_NOT_FOUND;
            break;
        }
        case NVSTORE_BUFF_TOO_SMALL:
        {
            status = MBED_PSA_ST_ERR_BUFF_TOO_SMALL;
            break;
        }
        default:
        {
            status = MBED_PSA_ST_ERR_INTERNAL_ERROR;
        }
    }

    if (p_data_length != NULL) {
        *p_data_length = (size_t)actual_size;
    }

    return status;
}

psa_error_t mbed_psa_st_get_size_impl(uint32_t file_id, size_t *p_size)
{
    NVStore &nvstore = NVStore::get_instance();
    uint16_t actual_size = 0;
    psa_error_t status = MBED_PSA_ST_SUCCESS;

    if (!validate_file_id(file_id)) {
        return MBED_PSA_ST_ERR_BAD_PARAMS;
    }

    int nvstore_status = nvstore.get_item_size((uint16_t)file_id, &actual_size);
    switch(status) {
        case NVSTORE_SUCCESS:
        {
            status = MBED_PSA_ST_SUCCESS
            break;
        }
        case NVSTORE_NOT_FOUND:
        {
            status = MBED_PSA_ST_ERR_NOT_FOUND;
            break;
        }
        default:
        {
            status = MBED_PSA_ST_ERR_INTERNAL_ERROR;
            break;
        }
    }

    if (p_size != NULL) {
        *p_size = (size_t)actual_size;
    }

    return status;

}

psa_error_t mbed_psa_st_file_exists_impl(uint32_t file_id, bool *p_exists)
{
    size_t item_size = 0;
    bool exists = false;
    psa_error_t status = mbed_psa_st_get_size(file_id, &item_size);
    switch(status) {
        case MBED_PSA_ST_SUCCESS:
            exists = true;
            break;
        case MBED_PSA_ST_ERR_NOT_FOUND:
            status = MBED_PSA_ST_SUCCESS;
            exists = false;
            break;
        default:
            break;
    }
    if ((NULL != p_exists) && (status == MBED_PSA_ST_SUCCESS)) {
        *p_exists = exists;
    }

    return status;
}

psa_error_t mbed_psa_st_erase_impl(uint32_t file_id)
{
    NVStore &nvstore = NVStore::get_instance();
    psa_error_t status = MBED_PSA_ST_SUCCESS;

    if (!validate_file_id(file_id)) {
        return MBED_PSA_ST_ERR_BAD_PARAMS;
    }

    if ((nvstore.remove((uint16_t)file_id)) == NVSTORE_SUCCESS) {
        status = MBED_PSA_ST_SUCCESS;
    } else {
        status = MBED_PSA_ST_ERR_INTERNAL_ERROR;
    }

    return status;
}
