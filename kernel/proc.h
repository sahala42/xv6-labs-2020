// Saved registers for kernel context switches.
// 用于内核上下文切换的保存的寄存器
struct context {
  uint64 ra;
  uint64 sp;

  // callee-saved
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

// Per-CPU state.
struct cpu {
  struct proc *proc;          // The process running on this cpu, or null.
  struct context context;     // swtch() here to enter scheduler().
  int noff;                   // Depth of push_off() nesting.
  int intena;                 // Were interrupts enabled before push_off()?
};

extern struct cpu cpus[NCPU];

// per-process data for the trap handling code in trampoline.S.
// sits in a page by itself just under the trampoline page in the
// user page table. not specially mapped in the kernel page table.
// the sscratch register points here.
// uservec in trampoline.S saves user registers in the trapframe,
// then initializes registers from the trapframe's
// kernel_sp, kernel_hartid, kernel_satp, and jumps to kernel_trap.
// usertrapret() and userret in trampoline.S set up
// the trapframe's kernel_*, restore user registers from the
// trapframe, switch to the user page table, and enter user space.
// the trapframe includes callee-saved user registers like s0-s11 because the
// return-to-user path via usertrapret() doesn't return through
// the entire kernel call stack.
struct trapframe {
  /*   0 */ uint64 kernel_satp;   // kernel page table
  /*   8 */ uint64 kernel_sp;     // top of process's kernel stack
  /*  16 */ uint64 kernel_trap;   // usertrap()
  /*  24 */ uint64 epc;           // saved user program counter
  /*  32 */ uint64 kernel_hartid; // saved kernel tp
  /*  40 */ uint64 ra;
  /*  48 */ uint64 sp;
  /*  56 */ uint64 gp;
  /*  64 */ uint64 tp;
  /*  72 */ uint64 t0;
  /*  80 */ uint64 t1;
  /*  88 */ uint64 t2;
  /*  96 */ uint64 s0;
  /* 104 */ uint64 s1;
  /* 112 */ uint64 a0;
  /* 120 */ uint64 a1;
  /* 128 */ uint64 a2;
  /* 136 */ uint64 a3;
  /* 144 */ uint64 a4;
  /* 152 */ uint64 a5;
  /* 160 */ uint64 a6;
  /* 168 */ uint64 a7;
  /* 176 */ uint64 s2;
  /* 184 */ uint64 s3;
  /* 192 */ uint64 s4;
  /* 200 */ uint64 s5;
  /* 208 */ uint64 s6;
  /* 216 */ uint64 s7;
  /* 224 */ uint64 s8;
  /* 232 */ uint64 s9;
  /* 240 */ uint64 s10;
  /* 248 */ uint64 s11;
  /* 256 */ uint64 t3;
  /* 264 */ uint64 t4;
  /* 272 */ uint64 t5;
  /* 280 */ uint64 t6;
};

// 进程状态：未使用，正在睡眠，可运行，运行中，已终止
enum procstate { UNUSED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state
struct proc {
  struct spinlock lock;  // 自旋锁，用于保护进程结构体中的共享数据

  // p->lock must be held when using these:
  // 以下字段在访问或修改时，必须持有 p->lock，以确保并发安全
  enum procstate state;        // Process state 一个枚举类型
  struct proc *parent;         // Parent process  指向父进程指针
  void *chan;                  // If non-zero, sleeping on chan 如果非零，正在某个通道上睡眠
  int killed;                  // If non-zero, have been killed 
  int xstate;                  // Exit status to be returned to parent's wait  进程的退出状态码
  int pid;                     // Process ID  进程的唯一标识

  // these are private to the process, so p->lock need not be held.
  // 以下字段是进程私有的，访问或修改时不需要持有 p->lock
  uint64 kstack;               // Virtual address of kernel stack  内核栈的虚拟地址
  uint64 sz;                   // Size of process memory (bytes)   进程内存的大小（字节）
  pagetable_t pagetable;       // User page table                  用户页表的指针，页表用于将虚拟地址映射到物理地址
  struct trapframe *trapframe; // data page for trampoline.S       指向trapfram的指针
  struct context context;      // swtch() here to run process      当进程切换时，当前 CPU 的寄存器状态会被保存到 context 中，以便稍后恢复
  struct file *ofile[NOFILE];  // Open files                       进程打开的文件表  NOFILE 是常量：一个进程最多可以打开的文件数
  struct inode *cwd;           // Current directory                指向当前工作目录的 inode 是文件系统中表示文件或目录的数据结构。
  char name[16];               // Process name (debugging)         进程的名称
  pagetable_t zst_kernelpgtbl; // 存储进程独享的内核态页表
};
