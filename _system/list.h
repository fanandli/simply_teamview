#pragma once
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

struct list_head {
  struct list_head *next, *prev;
};

#define list_entry(ptr, type, member) container_of(ptr, type, member)

//#define offsetof(TYPE, MEMBER) ((size_t)& ((TYPE *)0)->MEMBER)  

#define container_of(ptr, type, member) (type*)((char *)ptr - offsetof(type,member))

#define list_for_each(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_del(_head) \
  for (list_head* __next, *_pos = (_head)->next; __next=_pos->next, _pos != (_head); _pos =__next)

#define container_of_del(_type, _member) (_type*)((char *)_pos - offsetof(_type,_member))

//#define LIST_HEAD_INIT(name) { &(name), &(name) }                 //prev和next都指向自己

inline void INIT_LIST_HEAD(struct list_head *list) {
  list->next = list;
  list->prev = list;
}

//new在编译器里被认作关键字   原代码是new 被改为l_new
//l_new 是要被插入的节点
//prev 是插入点前面的一个节点
//next 是插入点后面的一个节点
inline void __list_add(struct list_head *l_new,
  struct list_head *prev,
  struct list_head *next) {
  next->prev = l_new;
  l_new->next = next;
  l_new->prev = prev;
  prev->next = l_new;
}

//这个函数对上面简化 新加入的节点在head和head->next之间  也就是head之后
inline void list_add(struct list_head *l_new, struct list_head *head) {
  __list_add(l_new, head, head->next);
}

//这个函数对上面简化 新加入的节点在head->prev和head之间  也就是head之前
inline void list_add_tail(struct list_head *l_new, struct list_head *head) {
  __list_add(l_new, head->prev, head);
}

//删除一个双向列表中的一个节点 删除节点在prev和next之间
inline void __list_del(struct list_head *prev, struct list_head *next) {
  next->prev = prev;
  prev->next = next;
}

//从列表中删除entry节点，这个函数是对上面函数的简化
//entry->next = LIST_POISON1;
//entry->prev = LIST_POISON2;
inline void list_del(struct list_head *entry) {
  __list_del(entry->prev, entry->next);
//  entry->next = nullptr;    //这两个指针可以不处理，如果从list中del掉了一个节点，
//  entry->prev = nullptr;    //接下来 这个节点就要被删除
}

//判断list是不是最后一个节点
//head节点是队列的第一个节点
//return list->next == head  或者 return head->prev == list一样
inline int list_is_last(const struct list_head *list,
  const struct list_head *head) {
  return list->next == head;
}

//列表是不是为空
//head节点 是队列的头节点
inline int list_empty(const struct list_head *head) {
  return head->next == head;
}

#ifdef __cplusplus
}
#endif //__cplusplus

