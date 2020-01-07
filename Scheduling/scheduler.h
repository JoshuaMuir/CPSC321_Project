/*
 * scheduler.h
 */ 
#include <stdbool.h>
#define SCHEDULER_PROCESS_CREATE_FAILED		0
#define SCHEDULER_PROCESS_CREATE_SUCCESS	1
#define SCHEDULER_MAX_NUM_PROCESSES			10

typedef uint32_t tProcessState;
enum tProcessState { ProcessStateReady = 0, ProcessStateRunning, ProcessStateDead, ProcessStateNull,ProcessStateSleep  };	
//Changed tMiniProcess to include an ID, this was done for part 3.6 to be able to distinguish the 
//idle process.
typedef struct{
	uint8_t* name;
	tProcessState state;
	uint32_t id;
}tMiniProcess;

typedef struct{
	tMiniProcess list[SCHEDULER_MAX_NUM_PROCESSES];
	uint32_t count;
}tProcessList;
void get_scheduler_calls(void);
void scheduler_init(void);
void tick_callback(void);
void context_switch(void);
bool check_is_sleep(void);
void optimize(void);
uint32_t sleepy(void);
void scheduler_set_active_process(uint8_t* process_name);
void scheduler_process_sleep(uint32_t sleep_duration);
void scheduler_process_unsleep(uint32_t unsleep_index);
uint32_t scheduler_process_create(uint8_t* process_name, uint32_t identification);
void scheduler_process_current_stop(void);
