
#include "map.h"
#include "mem.h"
#include <assert.h>
#include <string.h>

#define branch_i(p,ch) ((p)->children[0]==(ch) ? 0 : 1)

map_t map_create(heap_t h) {
	map_t res = (map_t) heap_alloc(h, sizeof(map_s));
	res->heap = h;
	res->node = 0;
	res->length = 0;
	return res;
}

map_node_t map_begin(map_t m) {
	return m->node;
}

map_node_t map_next(map_node_t n) {
	assert(n);
	if(n->children[0])
		return n->children[0];
	else if(n->children[1])
		return n->children[1];
	else {
		while(n->parent) {
			if(n->parent->children[0]==n && n->parent->children[1])
				return n->parent->children[1];
			else
				n = n->parent;
		}
		return 0;
	}
}

map_node_t map_find(map_t m, str_t s) {
	return map_find_bin(m, str_begin(s), str_length(s));
}
map_node_t map_find_cs(map_t m, const char *key) {
	return map_find_bin(m, key, strlen(key));
}
map_node_t map_find_bin(map_t m, const void *key, size_t key_len) {
	map_node_t i = m->node;
	while(i) {
		int cmp_res = str_compare_bin(i->key, key, key_len);
		if(cmp_res==0)
			return i;
		i = i->children[(cmp_res>0) ? 0 : 1];
	}
	return 0;
}

void *map_get(map_t m, str_t key) {
	return map_get_bin(m, key->data, key->length);
}

void *map_get_cs(map_t m, const char *key) {
	return map_get_bin(m, key, strlen(key));
}

void *map_get_bin(map_t m, const void *key, size_t key_len) {
	map_node_t i = m->node;
	while(i) {
		int cmp_res = str_compare_bin(i->key, key, key_len);
		if(cmp_res==0)
			return i->value;
		i = i->children[(cmp_res>0) ? 0 : 1];
	}
	return 0;
}

static map_t map_set_internal(map_t m, str_t key, void *value) {

}

map_t map_set(map_t m, str_t key, void *value) {
	return map_set_bin(m, str_begin(key), str_length(key), value);
}

map_t map_set_cs(map_t m, const char *key, void *value) {
	return map_set_bin(m, key, strlen(key), value);
}

map_t map_set_bin(map_t m, const void *key, size_t key_len, void *value) {
	map_node_t i = m->node;
	if(i) {
		while(1) {
			int cmp_res = str_compare_bin(i->key, key, key_len);
			if(cmp_res==0) {
				i->value = value;
				break;
			} else {
				int ci = cmp_res<0 ? 1 : 0;
				if(i->children[ci])
					i = i->children[ci];
				else {
					map_node_t j = (map_node_t) heap_alloc(m->heap, sizeof(map_node_s));
					j->length = 1;
					j->parent = i;
					j->children[0] = 0;
					j->children[1] = 0;
					j->key = str_from_bin(m->heap, key, key_len);
					j->value = value;
					i->children[ci] = j;
					for(; i; i=i->parent)
						i->length++;
					m->length++;
					break;
				}
			}
		}
	} else {
		i = m->node = (map_node_t) heap_alloc(m->heap, sizeof(map_node_s));
		m->length = i->length = 1;
		i->parent = 0;
		i->children[0] = 0;
		i->children[1] = 0;
		i->key = str_from_bin(m->heap, key, key_len);
		i->value = value;
	}
	return m;
}

void map_remove(map_t m, str_t key) {
	map_remove_node(m, map_find_bin(m, str_begin(key), str_length(key)));
}
void map_remove_cs(map_t m, const char *key) {
	map_remove_node(m, map_find_bin(m, key, strlen(key)));
}
void map_remove_bin(map_t m, const void *key, size_t key_len) {
	map_remove_node(m, map_find_bin(m, key, key_len));
}
map_node_t map_remove_node(map_t m, map_node_t node) {
	if(node) {
		if(node->children[0] && node->children[1]) {
			size_t rlen = node->children[1]->length;
			map_node_t i = node->children[0];
			while(i->children[1]) {
				i->length += rlen;
				i = i->children[1];
			}
			i->length += rlen;
			i->children[1] = node->children[1];
			i->children[1]->parent = i;
			node->children[1] = 0;
		}
		if(node->parent) {
			map_node_t p = node->parent;
			int cn = p->children[0]==node ? 0 : 1;
			p->children[cn] = node->children[0] ? node->children[0] : node->children[1];
			p->children[cn]->parent = p;
			for(; p; p = p->parent)
				p->length--;
		}
		str_delete(node->key);
		heap_free(node);
		m->length--;
	}
	return 0;
}

typedef struct node_list_ss {
	map_node_t first;
	map_node_t last;
} node_list_s;

static node_list_s to_list(map_node_t nd) {
	node_list_s res;
	if(nd->children[0] && nd->children[1]) {
		node_list_s l1 = to_list(nd->children[0]);
		res = to_list(nd->children[1]);
		nd->children[0] = 0;
		nd->children[1] = 0;
		l1.last->parent = nd;
		nd->parent = res.first;
		res.first = l1.first;
	} else if(nd->children[0]) {
		res = to_list(nd->children[0]);
		nd->children[0] = 0;
		nd->parent = 0;
		res.last->parent = nd;
		res.last = nd;
	} else if(nd->children[1]) {
		res = to_list(nd->children[1]);
		nd->children[1] = 0;
		nd->parent = res.first;
		res.first = nd;
	} else {
		nd->parent = 0;
		res.first = nd;
		res.last = nd;
	}
	return res;
}

static map_node_t to_tree(node_list_s *l, int len) {
	int tmp_c = 0;
	map_node_t tmp_x;
	for(tmp_x = l->first; tmp_x; tmp_x = tmp_x->parent)
		tmp_c++;
	if(tmp_c!=len)
		return 0;
	if(len==1) {
		l->first->length = 1;
		return l->first;
	} else if(len==2) {
		l->last->children[0] = l->first;
		l->first->parent = l->last;
		l->first->length = 1;
		l->last->length = 2;
		return l->last;
	} else {
		int fl = len/2;
		map_node_t i = l->first;
		int c;
		node_list_s l1, l2;
		for(c=0; c<(fl-1); c++)
			i = i->parent;
		l1.first = l->first;
		l1.last = i;
		l2.first = i->parent->parent;
		l2.last = l->last;
		i = i->parent;
		l2.last->parent = 0;
		l1.last->parent = 0;
		i->children[0] = to_tree(&l1, fl);
		i->children[1] = to_tree(&l2, len-fl-1);
		i->children[0]->parent = i;
		i->children[1]->parent = i;
		i->length = len;
		return i;
	}
}

map_t map_optimize(map_t m) {
	if(m->node) {
		int len = m->node->length;
		node_list_s l = to_list(m->node);
		m->node = to_tree(&l, len);
		m->node->parent = 0;
	}
	return m;
}
