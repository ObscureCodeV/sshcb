#ifndef THREADS_H
#define THREADS_H

#ifdef _WIN32
  #include <windows.h>
  typedef CRITICAL_SECTION mutex_t;
  static inline void mutex_init(mutex_t *m) { InitializeCriticalSection(m); }
  static inline void mutex_lock(mutex_t *m) { EnterCriticalSection(m); }
  static inline void mutex_unlock(mutex_t *m) { LeaveCriticalSection(m); }
  static inline void mutex_destroy(mutex_t *m) { DeleteCriticalSection(m); }

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
  typedef pthread_mutex_t mutex_t;
  static inline void mutex_init(mutex_t *m) { pthread_mutex_init(m, NULL); }
  static inline void mutex_lock(mutex_t *m) { pthread_mutex_lock(m); }
  static inline void mutex_unlock(mutex_t *m) { pthread_mutex_unlock(m); }
  static inline void mutex_destroy(mutex_t *m) { pthread_mutex_destroy(m); }

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
