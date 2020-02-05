#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LIST_LOOKUP    ((int)1)
#define LIST_REMOVE    ((int)2)

#define LIST_MATCH     ((int)1)
#define LIST_NOMATCH   ((int)0)
#define LIST_STOP      ((int)-1)

#undef min
#undef max

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)

//
// private defines
//

#define list_size(list) ((list)->size)
#define list_head(list) ((list)->head)
#define list_tail(list) ((list)->tail)
#define list_data(element) ((element)->data)
#define list_next(element) ((element)->next)
#define list_prev(element) ((element)->prev)

#define list_is_head(list, element) ((element) == (list)->head ? 1 : 0)
#define list_is_tail(element) ((element)->next == NULL ? 1 : 0)

//
// private structs
//

typedef struct tagListElmt;
typedef struct tagList;

typedef struct tagListElmt* ListElmt;
typedef struct tagList* List;

#pragma pack(1)
typedef struct tagListElmt
{
    char                isDynamic;
    void                *data;
    struct tagListElmt	*next;
    struct tagListElmt	*prev;
} _ListElmt;

typedef struct tagList
{
    int						size;
    int						(*match)(const int mode, const void *key1, const void *key2);
    void					(*destroy)(void *data);
    struct tagListElmt*		head;
    struct tagListElmt*		tail;
} _List;
#pragma pack()

typedef void(*list_exhandler)(int size);
list_exhandler gListHdlr = NULL;

list_exhandler list_exception_handler(list_exhandler hdlr)
{
    list_exhandler cHdlr = gListHdlr;
    gListHdlr = hdlr;
    return cHdlr;
}

//
// private methods
//

// INTERNAL
int list_ins_next_ex(void* vList, ListElmt element, const void *data, ListElmt new_element)
{
    List list = (List)vList; 

    if (!list) return 0;
    if (!new_element) return 0;

    // Insert the element into the list.
    list_data(new_element) = (void *)data;

    if (element == NULL)
    {
        // Handle insertion at the head of the list.
        if (list_size(list) == 0)
        {
            list_tail(list) = new_element;
        }
        else
        {
            list_prev(list_head(list)) = new_element;
        }

        list_prev(new_element) = NULL;
        list_next(new_element) = list_head(list);
        list_head(list) = new_element;
    }
    else
    {
        // Handle insertion somewhere other than at the head.
        if(list_is_tail(list_next(element)))
        {
            list_tail(list) = new_element;
        }

        list_prev(new_element) = element;
        list_next(new_element) = list_next(element);
        list_next(element) = new_element;
    }

    // Adjust the size of the list to account for the inserted element.
    list_size(list)++;

    return 1;
}

// INTERNAL
int list_ins_next(void* vList, ListElmt element, const void *data)
{
    List list = (List)vList; 
    ListElmt new_element = NULL;

    if (!list) return 0;

    // Allocate storage for the element.
    new_element = (ListElmt)malloc(sizeof(_ListElmt));

    if(!new_element)
    {
        if(gListHdlr) gListHdlr(sizeof(_ListElmt));
        return 0;
    }

    new_element->isDynamic = 1;

    return list_ins_next_ex(list, element, data, new_element);
}

// INTERNAL
ListElmt list_rem_next_ex(void* vList, ListElmt element, void **data)
{
    List list = (List)vList; 
    ListElmt old_element = NULL;

    // Do not allow removal from an empty list.
    if (list_size(list) == 0) return old_element;

    // Remove the element from the list.
    if (element == NULL)
    {
        // Handle removal from the head of the list.
        if(data) *data = list_data(list_head(list));
        old_element = list_head(list);
        list_head(list) = list_next(list_head(list));

        if (list_size(list) == 1)
            list_tail(list) = NULL;
        else
            list_prev(list_head(list)) = NULL;
    }
    else
    {
        // Handle removal from somewhere other than the head.
        if (list_is_tail(element)) return old_element;

        if(data) *data = list_data(list_next(element));
        old_element = list_next(element);
        list_next(element) = list_next(list_next(element));

        if (list_is_tail(element))
            list_tail(list) = element;
    }

    // Adjust the size of the list to account for the removed element.
    list_size(list)--;

    list_prev(old_element) = NULL;
    list_next(old_element) = NULL;

    return old_element;
}

