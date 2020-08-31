#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>
#define capacity (USER_HEAP_MAX - USER_HEAP_START) /PAGE_SIZE
//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
uint32 pointer_heap =(uint32)KERNEL_HEAP_START  ;

// ------------------------ best fit struct ----------//
//contains start address for the empty space
//size => of the empty space
//use size => the size that actual uses that entered
struct stratgy_fit
{
	uint32 start_address ;
	uint32 size ;
	uint32 use_size ;
};
// ----------------- array for used space that taken with best fit -----//
struct stratgy_fit array [capacity] ;
struct stratgy_fit taken_array [capacity];
uint32 index_array= 0 ;
// taken array counter
uint32 taken_index = 0 ;
void* kmalloc(unsigned int size)
{
	//struct stratgy_fit array [1000] ;
	// ----------------- array for free spaces in the kernal heap -----//
	index_array=0;
	//kernal heap free spaces counter
	size = ROUNDUP(size , PAGE_SIZE);
	// convert size into #of pages that he need
	uint32 actual_size = size/PAGE_SIZE ;
	//cprintf("actual=%d\n",actual_size);
	// counter to calc the free space to each space
	uint32 empty_size = 0 ;
	// virsual address to the start of kernal heap
	uint32 virsual = KERNEL_HEAP_START ;
	bool key = 0 ;
	// key = 1 when find start of free space
	// key = 0 when find this free space ende
	while(virsual != KERNEL_HEAP_MAX)
	   {
	    	 //get the entry of each virtaul to scan the kernal heap

  	   	uint32 * page_table = NULL ;
	   	int page_ptx = PTX(virsual);
	    get_page_table(ptr_page_directory, (void *) virsual , &page_table);
     	int test_present = page_table[page_ptx] & PERM_PRESENT ;
	    	        // check if the present == 0 so the virsual is free and
	    	    	// when locked that means start of free space
	    	    	// and set key = 1
	   if (test_present == 0 && key == 0)
	  	  {
	    	key = 1 ;
	    	array[index_array].start_address =virsual;
	    	//cprintf("1=%x\n",array[index_best_fit].start_address);
	    	empty_size ++ ;
	    }
	    // check if the present == 0 so the virsual is free and
	  	// key == 1 that means the free space started
	 else if (test_present == 0 && key == 1 )
		 	 empty_size ++;

	  // check if the present != 0 so the virsual is not free and
	   // close the key
	else if (test_present != 0 && key == 1)
		{
	    array[index_array].size = empty_size ;
	    //cprintf("size1=%d\n",array[index_best_fit].size);
	    array[index_array].use_size = 0 ;
	  //cprintf("use size1=%d\n",array[index_best_fit].use_size);
	    key = 0 ;
	    index_array ++;
	    empty_size = 0 ;
	   }
	 virsual += PAGE_SIZE;
   }
	    	    // when (while) ended and the key still opened
	    	    // that means there exist new free space in the last of the kernal still not be saved
	    	    // save its size and increament the array counter
	if (key == 1 )
	{
	   array[index_array].size = empty_size ;
	  //cprintf("size1=%d\n",array[index_best_fit].size);
	   array[index_array].use_size = 0 ;
	   //cprintf("use size1=%d\n",array[index_best_fit].use_size)
	   key = 0 ;
	   index_array ++;
	   }

     if(isKHeapPlacementStrategyBESTFIT())
     { //best code
    	    // set initially minim the first element of  the array
    	     int index_min = -1 ;
    	     for (int j=0; j<index_array; j++)
    	             {
    	                 if (array[j].size >= actual_size)
    	                 {
    	                     if (index_min == -1)

    	                    	 {
    	                    	 index_min = j;
    	                    	 }
    	                     else if (array[index_min].size > array[j].size)
    	                    	 index_min = j;
    	                 }
    	         }

    	    if ( index_min == -1 )
    	    {
    	    	return NULL;
    	    }
    	    ////// allocate and map the min space that choosed
    	    uint32 start_mapping = array[index_min].start_address ;
    	    int size_mapping = array[index_min].size ;

    	    for (uint32 j = start_mapping ; j<start_mapping + size; j+= PAGE_SIZE)
    	    {
    	       	struct Frame_Info * frame_info = NULL ;
    	       	int y = allocate_frame(&frame_info);

    	       	if (y != E_NO_MEM)
    	       	{
    	       		map_frame(ptr_page_directory , frame_info , (void *) j , PERM_PRESENT|PERM_WRITEABLE);
    	       	}
    	    }
    	    // save the acual size that i used
    	    array[index_min].use_size = actual_size ;
    	    //cprintf("---------------\n");

    	    // save used space in the second array
    	    taken_array[taken_index].start_address = array[index_min].start_address;
    	    //cprintf("2=%x\n",taken_array[taken_index].start_address);
    	    taken_array[taken_index].size = array[index_min].size;
    	    //cprintf("2size=%d\n",taken_array[taken_index].size);
    	    taken_array[taken_index].use_size = array[index_min].use_size;
    	    //cprintf("2used=%d\n",taken_array[taken_index].use_size);
    	    taken_index++;
    	    // return the start of the min space
    	     return (uint32 *)array[index_min].start_address ;
    		//NOTE: Allocation is based on BEST FIT strategy
    		//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
    		//refer to the project presentation and documentation for details
     }
     else if(isKHeapPlacementStrategyFIRSTFIT())
     {
     	// First fit code
		  int index=-1;
		  for(int i=0 ;i<index_array;i++)
			{
				if(array[i].size>=actual_size)
				{
					index=i;
					break;
				}
			}

		  if(index==-1)
		  {
			  return NULL;
		  }
		//uint32 start_mapping = start_var_address_first_fit ;
		  uint32 start_mapping=array[index].start_address;
		//int size_mapping = size_first_fit ;

		for (uint32 j = start_mapping ; j<start_mapping + size; j+= PAGE_SIZE)
		{
			struct Frame_Info * frame_info = NULL ;
			int y = allocate_frame(&frame_info);

			if (y != E_NO_MEM)
			{
				map_frame(ptr_page_directory , frame_info , (void *) j , PERM_PRESENT|PERM_WRITEABLE);
			}
		}

		//used_size_first_fit= actual_size ;

		taken_array[taken_index].start_address = start_mapping;

		//cprintf("2=%x\n",taken_array[taken_index].start_address);
		taken_array[taken_index].size = array[index].size;

		//cprintf("2size=%d\n",taken_array[taken_index].size);
		taken_array[taken_index].use_size =actual_size;
		//cprintf("2used=%d\n",taken_array[taken_index].use_size);
        taken_index++;


		return (void *)start_mapping;
     }

	//change this "return" according to your answer
	return NULL;

}

