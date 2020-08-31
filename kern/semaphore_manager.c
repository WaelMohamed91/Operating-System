#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/environment_definitions.h>

#include <kern/semaphore_manager.h>
#include <kern/memory_manager.h>
#include <kern/sched.h>
#include <kern/kheap.h>

//==================================================================================//
//============================== HELPER FUNCTIONS ==================================//
//==================================================================================//
//Helper functions to deal with the semaphore queue
//void enqueue(struct Env_Queue* queue, struct Env* env);
//struct Env* dequeue(struct Env_Queue* queue);

///// Helper Functions
//void enqueue(struct Env_Queue* queue, struct Env* env)
//{
//	LIST_INSERT_HEAD(queue, env);
//}
//
//struct Env* dequeue(struct Env_Queue* queue)
//{
//	struct Env* envItem = LIST_LAST(queue);
//	LIST_REMOVE(queue, envItem);
//	return envItem;
//}

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//===============================
// [1] Create "semaphores" array:
//===============================
//Dynamically allocate the "semaphores" array
//initialize the "semaphores" array by 0's and empty = 1
void create_semaphores_array(uint32 numOfSemaphores)
{
	semaphores = (struct Semaphore*) kmalloc(numOfSemaphores*sizeof(struct Semaphore));
	if (semaphores == NULL)
	{
		panic("Kernel runs out of memory\nCan't create the array of semaphores.");
	}
	for (int i = 0; i < MAX_SEMAPHORES; ++i)
	{
		memset(&(semaphores[i]), 0, sizeof(struct Semaphore));
		semaphores[i].empty = 1;
		LIST_INIT(&(semaphores[i].env_queue));
	}
}


//========================
// [2] Allocate Semaphore:
//========================
//Allocates a new (empty) semaphore object from the "semaphores" array
//Return:
//	a) if succeed:
//		1. allocatedSemaphore (pointer to struct Semaphore) passed by reference
//		2. SempahoreObjectID (its index in the array) as a return parameter
//	b) E_NO_SEMAPHORE if the the array of semaphores is full (i.e. reaches "MAX_SEMAPHORES")
int allocate_semaphore_object(struct Semaphore **allocatedObject)
{
	int32 semaphoreObjectID = -1 ;
	for (int i = 0; i < MAX_SEMAPHORES; ++i)
	{
		if (semaphores[i].empty)
		{
			semaphoreObjectID = i;
			break;
		}
	}

	if (semaphoreObjectID == -1)
	{
		//try to double the size of the "semaphores" array
		if (USE_KHEAP == 1)
		{
			semaphores = (struct Semaphore*) krealloc(semaphores, 2*MAX_SEMAPHORES);
			if (semaphores == NULL)
			{
				*allocatedObject = NULL;
				cprintf ("NO Semphaore >>>>>>>>>>>>>>>") ;
				return E_NO_SEMAPHORE;
			}
			else
			{
				semaphoreObjectID = MAX_SEMAPHORES;
				MAX_SEMAPHORES *= 2;
			}
		}
		else
		{
			*allocatedObject = NULL;
			cprintf ("NO Semphaore >>>>>>>>>>>>>>>") ;
			return E_NO_SEMAPHORE;
		}
	}

	*allocatedObject = &(semaphores[semaphoreObjectID]);
	semaphores[semaphoreObjectID].empty = 0;

	return semaphoreObjectID;
}

//======================
// [3] Get Semaphore ID:
//======================
//Search for the given semaphore object in the "semaphores" array
//Return:
//	a) if found: SemaphoreObjectID (index of the semaphore object in the array)
//	b) else: E_SEMAPHORE_NOT_EXISTS
int get_semaphore_object_ID(int32 ownerID, char* name)
{
	int i=0;
	for(; i < MAX_SEMAPHORES; ++i)
	{
		if (semaphores[i].empty)
			continue;

		if(semaphores[i].ownerID == ownerID && strcmp(name, semaphores[i].name)==0)
		{
			return i;
		}
	}
	return E_SEMAPHORE_NOT_EXISTS;
}

