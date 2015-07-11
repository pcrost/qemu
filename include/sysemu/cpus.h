#ifndef QEMU_CPUS_H
#define QEMU_CPUS_H

/* cpus.c */
bool qemu_in_vcpu_thread(void);
void qemu_init_cpu_loop(void);
void resume_all_vcpus(void);
void pause_all_vcpus(void);
void cpu_stop_current(void);

void cpu_synchronize_all_states(void);
void cpu_synchronize_all_post_reset(void);
void cpu_synchronize_all_post_init(void);

void qtest_clock_warp(int64_t dest);

#ifndef CONFIG_USER_ONLY
/* vl.c */
extern int smp_cores;
extern int smp_threads;
#else
/* *-user doesn't have configurable SMP topology */
#define smp_cores   1
#define smp_threads 1
#endif

void cpu_list_add(void (*fn)(FILE *, fprintf_function cpu_fprintf));
void list_cpus(FILE *f, fprintf_function cpu_fprintf, const char *optarg);

#define cpu_list_register(fn)                                               \
static __attribute__((constructor)) void register_cpu_list ## fn(void)      \
{                                                                           \
    cpu_list_add(fn);                                                       \
}

#endif
