//**************************************************************
 
#ifndef _CACHE_H
#define _CACHE_H
 
/*
 * priority caches
 */

typedef struct cacheNode_st  *cacheNode_t;
struct cacheNode_st {
    void  *data;
    cacheNode_t  next;
	size_t  dataSize;
};

typedef struct cache_st  *cache_t;
struct cache_st {
    cacheNode_t  cacheNodes;

	pthread_mutex_t lock;
	size_t 	nodeSize;
    size_t		cacheSize;
};

#ifdef __cplusplus
extern "C"
{
#endif
	cache_t cache_new(size_t cacheSize,size_t nodeSize); 
	void cache_free(cache_t q); 
	//将节点压入缓存
	int cache_push(cache_t q, cacheNode_t node);
	//从缓存获取节点
	cacheNode_t cache_pull(cache_t q);
 
#ifdef __cplusplus
}
#endif



 
 
#endif




