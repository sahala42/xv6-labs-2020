struct stat;
struct rtcdate;

struct sysinfo;         		   // 声明sysinfo结构体，使用户程序可以使用这个结构体


// system calls
int fork(void);                          // 创建一个新进程（子进程），子进程是父进程的副本。返回值：父进程返回子进程的 PID，子进程返回 0
int exit(int) __attribute__((noreturn)); // 终止当前进程，并将状态值传递给父进程。返回值：退出的子进程的 PID
int wait(int*);                          // 等待任意一个子进程退出，并将子进程的退出状态存储在传入的指针中
int pipe(int*);                          // 创建一个管道，用于进程间通信。参数是一个长度为 2 的数组，用于存储管道的读端和写端文件描述符
int write(int, const void*, int);        // 将缓冲区中的数据写入文件描述符。参数是文件描述符、缓冲区和写入的字节数
int read(int, void*, int);               // 从文件描述符中读取数据到缓冲区。参数是文件描述符、缓冲区和读取的字节数
int close(int);                          // 关闭一个文件描述符
int kill(int);                           // 终止指定 PID 的进程
int exec(char*, char**);                 // 加载并执行一个新的程序，替换当前进程的地址空间。参数是程序路径和参数列表
int open(const char*, int);              // 打开一个文件，返回文件描述符（fd）。参数是文件路径和打开模式（如只读、只写等）
int mknod(const char*, short, short);    // 创建一个设备文件。参数是文件路径、主设备号和次设备号
int unlink(const char*);                 // 删除一个文件
int fstat(int fd, struct stat*);         // 获取文件描述符对应的文件状态信息，存储在 struct stat 中
int link(const char*, const char*);      // 创建一个硬链接，将一个文件名链接到另一个文件
int mkdir(const char*);                  // 创建一个目录
int chdir(const char*);                  // 改变当前工作目录
int dup(int);                            // 复制一个文件描述符。
int getpid(void);                        // 获取当前进程的 PID
char* sbrk(int);                         // 增加或减少进程的堆空间。参数是增加的字节数，返回新分配的内存地址
int sleep(int);                          // 让当前进程休眠指定的时间（以时钟滴答为单位）
int uptime(void);                        // 获取系统启动以来的时间（以时钟滴答为单位）
int trace(int);                          // 用户态程序可以找到trace系统调用的跳板入口函数
int sysinfo(struct sysinfo *);           // 用户态程序可以找到sysinfo系统调用的跳板入口函数

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void fprintf(int, const char*, ...);
void printf(const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
int memcmp(const void *, const void *, uint);
void *memcpy(void *, const void *, uint);