/*void* kmalloc(unsigned int size)
{
	// size : by byte
	//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kmalloc()
	// Write your code here, remove the panic and write your code
	struct best_fit array [100] ;
	// ----------------- array for free spaces in the kernal heap -----//
	uint32 index_best_fit = 0 ;
	//kernal heap free spaces counter
	size = ROUNDUP(size , PAGE_SIZE);
	 //cprintf("size=%d\n",size);
	// convert size into #of pages that he need
    uint32 actual_size = size/PAGE_SIZE ;
   // cprintf("actual=%d\n",actual_size);
    // counter to calc the free space to each space
    uint32 empty_size = 0 ;
    // virsual address to the start of kernal heap
    uint32 virsual = KERNEL_HEAP_START ;

    bool key = 0 ;
    // key = 1 when find start of free space
    // key = 0 when find this free space ended
    //cprintf("*******\n");
    while(virsual != KERNEL_HEAP_MAX)
    {
    	//get the entry of each virtaul to scan the kernal heap

    	uint32 * page_table = NULL ;
    	int page_ptx = PTX(virsual);
    	get_page_table(ptr_page_directory, (void *) virsual , &page_table);

    	int test_present = page_table[page_ptx] & PERM_PRESENT ;
        // check if the present == 0 so the virsual is free and
    	// when locked that means start of free space
    	// and set key = 1
    	if (test_present == 0 && key == 0)
    	{
    		key = 1 ;
    		array[index_best_fit].start_address =virsual;
    		//cprintf("1=%x\n",array[index_best_fit].start_address);
    		empty_size ++ ;
    	}
    	// check if the present == 0 so the virsual is free and
    	// key == 1 that means the free space started
    	else if (test_present == 0 && key == 1 )
    	      empty_size ++;

    	// check if the present != 0 so the virsual is not free and
    	// close the key
    	else if (test_present != 0 && key == 1)
    	{
    		array[index_best_fit].size = empty_size ;
    		//cprintf("size1=%d\n",array[index_best_fit].size);
    		array[index_best_fit].use_size = 0 ;
    		//cprintf("use size1=%d\n",array[index_best_fit].use_size);
    		key = 0 ;
    		index_best_fit ++;
    		empty_size = 0 ;
    	}

    	virsual += PAGE_SIZE;

    }
    // when (while) ended and the key still opened
    // that means there exist new free space in the last of the kernal still not be saved
    // save its size and increament the array counter
    if (key == 1 )
    {
    	array[index_best_fit].size = empty_size ;
    	//cprintf("size1=%d\n",array[index_best_fit].size);
    	array[index_best_fit].use_size = 0 ;
    	//cprintf("use size1=%d\n",array[index_best_fit].use_size);
		key = 0 ;
		index_best_fit ++;
    }
    //cprintf("*******\n");
    // ------------- get the minim space of the kernal heap ---//
    // set initially minim the first element of  the array
     int index_min = -1 ;
     for (int j=0; j<index_best_fit; j++)
             {
                 if (array[j].size >= actual_size)
                 {
                     if (index_min == -1)

                    	 {
                    	 index_min = j;
                    	 }
                     else if (array[index_min].size > array[j].size)
                    	 index_min = j;
                 }
             }

    //cprintf("min=%d\n",min);
    //cprintf("index_best_fit=%d\n",index_best_fit);
    //cprintf("index=%d\n",index_min);
    if ( index_min == -1 )
    {
    	return NULL;
    }
    ////// allocate and map the min space that choosed
    uint32 start_mapping = array[index_min].start_address ;
    int size_mapping = array[index_min].size ;

    for (uint32 j = start_mapping ; j<start_mapping + size; j+= PAGE_SIZE)
    {
       	struct Frame_Info * frame_info = NULL ;
       	int y = allocate_frame(&frame_info);

       	if (y != E_NO_MEM)
       	{
       		map_frame(ptr_page_directory , frame_info , (void *) j , PERM_PRESENT|PERM_WRITEABLE);
       	}
    }
    // save the acual size that i used
    array[index_min].use_size = actual_size ;
    //cprintf("---------------\n");

    // save used space in the second array
    taken_array[taken_index].start_address = array[index_min].start_address;
    //cprintf("2=%x\n",taken_array[taken_index].start_address);
    taken_array[taken_index].size = array[index_min].size;
    //cprintf("2size=%d\n",taken_array[taken_index].size);
    taken_array[taken_index].use_size = array[index_min].use_size;
    //cprintf("2used=%d\n",taken_array[taken_index].use_size);
    taken_index++;
    // return the start of the min space
     return (uint32 *)array[index_min].start_address ;
	//NOTE: Allocation is based on BEST FIT strategy
	//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	//refer to the project presentation and documentation for details


	//change this "return" according to your answer
	return NULL;

}*/
void kfree(void* virtual_address)
{

     //TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kfree()
    // Write your code here, remove the panic and write your code
	// check on the existed used previously array
	int index ;
	int arr_index ;
	int i ;
    for ( i = 0 ;i<taken_index; i++ )
    {
       if ( taken_array[i].start_address == (uint32)virtual_address)
       {
    	   //cprintf("find !!\n");

		   index = i ;
		   break ;
       }
    }

    //cprintf("free=%x\n",taken_array[index].start_address);
    // if this virtual existed in this array
    int total=0;
    if ( i < taken_index)
    {
    	//you need to get the size of the given allocation using its address
    	// remove this size (unmap) and become free again
        uint32 size = taken_array[index].use_size;
        //cprintf("freesize=%d\n",size);
		for (uint32 j = (uint32)virtual_address ; j<(uint32)virtual_address + size*PAGE_SIZE; j+=PAGE_SIZE)
		{
			unmap_frame(ptr_page_directory , (void *)j);
		}
		// remove this item from used array becouse it become free now ..
       for(int i=index;i<taken_index;i++)
       {

    	   taken_array[i].start_address=taken_array[i+1].start_address;
    	   taken_array[i].size=taken_array[i+1].size;
    	   taken_array[i].use_size=taken_array[i+1].use_size;
       }
       //decrement the counter of used array
       taken_index--;
//
    }

	//refer to the project presentation and documentation for details


}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kheap_virtual_address()
	// Write your code here, remove the panic and write your code


	//result =0 if not found va of given pa
	unsigned int result=0;
	struct Frame_Info*ptr_fram=to_frame_info(physical_address);
	if(ptr_fram->references==0)
		return 0;

	//loop from start address of heap to max address and counter by page_size
      for (unsigned int i=KERNEL_HEAP_START;i<KERNEL_HEAP_MAX;i+=PAGE_SIZE)
          {

              int table_index=PTX(i);
              unsigned int *ptr =NULL;
              get_page_table(ptr_page_directory,(void*)i,&ptr);
              if (ptr!=NULL)
              {

            	  unsigned int entry =ptr[table_index];
            	  //pa = entry without permissions by and with 0 to 12 bit (3 zero's)such as entry*4k
            	  unsigned int pa=entry &0xfffff000;
            	  //check of perm in entry
            	  unsigned int pentry =entry&=PERM_PRESENT;
            	  //if pentry = 0 means the present in entry =0
            	  //pentry !=0 that's mean present =1
            	  if (pentry!=+0x00000000)
            	  {
            		  //check if pa of va == given physical_address
            		  //if true :-put result =i and break from loop
            		  if (physical_address ==pa )
            		  {
            			  result = i;

            			  break;
            		  }
            	  }
              }
          }


      //finally return result (0 or va)
      return result;
}
unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kheap_physical_address()
	// Write your code here, remove the panic and write your code
	uint32 Fram;
	uint32 *ptr_page_table=NULL;
	//get page table to this virtuall
		get_page_table(ptr_page_directory,(void*)virtual_address,&ptr_page_table);
		if(ptr_page_table!=NULL)
		{
			//get page table entry
			uint32 table_entry = ptr_page_table [PTX(virtual_address)];
			//get  frame number from entry of this virtual
			 Fram=table_entry>>12;
		}
		//return physical=Fram*PAGE_SIZE
		return Fram*PAGE_SIZE;
}




