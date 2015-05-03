// Copyright 2013 15418 Course Staff.

#ifndef UTIL_WORKQUEUE_H_
#define UTIL_WORKQUEUE_H_

#include <vector>

template <class T>
class WorkQueue {
private:
  std::vector<T> storage;
  pthread_mutex_t queue_lock;
  pthread_cond_t queue_cond;

public:
  WorkQueue() {
    pthread_cond_init(&queue_cond, NULL);
    pthread_mutex_init(&queue_lock, NULL);
  }

  size_t size() {
    return storage.size();
  }

  T dequeue() {
    pthread_mutex_lock(&queue_lock);
    while (storage.size() == 0) {
      pthread_cond_wait(&queue_cond, &queue_lock);
    }

    T item = storage.front();
    storage.erase(storage.begin());

    pthread_mutex_unlock(&queue_lock);
    return item;
  }

  void enqueue(const T& item) {
    pthread_mutex_lock(&queue_lock);
    storage.push_back(item);
    pthread_mutex_unlock(&queue_lock);
    pthread_cond_signal(&queue_cond);
  }
};

#endif