// INTERNAL
int list_rem_next(void* vList, ListElmt element, void **data)
{
    List list = (List)vList; 
    ListElmt old_element = list_rem_next_ex(list, element, data);

    if(old_element)
    {
        // Free the storage allocated by the abstract data type.
        if(old_element->isDynamic) free(old_element);
        return 1;
    }
    else
        return 0;
}

// INTERNAL
int stringMatchFunction(const int mode, const void *key1, const void *key2)
{
    int res = 0; mode;
    res = strcmp((char*)key1, (char*)key2);
    return res == 0 ? LIST_MATCH : LIST_NOMATCH;
}

// INTERNAL
void stringFreeFunction(void *ptr)
{
    if (ptr != NULL) free(ptr);
}

// INTERNAL
int fakeMatch(const int mode, const void *key1, const void *key2)
{
    mode;
    return (key1 == key2) ? LIST_MATCH : LIST_NOMATCH;
}

// INTERNAL
void fakeFree(void *ptr)
{
}

// INTERNAL
int list_internal_lookup(void* vList, void **data)
{
    List list = (List)vList; 
    int mCode;
    ListElmt element;

    if(list->match && data)
    {
        // Search for the data in the bucket.
        for (element = list_head(list); element != NULL; element = list_next(element))
        {
            mCode = list->match(LIST_LOOKUP, *data, list_data(element));

            if(mCode == LIST_MATCH)
            {
                // Pass back the data from the table.
                *data = list_data(element);
                return 1;
            }
            else if(mCode == LIST_STOP)
            {
                return 0;
            }
        }
    }

    // Return that the data was not found.
    return 0;
}

//
// public methods
//
void* list_new(int (*match)(const int mode, const void *key1, const void *key2), void (*destroy)(void *data))
{
    List new_list = (List)malloc(sizeof(_List));

    if(!new_list)
    {
        if(gListHdlr) gListHdlr(sizeof(_List));
        return NULL;
    }

    // Initialize the list.
    list_size(new_list) = 0;

    new_list->destroy = destroy;
    new_list->match = match;

    list_head(new_list) = NULL;
    list_tail(new_list) = NULL;

    return (void*)new_list;
}

int list_delete(void** vPList)
{
    List* pList = (List*)vPList;
    void *data = NULL;
    List list = NULL;
    int count = 0;

    if(!pList || !(*pList)) return count;
    list = (*pList);

    // Remove each element.
    while (list_size(list) > 0)
    {
        if (list_rem_next(list, NULL, (void **)&data) == 1)
        {
            count++;
            if(list->destroy && data)
            {
                // Call a user-defined function to free dynamically allocated data.
                list->destroy(data);
            }
        }
    }

    free(list);
    (*pList) = NULL;

    return count;
}

void list_delete_element_array(void** arr)
{
    if(!arr || !(*arr)) return;

    free((*arr));

    (*arr) = NULL;
}

void* list_new_element_array(int extraBytes, int count)
{
    char* buffer = NULL;
    ptrdiff_t* arrayBuffer = NULL;
    char* elementBuffer = NULL;
    ListElmt element;
    int idx;
    int arraySize = sizeof(ListElmt) * count;
    int blockSize = sizeof(_ListElmt) + extraBytes;

    if(extraBytes < 0 || count < 0) return NULL;

    buffer = (char*)malloc(arraySize + (blockSize * count));

    if(!buffer)
    {
        if(gListHdlr) gListHdlr(arraySize + (blockSize * count));
        return NULL;
    }

    arrayBuffer = (ptrdiff_t*)buffer;
    elementBuffer = buffer + arraySize;

    for(idx = 0; idx < count; idx++)
    {
        element = (ListElmt)&elementBuffer[(blockSize * idx)];
        memset(element, '\x00', sizeof(_ListElmt));

        arrayBuffer[idx] = (ptrdiff_t)(((char*)element) + sizeof(_ListElmt));
        memset(&elementBuffer[(blockSize * idx) + sizeof(_ListElmt)], '\x00', extraBytes);
    }

    return buffer;
}

