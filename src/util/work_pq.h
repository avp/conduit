#ifndef __WORKER_WORK_PQ_H__
#define __WORKER_WORK_PQ_H__


#include <queue>

template <class T>
class PQNode {
public:
  double priority;
  T data;

  PQNode(const T& newData, double newPriority) : priority(newPriority), data(newData) {
  }

  bool operator< (const PQNode<T>& x) const
  {
    return this->priority < x.priority;
  }
};


template <class T>
class WorkPQ {
private:
  std::priority_queue<PQNode<T>> pq;
  pthread_mutex_t queue_lock;
  pthread_cond_t queue_cond;

public:

  WorkPQ() {
    pthread_cond_init(&queue_cond, NULL);
    pthread_mutex_init(&queue_lock, NULL);
  }

  T dequeue() {
    pthread_mutex_lock(&queue_lock);
    while (pq.size() == 0) {
      pthread_cond_wait(&queue_cond, &queue_lock);
    }

    PQNode<T> item = pq.top();
    pq.pop();

    pthread_mutex_unlock(&queue_lock);
    return item.data;
  }

  void enqueue(const T& item, double priority) {
    pthread_mutex_lock(&queue_lock);
    pq.push(PQNode<T>(item, priority));
    pthread_mutex_unlock(&queue_lock);
    pthread_cond_signal(&queue_cond);
  }

  size_t size() {
    return pq.size();
  }
};

#endif  // __WORKER_WORK_PQ_H__
