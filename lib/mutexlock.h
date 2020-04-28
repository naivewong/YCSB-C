#ifndef YCSB_C_LIB_MUTEXLOCK_H_
#define YCSB_C_LIB_MUTEXLOCK_H_

#include <pthread.h>
#include <cstring>

static int PthreadCall(const char* label, int result) {
  if (result != 0 && result != ETIMEDOUT) {
    fprintf(stderr, "pthread %s: %s\n", label, strerror(result));
    abort();
  }
  return result;
}

class Mutex {
 public:
  explicit Mutex() {
    PthreadCall("init mutex", pthread_mutex_init(&mu_, nullptr));
  }
  // No copying
  Mutex(const Mutex&) = delete;
  void operator=(const Mutex&) = delete;

  ~Mutex() {
    PthreadCall("destroy mutex", pthread_mutex_destroy(&mu_));
  }

  void Lock() {
    PthreadCall("lock", pthread_mutex_lock(&mu_));
  }
  void Unlock() {
    PthreadCall("unlock", pthread_mutex_unlock(&mu_));
  }
  // this will assert if the mutex is not locked
  // it does NOT verify that mutex is held by a calling thread
  void AssertHeld() {}

 private:
  // friend class CondVar;
  pthread_mutex_t mu_;
};

class MutexLock {
 public:
  explicit MutexLock(Mutex *mu) : mu_(mu) {
    this->mu_->Lock();
  }
  // No copying allowed
  MutexLock(const MutexLock&) = delete;
  void operator=(const MutexLock&) = delete;

  ~MutexLock() { this->mu_->Unlock(); }

 private:
  Mutex *const mu_;
};

// From rocksdb/port/port_posix.h
class RWMutex {
 public:
  RWMutex() {
    PthreadCall("init mutex", pthread_rwlock_init(&mu_, nullptr));
  }
  // No copying allowed
  RWMutex(const RWMutex&) = delete;
  void operator=(const RWMutex&) = delete;

  ~RWMutex() {
    PthreadCall("destroy mutex", pthread_rwlock_destroy(&mu_));
  }

  void ReadLock() {
    PthreadCall("read lock", pthread_rwlock_rdlock(&mu_));
  }
  void WriteLock() {
    PthreadCall("write lock", pthread_rwlock_wrlock(&mu_));
  }
  void ReadUnlock() {
    PthreadCall("read unlock", pthread_rwlock_unlock(&mu_));
  }
  void WriteUnlock() {
    PthreadCall("write unlock", pthread_rwlock_unlock(&mu_));
  }
  void AssertHeld() { }

 private:
  pthread_rwlock_t mu_; // the underlying platform mutex
};

// From rocksdb/util/mutexlock.h
class ReadLock {
 public:
  explicit ReadLock(RWMutex *mu) : mu_(mu) {
    this->mu_->ReadLock();
  }
  // No copying allowed
  ReadLock(const ReadLock&) = delete;
  void operator=(const ReadLock&) = delete;

  ~ReadLock() { this->mu_->ReadUnlock(); }

 private:
  RWMutex *const mu_;
};

class ReadUnlock {
 public:
  explicit ReadUnlock(RWMutex *mu) : mu_(mu) { mu->AssertHeld(); }
  // No copying allowed
  ReadUnlock(const ReadUnlock &) = delete;
  ReadUnlock &operator=(const ReadUnlock &) = delete;

  ~ReadUnlock() { mu_->ReadUnlock(); }

 private:
  RWMutex *const mu_;
};

class WriteLock {
 public:
  explicit WriteLock(RWMutex *mu) : mu_(mu) {
    this->mu_->WriteLock();
  }
  // No copying allowed
  WriteLock(const WriteLock&) = delete;
  void operator=(const WriteLock&) = delete;

  ~WriteLock() { this->mu_->WriteUnlock(); }

 private:
  RWMutex *const mu_;
};

#endif