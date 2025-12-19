
/**

@mainpage C++11 Thread Pool Documentation

Thread pool using std::* primitives from C++11. The entire library is only one class (ThreadPool)
with one method (async) for submitting tasks.

ThreadPool::async is a templated method that accepts any callable function and arguments to pass to the
function. It returns a std::future<Ret> where Ret is the return type of the aforementioned
function.

The ThreadPool itself is a singleton object. It should be created at global scope.

To submit a task: future<Ret> fut(pool.async(myFunction, args...));.

To wait on fut to complete: Ret result = fut.get();

@section example Example Usage (with classic function pointers):
@code

#include <iostream>
#include <thread>
#include <chrono>
#include "threadPool.h"

ThreadPool& pool = ThreadPool::instance();

bool prime(int n)
{
	bool prime = true;

	if (n < 2)
		prime = false;
	for (int i = 2; i < n; i++)
		if (n % i == 0)
			prime = false;

	return prime;
}

int main()
{
	std::vector<bool> results;
	std::vector<std::future<bool>> primes;

	// Spawn the worker tasks
	for (int i = 0; i < 100000; i++)
	{
		primes.emplace_back(pool.async(prime, i));
	}

	//.... other code can execute here, creating a pipeline.

	// it's important to spawn all the worker tasks before 'getting' any, to avoid unnecessary
	// blocking.
	for (auto itr = primes.begin(); itr != primes.end(); itr++)
	{
		results.push_back(itr->get());
		if (results.back())
		{
			std::cout << results.size() - 1 << " is prime." << std::endl;
		}
	}

}

@endcode

@section lambda Usage with Lambda Functions
@code

...

for (int i = 0; i < numruns; i++)
{
	primes.emplace_back(pool.async([](int n)
	{
		// determine if n is prime
		for (int i = 2; i < n; ++i)
			if (n%i == 0)
				return false;

		return true;
	}, i));
}

...

@endcode

@section functor Usage with Functors
@code

...

struct task
{
	bool operator()(int n)
	{
		for (int i = 2; i < n; ++i)
			if (n%i == 0)
				return false;

		return true;
	}
};

for (int i = 0; i < numruns; i++)
{
	primes.emplace_back(pool.async(task, i));
}

...

@endcode

@section members Usage with Member Functions
@code

...

class algorithm
{
public:

	bool calculatePrime(int n)
	{
		for (int i = 2; i < n; ++i)
		if (n%i == 0)
			return false;

		return true;
	}
} myAlgorithm;

std::function<bool(int)> task = std::bind(&algorithm::calculatePrime, myAlgorithm, std::placeholders::_1);

for (int i = 0; i < numruns; i++)
{
	primes.emplace_back(pool->async(task, i));
}

...

@endcode

*/

#ifndef threadPool_h__
#define threadPool_h__

//------------------------
//	INCLUDES
//------------------------
#include <atomic>
#include <deque>
#include <functional>
#include <future>
#include <vector>
#include <iostream>

#include "concurrentQueue.h"

//	--------------------------------------------------------------------------------
///	@class		ThreadPool
///	@brief		A thread pool with a selectable number of worker threads.
///	@details	This class requires a c++11 complaint compiler.
//  --------------------------------------------------------------------------------
class ThreadPool {
public:

    /**
     * @brief		Returns pointer to the instance of the thread pool.
     * @details		The thread pool is a singleton object with a lifespan equal to that of the
     *				application.
     */
    static ThreadPool& instance()
    {
        static ThreadPool instance;
        return instance;
    }

	// Not copyable
	ThreadPool(const ThreadPool&) = delete;
	void operator=(const ThreadPool&) = delete;

    /**
     * @brief		Thread pool destructor.
     * @details		Signals all worker threads to join, and blocks on them before deleting.
     */
    ~ThreadPool()
    {
        join.store(true);
#ifndef _MSC_VER	// static instance destructors can't wait on thread termination in windows due to CRT bug: http://stackoverflow.com/questions/17093164/why-does-this-c-static-singleton-never-stop
		for (auto i = threads.begin(); i != threads.end(); i++)
		{
			i->join();
		}
#else
		// give the threads a little bit of time to exit cleanly. This is the best we can do on windows.
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
#endif
    }

