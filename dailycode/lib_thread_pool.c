/************************************************************************
* module			: 线程池实现
* file name		: lib_thread_pool.c
* Author 			: 
* version			: V1.0
* DATE			: 
* directory 		: 
* description		: 
* related document: 
* 
************************************************************************/
/*-----------------------includes-------------------------------*/
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <pthread.h> 
#include <assert.h> 
#include <string.h>
#include "lib_thread_pool.h"


/*---------------constants/macro definition---------------------*/

/*---------------global variables definition-----------------------*/

/*---------------functions declaration--------------------------*/
static void * ThreadPoolRoutine (void *arg); 

/*---------------functions definition---------------------------*/

/****************************************************************
* function name 		: ThreadPoolAddWorkLimit
* functional description	: 向线程池投递任务,无空闲线程则阻塞
* input parameter		: pthis	线程池指针
					  process	回调函数
					  arg		回调函数的参数
* output parameter	: 
* return value			: 0 - 成功;-1 - 失败 
* history				: 
*****************************************************************/
static int ThreadPoolAddWorkLimit(void* pthis,void *(*process) (void *arg), void *arg) 
{ 
	CThread_pool_t *pool = (CThread_pool_t *)pthis;
    worker_t *newworker = (worker_t *) malloc (sizeof (worker_t)); 
	if (NULL == newworker)
	{
		return -1;
	}
    newworker->process 	= process;	/**< 回调函数,在线程ThreadPoolRoutine()中执行 */
    newworker->arg 		= arg;		/**< 回调函数参数 */
    newworker->next 	= NULL;

    pthread_mutex_lock(&(pool->queue_lock)); 
	
    worker_t *member = pool->queue_head;	/**< 等待队列任务链表 */
    if (member != NULL) 
    { 
        while (member->next != NULL) 
        {
        	member = member->next; 
        }
        member->next = newworker;	/**< 放入链表尾 */
    } 
    else 
    { 
        pool->queue_head = newworker;	/**< 放入链表头 */
    } 
	
    assert (pool->queue_head != NULL); 
    pool->cur_queue_size++;		/**< 等待队列加1 */

	int FreeThreadNum = pool->current_pthread_num - pool->current_pthread_task_num;		
    if((0 == FreeThreadNum) && (pool->current_pthread_num < pool->max_thread_num))
    {/**< 如果没有空闲线程且池中当前线程数不超过可容纳最大线程 */
    	int current_pthread_num = pool->current_pthread_num;
    	pool->threadid = (pthread_t *) realloc(pool->threadid,(current_pthread_num + 1) * sizeof (pthread_t));	 /**< 新增线程 */
		pthread_create (&(pool->threadid[current_pthread_num]), NULL, ThreadPoolRoutine,  (void*)pool);
		pool->current_pthread_num++;	/**< 当前池中线程总数加1 */	
		
		pool->current_pthread_task_num++;	/**< 分配任务的线程数加1 */	
		pthread_mutex_unlock (&(pool->queue_lock)); 
    	pthread_cond_signal (&(pool->queue_ready));	/**< 发送信号给1个处于条件阻塞等待状态的线程 */	
		return 0;
    }

	pool->current_pthread_task_num++; 
    pthread_mutex_unlock(&(pool->queue_lock)); 
    pthread_cond_signal(&(pool->queue_ready));
//	usleep(10);	//看情况加
    return 0; 
} 

