//--------------------------------------------------------------------------------------------------
//
//	Coordinates: A compile-time c++23 coordinate conversion library based on `units`
//
//--------------------------------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//--------------------------------------------------------------------------------------------------
//
// Copyright (c) 2016 Nic Holthaus
//
//--------------------------------------------------------------------------------------------------
//  Cross-platform thread pool that preserves the existing public interface:
//
//    - static ThreadPool& instance();
//    - template<class F, class... Args> auto async(F&&, Args&&...) -> std::future<R>;
//    - std::size_t size() const noexcept;
//
//  Implementation notes:
//    - Uses the project's concurrentQueue<T> for the task queue.
//    - Uses std::jthread for RAII joining and stop_token support.
//    - Shutdown is cooperative: set m_stopping, request_stop, and push N sentinel tasks
//      to wake workers even if they are blocked in pop().
//
//  Requirements:
//    - C++23 (works fine in C++20 too; written to be modern and clean)
//--------------------------------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>
#include <stop_token>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include "concurrentQueue.h"

/**
 * @brief A small thread pool singleton for running work asynchronously.
 */
class ThreadPool
{
public:
	ThreadPool(const ThreadPool&)            = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	/**
	 * @brief Get the global thread pool instance (Meyers singleton).
	 */
	static ThreadPool& instance()
	{
		static ThreadPool s_pool;
		return s_pool;
	}

	/**
	 * @brief Submit a callable to run on the pool.
	 *
	 * @tparam F Callable type
	 * @tparam Args Argument types
	 * @return std::future<R> where R is the invoke result type
	 *
	 * @throws std::runtime_error if called during shutdown
	 */
	template<class F, class... Args>
	[[nodiscard]] auto async(F&& f, Args&&... args) -> std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
	{
		using R = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;

		if (m_stopping.load(std::memory_order_acquire))
		{
			throw std::runtime_error("ThreadPool::async called during shutdown");
		}

		// Wrap as packaged_task so exceptions propagate to the returned future.
		auto taskPtr = std::make_shared<std::packaged_task<R()>>(
		        [fn = std::forward<F>(f), ... a = std::forward<Args>(args)]() mutable -> R
		        {
			        if constexpr (std::is_void_v<R>)
			        {
				        std::invoke(std::move(fn), std::move(a)...);
				        return;
			        }
			        else
			        {
				        return std::invoke(std::move(fn), std::move(a)...);
			        }
		        });

		std::future<R> fut = taskPtr->get_future();

		// Enqueue a copyable callable; concurrentQueue is copy/move friendly.
		// We keep this as std::function<void()> to match queue expectations cleanly.
		m_tasks.push(std::function<void()>(
		        [taskPtr]() mutable
		        {
			        (*taskPtr)();    // packaged_task stores exceptions in its future
		        }));

		return fut;
	}

	/**
	 * @brief Number of worker threads.
	 */
	[[nodiscard]] std::size_t size() const noexcept { return m_workers.size(); }

private:
	ThreadPool()
	{
		const unsigned    hw = std::thread::hardware_concurrency();
		const std::size_t n  = (hw == 0u) ? 1u : static_cast<std::size_t>(hw);

		m_workers.reserve(n);
		for (std::size_t i = 0; i < n; ++i)
		{
			m_workers.emplace_back([this](std::stop_token stop) { this->workerLoop(stop); });
		}
	}

	~ThreadPool()
	{
		// Signal stop first so workers that wake up will exit.
		m_stopping.store(true, std::memory_order_release);

		// Ask all workers to stop (cooperative).
		for (auto& w : m_workers)
		{
			w.request_stop();
		}

		// Wake workers that might be blocked in pop() by pushing sentinel "empty" tasks.
		// We push at least one per worker to guarantee everyone wakes.
		const std::size_t n = m_workers.size();
		for (std::size_t i = 0; i < n; ++i)
		{
			m_tasks.push(std::function<void()>());    // empty => sentinel
		}

		// std::jthread joins in its destructor. No explicit join needed.
	}

	void workerLoop(const std::stop_token& stop)
	{
		std::function<void()> task;

		while (!stop.stop_requested())
		{

			if (const bool got = m_tasks.pop(task); !got)
			{
				// Timed out waiting for work; check stop flags.
				if (m_stopping.load(std::memory_order_acquire) || stop.stop_requested())
					return;
				continue;
			}

			// Sentinel wake-up task: empty std::function.
			if (!task)
			{
				if (m_stopping.load(std::memory_order_acquire) || stop.stop_requested())
					return;
				// Otherwise ignore spurious sentinel.
				continue;
			}

			try
			{
				task();
			}
			catch (...)
			{
				// If someone enqueues a raw task that throws (not via packaged_task),
				// swallow to keep worker alive. packaged_task already captures exceptions.
			}
		}
	}

private:
	concurrentQueue<std::function<void()>> m_tasks;
	std::vector<std::jthread>              m_workers;
	std::atomic<bool>                      m_stopping{false};
};
