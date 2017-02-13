#include <iostream>
#include <stdio.h>
#include <pthread.h>
int *bounds = new int[2]{0,0};
using namespace std;
pthread_mutex_t mutx = PTHREAD_MUTEX_INITIALIZER;

void* counting_function(void *arg)
{
	int *bounds = (int*)arg;

	pthread_mutex_lock(& mutx);
	bounds[0]= 500;
	bounds[1]= 7;
	cout<<"IN FUNCTION "<<bounds[0]<<" "<<bounds[1]<<endl;
	cout<<"leaving function"<<endl;
	pthread_mutex_unlock(& mutx);
	pthread_exit(NULL);
}

int main(void)
{
	pthread_t id1;
	pthread_create(&id1,NULL,counting_function,bounds);

	pthread_t id2;
	pthread_create(&id2,NULL,counting_function,bounds);

	pthread_join(id1,NULL);
	pthread_join(id2,NULL);

	cout<<"IN MAIN:"<<bounds[0]<<" "<<bounds[1]<<endl;

	return 0;

}
