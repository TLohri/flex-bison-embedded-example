#include "memory_management.h"
#include <stdio.h>

#define MY_NUMBER_OF_PAGES 16
#define MY_PAGE_SIZE 512
#define MY_ALIGNMENT 8
#define MY_MEMORY_SIZE (MY_NUMBER_OF_PAGES*MY_PAGE_SIZE*MY_ALIGNMENT)

#define DEBUG_ENABLE

static long long my_memory[MY_NUMBER_OF_PAGES * MY_PAGE_SIZE];

typedef struct memory_page
{
	size_t used_memory;
	unsigned char consecutive;
}memory_page;

static memory_page memory_info[MY_NUMBER_OF_PAGES];

void print_info(char* action, int number)
{
	printf(action);
	printf(" %i bytes\n", number);
}

void my_memcpy(void* destination, void* source, int size)
{
	int i;
	char* dest_ptr = (char*)destination;
	char* source_ptr = (char*)source;

	if (source == NULL || destination == NULL || size == 0)
	{
		return;
	}

	if (destination < source)
	{
		// copy source from left to right (ascending memory order)
		for (i = 0; i < size; i++)
		{
			dest_ptr[i] = source_ptr[i];
		}
	}
	else
	{
		//copy memory from right to left (descending memory order)
		for (i = size - 1; i >= 0; i--)
		{
			dest_ptr[i] = source_ptr[i];
		}
	}
}

void sanity_check()
{
	int i;
	for (i = 0; i < MY_NUMBER_OF_PAGES; i++)
	{
		if (memory_info[i].used_memory == 0)
		{
			memory_info[i].consecutive = 0;
		}
		
	}
	memory_info[MY_NUMBER_OF_PAGES - 1].consecutive = 0;
}


void initialize()
{
	int i;
	for (i = 0; i < MY_NUMBER_OF_PAGES; i++)
	{
		memory_info[i].consecutive = 0;
		memory_info[i].used_memory = 0;
	}

	for (i = 0; i < MY_NUMBER_OF_PAGES * MY_PAGE_SIZE; i++)
	{
		my_memory[i] = 0xaaaaaaaaaaaaaaaa;
	}

}

void* my_alloc(size_t size)
{

	long long* memory_address = NULL;
	int i,j;
	int start_i = 0;
	int required_number_of_pages;
	int number_of_consecutive_free_pages;

#ifdef DEBUG_ENABLE
	print_info("Allocating", size);
#endif


	sanity_check();

	if (size <= MY_PAGE_SIZE * MY_ALIGNMENT)
	{
		for (i = 0; i < MY_NUMBER_OF_PAGES; i++)
		{
			if (memory_info[i].used_memory == 0)
			{
				memory_info[i].used_memory = size;
				memory_info[i].consecutive = 0;

				memory_address = my_memory + (i * MY_PAGE_SIZE);
				
				return memory_address;
			}
		}
		// No free page available
		return NULL;
	}
	else
	{
		required_number_of_pages = size / (MY_PAGE_SIZE * MY_ALIGNMENT);
		if (required_number_of_pages > MY_NUMBER_OF_PAGES)
		{
			// Never enough memory available
			return NULL;
		}
		number_of_consecutive_free_pages = 0;
		for (i = 0; i < MY_NUMBER_OF_PAGES; i++)
		{
			if (memory_info[i].used_memory != 0)
			{
				// If current page is used, reset variables
				memory_address = NULL;
				number_of_consecutive_free_pages = 0;
				start_i = i + 1;
			}
			else
			{
				// Current Page is free
				if (memory_address == NULL)
				{
					// If this is the first free page, set the memory address
					memory_address = my_memory + (i * MY_PAGE_SIZE);
				}

				number_of_consecutive_free_pages++;

				if (number_of_consecutive_free_pages == required_number_of_pages)
				{
					//Enough memory found, allocate all of it
					for (j = 0; j < required_number_of_pages - 1; j++)
					{
						memory_info[j + start_i].used_memory = MY_PAGE_SIZE * MY_ALIGNMENT;
						memory_info[j + start_i].consecutive = 1;
					}
					memory_info[i].used_memory = size - (required_number_of_pages - 1) * MY_ALIGNMENT * MY_PAGE_SIZE;
					memory_info[i].consecutive = 0;
					return memory_address;
				}
			}
		}
		return NULL;
	}
	return NULL;
}

