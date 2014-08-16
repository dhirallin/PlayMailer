#include <stdlib.h>
#include "data_structures.h"

size_t LL_Size(LinkedList_PTR list)
{
	if(list->head != NULL) 
		list = list->head;

	return list->size;
}

LinkedList_PTR LL_Add(LinkedList_PTR list, void *item)
{
	LinkedList_PTR newNode;
	
	if(list->head == NULL) 
	{
		list->tail = list;
		list->head = list;
	}
	list = list->head;

	newNode = (LinkedList_PTR)malloc(sizeof(LinkedList));
	newNode->prev = list->tail;
	newNode->head = list;
	newNode->next = NULL;
	newNode->item = item;
	(list->tail)->next = newNode;
	list->tail = newNode;

	list->size++;

	return newNode;
}

LinkedList_PTR LL_Insert(LinkedList_PTR list, void *item, int index)
{
	LinkedList_PTR newNode, iter;
	
	if(list->head == NULL) 
	{
		list->tail = list;
		list->head = list;
	}
	list = list->head;

	newNode = (LinkedList_PTR)malloc(sizeof(LinkedList));
	newNode->head = list;
	newNode->item = item;
	
	iter = list;
	for(; index > 0; index--)
	{
		if(!iter->next) break;
		iter = iter->next;
	}

	newNode->next = iter->next;
	newNode->prev = iter;

	if(!iter->next)
		list->tail = newNode;
	else
		(iter->next)->prev = newNode;

	iter->next = newNode;

	list->size++;

	return newNode;
}

void *LL_RemoveIndex(LinkedList_PTR list, int index)
{
	while(list->next != NULL)
	{
		list = list->next;
		index--;
		if(index == -1) 
			return(LL_Remove(list));
	}
	return NULL;
}

void *LL_Remove(LinkedList_PTR list)
{
	void *item;

	(list->prev)->next = list->next;
	if(list->next) (list->next)->prev = list->prev;
	else (list->head)->tail = list->prev;
	item = list->item;
	(list->head)->size--;

	free(list);

	return item;
}

void *LL_RemoveLast(LinkedList_PTR list)
{
	LinkedList_PTR tail;
	void *item;

	if(list->tail == list || list->tail == NULL) return NULL;
	tail = list->tail;

	(tail->prev)->next = NULL;
	list->tail = tail->prev;

	item = tail->item;
	list->size--;

	free(tail);

	return item;
}

void *LL_GetItem(LinkedList_PTR list, int index)
{
	if(index < 0) return NULL;
	
	while(list->next != NULL)
	{
		list = list->next;
		index--;
		if(index == -1) 
			return list->item;
	}
	return NULL;
}

int LL_GetItemIndex(LinkedList_PTR list, void *item)
{
	int index = -1;

	while(list->next != NULL)
	{
		list = list->next;
		index++;

		if(item == list->item) 
			return index;
	}
	return index;
}

void LL_Free(LinkedList_PTR list)
{
	LinkedList_PTR curr, head;
	
	head = list;
	list = list->next;

	while(list)
	{
		curr = list;
		list = list->next;
		free(curr);
	}

	head->next = NULL;
	head->tail = head;
	head->size = 0;
}

void LL_FreeAll(LinkedList_PTR list)
{
	LinkedList_PTR curr, head;
	
	head = list;
	list = list->next;

	while(list)
	{
		curr = list;
		list = list->next;
		if(curr->item) free(curr->item);
		free(curr);
	}

	head->next = NULL;
	head->tail = head;
	head->size = 0;
}

int QS_CompareInt(const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

void QuickSortInt(int *base, int num)
{
	qsort(base, num, sizeof(int), QS_CompareInt);
}

