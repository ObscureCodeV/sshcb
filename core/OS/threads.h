#ifndef THREADS_H
#define THREADS_H

#ifdef _WIN32
  #include <windows.h>
  typedef CRITICAL_SECTION mutex_t;
  static inline void mutex_init(mutex_t *m) { InitializeCriticalSection(m); }
  static inline void mutex_lock(mutex_t *m) { EnterCriticalSection(m); }
  static inline void mutex_unlock(mutex_t *m) { LeaveCriticalSection(m); }
  static inline void mutex_destroy(mutex_t *m) { DeleteCriticalSection(m); }

#else
  #include <pthread.h>
  typedef pthread_mutex_t mutex_t;
  static inline void mutex_init(mutex_t *m) { pthread_mutex_init(m, NULL); }
  static inline void mutex_lock(mutex_t *m) { pthread_mutex_lock(m); }
  static inline void mutex_unlock(mutex_t *m) { pthread_mutex_unlock(m); }
  static inline void mutex_destroy(mutex_t *m) { pthread_mutex_destroy(m); }
#endif

#endif
