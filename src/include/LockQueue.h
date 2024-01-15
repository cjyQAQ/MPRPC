#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

// 异步日志队列
template<typename T>
class LockQueue
{
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condition_variable;
public:
    LockQueue(){}
    ~LockQueue(){}
    void Push(const T &data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_condition_variable.notify_one();
    }
    T Pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_queue.empty())
        {
            // 日志队列为空，线程等待
            m_condition_variable.wait(lock);
        }
        T data = m_queue.front();
        m_queue.pop();
        return data;
    }
};


