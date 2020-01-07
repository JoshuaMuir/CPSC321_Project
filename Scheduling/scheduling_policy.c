/*
 * scheduling_policy.c
 */ 
#include <stdio.h>
#include <unistd.h>
#include "stdint.h"
#include "scheduler.h"

uint32_t process_mark = 0;

/*
* NOTE: This needs a better name. Scheduling Policy sounds so
*       INSURANCEsy
*
*       I believe this is round robin! It jsut looks different thatn what
*		it appears on textsbooks.
*/
tMiniProcess* scheduling_policy_next( tMiniProcess* active_proc, tProcessList* proc_list  ){
	//The scheduling policy has been changed to skip the idle process unless
	//there is no other processes to run. This works by constantly skipping the
	//idle process until count gets too big. Count getting too big means that 
	//there are no other processes to run and therefore it must execute the
	//idle process.
	//Increment process mark
	//(skip dead processes)
	bool detect_infinity=false;
	uint32_t count=0;
	do {
		count++;
		process_mark = (process_mark + 1) % proc_list->count;
		
		if(count>100){return &(proc_list->list[0]); }
	} while(!detect_infinity && (proc_list->list[process_mark].state == ProcessStateDead || proc_list->list[process_mark].state == ProcessStateSleep || proc_list->list[process_mark].id==-1));
	

	return &(proc_list->list[process_mark]);
}
