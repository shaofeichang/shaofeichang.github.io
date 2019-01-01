/************************************************************************
* module			: �̳߳�ʵ��
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
* functional description	: ���̳߳�Ͷ������,�޿����߳�������
* input parameter		: pthis	�̳߳�ָ��
					  process	�ص�����
					  arg		�ص������Ĳ���
* output parameter	: 
* return value			: 0 - �ɹ�;-1 - ʧ�� 
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
    newworker->process 	= process;	/**< �ص�����,���߳�ThreadPoolRoutine()��ִ�� */
    newworker->arg 		= arg;		/**< �ص��������� */
    newworker->next 	= NULL;

    pthread_mutex_lock(&(pool->queue_lock)); 
	
    worker_t *member = pool->queue_head;	/**< �ȴ������������� */
    if (member != NULL) 
    { 
        while (member->next != NULL) 
        {
        	member = member->next; 
        }
        member->next = newworker;	/**< ��������β */
    } 
    else 
    { 
        pool->queue_head = newworker;	/**< ��������ͷ */
    } 
	
    assert (pool->queue_head != NULL); 
    pool->cur_queue_size++;		/**< �ȴ����м�1 */

	int FreeThreadNum = pool->current_pthread_num - pool->current_pthread_task_num;		
    if((0 == FreeThreadNum) && (pool->current_pthread_num < pool->max_thread_num))
    {/**< ���û�п����߳��ҳ��е�ǰ�߳�������������������߳� */
    	int current_pthread_num = pool->current_pthread_num;
    	pool->threadid = (pthread_t *) realloc(pool->threadid,(current_pthread_num + 1) * sizeof (pthread_t));	 /**< �����߳� */
		pthread_create (&(pool->threadid[current_pthread_num]), NULL, ThreadPoolRoutine,  (void*)pool);
		pool->current_pthread_num++;	/**< ��ǰ�����߳�������1 */	
		
		pool->current_pthread_task_num++;	/**< ����������߳�����1 */	
		pthread_mutex_unlock (&(pool->queue_lock)); 
    	pthread_cond_signal (&(pool->queue_ready));	/**< �����źŸ�1���������������ȴ�״̬���߳� */	
		return 0;
    }

	pool->current_pthread_task_num++; 
    pthread_mutex_unlock(&(pool->queue_lock)); 
    pthread_cond_signal(&(pool->queue_ready));
//	usleep(10);	//�������
    return 0; 
} 

/****************************************************************
* function name 		: ThreadPoolAddWorkUnlimit
* functional description	: ���̳߳�Ͷ������
* input parameter		: pthis	�̳߳�ָ��
					  process	�ص�����
					  arg		�ص������Ĳ���
* output parameter	: 
* return value			: 0 - �ɹ�;-1 - ʧ�� 
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
    if(0 == FreeThreadNum)	/**< ֻ�ж��Ƿ�û�п����߳� */
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
//	usleep(10);	//�������
    return 0; 
} 

/****************************************************************
* function name 		: ThreadPoolGetThreadMaxNum
* functional description	: ��ȡ�̳߳ؿ����ɵ�����߳���
* input parameter		: pthis	�̳߳�ָ��
* output parameter	: 
* return value			: �̳߳ؿ����ɵ�����߳���
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
* functional description	: ��ȡ�̳߳ش�ŵ��߳���
* input parameter		: pthis	�̳߳�ָ��
* output parameter	: 
* return value			: �̳߳ش�ŵ��߳���
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
* functional description	: ��ȡ��ǰ����ִ��������ѷ���������߳���Ŀ��
* input parameter		: pthis	�̳߳�ָ��
* output parameter	: 
* return value			: ��ǰ����ִ��������ѷ���������߳���Ŀ��
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
* functional description	: ��ȡ�̳߳صȴ�����������
* input parameter		: pthis	�̳߳�ָ��
* output parameter	: 
* return value			: �ȴ�����������
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
* functional description	: �����̳߳�
* input parameter		: pthis	�̳߳�ָ��
* output parameter	: 
* return value			: 0 - �ɹ�;-1 - ʧ��
* history				: 
*****************************************************************/
static int ThreadPoolDestroy (void *pthis) 
{ 
	CThread_pool_t *pool = (CThread_pool_t *)pthis;
	
    if (pool->shutdown) /**< ������ */
    {
    	return -1;
    }
    pool->shutdown = 1;	/**< ���ٱ�־��λ */
	
    pthread_cond_broadcast (&(pool->queue_ready)); /**< ��������pthread_cond_wait()�ȴ��߳� */
    int i; 
    for (i = 0; i < pool->current_pthread_num; i++) 
    {
    	pthread_join (pool->threadid[i], NULL); /**< �ȴ������߳�ִ�н��� */
    }
	
    free (pool->threadid);	/**< �ͷ� */
    worker_t *head = NULL; 
	
    while (pool->queue_head != NULL) 
    { 
        head = pool->queue_head; 
        pool->queue_head = pool->queue_head->next; 
        free (head);	/**< �ͷ� */
    } 
	
    pthread_mutex_destroy(&(pool->queue_lock));	/**< ���� */
    pthread_cond_destroy(&(pool->queue_ready)); /**< ���� */     
    free (pool);	/**< �ͷ� */ 
    pool=NULL; 	
    return 0; 
} 


