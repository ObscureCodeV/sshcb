#ifndef THREADS_H
#define THREADS_H

#ifdef _WIN32
  #include <windows.h>
  #include <sys/timeb.h>

  typedef CRITICAL_SECTION mutex_t;
  static inline void mutex_init(mutex_t *m) { InitializeCriticalSection(m); }
  static inline void mutex_lock(mutex_t *m) { EnterCriticalSection(m); }
  static inline void mutex_unlock(mutex_t *m) { LeaveCriticalSection(m); }
  static inline void mutex_destroy(mutex_t *m) { DeleteCriticalSection(m); }

  typedef struct {
      CONDITION_VARIABLE cond;
      int waiters_count;
  } cond_t;
  
  static inline void cond_init(cond_t *c) {
      InitializeConditionVariable(&c->cond);
      c->waiters_count = 0;
  }
  
  static inline void cond_signal(cond_t *c) {
      WakeConditionVariable(&c->cond);
  }
  
  static inline void cond_broadcast(cond_t *c) {
      WakeAllConditionVariable(&c->cond);
  }
  
  static inline void cond_wait(cond_t *c, mutex_t *m) {
      c->waiters_count++;
      SleepConditionVariableCS(&c->cond, m, INFINITE);
      c->waiters_count--;
  }
  
  static inline int cond_timedwait(cond_t *c, mutex_t *m, int timeout_ms) {
      c->waiters_count++;
      BOOL result = SleepConditionVariableCS(&c->cond, m, timeout_ms);
      c->waiters_count--;
 //INFO:: 0 = success, -1 = timeout
      return result ? 0 : -1;
  }

  static inline void cond_destroy(cond_t *c) {
//INFO:: Windows: CONDITION_VARIABLE not need free
      (void)c;
  }

  typedef HANDLE thread_t;
  typedef DWORD (WINAPI *thread_func_t)(void*);
  
  static inline int thread_create(thread_t *thread, thread_func_t func, void *arg) {
      *thread = CreateThread(NULL, 0, func, arg, 0, NULL);
      return (*thread == NULL) ? -1 : 0;
  }
  
  static inline int thread_join(thread_t thread) {
      return (WaitForSingleObject(thread, INFINITE) == WAIT_OBJECT_0) ? 0 : -1;
  }
  
  static inline void thread_detach(thread_t thread) {
      CloseHandle(thread);
  }

#else
  #include <pthread.h>
  #include <sys/time.h>
  #include <errno.h>

  typedef pthread_mutex_t mutex_t;
  static inline void mutex_init(mutex_t *m) { pthread_mutex_init(m, NULL); }
  static inline void mutex_lock(mutex_t *m) { pthread_mutex_lock(m); }
  static inline void mutex_unlock(mutex_t *m) { pthread_mutex_unlock(m); }
  static inline void mutex_destroy(mutex_t *m) { pthread_mutex_destroy(m); }

  typedef pthread_cond_t cond_t;
  
  static inline void cond_init(cond_t *c) {
      pthread_cond_init(c, NULL);
  }
  
  static inline void cond_signal(cond_t *c) {
      pthread_cond_signal(c);
  }
  
  static inline void cond_broadcast(cond_t *c) {
      pthread_cond_broadcast(c);
  }
  
  static inline void cond_wait(cond_t *c, mutex_t *m) {
      pthread_cond_wait(c, m);
  }

  static inline int cond_timedwait(cond_t *c, mutex_t *m, int timeout_ms) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
            
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000;
    
    if (ts.tv_nsec >= 1000000000) {
      ts.tv_sec++;
      ts.tv_nsec -= 1000000000;
    }
                                                        
    int rc = pthread_cond_timedwait(c, m, &ts);
    return (rc == ETIMEDOUT) ? -1 : 0;
 }

  static inline void cond_destroy(cond_t *c) {
    pthread_cond_destroy(c);
  }

  typedef pthread_t thread_t;
  typedef void* (*thread_func_t)(void*);
  
  static inline int thread_create(thread_t *thread, thread_func_t func, void *arg) {
      return pthread_create(thread, NULL, func, arg);
  }
  
  static inline int thread_join(thread_t thread) {
      return pthread_join(thread, NULL);
  }
  
  static inline void thread_detach(thread_t thread) {
      pthread_detach(thread);
  }

#endif

#endif
