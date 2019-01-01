/************************************************************************
* module			: 线程池头文件
* file name		: lib_thread_pool.h
* Author 			: 
* version			: V1.0
* DATE			: 
* directory 		: 
* description		: 
* related document: 
* 
************************************************************************/
/*-----------------------includes-------------------------------*/
#ifndef __PTHREAD_POOL_H__
#define __PTHREAD_POOL_H__

#include <pthread.h>

/*---------------constants/macro definition---------------------*/
#define DEFAULT_MAX_THREAD_NUM		100
#define DEFAULT_FREE_THREAD_NUM		10	
typedef struct worker_t worker_t;
typedef struct CThread_pool_t CThread_pool_t;

/*---------------global variables definition-----------------------*/
/*线程池任务结点*/
struct worker_t
{
	void *(*process) (void *arg);	/**< 回调函数 */
    int   paratype;					/**< 函数类型(预留) */
    void *arg;						/**< 回调函数参数 */
    struct worker_t *next;			/**< 连接下一个任务结点 */
};

/*线程池控制器*/
struct CThread_pool_t
{
	pthread_mutex_t queue_lock;	/**< 互斥锁 */
    pthread_cond_t queue_ready;	/**< 条件变量 */

    worker_t *queue_head;	/**< 任务结点链表,保存所有投递的任务 */
    int shutdown;			/**< 线程池销毁标志,1 - 销毁 */
    pthread_t *threadid;	/**< 线程ID */
    int max_thread_num;		/**< 线程池可容纳的最大线程数 */
	int current_pthread_num;	/**< 当前线程池存放的线程数 */
	int current_pthread_task_num;	/**< 当前正在执行任务和已分配任务的线程数目和 */
    int cur_queue_size;		/**< 当前等待队列的任务数目 */
	int	free_pthread_num;	/**< 线程池内允许存在的最大空闲线程数 */
	
	/****************************************************************
	* function name 		: ThreadPoolAddWorkLimit
	* functional description	: 向线程池投递任务
	* input parameter		: pthis	线程池指针
						  process	回调函数
						  arg		回调函数的参数
	* output parameter	: 
	* return value			: 0 - 成功;-1 - 失败 
	* history				: 
	*****************************************************************/
	int (*AddWorkUnlimit)(void* pthis,void *(*process) (void *arg), void *arg); 

	/****************************************************************
	* function name 		: ThreadPoolAddWorkUnlimit
	* functional description	: 向线程池投递任务,无空闲线程则阻塞
	* input parameter		: pthis	线程池指针
						  process	回调函数
						  arg		回调函数的参数
	* output parameter	: 
	* return value			: 0 - 成功;-1 - 失败 
	* history				: 
	*****************************************************************/
	int (*AddWorkLimit)(void* pthis,void *(*process) (void *arg), void *arg); 
	
	/****************************************************************
	* function name 		: ThreadPoolGetThreadMaxNum
	* functional description	: 获取线程池可容纳的最大线程数
	* input parameter		: pthis	线程池指针
	* output parameter	: 
	* return value			: 线程池可容纳的最大线程数
	* history				: 
	*****************************************************************/
	int (*GetMaxThreadNum) (void *pthis); 

	/****************************************************************
	* function name 		: ThreadPoolGetCurrentThreadNum
	* functional description	: 获取线程池存放的线程数
	* input parameter		: pthis	线程池指针
	* output parameter	: 
	* return value			: 线程池存放的线程数
	* history				: 
	*****************************************************************/	
	int (*GetCurThreadNum) (void *pthis); 
	
	/****************************************************************
	* function name 		: ThreadPoolGetCurrentTaskThreadNum
	* functional description	: 获取当前正在执行任务和已分配任务的线程数目和
	* input parameter		: pthis	线程池指针
	* output parameter	: 
	* return value			: 当前正在执行任务和已分配任务的线程数目和
	* history				: 
	*****************************************************************/
	int (*GetCurTaskThreadNum) (void *pthis); 
	
	/****************************************************************
	* function name 		: ThreadPoolGetCurrentTaskNum
	* functional description	: 获取线程池等待队列任务数
	* input parameter		: pthis	线程池指针
	* output parameter	: 
	* return value			: 等待队列任务数
	* history				: 
	*****************************************************************/
	int (*GetCurTaskNum) (void *pthis); 
	
	/****************************************************************
	* function name 		: ThreadPoolDestroy
	* functional description	: 销毁线程池
	* input parameter		: pthis	线程池指针
	* output parameter	: 
	* return value			: 0 - 成功;-1 - 失败
	* history				: 
	*****************************************************************/
	int (*Destruct) (void *pthis); 
};

/*---------------functions declaration--------------------------*/
/****************************************************************
* function name 		: ThreadPoolConstruct
* functional description	: 创建线程池
* input parameter		: max_num	线程池可容纳的最大线程数
					  free_num	线程池允许存在的最大空闲线程,超过则将线程释放回操作系统
* output parameter	: 
* return value			: 线程池指针
* history				: 
*****************************************************************/
CThread_pool_t* ThreadPoolConstruct(int max_num,int free_num);

/****************************************************************
* function name 		: ThreadPoolConstructDefault
* functional description	: 创建线程池,以默认的方式初始化,未创建线程
* input parameter		: 
* output parameter	: 
* return value			: 线程池指针
* history				: 
*****************************************************************/
CThread_pool_t* ThreadPoolConstructDefault(void);

#endif

