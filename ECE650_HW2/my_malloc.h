//
//  Header.h
//  650_malloc
//
//

#ifndef my_malloc_h
#define my_malloc_h
typedef struct node {
    size_t size;
    struct node* next;
    struct node* prev;
}Node;

#endif /* Header_h */
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);

void* newNode(size_t size);
void ff_free(void *ptr);
void *lock_malloc(size_t size);
void bf_free(void *ptr);
void splitNode(Node* temp, size_t size);
void removeNode(Node *temp);
void splitNode_nolock(Node* temp, size_t size);
void removeNode_nolock(Node *temp);
void AddAndCheck(Node * target);
void AddAndCheck_nolock(Node * target);
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();
void *findBest_nolock(size_t size);
void *findBest(size_t size);
