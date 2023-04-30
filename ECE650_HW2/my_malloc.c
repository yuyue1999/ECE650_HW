#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "my_malloc.h"
#include <time.h>
#include <pthread.h>


unsigned long HeapSize = 0;
unsigned long FreeSize = 0;
Node* head=NULL;
Node* tail=NULL;
__thread Node* nolockhead=NULL;
__thread Node* nolocktail=NULL;
pthread_mutex_t lock;

unsigned long get_data_segment_size() {
  return HeapSize;
}

unsigned long get_data_segment_free_space_size() {
  return FreeSize;
}


void *ts_malloc_lock(size_t size){
    pthread_mutex_lock(&lock);
    void * result = lock_malloc(size);
    pthread_mutex_unlock(&lock);
    return result;
}
void ts_free_lock(void *ptr){
    pthread_mutex_lock(&lock);
    ff_free(ptr);
    pthread_mutex_unlock(&lock);
}

void *lock_malloc(size_t size){
    void* target=NULL;
    target=findBest(size);
    if(target==NULL){
        return newNode(size);
    }else{
        return target+sizeof(Node);
    }
}
/*
void *bf_malloc(size_t size){
    void* target=NULL;
    target=findBest(size);
    if(target==NULL){
        return newNode(size);
    }else{
        return target+sizeof(Node);
    }
}*/


void *ts_malloc_nolock(size_t size){
    void* target=NULL;
    target=findBest_nolock(size);
    if(target==NULL){
        pthread_mutex_lock(&lock);
        Node* new=(Node *)sbrk(size+sizeof(Node));
        pthread_mutex_unlock(&lock);
        if(new==(Node*)-1){
            return NULL;
        }
        new->size=size;
        new->next=NULL;
        new->prev=NULL;
        HeapSize=HeapSize+size+sizeof(Node);
        return (void *)new +sizeof(Node);
    }else{
        return target+sizeof(Node);
    }
}

void* newNode(size_t size){
    Node* new=(Node *)sbrk(size+sizeof(Node));
    if(new==(Node*)-1){
        return NULL;
    }
    new->size=size;
    new->next=NULL;
    new->prev=NULL;
    HeapSize=HeapSize+size+sizeof(Node);
    return (void *)new +sizeof(Node);

}
void ts_free_nolock(void *ptr){
    ff_free(ptr);
}
void *findBest_nolock(size_t size){
    Node* target=nolockhead;
    int marked=0;
    size_t min=10000000;
    Node* temp=NULL;
    while(target!=NULL){
        long diff=target->size -size;
        if(diff==0){
            temp=target;
            marked=1;
            break;
        }else if(diff>0 && diff<min){
            min=target->size;
            temp=target;
            marked=1;
        }
        target=target->next;
    }
    if(marked==0){
        return NULL;
    }else{
        if(temp->size>size+sizeof(Node)){
            splitNode_nolock(temp,size);
            FreeSize=FreeSize-size-sizeof(Node);
        }else{
            size_t tempSize=temp->size;
            removeNode_nolock(temp);
            FreeSize=FreeSize-tempSize-sizeof(Node);
        }
        return (void*) temp;
    }
    
}

void *findBest(size_t size){
    Node* target=head;
    int marked=0;
    size_t min=10000000;
    Node* temp=NULL;
    while(target!=NULL){
        long diff=target->size -size;
        if(diff==0){
            temp=target;
            marked=1;
            break;
        }else if(diff>0 && diff<min){
            min=target->size;
            temp=target;
            marked=1;
        }
        target=target->next;
    }
    if(marked==0){
        return NULL;
    }else{
        if(temp->size>size+sizeof(Node)){
            splitNode(temp,size);
            FreeSize=FreeSize-size-sizeof(Node);
        }else{
            size_t tempSize=temp->size;
            removeNode(temp);
            FreeSize=FreeSize-tempSize-sizeof(Node);
        }
        return (void*) temp;
    }
    
}
void splitNode_nolock(Node* SplitTarget, size_t size){
    Node* temp= (void*) SplitTarget+size+sizeof(Node);// maybe wrong;
    temp->size=SplitTarget->size-size-sizeof(Node);
    if(SplitTarget->next==NULL &&SplitTarget->prev==NULL){
        temp->next=SplitTarget->next;
        temp->prev=SplitTarget->prev;
        nolocktail=temp;
        nolockhead=temp;
        SplitTarget->next=NULL;
        SplitTarget->prev=NULL;
        SplitTarget->size=size;
        return;
    }
    if(SplitTarget->next==NULL){
        temp->next=SplitTarget->next;
        temp->prev=SplitTarget->prev;
        nolocktail=temp;
        SplitTarget->prev->next=temp;
        SplitTarget->next=NULL;
        SplitTarget->prev=NULL;
        SplitTarget->size=size;
        return;
    }
    if(SplitTarget->prev==NULL){
        temp->next=SplitTarget->next;
        temp->prev=SplitTarget->prev;
        nolockhead=temp;
        SplitTarget->next->prev=temp;
        SplitTarget->next=NULL;
        SplitTarget->prev=NULL;
        SplitTarget->size=size;
        return;
    }
    temp->next=SplitTarget->next;
    temp->prev=SplitTarget->prev;
    SplitTarget->prev->next=temp;
    SplitTarget->next->prev=temp;
    SplitTarget->next=NULL;
    SplitTarget->prev=NULL;
    SplitTarget->size=size;
    return;

    }

