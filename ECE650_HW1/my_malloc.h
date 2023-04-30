//
//  Header.h
//  650_malloc
//
//  Created by 于越 on 1/19/23.
//

#ifndef my_malloc_h
#define my_malloc_h
typedef struct node {
    size_t size;
    struct node* next;
    struct node* prev;
}Node;

#endif /* Header_h */
void* newNode(size_t size);
void *findFirst(size_t size);
void *ff_malloc(size_t size);
void ff_free(void *ptr);
void *bf_malloc(size_t size);
void bf_free(void *ptr);
void splitNode(Node* temp, size_t size);
void removeNode(Node *temp);
void AddAndCheck(Node * target);
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();
void *findBest(size_t size);