void* my_realloc(void* ptr, size_t size)
{
	int current_number_of_pages = 0;
	int required_number_of_pages = size / (MY_PAGE_SIZE * MY_ALIGNMENT);;

	int i;

	int page_difference;

	long long* memory_address = (long long*)ptr;

	int distance = memory_address - my_memory;
	int page_number = distance / MY_PAGE_SIZE;

	int current_last_page = page_number;

	int current_size = 0;

#ifdef DEBUG_ENABLE
	print_info("Reallocating", size);
#endif

	sanity_check();

	current_last_page--;
	do
	{
		current_last_page++;
		current_number_of_pages++;
		current_size += memory_info[current_last_page].used_memory;
	} while (memory_info[current_last_page].consecutive == 1);


	// negative if we are freeing pages
	page_difference = required_number_of_pages - current_number_of_pages;

	if (required_number_of_pages == current_number_of_pages)
	{
		//dont change pointer
		memory_info[current_last_page].used_memory = size - required_number_of_pages * MY_ALIGNMENT * MY_PAGE_SIZE;
		return ptr;
	}


	if (required_number_of_pages < current_number_of_pages)
	{
		for (; page_difference != 0; page_difference++, current_last_page--)
		{
			memory_info[current_last_page].consecutive = 0;
			memory_info[current_last_page].used_memory = 0;
		}

		memory_info[current_last_page].used_memory = size - required_number_of_pages * MY_ALIGNMENT * MY_PAGE_SIZE;
		//dont change pointer
		return ptr;
	}

	if (required_number_of_pages > current_number_of_pages)
	{
		// count previous and following free pages

		int following_free_pages = 0;
		int previous_free_pages = 0;
		int allocated_pages = 0;
		i = current_last_page + 1;
		while (i < MY_NUMBER_OF_PAGES && memory_info[i].used_memory == 0 && following_free_pages < page_difference)
		{
			following_free_pages++;
			i++;
		}

		i = page_number - 1;
		while (i >= 0 && memory_info[i].used_memory == 0 && previous_free_pages < page_difference)
		{
			previous_free_pages++;
			i--;
		}

		// check if enough space is available in following pages

		if (following_free_pages == page_difference)
		{
			// success. allocate following pages and keep pointer

			for (i = 0; i < page_difference; i++)
			{
				memory_info[current_last_page + i].used_memory = MY_ALIGNMENT * MY_PAGE_SIZE;
				memory_info[current_last_page + i].consecutive = 1;
			}
			memory_info[current_last_page + i].used_memory = size - required_number_of_pages * MY_ALIGNMENT * MY_PAGE_SIZE;
			memory_info[current_last_page + i].consecutive = 0;

			return ptr;
		}

		// check if enough space is available in all surrounding pages
		if (following_free_pages + previous_free_pages >= page_difference)
		{
			// success. allocate previous pages first, then copy current memory, then return new pointer

			allocated_pages = previous_free_pages;

			for (i = 1; i <= previous_free_pages; i++)
			{
				memory_info[page_number - i].consecutive = 1;
				memory_info[page_number - i].used_memory = MY_ALIGNMENT * MY_PAGE_SIZE;
			}
			
			//allocate remaining pages after allocated memory
			for (i = 0; i < page_difference - allocated_pages; i++)
			{
				memory_info[current_last_page + i].consecutive = 1;
				memory_info[current_last_page + i].used_memory = MY_ALIGNMENT * MY_PAGE_SIZE;
			}

			memory_info[current_last_page + i].used_memory = size - required_number_of_pages * MY_ALIGNMENT * MY_PAGE_SIZE;
			memory_info[current_last_page + i].consecutive = 0;

			

			my_memcpy(memory_address - (MY_PAGE_SIZE * previous_free_pages), ptr, (current_last_page - page_number + 1) * MY_PAGE_SIZE * MY_ALIGNMENT);

			return memory_address - (MY_PAGE_SIZE * previous_free_pages);
		}

		// try to allocate new memory and copy data
		void* newPointer = my_alloc(size);
		if (newPointer != NULL)
		{
			// success. copy memory pages and free old memory segment, then return new pointer

			my_memcpy(newPointer, ptr, size);
			my_free(ptr);

			return newPointer;
		}		

		// fail
		return NULL;
	}



	return NULL;
}

void my_free(void* ptr)
{
#ifdef DEBUG_ENABLE
	print_info("Freeing", 0);
#endif
	long long* memory_address = (long long*)ptr;

	int distance = memory_address - my_memory;
	int page_number = distance / MY_PAGE_SIZE;

	sanity_check();
	
	page_number--;
	do
	{
		page_number++;
		memory_info[page_number].used_memory = 0;

	} while (memory_info[page_number].consecutive == 1);
}

