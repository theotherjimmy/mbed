#include "spm_server.h"
#include "spm_panic.h"
#include "secure_add.h"
#include "secure_add_impl.h"
#include "psa_secure_add_partition.h"

static psa_error_t add(psa_msg_t *msg)
{
    int a, b, c;
    if ((msg->in_size[0] != sizeof(a)) || (msg->in_size[1] != sizeof(b)) || (msg->out_size[0] != sizeof(c))) {
        return PSA_DROP_CONNECTION;
    }

    if (psa_read(msg->handle, 0, &a, sizeof(a)) != sizeof(a)) {
        return PSA_DROP_CONNECTION;
    }

    if (psa_read(msg->handle, 1, &b, sizeof(b)) != sizeof(b)) {
        return PSA_DROP_CONNECTION;
    }

    c = secure_add_impl(a, b);
    psa_write(msg->handle, 0, &c, sizeof(c));
    return PSA_SUCCESS;
}

void secure_add_entry(void *ptr)
{
    uint32_t signals = 0;
    psa_msg_t msg = {0};
    while (1) {
        signals = psa_wait_any(PSA_BLOCK);
        if ((signals & PSA_SECURE_ADD_MSK) != 0) {
            psa_get(PSA_SECURE_ADD_MSK, &msg);
            psa_error_t status = PSA_SUCCESS;
            switch (msg.type) {
                case PSA_IPC_CONNECT:
                case PSA_IPC_DISCONNECT:
                {
                    break;
                }
                case PSA_IPC_CALL:
                {
                    status = add(&msg);
                    break;
                }
                default:
                {
                    SPM_PANIC("Unexpected message type %d!", (int)(msg.type));
                    break;
                }
            }

            psa_end(msg.handle, status);
        }
    }
}

