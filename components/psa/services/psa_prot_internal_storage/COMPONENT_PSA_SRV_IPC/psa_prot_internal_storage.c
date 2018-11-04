#include "spm_client.h"
#include "psa_prot_internal_storage.h"
#include "psa_its_ifs.h"

psa_its_status_t psa_its_set(uint32_t uid, uint32_t data_length, const void *p_data, psa_its_create_flags_t create_flags)
{
    psa_invec_t msg[3] = {
        { &uid, sizeof(uid) },
        { p_data, data_length },
        { &create_flags, sizeof(create_flags) }
    };

    psa_handle_t conn = psa_connect(PSA_ITS_SET, 1);
    if (conn <= PSA_NULL_HANDLE) {
        return PSA_ITS_ERROR_STORAGE_FAILURE;
    }

    psa_error_t status = psa_call(conn, msg, 3, NULL, 0);
    if (status == PSA_DROP_CONNECTION) {
        status = PSA_ITS_ERROR_STORAGE_FAILURE;
    }

    psa_close(conn);
    return status;
}

psa_its_status_t psa_its_get(uint32_t uid, uint32_t data_offset,  uint32_t data_length, void *p_data)
{
    psa_invec_t msg[2] = {
        { &uid, sizeof(uid) },
        { &data_offset, sizeof(data_offset) }
    };
    psa_outvec_t resp = { p_data, data_length };

    psa_handle_t conn = psa_connect(PSA_ITS_GET, 1);
    if (conn <= PSA_NULL_HANDLE) {
        return PSA_ITS_ERROR_STORAGE_FAILURE;
    }

    psa_error_t status = psa_call(conn, msg, 2, &resp, 1);

    if (status == PSA_DROP_CONNECTION) {
        status = PSA_ITS_ERROR_STORAGE_FAILURE;
    }

    psa_close(conn);
    return status;
}

psa_its_status_t psa_its_get_info(uint32_t uid, struct psa_its_info_t *p_info)
{
    struct psa_its_info_t info = { 0 };
    psa_invec_t msg = { &uid, sizeof(uid) };
    psa_outvec_t resp = { &info, sizeof(info) };
    psa_handle_t conn = psa_connect(PSA_ITS_INFO, 1);
    if (conn <= PSA_NULL_HANDLE) {
        return PSA_ITS_ERROR_STORAGE_FAILURE;
    }

    psa_error_t status = psa_call(conn, &msg, 1, &resp, 1);
    if (NULL != p_info) {
        *p_info = info;
    }

    if (status == PSA_DROP_CONNECTION) {
        status = PSA_ITS_ERROR_STORAGE_FAILURE;
    }

    psa_close(conn);
    return status;
}

psa_its_status_t psa_its_remove(uint32_t uid)
{
    psa_invec_t msg = { &uid, sizeof(uid) };
    psa_handle_t conn = psa_connect(PSA_ITS_REMOVE, 1);
    if (conn <= PSA_NULL_HANDLE) {
        return PSA_ITS_ERROR_STORAGE_FAILURE;
    }

    psa_error_t status = psa_call(conn, &msg, 1, NULL, 0);
    if (status == PSA_DROP_CONNECTION) {
        status = PSA_ITS_ERROR_STORAGE_FAILURE;
    }

    psa_close(conn);
    return status;
}
