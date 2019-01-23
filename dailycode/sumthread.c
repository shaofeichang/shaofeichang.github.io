#include <pthread.h>
#include <stdio.h>

double arr1[200000];
double total=0.0;
int c;
pthread_mutex_t my_mutex;


void * count_func(void * arg)
{
    pthread_mutex_lock(&my_mutex);

    int id = (int)arg;
     if(id)
    {   int i;
        for(i=200000/c*(id-1);i<200000/c*id;i++)
        {
            total += arr1[i];
        }
        printf("%d: sum is %lf\n",id,total);
        id = 0;
    }

    pthread_mutex_unlock(&my_mutex);
  }




	int main(int argc, char* argv[]) {
	    pthread_mutex_init(&my_mutex,NULL);
	    int rc;
        int i= 0,*j;
	    for(i=0;i<200000;i++)   arr1[i] = 1;
	    printf("The remainder of 200000 divided by this number is zero\nPlease input this number and: ");
        scanf("%d",&c);
        while(200000%c != 0){
            printf("error ,please input again: \n");
            printf("please input a number: ");
            scanf("%d",&c);
        }
        pthread_t thread[c];
        for(i=0;i<c;i++)
        {
        rc = pthread_create(&thread[i], NULL,count_func, (void *)(i+1));
	    if (rc)
           printf("ERROR when creating default thread; Code is %d\n", rc);
	       pthread_join(thread[i], NULL);
         }

        printf("The final sum :%lf",total);

	    pthread_exit(NULL);
	}
