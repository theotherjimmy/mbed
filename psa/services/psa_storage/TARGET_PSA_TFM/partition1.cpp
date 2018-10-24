// -------------------------------------- Includes -----------------------------------

#include "cmsis_os2.h"
#include "spm_server.h"
#include "nvstore.h"
#include "psa_secure_storage_partition.h"
#include "mbed_psa_storage.h"

#ifdef   __cplusplus
extern "C"
{
#endif

typedef int32_t psa_nvstore_source_t;
#define PSA_NVSTORE_ACL_SOURCE_SIZE (sizeof(psa_nvstore_source_t))
#define PSA_NVSTORE_ACL_INVALID_SOURCE ((psa_nvstore_source_t)(0))
#define PSA_NVSTORE_DATA_PTR(ptr) ((void *)(((uintptr_t)ptr) + ((uintptr_t)PSA_NVSTORE_ACL_SOURCE_SIZE)))
typedef psa_error_t (*SignalHandler)(psa_msg_t *, NVStore &);

static psa_nvstore_source_t get_item_source(void *data)
{
    psa_nvstore_source_t res = 0;
    memcpy(&res, data, sizeof(res));
    return res;
}

static void set_item_source(void *data, psa_nvstore_source_t source)
{
    memcpy(data, &source, sizeof(source));
}

// Gets the owner partition ID of the key requested
static int get_key_source(uint16_t key, NVStore &instance, psa_nvstore_source_t *source)
{
    uint16_t actual_size = 0;
    int status = instance.get_item_size(key, actual_size);
    if (status == NVSTORE_SUCCESS)
    {
        void *data = malloc(actual_size);
        if (data == NULL) {
            return MBED_PSA_ST_ERR_INTERNAL_ERROR;
        }

        status = instance.get(key, actual_size, data, actual_size);
        if (status == NVSTORE_SUCCESS) {
            *source = get_item_source(data);
        }

        memset(data, 0, actual_size);
        free(data);
    } else if (status == NVSTORE_NOT_FOUND) {
        status = NVSTORE_SUCCESS;
    }

    return status;
}

static psa_error_t storage_get(psa_msg_t *msg, NVStore &nvstore)
{
    int status = NVSTORE_SUCCESS;
    uint16_t key = 0;
    uint16_t actual_size = 0;
    size_t alloc_size = msg->out_size[0] + PSA_NVSTORE_ACL_SOURCE_SIZE;
    bool write_key = false;
    bool write_size = false;
    void *data = NULL;
    if ((msg->in_size[0] != sizeof(key)) || (msg->out_size[1] != sizeof(actual_size))) {
        return PSA_DROP_CONNECTION;
    }

    if (psa_read(msg->handle, 0, &key, sizeof(key)) != sizeof(key)) {
        return PSA_DROP_CONNECTION;
    }

    psa_nvstore_source_t msg_source = psa_identity(msg->handle);
    psa_nvstore_source_t key_source = 0;

    if (get_key_source(key, nvstore, &key_source) != PSA_SUCCESS) {
        return MBED_PSA_ST_ERR_INTERNAL_ERROR;
    }

    data = malloc(alloc_size);
    if (data == NULL) {
        return MBED_PSA_ST_ERR_INTERNAL_ERROR;
    }

    status = nvstore.get(key, (uint16_t)alloc_size, data, actual_size);
    switch(status) {
        case NVSTORE_SUCCESS:
        {
            if (msg_source == key_source) {
                write_key = true;
                write_size = true;
            } else {
                status = MBED_PSA_ST_ERR_NOT_ALLOWED;
            }
            break;
        }
        case NVSTORE_NOT_FOUND:
        {
            status = MBED_PSA_ST_ERR_NOT_FOUND;
            break;
        }
        case NVSTORE_BUFF_TOO_SMALL:
        {
            if (msg_source == key_source) {
                write_size = true;
                status = MBED_PSA_ST_ERR_BUFF_TOO_SMALL;
            } else {
                status = MBED_PSA_ST_ERR_NOT_ALLOWED;
            }
            break;
        }
        default:
        {
            status = MBED_PSA_ST_ERR_INTERNAL_ERROR;
        }
    }

    if (write_key) {
        psa_write(msg->handle, 0, PSA_NVSTORE_DATA_PTR(data), msg->out_size[0]);
    }

    if (write_size) {
        actual_size -= (uint16_t)PSA_NVSTORE_ACL_SOURCE_SIZE;
        psa_write(msg->handle, 1, &actual_size, sizeof(actual_size));
    }

    memset(data, 0, alloc_size);
    free(data);
    return status;
}

static psa_error_t storage_size(psa_msg_t *msg, NVStore &nvstore)
{
    int status = NVSTORE_SUCCESS;
    uint16_t key = 0;
    uint16_t item_size = 0;
    bool write_size = false;
    if ((msg->in_size[0] != sizeof(key)) || (msg->out_size[0] != sizeof(item_size))) {
        return PSA_DROP_CONNECTION;
    }

    if (psa_read(msg->handle, 0, &key, sizeof(key)) != sizeof(key)) {
        return PSA_DROP_CONNECTION;
    }

    psa_nvstore_source_t source = 0;
    if (get_key_source(key, nvstore, &source) != PSA_SUCCESS) {
        return MBED_PSA_ST_ERR_INTERNAL_ERROR;
    }

    status = nvstore.get_item_size(key, item_size);
    switch(status) {
        case NVSTORE_SUCCESS:
        {
            if (source == psa_identity(msg->handle)) {
                item_size -= (uint16_t)PSA_NVSTORE_ACL_SOURCE_SIZE;
                write_size = true;

            } else {
                status = NVSTORE_OPERATION_NOT_ALLOWED;
            }
            break;
        }
        case NVSTORE_NOT_FOUND:
        {
            item_size = 0;
            write_size = true;
            status = MBED_PSA_ST_ERR_NOT_FOUND;
            break;
        }
        default:
        {
            status = MBED_PSA_ST_ERR_INTERNAL_ERROR;
            break;
        }

    }

    if (write_size) {
        psa_write(msg->handle, 0, &item_size, sizeof(item_size));
    }

    return status;
}

static psa_error_t storage_set(psa_msg_t *msg, NVStore &nvstore)
{
    int status = NVSTORE_SUCCESS;
    uint16_t key = 0;
    void *data = NULL;
    size_t alloc_size = msg->in_size[1] + PSA_NVSTORE_ACL_SOURCE_SIZE;
    psa_nvstore_source_t msg_source = psa_identity(msg->handle);

    if (msg->in_size[0] != sizeof(key)) {
        return PSA_DROP_CONNECTION;
    }

    if (psa_read(msg->handle, 0, &key, sizeof(key)) != sizeof(key)) {
        return PSA_DROP_CONNECTION;
    }

    data = malloc(alloc_size);
    if (data == NULL) {
        return MBED_PSA_ST_ERR_INTERNAL_ERROR;
    }

    psa_nvstore_source_t source = 0;
    if (get_key_source(key, nvstore, &source) != PSA_SUCCESS) {
        free(data);
        return MBED_PSA_ST_ERR_INTERNAL_ERROR;
    }

    set_item_source(data, msg_source);
    if (psa_read(msg->handle, 1, PSA_NVSTORE_DATA_PTR(data), msg->in_size[1]) != msg->in_size[1]) {
        status = MBED_PSA_ST_ERR_INTERNAL_ERROR;
    }

    if (status == NVSTORE_SUCCESS) {
        if ((source == 0) || (source == msg_source)) {
            if (nvstore.set(key, (uint16_t)alloc_size, data) == NVSTORE_SUCCESS) {
                status = MBED_PSA_ST_SUCCESS;
            } else {
                status = MBED_PSA_ST_ERR_INTERNAL_ERROR;
            }
        } else {
            status = NVSTORE_OPERATION_NOT_ALLOWED;
        }
    }

    memset(data, 0, alloc_size);
    free(data);
    return status;
}

static psa_error_t storage_remove(psa_msg_t *msg, NVStore &nvstore)
{
    int status = NVSTORE_SUCCESS;
    uint16_t key = 0;
    psa_nvstore_source_t key_source = 0;

    if (msg->in_size[0] != sizeof(key)) {
        return PSA_DROP_CONNECTION;
    }

    if (psa_read(msg->handle, 0, &key, sizeof(key)) != sizeof(key)) {
        return PSA_DROP_CONNECTION;
    }

    if (get_key_source(key, nvstore, &key_source) != PSA_SUCCESS) {
        return MBED_PSA_ST_ERR_INTERNAL_ERROR;
    }

    if ((key_source == 0) || (key_source == psa_identity(msg->handle))) {
        if (nvstore.remove(key) == NVSTORE_SUCCESS) {
            status = MBED_PSA_ST_SUCCESS;
        } else {
            status = MBED_PSA_ST_ERR_INTERNAL_ERROR;
        }
    } else {
        status = NVSTORE_OPERATION_NOT_ALLOWED;
    }

    return status;
}

static void message_handler(psa_msg_t *msg, SignalHandler handler, NVStore &instance)
{
    psa_error_t status = PSA_SUCCESS;
    switch (msg->type) {
        case PSA_IPC_CONNECT:
        case PSA_IPC_DISCONNECT:
        {
            break;
        }
        case PSA_IPC_CALL:
        {
            status = handler(msg, instance);
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

void secure_storage_entry(void *ptr)
{
    uint32_t signals = 0;
    psa_msg_t msg = {0};
    NVStore &nvstore = NVStore::get_instance();
    if (nvstore.init() != PSA_SUCCESS) {
        SPM_PANIC("Failed to initialize secure storage");
    }

    while (1) {
        signals = psa_wait_any(PSA_BLOCK);
        if ((signals & PSA_ST_GET_MSK) != 0) {
            psa_get(PSA_ST_GET_MSK, &msg);
            message_handler(&msg, storage_get, nvstore);
        }
        if ((signals & PSA_ST_SET_MSK) != 0) {
            psa_get(PSA_ST_SET_MSK, &msg);
            message_handler(&msg, storage_set, nvstore);
        }
        if ((signals & PSA_ST_SIZE_MSK) != 0) {
            psa_get(PSA_ST_SIZE_MSK, &msg);
            message_handler(&msg, storage_size, nvstore);
        }
        if ((signals & PSA_ST_ERASE_MSK) != 0) {
            psa_get(PSA_ST_ERASE_MSK, &msg);
            message_handler(&msg, storage_remove, nvstore);
        }
    }
}

#ifdef   __cplusplus
}
#endif