/****************************************************************
* function name 		: ThreadPoolAddWorkUnlimit
* functional description	: 向线程池投递任务
* input parameter		: pthis	线程池指针
					  process	回调函数
					  arg		回调函数的参数
* output parameter	: 
* return value			: 0 - 成功;-1 - 失败 
* history				: 
*****************************************************************/
static int ThreadPoolAddWorkUnlimit(void* pthis,void *(*process) (void *arg), void *arg) 
{ 
	CThread_pool_t *pool = (CThread_pool_t *)pthis;
    worker_t *newworker = (worker_t *) malloc (sizeof (worker_t)); 
	
	if (NULL == newworker)
	{
		return -1;
	}
    newworker->process 	= process;	
    newworker->arg 		= arg; 		
    newworker->next 	= NULL;		

    pthread_mutex_lock(&(pool->queue_lock)); 
	
    worker_t *member = pool->queue_head; 
    if (member != NULL) 	
    { 
        while (member->next != NULL) 
        {
        	member = member->next; 
        }
        member->next = newworker; 
    } 
    else 
    { 
        pool->queue_head = newworker; 
    } 
	
    assert (pool->queue_head != NULL); 
    pool->cur_queue_size++; 

	int FreeThreadNum = pool->current_pthread_num - pool->current_pthread_task_num;
    if(0 == FreeThreadNum)	/**< 只判断是否没有空闲线程 */
    {
    	int current_pthread_num = pool->current_pthread_num;
    	pool->threadid = (pthread_t *) realloc(pool->threadid,(current_pthread_num + 1) * sizeof (pthread_t)); 
		pthread_create (&(pool->threadid[current_pthread_num]), NULL, ThreadPoolRoutine,  (void*)pool);
		pool->current_pthread_num++;
		if (pool->current_pthread_num > pool->max_thread_num)
		{
			pool->max_thread_num = pool->current_pthread_num;
		}
		
		pool->current_pthread_task_num++;
		pthread_mutex_unlock (&(pool->queue_lock)); 
    	pthread_cond_signal (&(pool->queue_ready)); 
		return 0;
    }
	
	pool->current_pthread_task_num++;
    pthread_mutex_unlock(&(pool->queue_lock)); 
    pthread_cond_signal(&(pool->queue_ready)); 	
//	usleep(10);	//看情况加
    return 0; 
} 

/****************************************************************
* function name 		: ThreadPoolGetThreadMaxNum
* functional description	: 获取线程池可容纳的最大线程数
* input parameter		: pthis	线程池指针
* output parameter	: 
* return value			: 线程池可容纳的最大线程数
* history				: 
*****************************************************************/
static int ThreadPoolGetThreadMaxNum(void* pthis) 
{ 
	CThread_pool_t *pool = (CThread_pool_t *)pthis;
	
    pthread_mutex_lock(&(pool->queue_lock)); 
	int num = pool->max_thread_num;
    pthread_mutex_unlock(&(pool->queue_lock)); 
	
    return num; 
} 

/****************************************************************
* function name 		: ThreadPoolGetCurrentThreadNum
* functional description	: 获取线程池存放的线程数
* input parameter		: pthis	线程池指针
* output parameter	: 
* return value			: 线程池存放的线程数
* history				: 
*****************************************************************/
static int ThreadPoolGetCurrentThreadNum(void* pthis) 
{ 
	CThread_pool_t *pool = (CThread_pool_t *)pthis;
	
    pthread_mutex_lock(&(pool->queue_lock)); 
	int num = pool->current_pthread_num;
    pthread_mutex_unlock(&(pool->queue_lock)); 
	
    return num; 
} 

/****************************************************************
* function name 		: ThreadPoolGetCurrentTaskThreadNum
* functional description	: 获取当前正在执行任务和已分配任务的线程数目和
* input parameter		: pthis	线程池指针
* output parameter	: 
* return value			: 当前正在执行任务和已分配任务的线程数目和
* history				: 
*****************************************************************/
static int ThreadPoolGetCurrentTaskThreadNum(void* pthis) 
{ 
	CThread_pool_t *pool = (CThread_pool_t *)pthis;
	
    pthread_mutex_lock(&(pool->queue_lock)); 
	int num = pool->current_pthread_task_num;
    pthread_mutex_unlock(&(pool->queue_lock)); 
	
    return num; 
} 

/****************************************************************
* function name 		: ThreadPoolGetCurrentTaskNum
* functional description	: 获取线程池等待队列任务数
* input parameter		: pthis	线程池指针
* output parameter	: 
* return value			: 等待队列任务数
* history				: 
*****************************************************************/
static int ThreadPoolGetCurrentTaskNum(void* pthis) 
{ 
	CThread_pool_t *pool = (CThread_pool_t *)pthis;
	
    pthread_mutex_lock(&(pool->queue_lock)); 
	int num = pool->cur_queue_size;
    pthread_mutex_unlock(&(pool->queue_lock)); 
	
    return num; 
} 

