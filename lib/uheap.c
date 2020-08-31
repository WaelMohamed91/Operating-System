#include <inc/lib.h>
#define capacity (USER_HEAP_MAX - USER_HEAP_START) /PAGE_SIZE

// malloc()
//	This function use BEST FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space

//	To do this, we need to switch to the kernel, allocate the required space
//	in Page File then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//int capactiy = (USER_HEAP_MAX - USER_HEAP_START) /PAGE_SIZE ;

struct best_fit
{
	uint32 start_address ;
	uint32 size ;
	uint32 use_size ;
};
struct best_fit taken_array [capacity];
uint32 taken_index = 0 ;
//////////////////////////////////
struct PAGES {
	int index ; // size .. USER_HEAP_START + (index * PAGE_SIZE ) = virtual_address
	uint32 start_address ;
	bool perm_present   ; // true mean is used ..
	int length ;

};
struct PAGES arr_pages [capacity] = {0};
int count_mapped_pages = 0 ;
//////////////////////////////

struct best_fit array [capacity] ;
uint32 index_best_fit = 0 ;
int UserHeapBestFitStrategy(uint32 size)
{
	if (count_mapped_pages == capacity   )
				return -1 ;

	if ((capacity - count_mapped_pages) < size/ PAGE_SIZE)
		return -1 ;

 	index_best_fit = 0 ;

	size = ROUNDUP(size , PAGE_SIZE);

	uint32 actual_size = size/PAGE_SIZE ;
	uint32 empty_size = 0 ;

	uint32 virtual = USER_HEAP_START ;

	uint32 min = 1000000;
	int index_min = -1 ;
	bool key = 0 ;
	// key = 1 when find start of free space
	// key = 0 when find this free space ended

	for (int i = 0 ; i < capacity ; i++ )
	{
		virtual = USER_HEAP_START + (i*PAGE_SIZE) ;

		if (arr_pages[i].perm_present == 0 && key == 0)
		{
			key = 1 ;
			array[index_best_fit].start_address = virtual ;
			empty_size ++ ;
		}
		else if (arr_pages[i].perm_present ==0 && key == 1 )
			empty_size ++;

		else if (arr_pages[i].perm_present == 1 && key == 1)
		{

			array[index_best_fit].size = empty_size ; // Num of pages

			array[index_best_fit].use_size = 0 ;

			key = 0 ;
			index_best_fit ++;
			empty_size = 0 ;
		}

	}

	if (key == 1 )
	{

		array[index_best_fit].size = empty_size ;
		array[index_best_fit].use_size = 0 ;
		key = 0 ;
		index_best_fit ++;
	}
	// TO choose minimum space
	for (int j=0; j<index_best_fit; j++)
	{
		if (array[j].size >= actual_size)
		{
			if (index_min == -1)
			{
				index_min = j;
			}
			else if (array[index_min].size >= array[j].size)
				index_min = j;
		}
	}

	array[index_min].use_size = actual_size ;
	// mark taken pages
	uint32 start_mapping = array[index_min].start_address;

	int index = (start_mapping - USER_HEAP_START ) / PAGE_SIZE;

	int to_decrement = 0 ;

	for (uint32 j = start_mapping ; j< start_mapping + size ; j+= PAGE_SIZE)
	{
		arr_pages[index].perm_present = 1 ;

		arr_pages[index].start_address = j ;

		arr_pages[index].length = actual_size ;//- to_decrement ;

		count_mapped_pages ++ ;
		index++;
		to_decrement ++ ;
	}

	taken_array[taken_index].start_address = array[index_min].start_address;

	taken_array[taken_index].size = array[index_min].size;

	taken_array[taken_index].use_size = array[index_min].use_size;

	taken_index++;

	return index_min;

}
void* malloc(uint32 size)
{
	//cprintf("malloc \n") ;

	//TODO: [PROJECT 2019 - MS2 - [5] User Heap] malloc() [User Side]
	// Write your code here, remove the panic and write your code
	//Use sys_isUHeapPlacementStrategyBESTFIT() to check the current strategy
	if (sys_isUHeapPlacementStrategyBESTFIT())
	{
		int s = size;

		//	1) Implement BEST FIT strategy to search the heap for suitable space
		//		to the required allocation size (space should be on 4 KB BOUNDARY)
		int index_min = UserHeapBestFitStrategy(size);
		//	2) if no suitable space found, return NULL
		if ( index_min == -1 )
			return NULL;

		//	3) Call sys_allocateMem to invoke the Kernel for allocation
		sys_allocateMem(array[index_min].start_address , size ) ;
		// 	4) Return pointer containing the virtual address of allocated space,
		return (uint32 *)array[index_min].start_address ;
	}

	return NULL;
}
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{

	//TODO: [PROJECT 2019 - MS2 - [6] Shared Variables: Creation] smalloc() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("smalloc() is not implemented yet...!!");
	//Use sys_isUHeapPlacementStrategyBESTFIT() to check the current strategy
	if (sys_isUHeapPlacementStrategyBESTFIT())
	{

		//	1) Implement BEST FIT strategy to search the heap for suitable space
		//		to the required allocation size (space should be on 4 KB BOUNDARY)
		int index_min = UserHeapBestFitStrategy(size);
		//	2) if no suitable space found, return NULL
		if ( index_min == -1 )
			return NULL;
		//	3) Call sys_createSharedObject(...) to invoke the Kernel for allocation of shared variable
		uint32 StartVirtualAddressForBestSpace = array[index_min].start_address;
		int VariableID = sys_createSharedObject(sharedVarName,size,isWritable,(void*)StartVirtualAddressForBestSpace);
		//	4) If the Kernel successfully creates the shared variable, return its virtual address
		//	   Else, return NULL
		if(VariableID >= 0)
			return (uint32 *)StartVirtualAddressForBestSpace;
		else
			return NULL;

	}

	return NULL;
}

