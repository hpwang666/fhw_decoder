#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

/* priority jcaches */

#include "cache.h"

cache_t cache_new(size_t cacheSize,size_t nodeSize) 
{
    cache_t q;
    cacheNode_t n0=NULL,n1=NULL;
    q = (cache_t) calloc(1,sizeof(struct cache_st));
	q->cacheNodes = NULL;
	q->cacheSize  = cacheSize;
	q->nodeSize =  nodeSize;
	
        
    n1 = (cacheNode_t) calloc(1,sizeof(struct cacheNode_st));    
	n1->data = calloc(1,q->nodeSize);
	n1->next=NULL;
	while(cacheSize-1){
		n0 = (cacheNode_t) calloc(1,sizeof(struct cacheNode_st));
		n0->data = calloc(1,q->nodeSize);
		n0->dataSize=0;
		
		n0->next=n1;
		n1=n0;
		cacheSize--;
	}
    q->cacheNodes = n0;
    pthread_mutex_init(&(q->lock), NULL);
    return q;
}

void cache_free(cache_t q) 
{
    cacheNode_t n0=NULL,n1=NULL;
	n0 =q->cacheNodes;
	while(n0){
		n1=n0->next;
		free(n0->data);
		free(n0);
		n0=n1;
	}
	pthread_mutex_destroy(&(q->lock));
	free(q);
}

int cache_push(cache_t q, cacheNode_t node) 
{
    /* node to cache for later reuse */
	pthread_mutex_lock(&((cache_t)q)->lock);
    node->next = q->cacheNodes; //when run first time q->cacheNodes =NULL so here is safe
	q->cacheNodes = node;
	q->cacheSize++;
	pthread_mutex_unlock(&((cache_t)q)->lock);
    return 0;
}


cacheNode_t cache_pull(cache_t q)
{
    cacheNode_t n;

    pthread_mutex_lock(&((cache_t)q)->lock);
    if(q->cacheNodes == NULL){
		pthread_mutex_unlock(&((cache_t)q)->lock);
		return NULL;
	}
        
	n = q->cacheNodes;
    q->cacheNodes = n->next;
	q->cacheSize--;
    pthread_mutex_unlock(&((cache_t)q)->lock);
    return n;
}
