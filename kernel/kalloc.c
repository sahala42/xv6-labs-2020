// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];  // 每个CPU分配独立的freelist，多个CPU并发分配物理内存不会相互竞争

char* kmem_lock_names[] = {
  "kmem_cpu_0",
  "kmem_cpu_1",
  "kmem_cpu_2",
  "kmem_cpu_3",
  "kmem_cpu_4",
  "kmem_cpu_5",
  "kmem_cpu_6",
  "kmem_cpu_7",
};

void
kinit()
{
  for (int i=0; i<NCPU; ++i) {
    initlock(&kmem[i].lock, kmem_lock_names[i]);
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();

  int cpu = cpuid(); // 获取cpu编号。中断关闭时调用cpuid才是安全的，所以上面用push_off关闭中断

  acquire(&kmem[cpu].lock);  // 将释放的页插入当前CPU的freelist中
  r->next = kmem[cpu].freelist;
  kmem[cpu].freelist = r;
  release(&kmem[cpu].lock);

  pop_off();  // 重新打开中断
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off(); // 关闭中断

  int cpu = cpuid();

  acquire(&kmem[cpu].lock); // 执行操作时需要获取锁
  
  if (!kmem[cpu].freelist) { // 当前cpu没有freelist的时候，去其他cpu偷取内存页
    int steal_left = 64;     // 这里指定偷64个内存页
    for (int i=0; i<NCPU; ++i) {
      if (i==cpu)
        continue;   // 跳过当前CPU
      
      acquire(&kmem[i].lock);
      if (!kmem[i].freelist) {  // 想偷页的cpu也没有freelist，就需要释放锁跳过
        release(&kmem[i].lock);
        continue;
      }

      struct run* rr = kmem[i].freelist;
      while (rr &&  steal_left) { // 循环将kmem[i]的freelist移动到kmem[cpu]中
        kmem[i].freelist = rr->next; // 目标CPU空闲表头更新为下一节点
        rr->next = kmem[cpu].freelist; // 目标表头指向当前CPU表头
        kmem[cpu].freelist = rr; // 当前CPU表头更新为窃取节点
        rr = kmem[i].freelist;   // 更新rr为目标CPU的新表头
        steal_left--;
      }

      release(&kmem[i].lock);

      if (steal_left == 0) // 偷到指定页数后退出循环
        break;
    }
  }

  r = kmem[cpu].freelist;  // 偷取成功后，更新r为当前CPU的新表头
  if(r)
    kmem[cpu].freelist = r->next; // 偷取成功后，列表头指向下一个可用页面(r->next)
  release(&kmem[cpu].lock);

  pop_off();   // 打开中断

  if (r)
    memset((char*)r, 5, PGSIZE);   // 成功获取页面，用数值5填充整个页面(4KB)
  return (void*)r; // 返回分配的结果
}
