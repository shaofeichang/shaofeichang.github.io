/************************************************************************
* module			: �̳߳�ͷ�ļ�
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
/*�̳߳�������*/
struct worker_t
{
	void *(*process) (void *arg);	/**< �ص����� */
    int   paratype;					/**< ��������(Ԥ��) */
    void *arg;						/**< �ص��������� */
    struct worker_t *next;			/**< ������һ�������� */
};

/*�̳߳ؿ�����*/
struct CThread_pool_t
{
	pthread_mutex_t queue_lock;	/**< ������ */
    pthread_cond_t queue_ready;	/**< �������� */

    worker_t *queue_head;	/**< ����������,��������Ͷ�ݵ����� */
    int shutdown;			/**< �̳߳����ٱ�־,1 - ���� */
    pthread_t *threadid;	/**< �߳�ID */
    int max_thread_num;		/**< �̳߳ؿ����ɵ�����߳��� */
	int current_pthread_num;	/**< ��ǰ�̳߳ش�ŵ��߳��� */
	int current_pthread_task_num;	/**< ��ǰ����ִ��������ѷ���������߳���Ŀ�� */
    int cur_queue_size;		/**< ��ǰ�ȴ����е�������Ŀ */
	int	free_pthread_num;	/**< �̳߳���������ڵ��������߳��� */
	
	/****************************************************************
	* function name 		: ThreadPoolAddWorkLimit
	* functional description	: ���̳߳�Ͷ������
	* input parameter		: pthis	�̳߳�ָ��
						  process	�ص�����
						  arg		�ص������Ĳ���
	* output parameter	: 
	* return value			: 0 - �ɹ�;-1 - ʧ�� 
	* history				: 
	*****************************************************************/
	int (*AddWorkUnlimit)(void* pthis,void *(*process) (void *arg), void *arg); 

	/****************************************************************
	* function name 		: ThreadPoolAddWorkUnlimit
	* functional description	: ���̳߳�Ͷ������,�޿����߳�������
	* input parameter		: pthis	�̳߳�ָ��
						  process	�ص�����
						  arg		�ص������Ĳ���
	* output parameter	: 
	* return value			: 0 - �ɹ�;-1 - ʧ�� 
	* history				: 
	*****************************************************************/
	int (*AddWorkLimit)(void* pthis,void *(*process) (void *arg), void *arg); 
	
	/****************************************************************
	* function name 		: ThreadPoolGetThreadMaxNum
	* functional description	: ��ȡ�̳߳ؿ����ɵ�����߳���
	* input parameter		: pthis	�̳߳�ָ��
	* output parameter	: 
	* return value			: �̳߳ؿ����ɵ�����߳���
	* history				: 
	*****************************************************************/
	int (*GetMaxThreadNum) (void *pthis); 

	/****************************************************************
	* function name 		: ThreadPoolGetCurrentThreadNum
	* functional description	: ��ȡ�̳߳ش�ŵ��߳���
	* input parameter		: pthis	�̳߳�ָ��
	* output parameter	: 
	* return value			: �̳߳ش�ŵ��߳���
	* history				: 
	*****************************************************************/	
	int (*GetCurThreadNum) (void *pthis); 
	
	/****************************************************************
	* function name 		: ThreadPoolGetCurrentTaskThreadNum
	* functional description	: ��ȡ��ǰ����ִ��������ѷ���������߳���Ŀ��
	* input parameter		: pthis	�̳߳�ָ��
	* output parameter	: 
	* return value			: ��ǰ����ִ��������ѷ���������߳���Ŀ��
	* history				: 
	*****************************************************************/
	int (*GetCurTaskThreadNum) (void *pthis); 
	
	/****************************************************************
	* function name 		: ThreadPoolGetCurrentTaskNum
	* functional description	: ��ȡ�̳߳صȴ�����������
	* input parameter		: pthis	�̳߳�ָ��
	* output parameter	: 
	* return value			: �ȴ�����������
	* history				: 
	*****************************************************************/
	int (*GetCurTaskNum) (void *pthis); 
	
	/****************************************************************
	* function name 		: ThreadPoolDestroy
	* functional description	: �����̳߳�
	* input parameter		: pthis	�̳߳�ָ��
	* output parameter	: 
	* return value			: 0 - �ɹ�;-1 - ʧ��
	* history				: 
	*****************************************************************/
	int (*Destruct) (void *pthis); 
};

/*---------------functions declaration--------------------------*/
/****************************************************************
* function name 		: ThreadPoolConstruct
* functional description	: �����̳߳�
* input parameter		: max_num	�̳߳ؿ����ɵ�����߳���
					  free_num	�̳߳�������ڵ��������߳�,�������߳��ͷŻز���ϵͳ
* output parameter	: 
* return value			: �̳߳�ָ��
* history				: 
*****************************************************************/
CThread_pool_t* ThreadPoolConstruct(int max_num,int free_num);

/****************************************************************
* function name 		: ThreadPoolConstructDefault
* functional description	: �����̳߳�,��Ĭ�ϵķ�ʽ��ʼ��,δ�����߳�
* input parameter		: 
* output parameter	: 
* return value			: �̳߳�ָ��
* history				: 
*****************************************************************/
CThread_pool_t* ThreadPoolConstructDefault(void);

#endif

