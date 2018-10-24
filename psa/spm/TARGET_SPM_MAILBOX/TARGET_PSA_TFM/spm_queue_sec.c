#include "psa_defs.h"
#include "spm_client.h"
#include "spm_messages.h"
#include "spm_internal.h"
#include "mbed_assert.h"
#include "ipc_queue.h"
#include "spm_init_api.h"
#include "ipc_defs.h"

static os_mutex_t queue_mutex_storage;
static os_semaphore_t full_sema;
static os_semaphore_t read_sema;

static const osMutexAttr_t queue_mutex_attr = {
    .name      = "Q_MUT",
    .attr_bits = osMutexRecursive | osMutexPrioInherit | osMutexRobust,
    .cb_mem    = &queue_mutex_storage,
    .cb_size   = sizeof(queue_mutex_storage)
};

// Full queue semaphore attributes for the consumer queue
static const osSemaphoreAttr_t full_sem_attr = {
    .name      = "Q_W_SEM",
    .attr_bits = 0,
    .cb_mem    = &full_sema,
    .cb_size   = sizeof(full_sema)
};

// Read semaphore attributes for the consumer queue
static const osSemaphoreAttr_t read_sem_attr = {
    .name      = "Q_R_SEM",
    .attr_bits = 0,
    .cb_mem    = &read_sema,
    .cb_size   = sizeof(read_sema)
};


static ipc_producer_queue_t _prod_queue;
ipc_producer_queue_t *prod_queue = &_prod_queue;
static ipc_consumer_queue_t _cons_queue;
ipc_consumer_queue_t *cons_queue = &_cons_queue;

void ipc_interrupt_handler(void)
{
    osStatus_t os_status = osSemaphoreRelease(prod_queue->full_queue_sem);
    MBED_ASSERT((osOK == os_status) || (osErrorResource == os_status));

    os_status = osSemaphoreRelease(cons_queue->read_sem);
    MBED_ASSERT((osOK == os_status) || (osErrorResource == os_status));

    PSA_UNUSED(os_status);

    ipc_interrupt_handler_plat();
}

void on_popped_item(ipc_queue_item_t item)
{
    // item.a hold the message type (connect / call /close)
    switch (item.a)
    {
        case PSA_IPC_CONNECT:
        {
            psa_connect_async(item.c, (spm_pending_connect_msg_t *)(item.b));
            break;
        }

        case PSA_IPC_CALL:
        {
            psa_call_async((psa_handle_t)(item.c), (spm_pending_call_msg_t *)(item.b));
            break;
        }

        case PSA_IPC_DISCONNECT:
        {
            psa_close((psa_handle_t)(item.c));
            break;
        }

        case PSA_IPC_VERSION:
        {
            spm_pending_version_msg_t *msg = (spm_pending_version_msg_t *)(item.b);
            if (!is_buffer_accessible(msg, sizeof(*msg), NULL)) {
                SPM_PANIC("message data is inaccessible\n");
            }

            msg->rc = psa_version(item.c);
            nspe_done(msg->completion_sem_id);
            break;
        }

        default:
        {
            MBED_ASSERT(false);
        }
    }
}

void spm_ipc_queues_init(void)
{
    // The __shared_memory_start & __shared_memory_end symbols are declared in CM0+ Linker Script.
    // &__shared_memory_start denotes the start address of the memory shared between CM0+ and CM4.
    extern uint32_t __shared_memory_start;
    extern uint32_t __shared_memory_end;

    // This struct is set with initial values for the address table (addresses of CM0+ / CM4 shared memory)
    const addr_table_t shared_addr_table = {
        .magic        = ADDR_TABLE_MAGIC,
        .tx_queue_ptr = (uintptr_t)( &__shared_memory_start +
                                     (sizeof(addr_table_t) / sizeof(uint32_t))
                                   ),
        .rx_queue_ptr = (uintptr_t)( &__shared_memory_start +
                                     ((sizeof(addr_table_t) + sizeof(ipc_base_queue_t)) / sizeof(uint32_t))
                                   )
    };

    // This struct is set with initial values to be used for IPC queues initialization.
    // Same values are used to initial both tx and rx queues.
    const ipc_base_queue_t queue_mem = {
        .magic        = IPC_QUEUE_BASE_MAGIC,
        .read_idx     = 0,
        .write_idx    = 0,
        .data         = {{0, 0}}
    };

    ipc_base_queue_t *tx_queue_mem_ptr = (ipc_base_queue_t *)(shared_addr_table.tx_queue_ptr);
    ipc_base_queue_t *rx_queue_mem_ptr = (ipc_base_queue_t *)(shared_addr_table.rx_queue_ptr);

    // Copy initial queue values for tx & tx queues to the right location in the CM0+ / CM4 shared memory
    memcpy(tx_queue_mem_ptr, &queue_mem, sizeof(ipc_base_queue_t));
    memcpy(rx_queue_mem_ptr, &queue_mem, sizeof(ipc_base_queue_t));

    uintptr_t calc_shared_mem_end = (uintptr_t) ( &__shared_memory_start +
                                                  ((sizeof(addr_table_t) + sizeof(ipc_base_queue_t) + sizeof(ipc_base_queue_t)) / sizeof(uint32_t))
                                                );

    // Verify we did not overflow our shared memory allocated size
    MBED_ASSERT((uintptr_t)(&__shared_memory_end) >= (uintptr_t)calc_shared_mem_end);

    PSA_UNUSED(calc_shared_mem_end);
    PSA_UNUSED(__shared_memory_end);

    // Copy the content of shared_addr_table to the start address of CM0+ / CM4 shared memory.
    // Since _shared_memory_start address is known to CM4, we will be able to read the address table from CM4 code
    memcpy(&__shared_memory_start, &shared_addr_table, sizeof(shared_addr_table));

    // Init producer queue and consumer queue

    osMutexId_t queue_mutex = osMutexNew(&queue_mutex_attr);
    MBED_ASSERT(queue_mutex != NULL);    // TODO: Panic instead

    osSemaphoreId_t full_queue_sem = osSemaphoreNew(IPC_QUEUE_SEM_MAX_COUNT, IPC_QUEUE_SEM_INITIAL_COUNT, &full_sem_attr);
    MBED_ASSERT(full_queue_sem != NULL);    // TODO: Panic instead

    osSemaphoreId_t queue_read_sem = osSemaphoreNew(IPC_QUEUE_SEM_MAX_COUNT, IPC_QUEUE_SEM_INITIAL_COUNT, &read_sem_attr);
    MBED_ASSERT(queue_read_sem != NULL);    // TODO: Panic instead

    ipc_producer_queue_init(prod_queue, rx_queue_mem_ptr, queue_mutex, full_queue_sem);
    ipc_consumer_queue_init(cons_queue, tx_queue_mem_ptr, queue_read_sem);

    spm_ipc_queues_init_plat();
}

void nspe_done(osSemaphoreId_t completion_sem_id)
{
    ipc_queue_item_t item_to_enqueue = {
        .b = (uint32_t)(completion_sem_id)
    };

    ipc_queue_enqueue(prod_queue, item_to_enqueue);
}
