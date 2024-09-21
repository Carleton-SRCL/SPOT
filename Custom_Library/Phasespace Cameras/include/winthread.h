// winthread.h

/***
Copyright (c) PhaseSpace, Inc 2017

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

#ifndef WINTHREAD_H
#define WINTHREAD_H

#include <stdexcept>

#include <windows.h>

//// Mutex ////

class Mutex {

  HANDLE mutex;

public:

  Mutex()
  {
    if((mutex = CreateMutex(0, 0, 0)) == 0) throw std::runtime_error("CreateMutex");
  }

  ~Mutex()
  {
    if(!CloseHandle(mutex)) throw std::runtime_error("CloseHandle");
  }

  bool try_lock()
  {
    int ret = WaitForSingleObject(mutex, 0);
    if(ret == WAIT_OBJECT_0) return true;
    if(ret == WAIT_TIMEOUT) return false;
    throw std::runtime_error("WaitForSingleObject(0)");
  }

  void lock()
  {
    if(WaitForSingleObject(mutex, INFINITE) != WAIT_OBJECT_0) throw std::runtime_error("WaitForSingleObject(INFINITE");
  }

  void unlock()
  {
    if(ReleaseMutex(mutex) != TRUE) throw std::runtime_error("ReleaseMutex");
  }

  void swap(Mutex &m) { HANDLE tmp = mutex; mutex = m.mutex; m.mutex = tmp; }
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

//// Thread ////

class Thread {
public:

  typedef LPTHREAD_START_ROUTINE thread_start;

  HANDLE thread;

  volatile int status;

  Thread() : thread(0), status(0)
  {
  }

  ~Thread()
  {
    if(status) detach();
  }

  bool create(thread_start start, void *arg)
  {
    if((thread = CreateThread(0, 0, start, arg, 0, 0))) { status = 1; return true; }
    return false;
  }

  bool join()
  {
    if(WaitForSingleObject(thread, INFINITE) == WAIT_OBJECT_0) { status = 0; return true; }
    return false;
  }

  bool detach()
  {
    if(CloseHandle(thread) == TRUE) { status = 0; return true; }
    return false;
  }

  bool set_affinity(int n)
  {
    DWORD_PTR mask = n > -1 ? (1 << n) : -1;
    return SetThreadAffinityMask(thread, mask) != 0;
  }

  static HANDLE self() { return GetCurrentThread(); }

  bool start()
  {
    return create((thread_start)Thread::startThread, this);
    return false;
  }

  virtual void run() { }

  static void* startThread(void *arg) { ((Thread*)arg)->run(); return 0; }
};

////

#endif // WINTHREAD_H
