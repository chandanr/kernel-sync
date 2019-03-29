#ifndef MYRWLOCK_H
#define MYRWLOCK_H

#include <linux/atomic.h>
#include <asm/processor.h>

#define WRITE_LOCKED	(1 << 31)	/* Writer holds the lock */
#define READ_LOCKED     ~WRITE_LOCKED	/* Some reader(s) hold the lock */
#define UNLOCKED	(0)		/*
					 * Neither readers nor writers
					 * hold the lock
					 */
#define COUNT_MASK      READ_LOCKED	/* Bits corresponding to the count */

struct myrwlock {
	u32 lock_and_count;
};

#define DEFINE_MYRWLOCK(x)	struct myrwlock x = {.lock_and_count = UNLOCKED}

static inline void init_my_rwlock(struct myrwlock *l)
{
	l->lock_and_count = UNLOCKED;
}

static inline void read_lock_my_rwlock(struct myrwlock *l)
{

	/* 1) Wait while then lock is held by a writer
	 *
	 * 2) Try to atomically increment the counter bits by one as
	 * long as the WRITE_LOCKED is not set, using cmpxchg
	 *
	 * Hint: While reading the count value, ignore the
	 *       WRITE_LOCKED bit since we want to atomically update
	 *       the count only if the locked bit is unset.
	 */

}

static inline void read_unlock_my_rwlock(struct myrwlock *l)
{
	/*
	 * 1) Warn if the WRITE_LOCKED bit is set. It is not supposed to
	 *
	 * 2) Atomically decrement the count using cmpxchg
	 */
}

static inline void write_lock_my_rwlock(struct myrwlock *l)
{
	/*
	 * 1) Wait while then lock is held by either a reader or a writer
	 *
	 * 2) We now try to atomically check if the l->lock_and_count
	 * is UNLOCKED and if so, set it to WRITE_LOCKED. Use cmpxchg
	 *
	 * Hint: This part should be similar to lock_my_spinlock()
	 */
}

static inline void write_unlock_my_rwlock(struct myrwlock *l)
{
	/*
	 * 1) Warn if the state is not WRITE_LOCKED.
	 *
	 * 2) We now try to atomically check if the l->lock_and_count
	 * is WRITE_LOCKED and if so, set it to UNLOCKED. Use cmpxchg
	 *
	 * Hint: This part should be similar to unlock_my_spinlock()
	 */
}
#endif MYRWLOCK_H
