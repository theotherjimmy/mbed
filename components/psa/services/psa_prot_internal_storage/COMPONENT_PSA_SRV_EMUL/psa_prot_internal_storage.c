#include "psa_prot_internal_storage.h"
#include "pits_impl.h"

psa_its_status_t psa_its_set(uint32_t uid, uint32_t data_length, const void *p_data, psa_its_create_flags_t create_flags)
{
    if (p_data == NULL) {
        return PSA_ITS_ERROR_BAD_POINTER;
    }

    uint8_t *record = (uint8_t *)malloc(data_length + PITS_HEADER_SIZE);
    if (record == NULL) {
        return PSA_ITS_ERROR_STORAGE_FAILURE;
    }

    add_headers_to_record(record, create_flags, PITS_ACL_INVALID_SOURCE);
    memcpy(PITS_DATA_PTR(record), p_data, data_length);
    psa_its_status_t res = psa_its_set_impl(uid, data_length + PITS_HEADER_SIZE, record, create_flags);
    memset(record, 0, data_length + PITS_HEADER_SIZE);
    free(record);
    return res;
}

psa_its_status_t psa_its_get(uint32_t uid, uint32_t data_offset,  uint32_t data_length, void *p_data)
{
    if (p_data == NULL) {
        return PSA_ITS_ERROR_BAD_POINTER;
    }

    return psa_its_get_impl(uid, data_offset, data_length, p_data, PITS_ACL_INVALID_SOURCE);
}

psa_its_status_t psa_its_get_info(uint32_t uid, struct psa_its_info_t *p_info)
{
    if (p_info == NULL) {
        return PSA_ITS_ERROR_BAD_POINTER;
    }

    return psa_its_get_info_impl(uid, p_info, PITS_ACL_INVALID_SOURCE);
}

psa_its_status_t psa_its_remove(uint32_t uid)
{
    return psa_its_remove_impl(uid, PITS_ACL_INVALID_SOURCE);
}