void splitNode(Node* SplitTarget, size_t size){
    Node* temp= (void*) SplitTarget+size+sizeof(Node);// maybe wrong;
    temp->size=SplitTarget->size-size-sizeof(Node);
    if(SplitTarget->next==NULL &&SplitTarget->prev==NULL){
        temp->next=SplitTarget->next;
        temp->prev=SplitTarget->prev;
        tail=temp;
        head=temp;
        SplitTarget->next=NULL;
        SplitTarget->prev=NULL;
        SplitTarget->size=size;
        return;
    }
    if(SplitTarget->next==NULL){
        temp->next=SplitTarget->next;
        temp->prev=SplitTarget->prev;
        tail=temp;
        SplitTarget->prev->next=temp;
        SplitTarget->next=NULL;
        SplitTarget->prev=NULL;
        SplitTarget->size=size;
        return;
    }
    if(SplitTarget->prev==NULL){
        temp->next=SplitTarget->next;
        temp->prev=SplitTarget->prev;
        head=temp;
        SplitTarget->next->prev=temp;
        SplitTarget->next=NULL;
        SplitTarget->prev=NULL;
        SplitTarget->size=size;
        return;
    }
    temp->next=SplitTarget->next;
    temp->prev=SplitTarget->prev;
    SplitTarget->prev->next=temp;
    SplitTarget->next->prev=temp;
    SplitTarget->next=NULL;
    SplitTarget->prev=NULL;
    SplitTarget->size=size;
    return;

    }
void removeNode_nolock(Node *temp){
    //FreeSize=FreeSize-temp->size-sizeof(Node);// something wrong!
    if(temp->next==NULL && temp->prev==NULL){
        nolockhead=NULL;
        nolocktail=NULL;
        return;
    }
    if(temp->next==NULL){
        nolocktail=temp->prev;
        temp->prev->next=NULL;
        temp->prev=NULL;
        return;
    }else if (temp->prev==NULL){
        nolockhead=temp->next;
        temp->next->prev=NULL;
        temp->next=NULL;
        return;
    }
    temp->prev->next=temp->next;
    temp->next->prev=temp->prev;
    temp->prev=NULL;
    temp->next=NULL;
    
    return;
}
void removeNode(Node *temp){
    //FreeSize=FreeSize-temp->size-sizeof(Node);// something wrong!
    if(temp->next==NULL && temp->prev==NULL){
        head=NULL;
        tail=NULL;
        return;
    }
    if(temp->next==NULL){
        tail=temp->prev;
        temp->prev->next=NULL;
        temp->prev=NULL;
        return;
    }else if (temp->prev==NULL){
        head=temp->next;
        temp->next->prev=NULL;
        temp->next=NULL;
        return;
    }
    temp->prev->next=temp->next;
    temp->next->prev=temp->prev;
    temp->prev=NULL;
    temp->next=NULL;
    
    return;
}
//change name
void ff_free(void *ptr){
    if(ptr==NULL){
        return;
    }
    Node *ReadyToFree=(Node *) (ptr-sizeof(Node));
    AddAndCheck(ReadyToFree);
}
void AddAndCheck(Node * target){
    if(head==NULL && tail==NULL){
        head=target;
        tail=target;
    }else if(target<head){
        target->prev=NULL;
        target->next=head;
        head->prev=target;
        head=target;
    }else if (target>tail){
        target->next=NULL;
        target->prev=tail;
        tail->next=target;
        tail=target;
    }else{
        Node * current=head;
        while(current->next!=NULL && target>current->next){
            current=current->next;
            
        }
        target->next=current->next;
        target->prev=current;
        current->next->prev=target;
        current->next=target;
        
    }
    FreeSize=FreeSize+target->size+sizeof(Node);
    if(target->next!=NULL && (void *)target +target->size+sizeof(Node)==target->next){
        target->size=target->size+sizeof(Node)+target->next->size;
        removeNode(target->next);
    }
    if(target->prev!=NULL && (void *) target->prev+target->prev->size+sizeof(Node)==target){
        target->prev->size=target->prev->size+sizeof(Node)+target->size;
        removeNode(target);
    }
    
}
void AddAndCheck_nolock(Node * target){
    if(nolockhead==NULL && nolocktail==NULL){
        nolockhead=target;
        nolocktail=target;
    }else if(target<nolockhead){
        target->prev=NULL;
        target->next=nolockhead;
        nolockhead->prev=target;
        nolockhead=target;
    }else if (target>nolocktail){
        target->next=NULL;
        target->prev=nolocktail;
        nolocktail->next=target;
        nolocktail=target;
    }else{
        Node * current=nolockhead;
        while(current->next!=NULL && target>current->next){
            current=current->next;
            
        }
        target->next=current->next;
        target->prev=current;
        current->next->prev=target;
        current->next=target;
        
    }
    FreeSize=FreeSize+target->size+sizeof(Node);
    if(target->next!=NULL && (void *)target +target->size+sizeof(Node)==target->next){
        target->size=target->size+sizeof(Node)+target->next->size;
        removeNode_nolock(target->next);
    }
    if(target->prev!=NULL && (void *) target->prev+target->prev->size+sizeof(Node)==target){
        target->prev->size=target->prev->size+sizeof(Node)+target->size;
        removeNode_nolock(target);
    }
    
}



