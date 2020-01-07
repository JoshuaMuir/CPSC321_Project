/*
 * scheduler.c
 */
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include "stdint.h"
#include "scheduler.h"
#include "scheduling_policy.h"


#define QUANTA_FREQ_PROJ  5
uint32_t TICK_FREQ = 1;    //Tick frequency
tProcessList proc_list;    //Process list
tMiniProcess *sleep_list[10];
tMiniProcess *active_proc; //The active process
uint32_t proc_count = 0;
uint32_t sleep_duration_list[10];
uint32_t stack_top = 0 ; 
uint32_t scheduler_calls = 0 ;
uint32_t offset = 0 ;
static uint32_t current_quanta_proj = 0 ;  //current_quanta_proj starts with zero
static uint32_t sleep_counter = 0 ;     
static bool is_sleep = false;
//A null process (used to mark the lack of an active process)
static tMiniProcess null_proc = { 
			.name = "null", 
			.state = ProcessStateNull
	};	

int main()
{
    printf("Would you like adaptive tick length?y/N ");
    int c;
    bool willOptimize;
    c=getchar();
    //willOptimize represents whether the adaptive tick length 
    //will be used, this is chosen by the user.
    if(c=='y'){willOptimize=true;}
    else {willOptimize=false;offset=5;}
    scheduler_init();

    //Add some initial processes before starting the timer.

    uint8_t *name1 = "Test1 Process";
    scheduler_process_create(name1,1);

    uint8_t *name2 = "Test2 Process";
    scheduler_process_create(name2,2);

    uint8_t *name3 = "Test3 Process";
    scheduler_process_create(name3,3);
    uint32_t tickCounter = 0;

    //This loop is used to simulate the SysTick Timer. Everytime TICK_FREQ is up, the loop calls the
    //tick_callback() function to give control back to the scheduler.
    while (1)
    {
        if(willOptimize)
            optimize();


        sleep(TICK_FREQ);
        printf("Executing %ds Tick\n",TICK_FREQ);

        tick_callback();
        tickCounter++;
        if(tickCounter==3+offset)
        scheduler_process_sleep(9-offset);
        //Since there is no code actually associated with each process. This is used to simulate a process doing something.
        //For example, this 
        if(tickCounter == 4+offset) {
            
            scheduler_process_sleep(8-offset);
        }
        if(tickCounter==8+offset){

            scheduler_process_sleep(11-offset);
        }
        if(tickCounter==13+2*offset){

            scheduler_process_current_stop();
      
        }
        if(tickCounter==14+5*offset){
            scheduler_process_current_stop();
            
        }
        if(tickCounter == 15+7*offset ) {

         
            scheduler_process_current_stop();
         
        }
        if(active_proc->id==-1)get_scheduler_calls();//When it gets to the idle process we see how many scheduler calls th
                                                        //ere have been
    }

    return 0;
}
void get_scheduler_calls(){
    printf("Scheduler has been called %d times\n",scheduler_calls);
}
void scheduler_process_sleep(uint32_t sleep_duration){
//Sleeps the current process and increases top of stack 
//for the next process to be slept.
    sleep_list[stack_top]=active_proc;
    printf("%s went to sleep for %ds\n",active_proc->name,sleep_duration);
    active_proc->state=ProcessStateSleep;
    if(offset==0){
    sleep_duration_list[stack_top++]=sleep_duration;}
else{
    sleep_duration_list[stack_top++]=sleep_counter+sleep_duration-2;
}
    context_switch();


}
/** Initializes the scheduler by adding an idle process to it. */
void scheduler_init(void)
{
    active_proc = &null_proc;

    //Adds the idelProcess to the scheduler. This process should never be stopped.
    uint8_t *idelProcess = "Idle Process";
    scheduler_process_create(idelProcess,-1);
}
void scheduler_process_unsleep(uint32_t unsleep_index){
    //Wakes up the process at the specified index.
    sleep_list[unsleep_index]->state=ProcessStateReady;
    printf("%s has woken up\n",sleep_list[unsleep_index]->name);
    sleep_list[unsleep_index]=NULL;

    sleep_duration_list[unsleep_index]=0;
    uint32_t i = unsleep_index;

    while ( i < 9 ){
    //Moves all the processes down one slot in the array so that
    //sleeping processes can just go to the top/front of the array.
        sleep_duration_list[i]=sleep_duration_list[i+1];
        sleep_duration_list[i+1]=0;
        sleep_list[i]=sleep_list[i+1];
        sleep_list[i+1]=NULL;
        i++;

    }

    --stack_top;

}
/** Triggered every TICK_FREQ seconds to perform a context switch. */
void tick_callback(void)
{
    ++scheduler_calls;

    if (offset!=0) current_quanta_proj = current_quanta_proj + 1;
    uint32_t checker = sleepy();

    if(current_quanta_proj < QUANTA_FREQ_PROJ){
        //housekeeping part
        //unsleeps a process if it is ready.
        if(check_is_sleep() && checker!=60429){

            scheduler_process_unsleep(checker);

        }

        
    } else {
      //Switches context when the quanta is up.
      context_switch();

    }
      ++sleep_counter;
}
void optimize(){//For part 3.1
    //Adapts the tick length based on whether there is a process to unsleep
    //soon or a quanta sooner.
    uint32_t first_wake;//First wake repsresents the time in which the soonest
                        //process unsleep is.
    uint32_t sleepy_index = sleepy();
    if(sleepy_index!=60429){
        first_wake = sleep_duration_list[sleepy()];
    }
    else{
        first_wake = 100;
    }   
  
    if(first_wake<QUANTA_FREQ_PROJ-current_quanta_proj){
    //if the process needs to be woken before the next quanta,
    //the tick freq will change to the time for the process to 
    //wake up, otherwise the tick_freq will be the quanta.
        TICK_FREQ=first_wake;
        sleep_counter=first_wake;
        current_quanta_proj=current_quanta_proj+first_wake;
        uint32_t i = 0 ;
        while ( i < 10){
            if(sleep_duration_list[i]!=0 && sleep_duration_list[i]!=TICK_FREQ){

                sleep_duration_list[i]=sleep_duration_list[i]-TICK_FREQ;
            }
            i++;
        }
    } else {
    //both the while loop above and below function to decrease the sleep duration by the
    //tick length so that the time elapses for them properly in both cases.
        TICK_FREQ = QUANTA_FREQ_PROJ-current_quanta_proj;
        current_quanta_proj=QUANTA_FREQ_PROJ;
        uint32_t i = 0 ;
        while ( i < 10){

            if(sleep_duration_list[i]!=0){

                sleep_duration_list[i]=sleep_duration_list[i]-TICK_FREQ;

            }

            i++;
        }
    }

}
uint32_t sleepy(){
//the sleepy function simply returns whichever process needs to be woken up
//next
    uint32_t returnval=60429;//Sentinel value
    uint32_t i = 0 ;
    while( i < 10 ){
       
        if(offset!=0){
        if(sleep_duration_list[i]!=0 && sleep_duration_list[i]<sleep_counter && sleep_duration_list[i]<returnval){
            returnval=i;

        }
        }else{

            if(sleep_duration_list[i]!= 0 && sleep_duration_list[i]<returnval){
                returnval = i ; 
            }

        }
    
        i++;
    }

    return returnval;

}
bool check_is_sleep(){
//simply checks whether there is a process sleeping
//right now
    bool returnval = false;
    uint32_t i = 0 ;
    while(i < 10){
       
        if(sleep_duration_list[i]!=0){

            returnval=true;
        }
        i++;
    }

    return returnval;

}
void context_switch(){

    current_quanta_proj=0;
    active_proc = scheduling_policy_next(active_proc, &proc_list);

    scheduler_set_active_process(active_proc->name);
}
/** Sets the active process. */
void scheduler_set_active_process(uint8_t *process_name)
{
    printf("Currently Executing: %s \n", process_name);
}

/** Creates a new process and adds it to the scheduler. */
uint32_t scheduler_process_create(uint8_t *process_name,uint32_t identification)
{
    //Set process info
    proc_list.list[proc_list.count].name = process_name;
    proc_list.list[proc_list.count].state = ProcessStateReady;
    proc_list.list[proc_list.count].id=identification;

    //Increment counter in list
    proc_list.count++;

    return SCHEDULER_PROCESS_CREATE_SUCCESS;
}

/** Sets the active processes state to be stopped. */
void scheduler_process_current_stop(void)
{
    printf("%s has been stopped\n",active_proc->name);
    active_proc->state = ProcessStateDead;
}