//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().
void *krealloc(void *virtual_address, uint32 new_size)
{
	//return NULL ;
	//TODO: [PROJECT 2019 - BONUS2] Kernel Heap Realloc
	// Write your code here, remove the panic and write your code
     if (virtual_address == NULL)
     {
    	 uint32 new_start_virsual =(uint32) kmalloc(new_size);
    	 return (void *)new_start_virsual;
     }
     else if (new_size == 0)
     {
    	 kfree(virtual_address);
    	 return(void *) virtual_address;
     }
     else
     {
    	    int i ;
    	    int index = -1 ;
    	        for ( i = 0 ;i<taken_index; i++ )
    	        {
    	           if ( taken_array[i].start_address == (uint32)virtual_address)
    	           {
    	        	   //cprintf("find !!\n");
    	    		   index = i ;
    	    		   break ;
    	           }
    	        }
    	        if(index==-1)
    	        {
    	        	return NULL;
    	        }
    	     uint32 start_old_address = taken_array[i].start_address;
    	     int old_size = taken_array[i].use_size;
    	     if (old_size > new_size/PAGE_SIZE)
    	     {
    	    	 return(void *) start_old_address;
    	     }


    	     else if (new_size/PAGE_SIZE > old_size)
    	     {
			 // search about empty page before me or after me
			 //////////////////////////////////////////// before me
			  uint32 test_virtual = start_old_address ;
			  test_virtual  -= PAGE_SIZE;
			  int key = 0 ;
			  int empty_size = 0;			  ////////////////////////// after me
			  test_virtual = (uint32)virtual_address + old_size*PAGE_SIZE ;
			  test_virtual = ROUNDUP(test_virtual , PAGE_SIZE);
			   key = 0 ;
			   empty_size = 0;
			  while(1)
			  {

				  uint32 * page_table_test = NULL ;
				  int ptx_new = PTX(test_virtual);
				  get_page_table(ptr_page_directory ,(void *) test_virtual, &page_table_test);
				  int test_present =page_table_test[PTX(test_virtual)] & PERM_PRESENT;
				  if (test_present == 0 && key == 0)
				  {
					  empty_size ++;
					  key = 1 ;
				  }
				  if (test_present == 0 && key ==1 )
				  {
					  empty_size++;
				  }
				  if (test_present != 0)
				  {
					  key = 0;
					  break;
				  }
				  test_virtual += PAGE_SIZE;
			  }
			  if (key == 1)
			  {
				  key = 0 ;
				  empty_size ++;
			  }

			  if (empty_size > 0 && empty_size+old_size >= new_size/PAGE_SIZE)
			  {
				  uint32 start=start_old_address+(old_size*PAGE_SIZE)
					    , end =start_old_address+(new_size);

                  start =ROUNDUP(start , PAGE_SIZE);
                 end = ROUNDUP(end , PAGE_SIZE);
          for (uint32 j = start ; j<end; j+= PAGE_SIZE)
					{
					  struct Frame_Info * frame_info = NULL ;
						int y = allocate_frame(&frame_info);

						if (y != E_NO_MEM)
						{
							map_frame(ptr_page_directory , frame_info , (void *) j , PERM_PRESENT);
						}
					}

				  taken_array[index].use_size = ROUNDUP(new_size,PAGE_SIZE)/PAGE_SIZE;
				  return(void *) start_old_address;
			  }

			  else
			  {
				  int freeFrames = calculate_free_frames();
				  uint32 new_start_virsual =(uint32) kmalloc(new_size);
				  int free = calculate_free_frames();
				  uint32 ptr = new_start_virsual;
				  uint32 * old_start_virtual = virtual_address;
				  uint32 * new_start_ptr = (uint32 *)new_start_virsual;
				  for (int i =0 ; i<old_size ; i++)
				  {
					  for (int j = 0 ; j < 1024 ; j++)
					  {
						  new_start_ptr[j] = old_start_virtual[j];

					  }
					  old_start_virtual+=1024;
					  new_start_ptr+= 1024;
				  }

					  kfree((void*)virtual_address);
                      return (void *)ptr;
			  }
    	     }
     }



    return NULL;
}
