#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "LinkList.h"

/*
 * public structs
 */
#pragma pack(1)
typedef struct tagCHTbl
{
    int buckets;
	int (*h)(const void *key, const int keySize);
    int (*match)(const int mode, const void *obj1, const void *obj2);
	void (*destroy)(void *obj);
    List* table;

} CHTbl;

typedef struct tagCHItem
{
    char isDynamic;
    void* value;
	int keySize;
	unsigned int bucket;
    char* key;
	void (*destroy)(void *obj);

} CHItem;
#pragma pack()

/*
 * public methods
 */

typedef void(*chtbl_exhandler)(int size);
chtbl_exhandler chtbl_exception_handler(chtbl_exhandler hdlr);

int chtbl_init(CHTbl *htbl, int buckets, int (*h)(const void *key, const int keySize), int (*match)(const int mode, const void *obj1, const void *obj2), void (*destroy)(void *obj));
void chtbl_destroy(CHTbl *htbl);
int chtbl_size(CHTbl *htbl);

void* chtbl_for_each_call(CHTbl *htbl, void* (*func)(void *data, void *userdata), void *userdata);
int chtbl_remove_each_if_func_eq_1(CHTbl *htbl, int (*func)(void *data, void *userdata), void *userdata);
int chtbl_foreach_match(CHTbl *htbl, int (*matchfunc)(const void *key1, const void *key2), void (*cbfunc)(const void *data), void *userdata);

int chtbl_insert_ex(CHTbl *htbl, CHItem* hItem, ListElmt element, void* key, const int keySize, void* value);
void* chtbl_remove_ex(CHTbl *htbl, void* key, const int keySize, void* value);

int chtbl_insert(CHTbl *htbl, void* key, const int keySize, void* value);
void* chtbl_remove(CHTbl *htbl, void* key, const int keySize, void* value);
void* chtbl_lookup(CHTbl *htbl, void* key, const int keySize, void* value);

#endif //__HASH_TABLE_H__
