#include "spm_client.h"
#include "secure_add.h"
#include "psa_secure_add_ifs.h"


int secure_add(int a, int b)
{
    int res = 0;
    psa_invec_t msg[2] = {
        { &a, sizeof(a) },
        { &b, sizeof(b) },
    };

    psa_outvec_t resp = { &res, sizeof(res) };

    psa_handle_t conn = psa_connect(PSA_SECURE_ADD, 1);
    if (conn <= PSA_NULL_HANDLE) {
        return 0;
    }

    psa_error_t status = psa_call(conn, msg, 2, &resp, 1);
    if (status == PSA_DROP_CONNECTION) {
        res = 0;
    }

    psa_close(conn);
    return res;
}