/****************************************************************
* function name 		: ThreadPoolDestroy
* functional description	: 销毁线程池
* input parameter		: pthis	线程池指针
* output parameter	: 
* return value			: 0 - 成功;-1 - 失败
* history				: 
*****************************************************************/
static int ThreadPoolDestroy (void *pthis) 
{ 
	CThread_pool_t *pool = (CThread_pool_t *)pthis;
	
    if (pool->shutdown) /**< 已销毁 */
    {
    	return -1;
    }
    pool->shutdown = 1;	/**< 销毁标志置位 */
	
    pthread_cond_broadcast (&(pool->queue_ready)); /**< 唤醒所有pthread_cond_wait()等待线程 */
    int i; 
    for (i = 0; i < pool->current_pthread_num; i++) 
    {
    	pthread_join (pool->threadid[i], NULL); /**< 等待所有线程执行结束 */
    }
	
    free (pool->threadid);	/**< 释放 */
    worker_t *head = NULL; 
	
    while (pool->queue_head != NULL) 
    { 
        head = pool->queue_head; 
        pool->queue_head = pool->queue_head->next; 
        free (head);	/**< 释放 */
    } 
	
    pthread_mutex_destroy(&(pool->queue_lock));	/**< 销毁 */
    pthread_cond_destroy(&(pool->queue_ready)); /**< 销毁 */     
    free (pool);	/**< 释放 */ 
    pool=NULL; 	
    return 0; 
} 


/****************************************************************
* function name 		: ThreadPoolRoutine
* functional description	: 线程池中运行的线程
* input parameter		: arg	线程池指针
* output parameter	: 
* return value			: NULL
* history				: 
*****************************************************************/
static void * ThreadPoolRoutine (void *arg) 
{ 
	CThread_pool_t *pool = (CThread_pool_t *)arg;
	
    while (1) 
    { 
        pthread_mutex_lock (&(pool->queue_lock)); /**< 上锁, pthread_cond_wait()调用会解锁*/
		
        while ((pool->cur_queue_size == 0) && (!pool->shutdown))	/**< 队列没有等待任务*/
        { 
            pthread_cond_wait(&(pool->queue_ready), &(pool->queue_lock)); 	/**< 条件锁阻塞等待条件信号*/
		} 
        if (pool->shutdown) 
        { 
            pthread_mutex_unlock (&(pool->queue_lock)); 
            pthread_exit (NULL); 	/**< 释放线程 */
        } 

        assert (pool->cur_queue_size != 0); 
        assert (pool->queue_head != NULL); 
         
        pool->cur_queue_size--; 	/**< 等待任务减1,准备执行任务*/ 
        worker_t *worker 	= pool->queue_head;	/**< 取等待队列任务结点头*/ 
        pool->queue_head 	= worker->next; 	/**< 链表后移 */ 
		
        pthread_mutex_unlock (&(pool->queue_lock)); 
        (*(worker->process)) (worker->arg); 	/**< 执行回调函数 */ 
        pthread_mutex_lock (&(pool->queue_lock)); 
		
		pool->current_pthread_task_num--;	/**< 函数执行结束 */ 
        free (worker); 	/**< 释放任务结点 */
        worker = NULL; 

		if ((pool->current_pthread_num - pool->current_pthread_task_num) > pool->free_pthread_num)
		{
			pthread_mutex_unlock (&(pool->queue_lock)); 
			break;	/**< 当池中空闲线程超过 free_pthread_num则将线程释放回操作系统 */
		}
        pthread_mutex_unlock (&(pool->queue_lock)); 		
    } 
	
	pool->current_pthread_num--;	/**< 当前池中线程数减1 */
    pthread_exit (NULL);	/**< 释放线程*/
    return (void*)NULL;
} 

