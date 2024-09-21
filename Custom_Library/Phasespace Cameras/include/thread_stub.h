// thread_stub.h
// noop stub for thread.h

/***
Copyright (c) PhaseSpace, Inc 2017

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>

//// Mutex ////

class Mutex {
public:

  Mutex() { }
  ~Mutex() { }

  bool try_lock() { return true; }
  void lock() { }
  void unlock() { }
  void swap(Mutex &) { }

  friend class Condition;
};

//// MutexLocker ////

class MutexLocker {
public:

  MutexLocker(Mutex *) { }
  ~MutexLocker() { }

  void relock() { }
  void unlock() { }
};

//// Condition ////

class Condition {
public:

  Condition() { }
  ~Condition() { }

  void wait(Mutex &) { }
  bool wait(Mutex &, long long) { return true; }
  void signal() { }
  void broadcast() { }
};

//// ReadWriteLock ////

class ReadWriteLock {
public:

  ReadWriteLock() { }
  ~ReadWriteLock() { }

  void read_lock() { }
  bool try_read_lock() { return true; }
  void write_lock() { }
  bool try_write_lock() { return true; }
  void unlock() { }
};

//// ReadLocker ////

class ReadLocker {
public:

  ReadLocker(ReadWriteLock *) { }
  ~ReadLocker() { }

  void relock() { }
  void unlock() { }
};

//// WriteLocker ////

class WriteLocker {
public:

  WriteLocker(ReadWriteLock *) { }
  ~WriteLocker() { }

  void relock() { }
  void unlock() { }
};

//// Thread ////

class Thread {
public:

  typedef void* (*thread_start)(void*);

  Thread() { }
  ~Thread() { }

  operator int() const { return 0; }

  bool create(thread_start, void *) { return true; }
  bool join() { return true; }
  bool detach() { return true; }
  bool set_affinity(int) { return true; }
  static pthread_t self() { return pthread_self(); }
  bool start() { return true; }
  virtual void run() { }
  static void* start(void *) { return 0; }
};

////

#endif // THREAD_H