void* list_new_element_ex(int extraBytes)
{
    char* buffer = NULL;

    if(extraBytes < 0) return NULL;

    buffer = (char*)malloc(sizeof(_ListElmt) + extraBytes);

    if(!buffer)
    {
        if(gListHdlr) gListHdlr(sizeof(_ListElmt) + extraBytes);
        return NULL;
    }

    memset(buffer, '\x00', sizeof(_ListElmt));
    buffer += sizeof(_ListElmt);
    memset(buffer, '\x00', extraBytes);

    return buffer;
}
ListElmt list_new_element()
{
    return (ListElmt)list_new_element_ex(0);
}

void list_delete_element_ex(void** element)
{
    char* buffer;

    if(!element || !(*element)) return;

    buffer = (char*)(*element);
    buffer -= sizeof(_ListElmt);

    free(buffer);

    (*element) = NULL;
}
void list_delete_element(ListElmt** element)
{
    if(!element || !(*element)) return;

    free((*element));

    (*element) = NULL;
}

void* list_set_element_ex(void* element)
{
    ListElmt orig_element;
    char* buffer = (char*)element;

    if(!buffer) return NULL;

    buffer -= sizeof(_ListElmt);

    orig_element = (ListElmt)buffer;

    list_data(orig_element) = element;

    return (void*)orig_element;
}

ListElmt list_set_element(void* vElement, void *data)
{
    ListElmt element = (ListElmt)vElement;
    
    if(!element) return NULL;

    list_data(element) = data;

    return element;
}

void* list_get_element_ex(void* vElement)
{
    ListElmt element = (ListElmt)vElement;
    char* buffer = (char*)element;

    if(!buffer) return NULL;

    buffer += sizeof(_ListElmt);

    return buffer;
}

void* list_get_element(void* vElement)
{
    ListElmt element = (ListElmt)vElement;

    if(!element) return NULL;

    return list_data(element);
}

int list_getsize(void* vList)
{
    List list = (List)vList; 

    if(!list) return 0;

    return list_size(list);
}

int list_lookup(void* vList, void **data)
{
    List list = (List)vList; 

    return list_internal_lookup(list, data);
}

int list_insert_ex(void* vList, void *data, void* vNew_element)
{
    List list = (List)vList; 
    ListElmt new_element = (ListElmt)vNew_element;
    int retval = -1;
    void *temp = data;

    if(!list) return 0;

    // check if already there
    if(list->match && list_internal_lookup(list, &temp) == 1)
    {
        // element already there
        retval = 0;
    }
    else
    {
        // create element in list
        if(new_element)
            retval = list_ins_next_ex(list, NULL, data, new_element);
        else
            retval = list_ins_next(list, NULL, data);
    }

    return retval;
}
int list_insert(void* vList, void *data)
{
    List list = (List)vList; 

    return list_insert_ex(list, data, NULL);
}

