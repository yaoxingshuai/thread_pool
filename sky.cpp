#define THREAD 32
#define QUEUE  256

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <iostream>

#include "pool.h"

using namespace std;

int tasks = 0, done = 0;
pthread_mutex_t lock;

void dummy_task(void *arg) {
	usleep(10000);
	pthread_mutex_lock(&lock);
	/* 记录成功完成的任务数 */
	done++;
	pthread_mutex_unlock(&lock);
}

int main(int argc, char **argv)
{
	cout << "hello" << endl;
	Threadpool pool(THREAD, QUEUE);
	cout << "world" << endl;
	/* 初始化互斥锁 */
	pthread_mutex_init(&lock, NULL);

	/* 断言线程池创建成功 */

	
	cout << "begin..." << endl;
	bool state;
	/* 只要任务队列还没满，就一直添加 */
	while ( (state=pool.addTask(&(dummy_task), NULL)) !=false )
	{

		pthread_mutex_lock(&lock);
		tasks++;
		if (tasks % 10 == 0)
			cout << tasks << endl;
		pthread_mutex_unlock(&lock);
	}
	
	cout << "begin1..." << endl;

	fprintf(stderr, "Added %d tasks\n", tasks);

	/* 不断检查任务数是否完成一半以上，没有则继续休眠 */
	while ((tasks / 2) > done) {
		cout << "has done:" << done << endl;
		usleep(5000);
	}
	/* 这时候销毁线程池,0 代表 immediate_shutdown */
	fprintf(stderr, "Did %d tasks\n", done);
	
	pool.destroy();
	
	cout << "begin2..." << endl;

	while ( tasks> done) {
		usleep(10000);
	}
	fprintf(stderr, "Did %d tasks\n", done);
	return 0;
}
