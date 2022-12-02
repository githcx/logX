#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include <list>
#include <mutex>

template<class T>
class Pool {
public:
  ~Pool(){
    for(auto iter : m_obj_list) {
      delete iter;
    }
  }

  T* Get();
  void Put(T* obj);

private:
  std::mutex m_mutex;
  std::list<T*> m_obj_list;
};

template<class T>
T* Pool<T>::Get() {
  std::unique_lock<std::mutex> locker(m_mutex);
  if (m_obj_list.empty()) {
    auto obj = new T;
    return obj;
  }

  auto front = m_obj_list.front();
  m_obj_list.erase(m_obj_list.begin());

  return front;
}


template<class T>
void Pool<T>::Put(T* obj) {
  std::unique_lock<std::mutex> locker(m_mutex);

  m_obj_list.push_back(obj);
}

#endif
