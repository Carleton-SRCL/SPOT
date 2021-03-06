/***
Copyright (c) PhaseSpace, Inc 2016

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

// thread.h

#ifndef THREAD_H
#define THREAD_H

#include <stdexcept>

#include <errno.h>
#include <pthread.h>

//// Mutex ////

class Mutex {

  pthread_mutex_t mutex;

public:

  Mutex()
  {
    pthread_mutexattr_t attr;
    if(pthread_mutexattr_init(&attr)) throw std::runtime_error("pthread_mutexattr_init");
    if(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) throw std::runtime_error("pthread_mutexattr_settype");
    if(pthread_mutex_init(&mutex, &attr)) throw std::runtime_error("pthread_mutex_init");
    if(pthread_mutexattr_destroy(&attr)) throw std::runtime_error("pthread_mutexattr_destroy");
  }

  ~Mutex()
  {
    if(pthread_mutex_destroy(&mutex)) throw std::runtime_error("pthread_mutex_destroy");
  }

  bool try_lock()
  {
    int ret = pthread_mutex_trylock(&mutex);
    if(ret == 0) return true;
    if(ret == EBUSY) return false;
    throw std::runtime_error("pthread_mutex_trylock");
  }

  void lock()
  {
    if(pthread_mutex_lock(&mutex)) throw std::runtime_error("pthread_mutex_lock");
  }

  void unlock()
  {
    if(pthread_mutex_unlock(&mutex)) throw std::runtime_error("pthread_mutex_unlock");
  }

  void swap(Mutex &m) { pthread_mutex_t tmp = mutex; mutex = m.mutex; m.mutex = tmp; }

  friend class Condition;
};

//// MutexLocker ////

class MutexLocker {

  bool locked;
  Mutex *mutex;

public:

  MutexLocker(Mutex *mutex) : locked(false), mutex(mutex)
  {
    if(mutex) mutex->lock(), locked = true;
  }

  ~MutexLocker()
  {
    if(mutex && locked) mutex->unlock();
  }

  void relock()
  {
    if(mutex && !locked) { mutex->lock(); locked = true; }
  }

  void unlock()
  {
    if(mutex && locked) { mutex->unlock(); locked = false; }
  }
};

//// Condition ////

class Condition {

  pthread_cond_t cond;

public:

  Condition()
  {
    if(pthread_cond_init(&cond, 0)) throw std::runtime_error("pthread_cond_init");
  }

  ~Condition()
  {
    if(pthread_cond_destroy(&cond)) throw std::runtime_error("pthread_cond_destroy");
  }

  void wait(Mutex &mutex)
  {
    if(pthread_cond_wait(&cond, &mutex.mutex)) throw std::runtime_error("pthread_cond_wait");
  }

  bool wait(Mutex &mutex, long timeout) // usec
  {
    timespec ts; clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout / 1000000; ts.tv_nsec += (timeout % 1000000) * 1000;
    if(ts.tv_nsec >= 1000000000) ts.tv_sec++, ts.tv_nsec -= 1000000000;
    int ret = pthread_cond_timedwait(&cond, &mutex.mutex, &ts);
    if(ret == 0) return true;
    if(ret == ETIMEDOUT) return false;
    throw std::runtime_error("pthread_cond_timedwait");
  }

  void signal()
  {
    if(pthread_cond_signal(&cond)) throw std::runtime_error("pthread_cond_signal");
  }

  void broadcast()
  {
    if(pthread_cond_broadcast(&cond)) throw std::runtime_error("pthread_cond_broadcast");
  }
};

//// ReadWriteLock ////

class ReadWriteLock {

  pthread_rwlock_t lock;

public:

  ReadWriteLock()
  {
    if(pthread_rwlock_init(&lock, 0)) throw std::runtime_error("pthread_rwlock_init");
  }

  ~ReadWriteLock()
  {
    if(pthread_rwlock_destroy(&lock)) throw std::runtime_error("pthread_rwlock_destroy");
  }

  void read_lock()
  {
    if(pthread_rwlock_rdlock(&lock)) throw std::runtime_error("pthread_rwlock_rdlock");
  }

  bool try_read_lock()
  {
    int ret = pthread_rwlock_tryrdlock(&lock);
    if(ret == 0) return true;
    if(ret == EBUSY) return false;
    throw std::runtime_error("pthread_rwlock_tryrdlock");
  }

  void write_lock()
  {
    if(pthread_rwlock_wrlock(&lock)) throw std::runtime_error("pthread_rwlock_wrlock");
  }

  bool try_write_lock()
  {
    int ret = pthread_rwlock_trywrlock(&lock);
    if(ret == 0) return true;
    if(ret == EBUSY) return false;
    throw std::runtime_error("pthread_rwlock_trywrlock");
  }

  void unlock()
  {
    if(pthread_rwlock_unlock(&lock)) throw std::runtime_error("pthread_rwlock_unlock");
  }
};

//// Thread ////

class Thread {

  pthread_t thread;

  volatile int status;

public:

  typedef void* (*thread_start)(void*);

  Thread() : thread(0), status(0)
  {
  }

  ~Thread()
  {
    if(status) detach();
  }

  operator int() const { return status; }

  bool create(thread_start start, void *arg)
  {
    if(pthread_create(&thread, 0, start, arg) == 0) { status = 1; return true; }
    return false;
  }

  bool join()
  {
    if(pthread_join(thread, 0) == 0) { status = 0; return true; }
    return false;
  }

  bool detach()
  {
    if(pthread_detach(thread) == 0) { status = 0; return true; }
    return false;
  }

  bool set_affinity(int n)
  {
    cpu_set_t cpu;

    CPU_ZERO(&cpu);

    if(n > -1) CPU_SET(n, &cpu);

    return pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpu) == 0;
  }

  static pthread_t self() { return pthread_self(); }

  bool start() { return create(Thread::start, this); }

  virtual void run() { }

  static void* start(void *arg) { ((Thread*)arg)->run(); return 0; }
};

////

#endif // THREAD_H