/****************************************************************
* function name 		: ThreadPoolRoutine
* functional description	: �̳߳������е��߳�
* input parameter		: arg	�̳߳�ָ��
* output parameter	: 
* return value			: NULL
* history				: 
*****************************************************************/
static void * ThreadPoolRoutine (void *arg) 
{ 
	CThread_pool_t *pool = (CThread_pool_t *)arg;
	
    while (1) 
    { 
        pthread_mutex_lock (&(pool->queue_lock)); /**< ����, pthread_cond_wait()���û����*/
		
        while ((pool->cur_queue_size == 0) && (!pool->shutdown))	/**< ����û�еȴ�����*/
        { 
            pthread_cond_wait(&(pool->queue_ready), &(pool->queue_lock)); 	/**< �����������ȴ������ź�*/
		} 
        if (pool->shutdown) 
        { 
            pthread_mutex_unlock (&(pool->queue_lock)); 
            pthread_exit (NULL); 	/**< �ͷ��߳� */
        } 

        assert (pool->cur_queue_size != 0); 
        assert (pool->queue_head != NULL); 
         
        pool->cur_queue_size--; 	/**< �ȴ������1,׼��ִ������*/ 
        worker_t *worker 	= pool->queue_head;	/**< ȡ�ȴ�����������ͷ*/ 
        pool->queue_head 	= worker->next; 	/**< ������� */ 
		
        pthread_mutex_unlock (&(pool->queue_lock)); 
        (*(worker->process)) (worker->arg); 	/**< ִ�лص����� */ 
        pthread_mutex_lock (&(pool->queue_lock)); 
		
		pool->current_pthread_task_num--;	/**< ����ִ�н��� */ 
        free (worker); 	/**< �ͷ������� */
        worker = NULL; 

		if ((pool->current_pthread_num - pool->current_pthread_task_num) > pool->free_pthread_num)
		{
			pthread_mutex_unlock (&(pool->queue_lock)); 
			break;	/**< �����п����̳߳��� free_pthread_num���߳��ͷŻز���ϵͳ */
		}
        pthread_mutex_unlock (&(pool->queue_lock)); 		
    } 
	
	pool->current_pthread_num--;	/**< ��ǰ�����߳�����1 */
    pthread_exit (NULL);	/**< �ͷ��߳�*/
    return (void*)NULL;
} 

/****************************************************************
* function name 		: ThreadPoolConstruct
* functional description	: �����̳߳�
* input parameter		: max_num	�̳߳ؿ����ɵ�����߳���
					  free_num	�̳߳�������ڵ��������߳�,�������߳��ͷŻز���ϵͳ
* output parameter	: 
* return value			: �̳߳�ָ��
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
	
    pthread_mutex_init (&(pool->queue_lock), NULL);	/**< ��ʼ�������� */
    pthread_cond_init (&(pool->queue_ready), NULL);	/**< ��ʼ���������� */ 

    pool->queue_head 				= NULL; 
    pool->max_thread_num 			= max_num;	/**< �̳߳ؿ����ɵ�����߳��� */
    pool->cur_queue_size 			= 0; 
	pool->current_pthread_task_num 	= 0;
    pool->shutdown 					= 0; 
	pool->current_pthread_num 		= 0;
	pool->free_pthread_num 			= free_num;	/**< �̳߳�������ڵ��������߳� */
	pool->threadid					= NULL;
    pool->threadid 					= (pthread_t *) malloc (max_num * sizeof (pthread_t)); 
	pool->AddWorkUnlimit			= ThreadPoolAddWorkUnlimit;	/**< ������ָ�븳ֵ */
	pool->AddWorkLimit				= ThreadPoolAddWorkLimit;
	pool->Destruct					= ThreadPoolDestroy;
	pool->GetMaxThreadNum			= ThreadPoolGetThreadMaxNum;
	pool->GetCurThreadNum			= ThreadPoolGetCurrentThreadNum;
	pool->GetCurTaskThreadNum		= ThreadPoolGetCurrentTaskThreadNum;
	pool->GetCurTaskNum				= ThreadPoolGetCurrentTaskNum;
	
    int i = 0; 
    for (i = 0; i < max_num; i++) 
    {  
		pool->current_pthread_num++;	/**< ��ǰ���е��߳��� */
        pthread_create (&(pool->threadid[i]), NULL, ThreadPoolRoutine, (void*)pool);	/**< �����߳� */
		usleep(1000);
    } 

	return pool;
} 

/****************************************************************
* function name 		: ThreadPoolConstructDefault
* functional description	: �����̳߳�,��Ĭ�ϵķ�ʽ��ʼ��,δ�����߳�
* input parameter		: 
* output parameter	: 
* return value			: �̳߳�ָ��
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
    pool->max_thread_num 			= DEFAULT_MAX_THREAD_NUM;	/**< Ĭ��ֵ */
    pool->cur_queue_size 			= 0; 
	pool->current_pthread_task_num 	= 0;
    pool->shutdown 					= 0; 
	pool->current_pthread_num 		= 0;
	pool->free_pthread_num 			= DEFAULT_FREE_THREAD_NUM;	/**< Ĭ��ֵ */
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

