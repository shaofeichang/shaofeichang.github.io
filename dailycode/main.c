#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <pthread.h> 
#include <assert.h> 
#include <string.h>

#include "lib_thread_pool.h"


static void* thread_1(void* arg);
static void* thread_2(void* arg);
static void* thread_3(void* arg);
static void DisplayPoolStatus(CThread_pool_t* pPool);

int nKillThread = 0;

int main()
{
	CThread_pool_t* pThreadPool = NULL;
	pThreadPool = ThreadPoolConstruct(2, 1);
	int nNumInput = 5;
	char LogInput[] = "OK!";

	DisplayPoolStatus(pThreadPool);
	/*可用AddWorkLimit()替换看执行的效果*/
	pThreadPool->AddWorkUnlimit((void*)pThreadPool, thread_1, (void*)NULL);
	/*
	* 没加延迟发现连续投递任务时pthread_cond_wait()会收不到信号pthread_cond_signal() !!
	* 因为AddWorkUnlimit()进去后调用pthread_mutex_lock()把互斥锁锁上,导致pthread_cond_wait()
	* 收不到信号!!也可在AddWorkUnlimit()里面加个延迟,一般情况可能也遇不到这个问题
	*/
	usleep(10);	
	pThreadPool->AddWorkUnlimit((void*)pThreadPool, thread_2, (void*)nNumInput);
	usleep(10);
	pThreadPool->AddWorkUnlimit((void*)pThreadPool, thread_3, (void*)LogInput);
	usleep(10);
	DisplayPoolStatus(pThreadPool);

	nKillThread = 1;
	usleep(100);	/**< 先让线程退出 */
	DisplayPoolStatus(pThreadPool);
	nKillThread = 2;
	usleep(100);
	DisplayPoolStatus(pThreadPool);
	nKillThread = 3;
	usleep(100);
	DisplayPoolStatus(pThreadPool);

	pThreadPool->Destruct((void*)pThreadPool);
	return 0;
}

static void* thread_1(void* arg)
{
	printf("Thread 1 is running !\n");
	while(nKillThread != 1)
		usleep(10);
	return NULL;
}

static void* thread_2(void* arg)
{
	int nNum = (int)arg;
	
	printf("Thread 2 is running !\n");
	printf("Get Number %d\n", nNum);
	while(nKillThread != 2)
		usleep(10);
	return NULL;
}

static void* thread_3(void* arg)
{
	char *pLog = (char*)arg;
	
	printf("Thread 3 is running !\n");
	printf("Get String %s\n", pLog);
	while(nKillThread != 3)
		usleep(10);
	return NULL;
}

static void DisplayPoolStatus(CThread_pool_t* pPool)
{
	static int nCount = 1;
	
	printf("******************\n");
	printf("nCount = %d\n", nCount++);
	printf("max_thread_num = %d\n", pPool->GetMaxThreadNum((void*)pPool));
	printf("current_pthread_num = %d\n", pPool->GetCurThreadNum((void*)pPool));
	printf("current_pthread_task_num = %d\n", pPool->GetCurTaskThreadNum((void*)pPool));
	printf("cur_queue_size = %d\n", pPool->GetCurTaskNum((void*)pPool));
	printf("******************\n");
}

