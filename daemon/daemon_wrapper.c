#include "daemon_wrapper.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

static volatile int g_running = 1;

int daemon_is_running(void) {
  return g_running;
}

#ifdef _WIN32
  #include <windows.h>
  #include <io.h>

  static SERVICE_STATUS_HANDLE g_status_handle;
  static SERVICE_STATUS g_status;
  static int (*g_main_func)(void) = NULL;
  static FILE *g_log_file = NULL;
  
  void redirect_output_to_log(void) {
    char log_path[MAX_PATH];
    char program_data[MAX_PATH];
    
    if (GetEnvironmentVariable("PROGRAMDATA", program_data, MAX_PATH) == 0) {
      GetEnvironmentVariable("LOCALAPPDATA", program_data, MAX_PATH);
    }
    
    snprintf(log_path, sizeof(log_path), "%s\\SSHCB", program_data);
    CreateDirectory(log_path, NULL);
    
    snprintf(log_path, sizeof(log_path), "%s\\SSHCB\\sshcb.log", program_data);
    
    g_log_file = fopen(log_path, "a");
    if (g_log_file) {
      dup2(fileno(g_log_file), STDOUT_FILENO);
      dup2(fileno(g_log_file), STDERR_FILENO);
      setvbuf(stderr, NULL, _IONBF, 0);
      setvbuf(stdout, NULL, _IONBF, 0);
    }
    
    fprintf(stderr, "\n=== SSHCB Service Started at %s ===\n", 
            __TIMESTAMP__);
  }
  
  void WINAPI service_ctrl_handler(DWORD ctrl_code) {
    switch (ctrl_code) {
      case SERVICE_CONTROL_STOP:
        g_status.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(g_status_handle, &g_status);
        
        fprintf(stderr, "SSHCB Service stopping...\n");
        g_running = 0;
        break;
      case SERVICE_CONTROL_INTERROGATE:
        SetServiceStatus(g_status_handle, &g_status);
        break;
    }
  }
  
  void WINAPI service_main(DWORD argc, LPSTR *argv) {
    g_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_status.dwCurrentState = SERVICE_START_PENDING;
    g_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_status.dwWin32ExitCode = NO_ERROR;
    
    g_status_handle = RegisterServiceCtrlHandlerEx("SSHCB", service_ctrl_handler, NULL);
    if (!g_status_handle) return;
    
    g_status.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(g_status_handle, &g_status);
    
    redirect_output_to_log();
    
    fprintf(stderr, "SSHCB Service started successfully\n");
    
    int rc = g_main_func();
    
    fprintf(stderr, "SSHCB Service main function returned with code %d\n", rc);
    
    if (g_log_file) {
      fclose(g_log_file);
    }
    
    g_status.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_status_handle, &g_status);
  }
  
  int daemon_run(int (*main_func)(void)) {
    g_main_func = main_func;
    
    SERVICE_TABLE_ENTRY service_table[] = {
        {"SSHCB", service_main},
        {NULL, NULL}
    };
    
    return StartServiceCtrlDispatcher(service_table) ? 0 : -1;
  }  
    
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <signal.h>
  
  static int (*g_main_func)(void) = NULL;
  
  void signal_handler(int sig) {
    if (sig == SIGTERM) {
      g_running = 0;
    }
  }
    
  void daemonize(void) {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    
    setsid();
    umask(0);
    chdir("/");
    
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int log_fd = open("/var/log/sshcb.log",
      O_WRONLY | O_CREAT | O_APPEND,
      0644);

    //stdout, stdoerr to log
    dup2(log_fd, STDOUT_FILENO);
    dup2(log_fd, STDERR_FILENO);
    close(log_fd);
      
    // stdin to /dev/null
    open("/dev/null", O_RDONLY);
  }
    
  int daemon_run(int (*main_func)(void)) {
      g_main_func = main_func;
      
      daemonize();
      
      signal(SIGINT, SIG_IGN);
      signal(SIGTERM, signal_handler);
      signal(SIGHUP, SIG_IGN);

      int rc = g_main_func();
      
      return rc;
  }  
#endif
