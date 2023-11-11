/**
 * \author Lander Van Loock
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

/*
 * The real definition of struct list / struct node
 */

struct dplist_node {
    dplist_node_t *prev, *next;
    void *element;
};

struct dplist {
    dplist_node_t *head;

    void *(*element_copy)(void *src_element);

    void (*element_free)(void **element);

    int (*element_compare)(void *x, void *y);
};


dplist_t *dpl_create(// callback functions
        void *(*element_copy)(void *src_element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y)
) {
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

void dpl_free(dplist_t **list, bool free_element) {

    if (*list == NULL)
        return;
    else if ((*list)->head == NULL) {
        free(*list);
        *list = NULL;
    } else {
        dplist_t *p = *list;
        dplist_node_t *node = p->head;
        if (node->next == NULL) {
            if (free_element)
                p->element_free(&(node->element));
            free(node);
        } else {
            dplist_node_t *next_node = node->next;
            dplist_node_t *next_next_node = NULL;
            if (free_element)
                p->element_free(&(node->element));
            free(node);
            while (next_node != NULL) {
                next_next_node = next_node->next;
                if (free_element)
                    p->element_free(&(next_node->element));
                free(next_node);
                next_node = next_next_node;
            }
        }
        free(p);
        *list = NULL;
    }

}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {

    dplist_node_t *ref_at_index, *list_node;
    if (list == NULL) return NULL;

    list_node = malloc(sizeof(dplist_node_t));
    if(insert_copy)
        list_node->element = list->element_copy(element);
    else
        list_node->element = element;


    // pointer drawing breakpoint
    if (list->head == NULL) { // covers case 1
        list_node->prev = NULL;
        list_node->next = NULL;
        list->head = list_node;
        // pointer drawing breakpoint
    } else if (index <= 0) { // covers case 2
        list_node->prev = NULL;
        list_node->next = list->head;
        list->head->prev = list_node;
        list->head = list_node;
        // pointer drawing breakpoint
    } else {
        ref_at_index = dpl_get_reference_at_index(list, index);
        assert(ref_at_index != NULL);
        // pointer drawing breakpoint
        if (index < dpl_size(list)) { // covers case 4
            list_node->prev = ref_at_index->prev;
            list_node->next = ref_at_index;
            ref_at_index->prev->next = list_node;
            ref_at_index->prev = list_node;
            // pointer drawing breakpoint
        } else { // covers case 3
            assert(ref_at_index->next == NULL);
            list_node->next = NULL;
            list_node->prev = ref_at_index;
            ref_at_index->next = list_node;
            // pointer drawing breakpoint
        }
    }
    return list;

}

dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {
    int count = 1;

    if (list == NULL || list->head == NULL)
        return list;
    else {
        dplist_node_t *node = list->head;
        if (node->next == NULL) {
            if(free_element)
                list->element_free(&(node->element));
            free(node);
            list->head = NULL;
            return list;
        } else if(index <= 0) {
            dplist_node_t *next_node = node->next;
            next_node->prev = NULL;
            list->head = next_node;
            if(free_element)
                list->element_free(&(node->element));
            free(node);
            return list;
        } else {
            dplist_node_t *next_node = node->next;
            while (count != index) {
                if(next_node->next == NULL)
                    break;
                ++count;
                next_node = next_node->next;
            }
            if(next_node->next != NULL) {
                dplist_node_t *next_next_node = next_node->next;
                dplist_node_t *prev_node = next_node->prev;
                next_next_node->prev = next_node->prev;
                prev_node->next = next_node->next;
            } else {
                dplist_node_t *prev_node = next_node->prev;
                prev_node->next = NULL;
            }
            if(free_element)
                list->element_free(&(next_node->element));
            free(next_node);
            return list;
        }
    }

}

int dpl_size(dplist_t *list) {

    if (list == NULL)
        return -1;
    else if(list->head == NULL){
        return 0;
    } else {
        dplist_node_t *node = list->head;
        if(node->next == NULL) {
            return 1;
        } else {
            dplist_node_t *next_node = node->next;
            int elements = 1;
            while(next_node != NULL) {
                ++elements;
                next_node = next_node->next;
            }
            return elements;
        }
    }

}

void *dpl_get_element_at_index(dplist_t *list, int index) {
    int count = 1;

    if (list == NULL || list->head == NULL)
        return 0;
    else {
        dplist_node_t *node = list->head;
        if (node->next == NULL || index <= 0) {
            return node->element;
        } else {
            dplist_node_t *next_node = node->next;
            while (count != index) {
                if(next_node->next == NULL)
                    break;
                ++count;
                next_node = next_node->next;
            }
            return next_node->element;
        }
    }

}

int dpl_get_index_of_element(dplist_t *list, void *element) {
    int count = 1;

    if (list == NULL || list->head == NULL)
        return -1;
    else {
        dplist_node_t *node = list->head;
        if (list->element_compare(element, node->element) == 0) {
            return 0;
        } else if (node->next == NULL) {
            return -1;
        } else {
            dplist_node_t *next_node = node->next;
            while (list->element_compare(element, next_node->element) != 0) {
                if(next_node->next == NULL)
                    return -1;
                ++count;
                next_node = next_node->next;
            }
            return count;
        }
    }

}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {
    int count = 1;

    if (list == NULL || list->head == NULL)
        return 0;
    else {
        dplist_node_t *node = list->head;
        if (node->next == NULL || index <= 0) {
            return node;
        } else {
            dplist_node_t *next_node = node->next;
            while (count != index) {
                if(next_node->next == NULL)
                    break;
                ++count;
                next_node = next_node->next;
            }
            return next_node;
        }
    }

}

void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {

    if (list == NULL || list->head == NULL || reference == NULL)
        return NULL;
    else {
        dplist_node_t *node = list->head;
        if (node->next == NULL && node != reference) {
            return NULL;
        } else if(node == reference) {
            return node->element;
        } else {
            dplist_node_t *next_node = node->next;
            while (next_node != reference) {
                if(next_node->next == NULL)
                    return NULL;
                next_node = next_node->next;
            }
            return next_node->element;
        }
    }

}