void* list_remove_ex(void* vList, void **data)
{
    List list = (List)vList; 
    int mCode;
    ListElmt prev = NULL;
    ListElmt element = NULL;

    if(!list) return NULL;
    if(!list->match) return NULL;

    // Search for the data in list.
    for (element = list_head(list); element != NULL; element = list_next(element))
    {
        mCode = list->match(LIST_REMOVE, *data, list_data(element));

        if(mCode == LIST_MATCH)
        {
            return (void*)list_rem_next_ex(list, prev, data);
        }
        else if(mCode == LIST_STOP)
        {
            return NULL;
        }

        prev = element;
    }

    // Return that the data was not found.
    return NULL;
}
int list_remove(void* vList, void **data)
{
    List list = (List)vList; 
    int mCode;
    ListElmt prev = NULL;
    ListElmt element = NULL;

    if(!list) return 0;
    if(!list->match) return 0;

    // Search for the data in list.
    for (element = list_head(list); element != NULL; element = list_next(element))
    {
        mCode = list->match(LIST_REMOVE, *data, list_data(element));

        if(mCode == LIST_MATCH)
        {
            return list_rem_next(list, prev, data);
        }
        else if(mCode == LIST_STOP)
        {
            return 0;
        }

        prev = element;
    }

    // Return that the data was not found.
    return 0;
}

//
// callback methods
//

void* list_for_each_call(void* vList, void* (*func)(void *data, void *userdata), void *userdata)
{
    List list = (List)vList; 
    ListElmt element;
    int retval = 0;

    // Search for the data in list.
    for (element = list_head(list); element != NULL && retval==0; element = list_next(element))
    {
        userdata = func(list_data(element), userdata);
    }

    return userdata;
}


int list_remove_each_if_func_eq_1(void* vList, int (*func)(void *data, void *userdata), void *userdata)
{
    List list = (List)vList; 
    ListElmt next;
    ListElmt prev;
    ListElmt element;

    int count = 0;
    void *data = NULL;

    // Search for the data in list.
    prev = NULL;

    for (element = list_head(list); element != NULL;)
    {
        next = list_next(element);
        if (func(list_data(element), userdata) == 1)
        {
            // Remove the data from list.
            if (list_rem_next(list, prev, &data) == 1)
            {
                count++;
                if(list->destroy && data)
                {
                    // Call a user-defined function to free dynamically allocated data.
                    list->destroy(data);
                }
            }
        }
        else
        {
            prev = element;
        }
        element = next;
    }

    return count;
}

int list_foreach_match(void* vList, int (*matchfunc)(const void *key1, const void *key2), void (*callbackfunc)(const void *data), void *userdata)
{
    List list = (List)vList; 
    ListElmt element;
    int count = 0;

    for (element = list_head(list); element != NULL ; element = list_next(element))
    {
        if (matchfunc(userdata, list_data(element)))
        {
            callbackfunc(list_data(element));
            count++;
        }
    }

    return count;
}


//
// stack / queue methods
//

int list_push_front(void* vList, void* vElement)
{
    List list = (List)vList; 
    ListElmt element = (ListElmt)vElement;

    return list_ins_next_ex(list, NULL, element->data, element);
}
void* list_pop_front(void* vList)
{
    List list = (List)vList; 

    return (void*)list_rem_next_ex(list, NULL, NULL);
}

int list_push_back(void* vList, void* vElement)
{
    List list = (List)vList; 
    ListElmt element = (ListElmt)vElement;

    if (!list) return 0;
    if (!element) return 0;

    // Handle insertion at the head of the list.
    if (list_size(list) == 0)
    {
        list_head(list) = element;
    }
    else
    {
        list_next(list_tail(list)) = element;
    }

    list_next(element) = NULL;
    list_prev(element) = list_tail(list);
    list_tail(list) = element;

    // Adjust the size of the list to account for the inserted element.
    list_size(list)++;

    return 1;
}
void* list_pop_back(void* vList)
{
    List list = (List)vList; 
    ListElmt old_element = NULL;

    // Do not allow removal from an empty list.
    if (list_size(list) == 0) return old_element;

    // Handle removal from the head of the list.
    old_element = list_tail(list);
    list_tail(list) = list_prev(list_tail(list));

    if (list_size(list) == 1)
        list_head(list) = NULL;
    else
        list_next(list_tail(list)) = NULL;

    // Adjust the size of the list to account for the removed element.
    list_size(list)--;

    list_prev(old_element) = NULL;
    list_next(old_element) = NULL;

    return (void*)old_element;
}
