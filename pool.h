/*************************************************************************
> File Name: pool.h
> Author: sky
> Mail:yaoxing@bjtu.edu.cn
************************************************************************/

#ifndef _POOL_H
#define _POOL_H

#include <pthread.h>
#include <vector>
#include <deque>

//任务的结构体
typedef struct {
	void(*function)(void *);	//指定任务的  函数，参数为void*，用来传递任何参数
	void *args;			//用来指定函数的参数
} threadpool_task_t;


//定义线程池类
class Threadpool {

public:
	pthread_mutex_t myLock;				//线程池的互斥锁
	pthread_cond_t myCond;				//信号量，用来与互斥锁协调工作
	std::vector<pthread_t> threads_vec;		//用来存放 thread_count 个一直运行的线程
	std::deque<threadpool_task_t> task_que;		//用来存放 任务，没有线程及时处理的任务将被放入 该任务队列
	int thread_count;	//线程池一共申请的线程数量
	int queue_size;		//任务队列长度，最多排队的任务数量
	bool isShutdown;	//线程池是否关闭
	int started;		//工作中的线程数量,相当于正在运行的任务数




public:
	//创建线程池的api       
	//thread_count 开辟这么多的线程     
	//queue_size   最多有这么多个任务在排队等待
	Threadpool(int thread_count = 32, int queue_size = 1024);

	//添加任务到线程池的api
	//routine 指向要添加任务的函数
	//args   用来传递该任务函数的参数
	bool addTask(void(*routine)(void*), void *arg);

	//销毁线程池的api
	//flags 关闭的方式
	//如果为1，运行完所有排队中的任务再结束；
	//如果为0，线程执行完工作中的函数直接退出，不用等待任务队列中的函数
	bool destroy(int flags = 1);

};


const int threadpool_graceful = 1;


#endif
