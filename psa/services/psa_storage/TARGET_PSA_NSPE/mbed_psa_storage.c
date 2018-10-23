#include "spm_client.h"
#include "mbed_psa_storage.h"
#include "psa_secure_storage_ifs.h"

static bool validate_file_id(uint32_t file_id)
{
    return ((file_id & ((uint32_t)0xFFFF0000)) == 0);
}

psa_error_t mbed_psa_st_set(uint32_t file_id, const uint8_t *p_data, size_t data_length)
{
    if (!validate_file_id(file_id)) {
        return MBED_PSA_ST_ERR_BAD_PARAMS;
    }

    uint16_t key = (uint16_t)file_id;
    psa_invec_t msg[2] = {
        { &key, sizeof(key) },
        { p_data, data_length }
    };
    psa_handle_t conn = psa_connect(PSA_ST_SET, 1);
    if (conn <= PSA_NULL_HANDLE) {
        return MBED_PSA_ST_ERR_INTERNAL_ERROR;
    }

    psa_error_t status = psa_call(conn, msg, 2, NULL, 0);
    if (status == PSA_DROP_CONNECTION) {
        status = MBED_PSA_ST_ERR_BAD_PARAMS;
    }

    psa_close(conn);
    return status;
}

psa_error_t mbed_psa_st_get(uint32_t file_id, uint8_t *p_data, size_t data_size, size_t *p_data_length)
{
    if (!validate_file_id(file_id)) {
        return MBED_PSA_ST_ERR_BAD_PARAMS;
    }

    uint16_t key = (uint16_t)file_id;
    uint16_t actual_size_written = 0;
    psa_invec_t msg = { &key, sizeof(key) };
    psa_outvec_t resp[2] = {
        { p_data, data_size },
        { &actual_size_written, sizeof(actual_size_written) }
    };
    psa_handle_t conn = psa_connect(PSA_ST_GET, 1);
    if (conn <= PSA_NULL_HANDLE) {
        return MBED_PSA_ST_ERR_INTERNAL_ERROR;
    }

    psa_error_t status = psa_call(conn, &msg, 1, resp, 2);
    if (NULL != p_data_length) {
        *p_data_length = (size_t)actual_size_written;
    }

    if (status == PSA_DROP_CONNECTION) {
        status = MBED_PSA_ST_ERR_BAD_PARAMS;
    }

    psa_close(conn);
    return status;
}

psa_error_t mbed_psa_st_get_size(uint32_t file_id, size_t *p_size)
{
    if (!validate_file_id(file_id)) {
        return MBED_PSA_ST_ERR_BAD_PARAMS;
    }

    uint16_t key = (uint16_t)file_id;
    uint16_t item_size = 0;
    psa_invec_t msg = { &key, sizeof(key) };
    psa_outvec_t resp = { &item_size, sizeof(item_size) };
    psa_handle_t conn = psa_connect(PSA_ST_SIZE, 1);
    if (conn <= PSA_NULL_HANDLE) {
        return MBED_PSA_ST_ERR_INTERNAL_ERROR;
    }

    psa_error_t status = psa_call(conn, &msg, 1, &resp, 1);
    if (NULL != p_size) {
        *p_size = item_size;
    }

    if (status == PSA_DROP_CONNECTION) {
        status = MBED_PSA_ST_ERR_BAD_PARAMS;
    }

    psa_close(conn);
    return status;
}

psa_error_t mbed_psa_st_file_exists(uint32_t file_id, bool *p_exists)
{
    if (!validate_file_id(file_id)) {
        return MBED_PSA_ST_ERR_BAD_PARAMS;
    }

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

psa_error_t mbed_psa_st_erase(uint32_t file_id)
{
    if (!validate_file_id(file_id)) {
        return MBED_PSA_ST_ERR_BAD_PARAMS;
    }

    uint16_t key = (uint16_t)file_id;
    psa_invec_t msg = { &key, sizeof(key) };
    psa_handle_t conn = psa_connect(PSA_ST_ERASE, 1);
    if (conn <= PSA_NULL_HANDLE) {
        return MBED_PSA_ST_ERR_INTERNAL_ERROR;
    }

    psa_error_t status = psa_call(conn, &msg, 1, NULL, 0);
    if (status == PSA_DROP_CONNECTION) {
        status = MBED_PSA_ST_ERR_BAD_PARAMS;
    }

    psa_close(conn);
    return status;
}
