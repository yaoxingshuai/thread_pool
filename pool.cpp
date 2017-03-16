#include "pool.h"
#include <cstdlib>
#include <iostream>

using namespace std;

static void *threadpool_thread(void *pool);


Threadpool::Threadpool(int thread_count_, int queue_size_)
{
	if (thread_count_ <= 0 || thread_count_ >= 512
			|| queue_size_ <= 0 || queue_size_ >= 4096)
	{
		cout << "thread_count or queue_size, use default config" << endl;
		thread_count_ = 32;
		queue_size_ = 1024;
	}
	thread_count = 0;
	queue_size = queue_size_;
	isShutdown = false;
	started = 0;

	//threads_vec默认是空的，所以一定要先 push_back 或者赋值一些元素。
	//否则如果vec为空， pthread_create会出现段错误
	pthread_t tmp;
	threads_vec = vector<pthread_t>(thread_count_, tmp);

	//初始化互斥锁和条件变量
	if ((pthread_mutex_init(&myLock, NULL) != 0) ||
		(pthread_cond_init(&myCond, NULL) != 0))
	{
		cout << "lock or cond error..." << endl;
		exit(1);
	}
	for (int i = 0; i < thread_count_; ++i)
	{
		if (pthread_create(&(threads_vec[i]), NULL, threadpool_thread, this) != 0)
		{
			destroy(1);
			return;
		}
		thread_count++;
		started++;
	}
}

bool Threadpool::addTask(void(*routine)(void*), void *argument)
{
	if (routine == NULL)
	{
		cout << "routine is nullptr" << endl;
		return false;
	}

	//添加任务之前先取得互斥锁的资源
	if (pthread_mutex_lock(&myLock) != 0)
	{
		cout << "addTask: lock error..." << endl;
		return false;
	}

	bool isOk = true;

	do {
		//are we full
		if (task_que.size() >= queue_size) {
			cout << "task_que: too much task, full..." << endl;
			isOk = false;
			break;
		}
		if (isShutdown)
		{
			cout << "threadpool has shutdown..." << endl;
			isOk = false;
			break;
		}
		threadpool_task_t newtask;
		newtask.function = routine;
		newtask.args = argument;
		task_que.push_back(newtask);

		//发出信号，等待处理    (如果所有线程都处于工作状态，没有线程 cond_wait()也无所谓
		if (pthread_cond_signal(&myCond) != 0) {
			cout << "cond signal error..." << endl;
			break;
		}		
		
	} while (0);
	pthread_mutex_unlock(&myLock);
	return isOk;
}

bool Threadpool::destroy(int flags)
{
	pthread_mutex_lock(&myLock);

	isShutdown = true;
	pthread_cond_broadcast(&myCond);
	pthread_mutex_unlock(&myLock);

	for (int i = 0; i < threads_vec.size(); ++i)
		pthread_join(threads_vec[i], NULL);
	return true;
}

static void *threadpool_thread(void *pool_)
{
	Threadpool *pool = (Threadpool*)pool_;
	threadpool_task_t task;

	while (1)
	{
		pthread_mutex_lock(&(pool->myLock));

		//如果任务队列为空，并且pool还没有关闭。   就等待信号（有新的任务到达是，会cond_signal）
		while ((pool->task_que.size() == 0) && (!pool->isShutdown))
		{
			pthread_cond_wait(&(pool->myCond), &(pool->myLock));
		}

		//如果 线程池关闭，并且任务队列已经处理完成， break出去
		if (pool->isShutdown && pool->task_que.size() == 0)
		{
			break;
		}

		task = pool->task_que.front();
		pool->task_que.pop_front();

		pthread_mutex_unlock(&(pool->myLock));
		(*(task.function))(task.args);
	}
	pool->started--;

	pthread_mutex_unlock(&(pool->myLock));
	pthread_exit(NULL);
	return(NULL);
}