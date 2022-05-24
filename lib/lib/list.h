#ifndef  __LIST_KERNEL_LIST_H
#define  __LIST_KERNEL_LIST_H
#include "global.h"
#define  offset(struct_type,member) (int)(&((struct_type*)0)->member)
// elem_ptr是等待转换的地址，它属于某个结构体中某个成员的地址
// struct_member_name 是elem_ptr所在结构体中对应地址的成员名字
// struct_type 是elem_ptr是所属的结构体类型
#define  elem2entry(struct_type, struct_member_name, elem_ptr) \
    (struct_type*)((int)elem_ptr - offset(struct_type, struct_member_name))

//定义链表节点成员结构
//节点中不需要数据成员，只要求前驱和后驱结点指针
struct list_elem {
    struct list_elem* prev;  // 前驱
    struct list_elem* next;  // 后驱结点
};

// 链表结构，用来实现队列
struct list {
    // head是队首，是固定不变的，不是第一个元素，第一个元素是head.next
    struct list_elem head;
    struct list_elem tail;
};
// 自定义函数类型function,用于在list_tracersal中做会调函数
typedef  int (function)(struct list_elem*, int);

void list_init(struct list*);

void list_init(struct list*);
void list_insert_before(struct list_elem* before,struct list_elem* elem);
void list_push(struct list* plist,struct list_elem* elem);
void list_iterate(struct list* plist);
void list_append(struct list* plist, struct list_elem* elem);
void list_remove(struct list_elem* pelem);
struct list_elem* list_pop(struct list* plist);
int list_empty(struct list* plist);
uint32_t list_len(struct list* plist);
struct list_elem* list_traversal(struct list* plist, function func,int arg);
int elem_find(struct list* plist, struct list_elem* obj_elem);


#endif
