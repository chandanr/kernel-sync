#ifndef MYSPINLOCK_H
#define MYSPINLOCK_H

#include <linux/atomic.h>
#include <asm/processor.h>

#define LOCKED		(1 << 31)
#define UNLOCKED	(0)

struct myspinlock {
	u32 lock;
};

#define DEFINE_MYSPINLOCK(x)	struct myspinlock x = {.lock = UNLOCKED}

static inline void init_my_spinlock(struct myspinlock *l)
{
	l->lock = UNLOCKED;
}

static inline void lock_my_spinlock(struct myspinlock *l)
{
	u32 ret;

	/* Wait while then lock is held by someone */
	while (l->lock != UNLOCKED)
		cpu_relax();

	/*
	 * We now try to atomically check if the l->lock is UNLOCKED
	 * and if so, LOCK it.
	 */
	while (1) {

		ret = cmpxchg(&l->lock, UNLOCKED, LOCKED);

		if (ret == UNLOCKED)
			break;

		cpu_relax();
	}
}

static inline void unlock_my_spinlock(struct myspinlock *l)
{
	u32 ret;

	WARN_ON(l->lock != LOCKED);
	ret = cmpxchg(&l->lock, LOCKED, UNLOCKED);
}
#endif MYSPINLOCK_H
