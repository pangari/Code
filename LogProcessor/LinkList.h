#ifndef __LINK_LIST_H__
#define __LINK_LIST_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef void* ListElmt;
typedef void* List;

#define LIST_LOOKUP    ((int)1)
#define LIST_REMOVE    ((int)2)

#define LIST_MATCH     ((int)1)
#define LIST_NOMATCH   ((int)0)
#define LIST_STOP      ((int)-1)

//
// public methods
//
typedef void(*list_exhandler)(int size);
list_exhandler list_exception_handler(list_exhandler hdlr);

List list_new(int (*match)(const int mode, const void *key1, const void *key2), void (*destroy)(void *data));
int list_delete(List* list);
int list_getsize(List list);

int list_insert_ex(List list, void *data, ListElmt new_element);
int list_insert(List list, void *data);

ListElmt list_remove_ex(List list, void **data);
int list_remove(List list, void **data);

int list_lookup(List list, void **data);

//
// callback methods
//
void* list_for_each_call(List list, void* (*func)(void *data, void *userdata), void *userdata);
int list_remove_each_if_func_eq_1(List list, int (*func)(void *data, void *userdata), void *userdata);
int list_foreach_match(List list, int (*matchfunc)(const void *key1, const void *key2), void (*cbfunc)(const void *data), void *userdata);

//
// element methods
//
void* list_new_element_ex(int extraBytes);
ListElmt list_new_element();

void list_delete_element_ex(void** element);
void list_delete_element(ListElmt** element);

ListElmt list_set_element_ex(void *data);
ListElmt list_set_element(ListElmt element, void *data);

void* list_get_element_ex(ListElmt element);
void* list_get_element(ListElmt element);

void* list_new_element_array(int extraBytes, int count);
void list_delete_element_array(void** arr);

//
// stack / queue methods
//
int list_push_front(List list, ListElmt element);
int list_push_back(List list, ListElmt element);

ListElmt list_pop_front(List list);
ListElmt list_pop_back(List list);

//
// match / free methods
//
int stringMatchFunction(const int mode, const void *key1, const void *key2);
void stringFreeFunction(void *ptr);

int fakeMatch(const void *key1, const void *key2);
void fakeFree(void *ptr);

#endif //__LINK_LIST_H__
