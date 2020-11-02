#pragma once

#include <lib/bug.h>
#include <lib/type.h>
#include <lib/defs.h>

#define MIN(a,b) ((a)<(b)?(a):(b))

// get the `type` structure header pointer by having its field ptr
#define container_of(ptr, type, field) \
	((type *)((void *)(ptr) - (u64)(&(((type *)(0))->field))))

// same to `container_of` except that
// return NULL if ptr is NULL
#define container_of_safe(ptr, type, field) ({ \
	typeof (ptr) __ptr = (ptr); \
	type *__obj = container_of(__ptr, type, field); \
	(__ptr ? __obj : NULL); \
})

struct list_head {
	struct list_head *prev;
	struct list_head *next;
};

static inline void init_list_head(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
	new->next = head->next;
	new->prev = head;
	head->next->prev = new;
	head->next = new;
}

static inline void list_append(struct list_head *new, struct list_head *head)
{
	struct list_head *tail = head->prev;
	return list_add(new, tail);
}

static inline void list_del(struct list_head *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

static inline bool list_empty(struct list_head *head)
{
	return (head->prev == head && head->next == head);
}

#define next_container_of_safe(obj, type, field) ({ \
	typeof (obj) __obj = (obj); \
	(__obj ? \
	 container_of_safe(((__obj)->field).next, type, field) : NULL); \
})

#define list_entry(ptr, type, field) \
	container_of(ptr, type, field)

#if 0
#define for_each_node_in_list(node, head) \
	for (node = iter->next; node != head; node = node->next)
#endif

#define for_each_in_list(elem, type, field, head) \
	for (elem = container_of((head)->next, type, field); \
	     &((elem)->field) != (head); \
	     elem = container_of(((elem)->field).next, type, field))

#define __for_each_in_list_safe(elem, tmp, type, field, head) \
	for (elem = container_of((head)->next, type, field), \
	     tmp = next_container_of_safe(elem, type, field); \
	     &((elem)->field) != (head); \
	     elem = tmp, tmp = next_container_of_safe(tmp, type, field))

#define for_each_in_list_safe(elem, tmp, field, head) \
	__for_each_in_list_safe(elem, tmp, typeof (*elem), field, head)

struct hlist_head {
	struct hlist_node *next;
};
struct hlist_node {
	struct hlist_node *next;
	struct hlist_node **pprev;	/* the field that points to this node */
};

static inline void init_hlist_head(struct hlist_head *head)
{
	head->next = NULL;
}

static inline void init_hlist_node(struct hlist_node *node)
{
	node->next = NULL;
	node->pprev = NULL;
}

static inline void hlist_add(struct hlist_node *new, struct hlist_head *head)
{
	new->next = head->next;
	new->pprev = &head->next;
	if (head->next)
		head->next->pprev = &new->next;
	head->next = new;
}

static inline void hlist_del(struct hlist_node *node)
{
	if (node->next)
		node->next->pprev = node->pprev;
	*node->pprev = node->next;
}

static inline bool hlist_empty(struct hlist_head *head)
{
	return head->next == NULL;
}

#define hlist_entry(ptr, type, field) \
	container_of(ptr, type, field)

// the head is of `hlist_head`
// the elem is of `dentry`
#define __for_each_in_hlist(elem, type, field, head) \
	for (elem = container_of_safe((head)->next, type, field); \
	     elem; \
	     elem = elem ? \
	     container_of_safe(((elem)->field).next, type, field) : NULL)

// `elem` is the a element in a special linked list linked by the `field` in
// this element from `head`
#define for_each_in_hlist(elem, field, head) \
	__for_each_in_hlist(elem, typeof (*elem), field, head)

#define __for_each_in_hlist_safe(elem, tmp, type, field, head) \
	for (elem = container_of_safe((head)->next, type, field), \
	     tmp = next_container_of_safe(elem, type, field); \
	     elem; \
	     elem = tmp, \
	     tmp = next_container_of_safe(elem, type, field))

#define for_each_in_hlist_safe(elem, tmp, field, head) \
	__for_each_in_hlist_safe(elem, tmp, typeof (*elem), field, head)

#if 0
/*
 * We would better not add prints as the dependencies.
 * Too complicated.
 **/
static inline void kprint_hlist(struct hlist_head *head)
{
	struct hlist_node *node;

	kdebug(" ---- hlist: head=%p ---- \n", head);
	for (node = head->next; node; node = node->next) {
		kdebug("-> %p\n", node);
		if (node == node->next) {
			kdebug("looped list @%p!\n", node);
			BUG("dead loop");
		}
	}
}
#else
static inline void kprint_hlist(struct hlist_head *head)
{
}
#endif