    /**
     * @brief		number of threads in the thread pool.
     * @returns		number of worker threads.
     */
    int size()
    {
        return num_threads;
    }

public:

    /**
     * @brief		runs the given function with the given args in the thread pool.
     * @details
     * @param[in]	f function to be run on the thread pool.
     * @param[in]	args arguments to the function 'f'.
     * @returns		a std::future object of the type returned by 'f'. Calling 'get()' on the
     *				return value will retrieve the result of calculating 'f' with 'args'.
     */
#if	!defined(_MSC_VER) || _MSC_VER > 1800 // min VS2015 to support lambda forwarding
    template<class Function, class... Args>
    auto async(Function&& f, Args&&... args) -> std::future<decltype(f(args...))>
    {
        std::atomic<bool> *ready = new std::atomic<bool>(false);
        std::promise<decltype(f(args...))> *p = new std::promise<decltype(f(args...))>;

        auto task_wrapper = [p, ready](auto&& f, auto&&... args) mutable
        {
            p->set_value(f(std::forward<decltype(args)>(args)...));
            ready->store(true);
        };

        auto ret_wrapper = [p, ready]() mutable -> decltype(f(args...))
        {
            while (!ready->load())
            {
                std::this_thread::yield();
            }
            auto temp = std::move(p->get_future().get());
            // Clean up resources
            delete p;
            delete ready;
            return std::move(temp);
        };

        tasks.push(std::async(std::launch::deferred, std::move(task_wrapper), std::forward<Function>(f), std::forward<Args>(args)...));
        return std::async(std::launch::deferred, std::move(ret_wrapper));
    }
#else
    template<class Function, class... Args>
    auto async(Function f, Args... args) -> std::future<decltype(f(args...))>
    {
        std::atomic<bool> *ready = new std::atomic<bool>(false);
        std::promise<decltype(f(args...))> *p = new std::promise<decltype(f(args...))>;

        auto task_wrapper = [p, ready](Function f, Args... args) mutable
        {
            p->set_value(f(std::forward<decltype(args)>(args)...));
            ready->store(true);
        };

        auto ret_wrapper = [p, ready]() mutable -> decltype(f(args...))
        {
            while (!ready->load())
            {
                std::this_thread::yield();
            }
            auto temp = std::move(p->get_future().get());
            // Clean up resources
            delete p;
            delete ready;
            return std::move(temp);
        };

        tasks.push(std::async(std::launch::deferred, std::move(task_wrapper), f, args...));
        return std::async(std::launch::deferred, std::move(ret_wrapper));
    }
#endif

protected:

    /**
     * @brief		creates the worker threads and binds them to the thread function.
     * @details
     */
    void init_threads() {
        for (unsigned int i = 0; i < num_threads; i++) {
            std::function<void(void)> f = std::bind(&ThreadPool::thread_func, this);
            threads.push_back(std::thread(f));
        }
    }

    /**
     * @brief		main processing loop for worker threads.
     * @details		Worker threads query the task queue, and if it is
     *				not empty, they pop a task off and execute it. If
     *				the thread cannot access the queue or it is empty,
     *				it yields to the rest of the pool.
     */
    void thread_func()
    {
        std::future<void> f;
        while (!join.load())
        {
            if (tasks.pop(f)) f.get();
        }
    }

private:

    ThreadPool() : num_threads(std::thread::hardware_concurrency())
    {
        join.store(false);
        init_threads();
    }

    std::atomic<bool> join;																			///< Signal to the thread pool member threads that they should join themselves.
    unsigned int num_threads;																		///< Number of threads in the pool.
    std::mutex task_mutex;																			///< Mutex protecting access to the tasks dequeue. Note that std container classes are typically only thread safe when accessing different memory locations for reading.
    concurrentQueue<std::future<void>> tasks;														///< Queue to store the future values of the 'tasks' to be completed. These values are accessed by calling their .get() function, which will block until the thread returns.
    std::vector<std::thread> threads;																///< Container for the pools thread members.

};

#endif // threadPool_h__