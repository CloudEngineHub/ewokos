#ifndef PROC_H
#define PROC_H

#include <kernel/context.h>
#include <kernel/ipc.h>
#include <kernel/core.h>
#include <mm/mmu.h>
#include <mm/trunkmem.h>
#include <procinfo.h>
#include <stdbool.h>

#define PROC_FILE_MAX 128
#define SHM_MAX 128
#define LOCK_MAX 64
#define IPC_CTX_MAX 8

enum {
	SIG_STATE_IDLE = 0,
	SIG_STATE_BUSY
};

typedef struct {
	page_dir_entry_t *vm;
	malloc_t malloc_man;
	uint32_t heap_size;
	uint32_t kpage; //mapped page , share same address with kernel
	bool ready_ping;
	
	int32_t shms[SHM_MAX];

	struct {
		uint32_t  uid;
		uint32_t  entry;
		uint32_t  flags;
	  uint32_t  ipc;
		uint32_t  extra_data;
		context_t ctx;
	  uint32_t  state;
	} ipc_server;

	struct {
		uint32_t  entry;
		context_t ctx;
	  uint32_t  state;
	  uint32_t  proc_state;
	} signal;
} proc_space_t;

#define STACK_PAGES 32
#define THREAD_STACK_PAGES 4

typedef struct st_proc {
	procinfo_t info;

	uint32_t block_event;
	int64_t sleep_counter; //sleep usec

	proc_space_t* space;
	void* user_stack[STACK_PAGES];

	context_t ctx;
	ipc_t ipc_ctx;
	ipc_t* ipc_client;
} proc_t;

extern proc_t* get_current_proc(void);
extern bool _core_proc_ready;
extern int32_t _core_proc_pid;
extern uint32_t _ipc_uid;

extern void    procs_init(void);
extern int32_t proc_load_elf(proc_t *proc, const char *proc_image, uint32_t size);
extern int32_t proc_start(proc_t* proc, uint32_t entry);
extern proc_t* proc_get_next_ready(void);
extern proc_t* proc_get_core_ready(uint32_t core_id);
extern void    proc_switch(context_t* ctx, proc_t* to, bool quick);
extern void    set_current_proc(proc_t* proc);
extern void    proc_flush_tlb(proc_t *proc);
extern int32_t proc_expand_mem(proc_t *proc, int32_t page_num);
extern void    proc_shrink_mem(proc_t* proc, int32_t page_num);
extern void    proc_exit(context_t* ctx, proc_t *proc, int32_t res);
extern proc_t *proc_create(int32_t type, proc_t* parent);

extern void*   proc_malloc(uint32_t size);
extern void*   proc_realloc(void* p, uint32_t size);
extern void    proc_free(void* p);

extern void    proc_block_on(context_t* ctx, int32_t pid_by, uint32_t event);
extern void    proc_wakeup(int32_t pid, uint32_t event);
extern void    proc_waitpid(context_t* ctx, int32_t pid);
extern proc_t* proc_get(int32_t pid);
extern proc_t* proc_get_proc(proc_t* proc);
extern proc_t* kfork_raw(context_t* ctx, int32_t type, proc_t* parent);
extern proc_t* kfork(context_t* ctx, int32_t type);

extern procinfo_t* get_procs(int32_t* num);

extern void    renew_kernel_tic(uint32_t usec);
extern void    proc_usleep(context_t* ctx, uint32_t usec);
extern void    proc_ready(proc_t* proc);

extern uint32_t proc_num_in_core(uint32_t core);
#endif
