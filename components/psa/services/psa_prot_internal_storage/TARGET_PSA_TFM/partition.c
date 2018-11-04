// -------------------------------------- Includes -----------------------------------

#include <string.h>
#include "cmsis_os2.h"
#include "spm_server.h"
#include "spm_panic.h"
#include "psa_its_partition.h"
#include "psa_prot_internal_storage.h"
#include "pits_impl.h"

#ifdef   __cplusplus
extern "C"
{
#endif

typedef psa_error_t (*SignalHandler)(psa_msg_t *);

static psa_error_t storage_set(psa_msg_t *msg)
{
    uint32_t key = 0;
    void *data = NULL;
    uint32_t alloc_size = msg->in_size[1] + PITS_HEADER_SIZE;
    psa_its_create_flags_t flags = 0;

    if ((msg->in_size[0] != sizeof(key)) || (msg->in_size[2] != sizeof(flags))) {
        return PSA_DROP_CONNECTION;
    }

    if (psa_read(msg->handle, 0, &key, sizeof(key)) != sizeof(key)) {
        return PSA_DROP_CONNECTION;
    }

    if (psa_read(msg->handle, 2, &flags, sizeof(flags)) != sizeof(flags)) {
        return PSA_DROP_CONNECTION;
    }

    data = malloc(alloc_size);
    if (data == NULL) {
        return PSA_ITS_ERROR_STORAGE_FAILURE;
    }

    add_headers_to_record(data, flags, psa_identity(msg->handle));


    if (psa_read(msg->handle, 1, PITS_DATA_PTR(data), msg->in_size[1]) != msg->in_size[1]) {
        free(data);
        return PSA_ITS_ERROR_STORAGE_FAILURE;
    }

    psa_its_status_t status = psa_its_set_impl(key, alloc_size, data, flags);

    memset(data, 0, alloc_size);
    free(data);
    return status;
}

static psa_error_t storage_get(psa_msg_t *msg)
{
    uint32_t key = 0;
    uint32_t offset = 0;

    if ((msg->in_size[0] != sizeof(key)) || (msg->in_size[1] != sizeof(offset))) {
        return PSA_DROP_CONNECTION;
    }

    if (psa_read(msg->handle, 0, &key, sizeof(key)) != sizeof(key)) {
        return PSA_DROP_CONNECTION;
    }

    if (psa_read(msg->handle, 1, &offset, sizeof(offset)) != sizeof(offset)) {
        return PSA_DROP_CONNECTION;
    }

    uint8_t *data = (uint8_t *)malloc(msg->out_size[0]);
    if (data == NULL) {
        return PSA_ITS_ERROR_STORAGE_FAILURE;
    }

    psa_its_status_t status = psa_its_get_impl(key, offset, msg->out_size[0], data, psa_identity(msg->handle));
    if (status == PSA_ITS_SUCCESS) {
        psa_write(msg->handle, 0, data, msg->out_size[0]);
    }

    memset(data, 0, msg->out_size[0]);
    free(data);
    return status;
}

static psa_error_t storage_info(psa_msg_t *msg)
{
    struct psa_its_info_t info = { 0 };
    uint32_t key = 0;

    if ((msg->in_size[0] != sizeof(key)) || (msg->out_size[0] != sizeof(info))) {
        return PSA_DROP_CONNECTION;
    }

    if (psa_read(msg->handle, 0, &key, sizeof(key)) != sizeof(key)) {
        return PSA_DROP_CONNECTION;
    }

    psa_its_status_t status = psa_its_get_info_impl(key, &info, psa_identity(msg->handle));
    if (status == PSA_ITS_SUCCESS) {
        psa_write(msg->handle, 0, &info, msg->out_size[0]);
    }

    return status;
}

static psa_error_t storage_remove(psa_msg_t *msg)
{
    uint32_t key = 0;

    if (msg->in_size[0] != sizeof(key)) {
        return PSA_DROP_CONNECTION;
    }

    if (psa_read(msg->handle, 0, &key, sizeof(key)) != sizeof(key)) {
        return PSA_DROP_CONNECTION;
    }

    return psa_its_remove_impl(key, psa_identity(msg->handle));
}

static void message_handler(psa_msg_t *msg, SignalHandler handler)
{
    psa_error_t status = PSA_SUCCESS;
    switch (msg->type) {
        case PSA_IPC_CONNECT: //fallthrough
        case PSA_IPC_DISCONNECT:
        {
            break;
        }
        case PSA_IPC_CALL:
        {
            status = handler(msg);
            break;
        }
        default:
        {
            SPM_PANIC("Unexpected message type %d!", (int)(msg->type));
            break;
        }
    }
    psa_reply(msg->handle, status);
}

void pits_entry(void *ptr)
{
    uint32_t signals = 0;
    psa_msg_t msg = {0};

    while (1) {
        signals = psa_wait_any(PSA_BLOCK);
        if ((signals & PSA_ITS_SET_MSK) != 0) {
            psa_get(PSA_ITS_SET_MSK, &msg);
            message_handler(&msg, storage_set);
        }
        if ((signals & PSA_ITS_GET_MSK) != 0) {
            psa_get(PSA_ITS_GET_MSK, &msg);
            message_handler(&msg, storage_get);
        }
        if ((signals & PSA_ITS_INFO_MSK) != 0) {
            psa_get(PSA_ITS_INFO_MSK, &msg);
            message_handler(&msg, storage_info);
        }
        if ((signals & PSA_ITS_REMOVE_MSK) != 0) {
            psa_get(PSA_ITS_REMOVE_MSK, &msg);
            message_handler(&msg, storage_remove);
        }
    }
}

#ifdef   __cplusplus
}
#endif
