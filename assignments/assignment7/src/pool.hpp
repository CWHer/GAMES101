#ifndef __POOL_HPP__
#define __POOL_HPP__

#include <atomic>
#include <future>
#include <vector>
#include <queue>
#include <memory>
#include <iostream>
#include <functional>

// NOTE: a fake pool
//  generate thread for each task
class Pool
{
private:
    const int THREAD_NUM;
    std::atomic<int> thread_num;
    std::mutex commit_mutex, cv_lock_mutex;
    std::condition_variable task_cv;

public:
    Pool(int num = 4) : THREAD_NUM(num)
    {
        thread_num = THREAD_NUM;
    }

    template <class F, class... Args>
    void commit(F &&f, Args &&...args)
    {
        commit_mutex.lock();
        if (thread_num <= 0)
        {
            std::unique_lock<std::mutex> cv_lock(cv_lock_mutex);
            task_cv.wait(cv_lock);
        }
        thread_num--;
        commit_mutex.unlock();

        auto executor = [this](auto task)
        {
            task();

            this->task_cv.notify_one();
            this->thread_num++;
        };

        auto task = std::bind(f, args...);
        std::thread t(executor, task);
        t.detach();
    }

    void wait()
    {
        while (thread_num < THREAD_NUM)
        {
            std::unique_lock<std::mutex> cv_lock(cv_lock_mutex);
            task_cv.wait(cv_lock);
        }
    }
};

// // NOTE: a true pool
// // but it is really slow, and i don't know why...
// class Pool
// {
// private:
//     std::atomic<bool> running;
//     const int THREAD_NUM;
//     std::atomic<int> thread_num;
//     std::mutex worker_mutex, queue_mutex, done_mutex;
//     std::condition_variable task_cv, done_cv;

//     std::vector<std::thread> threads;
//     std::queue<std::function<void()>> tasks;

// private:
//     void initThreads()
//     {
//         running = true;
//         auto worker = [this](int worker_id)
//         {
//             while (running)
//             {
//                 std::function<void()> task;
//                 std::unique_lock<std::mutex> task_lock(worker_mutex);

//                 // while (running && tasks.empty())
//                 // while (!pred()) wait(lock);
//                 task_cv.wait(task_lock, [this]
//                              { return !running || !tasks.empty(); });
//                 // std::cout << "worker " << worker_id << " wake up" << std::endl;
//                 {
//                     std::lock_guard<std::mutex> lock(queue_mutex);
//                     if (tasks.empty())
//                         continue;
//                     task = std::move(tasks.front());
//                     tasks.pop();
//                     if (tasks.empty())
//                         done_cv.notify_one();
//                 }
//                 // execute task
//                 task();
//             };
//             // std::cout << "done" << std::endl;
//         };
//         for (int i = 0; i < THREAD_NUM; ++i)
//             threads.emplace_back(std::thread(worker, i));
//     }

// public:
//     Pool(int num = 4) : THREAD_NUM(num)
//     {
//         initThreads();
//     }

//     template <class F, class... Args>
//     auto commit(F &&f, Args &&...args)
//         -> std::future<decltype(f(args...))>
//     {
//         using RetType = decltype(f(args...));
//         auto task = std::make_shared<std::packaged_task<RetType()>>(
//             std::bind(std::forward<F>(f), std::forward<Args>(args)...));
//         std::future<RetType> future = task->get_future();

//         // enqueue task
//         {
//             std::lock_guard<std::mutex> lock(queue_mutex);
//             tasks.emplace([task]
//                           { (*task)(); });
//         }

//         // notify worker
//         task_cv.notify_one();
//         return future;
//     }

//     void wait()
//     {
//         // NOTE: this break while loop
//         std::unique_lock<std::mutex> done_lock(done_mutex);
//         // std::cout << tasks.size() << std::endl;
//         // while (!pred()) wait(lock);
//         done_cv.wait(done_lock, [this]
//                      { return tasks.empty(); });
//         running = false;

//         task_cv.notify_all();
//         for (auto &thread : threads)
//             if (thread.joinable())
//                 thread.join();
//     }
// };

#endif
