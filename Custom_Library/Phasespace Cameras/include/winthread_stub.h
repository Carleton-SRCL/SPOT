// winthread_stub.h
// noop stub for winthread.h

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

#include <windows.h>

//// Mutex ////

class Mutex {
public:

  Mutex() { }

  ~Mutex() { }

  bool try_lock() { return true; }
  void lock() { }
  void unlock() { }
  void swap(Mutex &) { }
};

//// MutexLocker ////

class MutexLocker {
public:

  MutexLocker(Mutex *) { }
  ~MutexLocker() { }

  void relock() { }
  void unlock() { }
};

//// Thread ////

class Thread {
public:

  typedef LPTHREAD_START_ROUTINE thread_start;

  Thread() { }

  ~Thread() { }

  bool create(thread_start, void *) { return true; }
  bool join() { return true; }
  bool detach() { return true; }

  bool set_affinity(int) { return true; }
  static HANDLE self() { return GetCurrentThread(); }

  bool start() { return true; }
  virtual void run() { }

  static void* startThread(void *) { return 0; }
};

////

#endif // WINTHREAD_H
