#ifndef MBED_SPM_INIT_API_H
#define MBED_SPM_INIT_API_H


#if defined(TARGET_SPM_MAILBOX)

void on_new_item(void);
void on_vacancy(void);
void ipc_interrupt_handler_plat(void);
void spm_ipc_queues_init_plat(void);
void spm_ipc_queues_init(void);

#endif // defined(TARGET_SPM_MAILBOX)

#if defined(TARGET_PSA_TFM)

void start_nspe(void);

#endif // defined(TARGET_PSA_TFM)

#endif // MBED_SPM_INIT_API_H