//====================
// [4] Free Semaphore:
//====================
//delete the semaphore with the given ID from the "semaphores" array
//Return:
//	a) 0 if succeed
//	b) E_SEMAPHORE_NOT_EXISTS if the semaphore is not exists
int free_semaphore_object(uint32 semaphoreObjectID)
{
	if (semaphoreObjectID >= MAX_SEMAPHORES)
		return E_SEMAPHORE_NOT_EXISTS;

	memset(&(semaphores[semaphoreObjectID]), 0, sizeof(struct Semaphore));
	semaphores[semaphoreObjectID].empty = 1;
	LIST_INIT(&(semaphores[semaphoreObjectID].env_queue));

	return 0;
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//======================
// [1] Create Semaphore:
//======================
int createSemaphore(int32 ownerEnvID, char* semaphoreName, uint32 initialValue)
{

	//cprintf("create_semphore \n ") ;
	// your code is here, remove the panic and write your code
	//create new semaphore object and initialize it by the given info (ownerID, name, value)
	struct Semaphore* ptr_semaphore  = NULL ;

	int check = -1 ; check =  get_semaphore_object_ID(ownerEnvID, semaphoreName) ;

	if (check == E_SEMAPHORE_NOT_EXISTS   ) { // when sempahore already exist

	int semphore_id = -1 ;

	semphore_id = allocate_semaphore_object(&ptr_semaphore) ;

	// when can not allocate .. NO space Array FULL
	if (semphore_id == E_NO_SEMAPHORE  )
			return E_NO_SEMAPHORE ;
	// succeed set information of semphore and  return SemaphoreID (its index in the array)
	// using array of semphores  or using pointer
	/*
	semaphores[semphore_id].ownerID = ownerEnvID ;
	strcpy (semaphores[semphore_id].name , semaphoreName) ;
	semaphores[semphore_id].value = initialValue ;

	// */
		// pointer

		ptr_semaphore->ownerID = ownerEnvID ;
		// for (int i = 0 ; i <sizeof (semaphoreName) / sizeof(semaphoreName[0]) ; i++ )
			//ptr_semaphore->name[i] = semaphoreName[i] ;
	strcpy (semaphores[semphore_id].name , semaphoreName) ;
		ptr_semaphore->value = initialValue ;

		//*/

		return semphore_id ;
	}

	return E_SEMAPHORE_EXISTS ;




}

//============
// [2] Wait():
//============
void waitSemaphore(int32 ownerEnvID, char* semaphoreName)
{

	// your code is here, remove the panic and write your code

	//cprintf("Wait \n ") ;
	struct Env* myenv = curenv; //The calling environment

	// Steps:
	//	1) Get the Semaphore


	//cprintf("wait_semphore   %d \n ",semphore_id ) ;
	int semphore_id = get_semaphore_object_ID(ownerEnvID, semaphoreName) ;
	//if (semphore_id == E_SEMAPHORE_NOT_EXISTS )
		// return  ;
	//	2) Decrement its value
	semaphores[semphore_id].value = semaphores[semphore_id].value - 1 ;
	//	3) If negative, block the calling environment "myenv", by
	if ( semaphores[semphore_id].value < 0) {
	//		a) removing it from ready queue
			sched_remove_ready (myenv) ;
	//		b) adding it to semaphore queue
			//enqueue (&semaphores[semphore_id].env_queue , myenv) ;
	//		c) changing its status to ENV_BLOCKED
			myenv->env_status = ENV_BLOCKED ;
			// or
			// curenv->env_status = ENV_BLOCKED ;
			 enqueue (&semaphores[semphore_id].env_queue , myenv) ;
			 //		d) set curenv with NULL
			 	 curenv =NULL ;
			// myenv = NULL ;
	}

	//	4) Call "fos_scheduler()" to continue running the remaining envs
		fos_scheduler() ;
}

//==============
// [3] Signal():
//==============
void signalSemaphore(int ownerEnvID, char* semaphoreName)
{
	//cprintf("signal_semphore \n ") ;
	// Steps:

	//	1) Get the Semaphore
	int semphore_id = get_semaphore_object_ID(ownerEnvID, semaphoreName) ;
	//cprintf("signal_semphore   %d \n ",semphore_id ) ;
	//	2) Increment its value
	semaphores[semphore_id].value = semaphores[semphore_id].value + 1 ;
	//	3) If less than or equal 0, release a blocked environment, by

	if (semaphores[semphore_id].value <= 0){
	//		a) removing it from semaphore queue		[refer to helper functions in doc]
		struct Env* myenv = dequeue (&semaphores[semphore_id].env_queue) ;
	//		b) adding it to ready queue				[refer to helper functions in doc]

	//		c) changing its status to ENV_READY
		myenv->env_status = ENV_READY ;
		sched_insert_ready(myenv) ;


	}

}

