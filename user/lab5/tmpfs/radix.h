#pragma once

#include <lib/bug.h>
#include <lib/type.h>
#include <lib/errno.h>
#include <lib/string.h>

#define RADIX_NODE_BITS (9)
#define RADIX_NODE_SIZE (1 << (RADIX_NODE_BITS))
#define RADIX_NODE_MASK (RADIX_NODE_SIZE - 1)
#define RADIX_MAX_BITS (64)

/* ceil(a/b) */
#define DIV_UP(a, b) (((a)+(b)-1)/(b))

#define RADIX_LEVELS (DIV_UP(RADIX_MAX_BITS, RADIX_NODE_BITS))

struct radix_node {
	union {
		struct radix_node *children[RADIX_NODE_SIZE];
		void *values[RADIX_NODE_SIZE];
	};
};
struct radix {
	struct radix_node *root;
	void (*value_deleter) (void *);
};

static void init_radix(struct radix *radix)
{
	/* radix->root = calloc(1, sizeof(*radix->root)); */
	radix->root = calloc(1, sizeof(*radix->root));
	BUG_ON(!radix->root);
	radix->value_deleter = NULL;
}

static void init_radix_w_deleter(struct radix *radix,
				 void (*value_deleter) (void *))
{
	init_radix(radix);
	radix->value_deleter = value_deleter;
}

static struct radix_node *new_radix_node(void)
{
	/* struct radix_node *n = calloc(1, sizeof(struct radix_node)); */
	struct radix_node *n = calloc(1, sizeof(struct radix_node));

	if (!n)
		return ERR_PTR(-ENOMEM);

	return n;
}

static int radix_add(struct radix *radix, u64 key, void *value)
{
	struct radix_node *node;
	struct radix_node *new;
	u16 index[RADIX_LEVELS];
	int i;
	int k;

	if (!radix->root) {
		new = new_radix_node();
		if (IS_ERR(new))
			return -ENOMEM;
		radix->root = new;
	}
	node = radix->root;

	/* calculate index for each level */
	for (i = 0; i < RADIX_LEVELS; ++i) {
		index[i] = key & RADIX_NODE_MASK;
		key >>= RADIX_NODE_BITS;
	}

	/* the intermediate levels */
	for (i = RADIX_LEVELS - 1; i > 0; --i) {
		k = index[i];
		if (!node->children[k]) {
			new = new_radix_node();
			if (IS_ERR(new))
				return -ENOMEM;
			node->children[k] = new;
		}
		node = node->children[k];
	}

	/* the leaf level */
	k = index[0];
	node->values[k] = value;

	return 0;
}

static void *radix_get(struct radix *radix, u64 key)
{
	struct radix_node *node;
	u16 index[RADIX_LEVELS];
	int i;
	int k;

	if (!radix->root)
		return NULL;
	node = radix->root;

	/* calculate index for each level */
	for (i = 0; i < RADIX_LEVELS; ++i) {
		index[i] = key & RADIX_NODE_MASK;
		key >>= RADIX_NODE_BITS;
	}

	/* the intermediate levels */
	for (i = RADIX_LEVELS - 1; i > 0; --i) {
		k = index[i];
		if (!node->children[k])
			return NULL;
		node = node->children[k];
	}

	/* the leaf level */
	k = index[0];
	return node->values[k];
}

/* We should allow users to store NULL in radix... */
__attribute__ ((__unused__))
static int radix_del(struct radix *radix, u64 key)
{
	return radix_add(radix, key, NULL);
}

static void radix_free_node(struct radix_node *node, int node_level,
			    void (*value_deleter) (void *))
{
	int i;

	WARN_ON(!node, "should not try to free a node pointed by NULL");

	if (node_level == RADIX_LEVELS - 1) {
		if (value_deleter) {
			for (i = 0; i < RADIX_NODE_SIZE; i++) {
				if (node->values[i])
					value_deleter(node->values[i]);
			}
		}
	} else {
		for (i = 0; i < RADIX_NODE_SIZE; i++) {
			if (node->children[i])
				radix_free_node(node->children[i],
						node_level + 1, value_deleter);
		}
	}

}

static int radix_free(struct radix *radix)
{
	if (!radix || !radix->root) {
		WARN("trying to free an empty radix tree");
		return -EINVAL;
	}
	// recurssively free nodes and values (if value_deleter is not NULL)
	radix_free_node(radix->root, 0, radix->value_deleter);

	return 0;
}
