#include "HashTable.h"

chtbl_exhandler gHtblHdlr = NULL;

chtbl_exhandler chtbl_exception_handler(chtbl_exhandler hdlr)
{
	chtbl_exhandler cHdlr = gHtblHdlr;
	gHtblHdlr = hdlr;
	return cHdlr;
}

void master_destroy(void *obj)
{
	CHItem* hItem = (CHItem*)obj;

	if(hItem)
	{
		if(hItem->destroy) hItem->destroy(hItem->value);
		if(hItem->isDynamic) free(hItem);
	}
}

int chtbl_init(CHTbl *htbl, int buckets, int (*h)(const void *key, const int keySize), int (*match)(const int mode, const void *obj1, const void *obj2), void (*destroy)(void *obj))
{
    int i;

    /* Allocate space for the hash table. */
    htbl->table = (List*)malloc(buckets * sizeof(List));

	if(!htbl->table) 
	{
		if(gHtblHdlr) gHtblHdlr(buckets * sizeof(List));
		return 0;
	}

    /* Initialize the buckets. */
    htbl->buckets = buckets;
    htbl->match = match;
    htbl->destroy = destroy;

    for (i = 0; i < htbl->buckets; i++)
    {
        htbl->table[i] = list_new(match, master_destroy);
    }

    /* Encapsulate the functions. */
    htbl->h = h;

    return 1;
}

void chtbl_destroy(CHTbl *htbl)
{
    int i;

    /* Destroy each bucket. */
    for (i = 0; i < htbl->buckets; i++) 
	{
        list_delete(&htbl->table[i]);
    }

    /* Free the storage allocated for the hash table. */
    free(htbl->table);

    /* No operations are allowed now, but clear the structure as a precaution. */
    memset(htbl, 0, sizeof(CHTbl));

    return;
}

int chtbl_size(CHTbl *htbl)
{
    int cnt = 0;
    int i;

    for (i = 0; i < htbl->buckets; i++) 
	{
        cnt += list_getsize(htbl->table[i]);
    }

    return cnt;
}

void* chtbl_for_each_call(CHTbl *htbl, void* (*func)(void *data, void *userdata), void *userdata)
{
    int i;

    for (i = 0; i < htbl->buckets; i++)
    {
        userdata = list_for_each_call(htbl->table[i], func, userdata);
    }

    return userdata;
}

int chtbl_remove_each_if_func_eq_1(CHTbl *htbl, int (*func)(void *data, void *userdata), void *userdata)
{
    int i;
    int count = 0;

    for (i = 0; i < htbl->buckets; i++)
    {
        count += list_remove_each_if_func_eq_1(htbl->table[i], func, userdata);
    }

    return count;
}

int chtbl_foreach_match(CHTbl *htbl, int (*matchfunc)(const void *key1, const void *key2), void (*cbfunc)(const void *data), void *userdata)
{
    int i;
    int count = 0;

    for (i = 0; i < htbl->buckets; i++)
    {
        count += list_foreach_match(htbl->table[i], matchfunc, cbfunc, userdata);
    }

    return count;
}

#undef min
#undef max

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)

int chtbl_insert(CHTbl *htbl, void* key, const int keySize, void* value)
{
	CHItem* data = (CHItem*)malloc(sizeof(CHItem) + keySize);
	if(!data) 
	{
		if(gHtblHdlr) gHtblHdlr(sizeof(CHItem) + keySize);
		return 0;
	}

	data->isDynamic = 1;
	data->key = ((char*)data) + sizeof(CHItem);
    data->keySize = keySize;
	data->destroy = htbl->destroy;

    memcpy(data->key, key, keySize);
    data->value = value;

    data->bucket = ((unsigned int)htbl->h(key, data->keySize)) % htbl->buckets;
    return list_insert(htbl->table[data->bucket], (void*)data);
}

int chtbl_insert_ex(CHTbl *htbl, CHItem* hItem, ListElmt element, void* key, const int keySize, void* value)
{
	hItem->isDynamic = 0;
	hItem->key = (char*)key;
    hItem->keySize = keySize;
	hItem->destroy = NULL;
    hItem->value = value;

    hItem->bucket = ((unsigned int)htbl->h(key, hItem->keySize)) % htbl->buckets;
    return list_insert_ex(htbl->table[hItem->bucket], (void*)hItem, element);
}

void* chtbl_remove(CHTbl *htbl, void* key, const int keySize, void* value)
{
    int retval;
    CHItem _data;
    CHItem* data = &_data;

	data->key = (char*)key;
	data->keySize = keySize;
	data->value = value;

    data->bucket = ((unsigned int)htbl->h(key, keySize)) % htbl->buckets;
    retval = list_remove(htbl->table[data->bucket], (void**)&data);

    if(data && retval)
    {
        value = data->value;
		if(data->isDynamic) free(data);
        return value;
    }

    return NULL;
}

void* chtbl_remove_ex(CHTbl *htbl, void* key, const int keySize, void* value)
{
    ListElmt element;
    CHItem _data;
    CHItem* data = &_data;

	data->key = (char*)key;
	data->keySize = keySize;
	data->value = value;

    data->bucket = ((unsigned int)htbl->h(key, keySize)) % htbl->buckets;
    element = list_remove_ex(htbl->table[data->bucket], (void**)&data);

    if(data && element)
    {
        return data->value;
    }

    return NULL;
}

void* chtbl_lookup(CHTbl *htbl, void* key, const int keySize, void* value)
{
    int retval;
    CHItem _data;
    CHItem* data = &_data;

	data->key = (char*)key;
	data->keySize = keySize;
	data->value = value;

    data->bucket = ((unsigned int)htbl->h(key, keySize)) % htbl->buckets;
    retval = list_lookup(htbl->table[data->bucket], (void**)&data);

    if(data && retval) return data->value;

    return NULL;
}