void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//TODO: [PROJECT 2019 - MS2 - [6] Shared Variables: Get] sget() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("sget() is not implemented yet...!!");

	//Use sys_isUHeapPlacementStrategyBESTFIT() to check the current strategy
	if (sys_isUHeapPlacementStrategyBESTFIT())
	{
		//	1) Get the size of the shared variable (use sys_getSizeOfSharedObject())
		int Size = sys_getSizeOfSharedObject(ownerEnvID,sharedVarName);
		//	2) If not exists, return NULL
		if(Size == E_SHARED_MEM_NOT_EXISTS)
			return NULL;
		//	3) Implement BEST FIT strategy to search the heap for suitable space
		int min_index = UserHeapBestFitStrategy(Size);
		//	4) if no suitable space found, return NULL
		if(min_index == -1)
			return NULL;
		//	5) Call sys_getSharedObject(...) to invoke the Kernel for sharing this variable
		int SharedID = sys_getSharedObject(ownerEnvID,sharedVarName,(void*)array[min_index].start_address);
		//		sys_getSharedObject(): if succeed, it returns the ID of the shared variable. Else, it returns -ve
		//	6) If the Kernel successfully share the variable, return its virtual address
		//	   Else, return NULL


		if(SharedID < 0)
			return NULL;
		return (uint32 *)array[min_index].start_address;
	}
	return NULL;
}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void* virtual_address)
{

	//TODO: [PROJECT 2019 - MS2 - [5] User Heap] free() [User Side]

	uint32 size, length = 0 ;
	bool check =0  ;

	uint32 i = (USER_HEAP_START + (uint32)virtual_address ) / PAGE_SIZE ;

	//for (int i = 0 ; i < capacity ; i++ ) {

		if (arr_pages[i].start_address == (uint32)virtual_address && arr_pages[i].perm_present == 1 ) {

			size = arr_pages[i].length * PAGE_SIZE ;
			length = arr_pages[i].length ;
			for (int j = i ; j <  i+  length ; j++) {
				arr_pages[j].perm_present = 0 ;
				arr_pages[j].length = 0 ;
				arr_pages[j].start_address = 0 ;
			}
			check = 1 ;
		//	break ;
		}

	//}
	count_mapped_pages -= length ;

	if (check)
	 sys_freeMem((uint32) virtual_address,  size);
	else
		 sys_freeMem(0,0);

//*/

	/*
	// Get index of Taken Array to get size
	uint32 allocated_length  ;
	int index ;


	// To get lengthe of pages mappen strated from this virtual address
	for (int i = 0; i < taken_index ;i++ ) {
		 if ( taken_array[i].start_address == (uint32)virtual_address) {
			 allocated_length =  taken_array[i].use_size ;
			 index = i;
			 break ;
		 }
	}
	// To free from User Heap
	for (int j = 0  ; j < capacity ; j++) {

		if (arr_pages[j].start_address == (uint32)virtual_address) {
			for (int i = j ; i < j+allocated_length ; i++) {
				arr_pages[i].perm_present = 0 ;
				arr_pages[i].start_address = 0 ;
			}
			break ;
		}
	}
	// To free from taken index array
	for(int i=index;i<taken_index -1 ;i++)
	       {

	    	   taken_array[i].start_address=taken_array[i+1].start_address;
	    	   taken_array[i].size=taken_array[i+1].size;
	    	   taken_array[i].use_size=taken_array[i+1].use_size;
	       }
	       //decrement the counter of used array
	       taken_index--;

	       sys_freeMem((uint32) virtual_address,  allocated_length *PAGE_SIZE );

	       //*/

	//you should get the size of the given allocation using its address
	//you need to call sys_freeMem()
	//refer to the project presentation and documentation for details
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=============
// [1] sfree():
//=============
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.

//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT 2019 - BONUS4] Free Shared Variable [User Side]
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");

	//	1) you should find the ID of the shared variable at the given address
	//	2) you need to call sys_freeSharedObject()

}


//===============
// [2] realloc():
//===============

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		in "memory_manager.c", then switch back to the user mode here
//	the moveMem function is empty, make sure to implement it.

void *realloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2019 - BONUS3] User Heap Realloc [User Side]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");

}
