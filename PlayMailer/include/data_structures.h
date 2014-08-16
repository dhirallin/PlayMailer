#ifndef _DATA_STRUCTURES_H
#define _DATA_STRUCTURES_H

typedef struct LinkedList_t 
{
	struct LinkedList_t *next;
	struct LinkedList_t *prev;
	struct LinkedList_t *tail;
	struct LinkedList_t *head;
	void *item;
	size_t size;
} LinkedList, *LinkedList_PTR;

LinkedList_PTR LL_Add(LinkedList_PTR list, void *item);
LinkedList_PTR LL_Insert(LinkedList_PTR list, void *item, int index);
void *LL_RemoveIndex(LinkedList_PTR list, int index);
void *LL_RemoveLast(LinkedList_PTR list);
void *LL_Remove(LinkedList_PTR list);
void *LL_GetItem(LinkedList_PTR list, int index);
int LL_GetItemIndex(LinkedList_PTR list, void *item);
void LL_FreeAll(LinkedList_PTR list);
void LL_Free(LinkedList_PTR list);
size_t LL_Size(LinkedList_PTR list);

int QS_CompareInt(const void * a, const void * b);
void QuickSortInt(int *base, int num);
void QuickSort(void **list, int m, int n, int (*compare)(void *, void *));

#endif