/****************************************************************
* function name 		: ThreadPoolConstruct
* functional description	: 创建线程池
* input parameter		: max_num	线程池可容纳的最大线程数
					  free_num	线程池允许存在的最大空闲线程,超过则将线程释放回操作系统
* output parameter	: 
* return value			: 线程池指针
* history				: 
*****************************************************************/
CThread_pool_t* ThreadPoolConstruct(int max_num,int free_num) 
{ 
    CThread_pool_t *pool = (CThread_pool_t *) malloc (sizeof (CThread_pool_t)); 
	if (NULL == pool)
	{
		return NULL;
	}
	memset(pool, 0, sizeof(CThread_pool_t));
	
    pthread_mutex_init (&(pool->queue_lock), NULL);	/**< 初始化互斥锁 */
    pthread_cond_init (&(pool->queue_ready), NULL);	/**< 初始化条件变量 */ 

    pool->queue_head 				= NULL; 
    pool->max_thread_num 			= max_num;	/**< 线程池可容纳的最大线程数 */
    pool->cur_queue_size 			= 0; 
	pool->current_pthread_task_num 	= 0;
    pool->shutdown 					= 0; 
	pool->current_pthread_num 		= 0;
	pool->free_pthread_num 			= free_num;	/**< 线程池允许存在的最大空闲线程 */
	pool->threadid					= NULL;
    pool->threadid 					= (pthread_t *) malloc (max_num * sizeof (pthread_t)); 
	pool->AddWorkUnlimit			= ThreadPoolAddWorkUnlimit;	/**< 给函数指针赋值 */
	pool->AddWorkLimit				= ThreadPoolAddWorkLimit;
	pool->Destruct					= ThreadPoolDestroy;
	pool->GetMaxThreadNum			= ThreadPoolGetThreadMaxNum;
	pool->GetCurThreadNum			= ThreadPoolGetCurrentThreadNum;
	pool->GetCurTaskThreadNum		= ThreadPoolGetCurrentTaskThreadNum;
	pool->GetCurTaskNum				= ThreadPoolGetCurrentTaskNum;
	
    int i = 0; 
    for (i = 0; i < max_num; i++) 
    {  
		pool->current_pthread_num++;	/**< 当前池中的线程数 */
        pthread_create (&(pool->threadid[i]), NULL, ThreadPoolRoutine, (void*)pool);	/**< 创建线程 */
		usleep(1000);
    } 

	return pool;
} 

/****************************************************************
* function name 		: ThreadPoolConstructDefault
* functional description	: 创建线程池,以默认的方式初始化,未创建线程
* input parameter		: 
* output parameter	: 
* return value			: 线程池指针
* history				: 
*****************************************************************/
CThread_pool_t* ThreadPoolConstructDefault(void) 
{ 
    CThread_pool_t *pool = (CThread_pool_t *) malloc (sizeof (CThread_pool_t)); 
	if (NULL == pool)
	{
		return NULL;
	}
	memset(pool, 0, sizeof(CThread_pool_t));
	
    pthread_mutex_init(&(pool->queue_lock), NULL); 
    pthread_cond_init(&(pool->queue_ready), NULL); 

    pool->queue_head 				= NULL; 
    pool->max_thread_num 			= DEFAULT_MAX_THREAD_NUM;	/**< 默认值 */
    pool->cur_queue_size 			= 0; 
	pool->current_pthread_task_num 	= 0;
    pool->shutdown 					= 0; 
	pool->current_pthread_num 		= 0;
	pool->free_pthread_num 			= DEFAULT_FREE_THREAD_NUM;	/**< 默认值 */
	pool->threadid					= NULL;
	pool->AddWorkUnlimit			= ThreadPoolAddWorkUnlimit;
	pool->AddWorkLimit				= ThreadPoolAddWorkLimit;
	pool->Destruct					= ThreadPoolDestroy;
	pool->GetMaxThreadNum			= ThreadPoolGetThreadMaxNum;
	pool->GetCurThreadNum			= ThreadPoolGetCurrentThreadNum;
	pool->GetCurTaskThreadNum		= ThreadPoolGetCurrentTaskThreadNum;
	pool->GetCurTaskNum				= ThreadPoolGetCurrentTaskNum;
	
	return pool;
}

