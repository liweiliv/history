#ifndef _ASM_X86_ATOMIC_H
#define _ASM_X86_ATOMIC_H
#include "cmpxchg.h"
#ifndef asm
#define asm __asm__
#endif
#ifndef volatile
#define volatile __volatile__
#endif
/*
 * Atomic operations that C can't guarantee us.  Useful for
 * resource counting etc..
 */
#define LOCK_PREFIX_HERE \
		".pushsection .smp_locks,\"a\"\n"	\
		".balign 4\n"				\
		".long 671f - .\n" /* offset */		\
		".popsection\n"				\
		"671:"

#define LOCK_PREFIX LOCK_PREFIX_HERE "\n\tlock; "

#define ATOMIC_INIT(i)	{ (i) }
typedef struct {
	int counter;
} atomic_t;

#ifdef __x86_64__
typedef struct {
	long counter;
} atomic64_t;
#endif
#define __GEN_RMWcc(fullop, var, cc, ...)				\
do {									\
	char c;								\
	asm volatile (fullop "; set" cc " %1"				\
			: "+m" (var), "=qm" (c)				\
			: __VA_ARGS__ : "memory");			\
	return c != 0;							\
} while (0)

#define GEN_UNARY_RMWcc(op, var, arg0, cc)				\
	__GEN_RMWcc(op " " arg0, var, cc)

#define GEN_BINARY_RMWcc(op, var, vcon, val, arg0, cc)			\
	__GEN_RMWcc(op " %2, " arg0, var, cc, vcon (val))

#ifndef likely
# define likely(x)  __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
# define unlikely(x)    __builtin_expect(!!(x), 0)
#endif
/**
 * atomic_read - read atomic variable
 * @v: pointer of type atomic_t
 *
 * Atomically reads the value of @v.
 */
static inline int atomic_read(const atomic_t *v)
{
	return (*(volatile int *)&(v)->counter);
}

/**
 * atomic_set - set atomic variable
 * @v: pointer of type atomic_t
 * @i: required value
 *
 * Atomically sets the value of @v to @i.
 */
static inline void atomic_set(atomic_t *v, int i)
{
	v->counter = i;
}

/**
 * atomic_add - add integer to atomic variable
 * @i: integer value to add
 * @v: pointer of type atomic_t
 *
 * Atomically adds @i to @v.
 */
static inline void atomic_add(int i, atomic_t *v)
{
	asm volatile(LOCK_PREFIX "addl %1,%0"
		     : "+m" (v->counter)
		     : "ir" (i));
}

/**
 * atomic_sub - subtract integer from atomic variable
 * @i: integer value to subtract
 * @v: pointer of type atomic_t
 *
 * Atomically subtracts @i from @v.
 */
static inline void atomic_sub(int i, atomic_t *v)
{
	asm volatile(LOCK_PREFIX "subl %1,%0"
		     : "+m" (v->counter)
		     : "ir" (i));
}

/**
 * atomic_sub_and_test - subtract value from variable and test result
 * @i: integer value to subtract
 * @v: pointer of type atomic_t
 *
 * Atomically subtracts @i from @v and returns
 * true if the result is zero, or false for all
 * other cases.

static inline int atomic_sub_and_test(int i, atomic_t *v)
{
	GEN_BINARY_RMWcc(LOCK_PREFIX "subl", v->counter, "er", i, "%0", "e");
}
 */
/**
 * atomic_inc - increment atomic variable
 * @v: pointer of type atomic_t
 *
 * Atomically increments @v by 1.
 */
static inline void atomic_inc(atomic_t *v)
{
	asm volatile(LOCK_PREFIX "incl %0"
		     : "+m" (v->counter));
}

/**
 * atomic_dec - decrement atomic variable
 * @v: pointer of type atomic_t
 *
 * Atomically decrements @v by 1.
 */
static inline void atomic_dec(atomic_t *v)
{
	asm volatile(LOCK_PREFIX "decl %0"
		     : "+m" (v->counter));
}

/**
 * atomic_dec_and_test - decrement and test
 * @v: pointer of type atomic_t
 *
 * Atomically decrements @v by 1 and
 * returns true if the result is 0, or false for all other
 * cases.

static inline int atomic_dec_and_test(atomic_t *v)
{
	GEN_UNARY_RMWcc(LOCK_PREFIX "decl", v->counter, "%0", "e");
} */

/**
 * atomic_inc_and_test - increment and test
 * @v: pointer of type atomic_t
 *
 * Atomically increments @v by 1
 * and returns true if the result is zero, or false for all
 * other cases.

static inline int atomic_inc_and_test(atomic_t *v)
{
	GEN_UNARY_RMWcc(LOCK_PREFIX "incl", v->counter, "%0", "e");
}
 */
/**
 * atomic_add_negative - add and test if negative
 * @i: integer value to add
 * @v: pointer of type atomic_t
 *
 * Atomically adds @i to @v and returns true
 * if the result is negative, or false when
 * result is greater than or equal to zero.

static inline int atomic_add_negative(int i, atomic_t *v)
{
	GEN_BINARY_RMWcc(LOCK_PREFIX "addl", v->counter, "er", i, "%0", "s");
}
*/
/**
 * atomic_add_return - add integer and return
 * @i: integer value to add
 * @v: pointer of type atomic_t
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline int atomic_add_return(int i, atomic_t *v)
{
	return i + xadd(&v->counter, i);
}

/**
 * atomic_sub_return - subtract integer and return
 * @v: pointer of type atomic_t
 * @i: integer value to subtract
 *
 * Atomically subtracts @i from @v and returns @v - @i
 */
static inline int atomic_sub_return(int i, atomic_t *v)
{
	return atomic_add_return(-i, v);
}

#define atomic_inc_return(v)  (atomic_add_return(1, v))
#define atomic_dec_return(v)  (atomic_sub_return(1, v))

static inline int atomic_cmpxchg(atomic_t *v, int old, int _new)
{
	return cmpxchg(&v->counter, old, _new);
}

static inline int atomic_xchg(atomic_t *v, int _new)
{
	return xchg(&v->counter, _new);
}

/**
 * __atomic_add_unless - add unless the number is already a given value
 * @v: pointer of type atomic_t
 * @a: the amount to add to v...
 * @u: ...unless v is equal to u.
 *
 * Atomically adds @a to @v, so long as @v was not already @u.
 * Returns the old value of @v.
 */
static inline int __atomic_add_unless(atomic_t *v, int a, int u)
{
	int c, old;
	c = atomic_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic_cmpxchg((v), c, c + (a));
		if (likely(old == c))
			break;
		c = old;
	}
	return c;
}

/**
 * atomic_inc_short - increment of a short integer
 * @v: pointer to type int
 *
 * Atomically adds 1 to @v
 * Returns the new value of @u
 */
static inline short int atomic_inc_short(short int *v)
{
	asm(LOCK_PREFIX "addw $1, %0" : "+m" (*v));
	return *v;
}

#ifdef __x86_64__

/**
 * atomic_or_long - OR of two long integers
 * @v1: pointer to type unsigned long
 * @v2: pointer to type unsigned long
 *
 * Atomically ORs @v1 and @v2
 * Returns the result of the OR
 */
static inline void atomic_or_long(unsigned long *v1, unsigned long v2)
{
	asm(LOCK_PREFIX "orq %1, %0" : "+m" (*v1) : "r" (v2));
}
#endif

/* These are x86-specific, used by some header files */
#define atomic_clear_mask(mask, addr)				\
	asm volatile(LOCK_PREFIX "andl %0,%1"			\
		     : : "r" (~(mask)), "m" (*(addr)) : "memory")

#define atomic_set_mask(mask, addr)				\
	asm volatile(LOCK_PREFIX "orl %0,%1"			\
		     : : "r" ((unsigned)(mask)), "m" (*(addr))	\
		     : "memory")

/* Atomic operations are already serializing on x86 */
#define smp_mb__before_atomic_dec()	barrier()
#define smp_mb__after_atomic_dec()	barrier()
#define smp_mb__before_atomic_inc()	barrier()
#define smp_mb__after_atomic_inc()	barrier()

#ifdef __x86_64__
#ifndef _ASM_X86_ATOMIC64_64_H
#define _ASM_X86_ATOMIC64_64_H


/* The 64-bit atomic type */

#define ATOMIC64_INIT(i)	{ (i) }

/**
 * atomic64_read - read atomic64 variable
 * @v: pointer of type atomic64_t
 *
 * Atomically reads the value of @v.
 * Doesn't imply a read memory barrier.
 */
static inline long atomic64_read(const atomic64_t *v)
{
	return (*(volatile long *)&(v)->counter);
}

/**
 * atomic64_set - set atomic64 variable
 * @v: pointer to type atomic64_t
 * @i: required value
 *
 * Atomically sets the value of @v to @i.
 */
static inline void atomic64_set(atomic64_t *v, long i)
{
	v->counter = i;
}

/**
 * atomic64_add - add integer to atomic64 variable
 * @i: integer value to add
 * @v: pointer to type atomic64_t
 *
 * Atomically adds @i to @v.
 */
static inline void atomic64_add(long i, atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "addq %1,%0"
		     : "=m" (v->counter)
		     : "er" (i), "m" (v->counter));
}

/**
 * atomic64_sub - subtract the atomic64 variable
 * @i: integer value to subtract
 * @v: pointer to type atomic64_t
 *
 * Atomically subtracts @i from @v.
 */
static inline void atomic64_sub(long i, atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "subq %1,%0"
		     : "=m" (v->counter)
		     : "er" (i), "m" (v->counter));
}

/**
 * atomic64_sub_and_test - subtract value from variable and test result
 * @i: integer value to subtract
 * @v: pointer to type atomic64_t
 *
 * Atomically subtracts @i from @v and returns
 * true if the result is zero, or false for all
 * other cases.
 */
static inline int atomic64_sub_and_test(long i, atomic64_t *v)
{
	GEN_BINARY_RMWcc(LOCK_PREFIX "subq", v->counter, "er", i, "%0", "e");
}

/**
 * atomic64_inc - increment atomic64 variable
 * @v: pointer to type atomic64_t
 *
 * Atomically increments @v by 1.
 */
static inline void atomic64_inc(atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "incq %0"
		     : "=m" (v->counter)
		     : "m" (v->counter));
}

/**
 * atomic64_dec - decrement atomic64 variable
 * @v: pointer to type atomic64_t
 *
 * Atomically decrements @v by 1.
 */
static inline void atomic64_dec(atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "decq %0"
		     : "=m" (v->counter)
		     : "m" (v->counter));
}

/**
 * atomic64_dec_and_test - decrement and test
 * @v: pointer to type atomic64_t
 *
 * Atomically decrements @v by 1 and
 * returns true if the result is 0, or false for all other
 * cases.
 */
static inline int atomic64_dec_and_test(atomic64_t *v)
{
	GEN_UNARY_RMWcc(LOCK_PREFIX "decq", v->counter, "%0", "e");
}

/**
 * atomic64_inc_and_test - increment and test
 * @v: pointer to type atomic64_t
 *
 * Atomically increments @v by 1
 * and returns true if the result is zero, or false for all
 * other cases.
 */
static inline int atomic64_inc_and_test(atomic64_t *v)
{
	GEN_UNARY_RMWcc(LOCK_PREFIX "incq", v->counter, "%0", "e");
}

/**
 * atomic64_add_negative - add and test if negative
 * @i: integer value to add
 * @v: pointer to type atomic64_t
 *
 * Atomically adds @i to @v and returns true
 * if the result is negative, or false when
 * result is greater than or equal to zero.
 */
static inline int atomic64_add_negative(long i, atomic64_t *v)
{
	GEN_BINARY_RMWcc(LOCK_PREFIX "addq", v->counter, "er", i, "%0", "s");
}

/**
 * atomic64_add_return - add and return
 * @i: integer value to add
 * @v: pointer to type atomic64_t
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static inline long atomic64_add_return(long i, atomic64_t *v)
{
	return i + xadd(&v->counter, i);
}

static inline long atomic64_sub_return(long i, atomic64_t *v)
{
	return atomic64_add_return(-i, v);
}

#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long _old, long _new)
{
	return cmpxchg(&v->counter, _old, _new);
}

static inline long atomic64_xchg(atomic64_t *v, long _new)
{
	return xchg(&v->counter, _new);
}

/**
 * atomic64_add_unless - add unless the number is a given value
 * @v: pointer of type atomic64_t
 * @a: the amount to add to v...
 * @u: ...unless v is equal to u.
 *
 * Atomically adds @a to @v, so long as it was not @u.
 * Returns the old value of @v.
 */
static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
			break;
		c = old;
	}
	return c != (u);
}

#define atomic64_inc_not_zero(v) atomic64_add_unless((v), 1, 0)

/*
 * atomic64_dec_if_positive - decrement by 1 if old value positive
 * @v: pointer of type atomic_t
 *
 * The function returns the old value of *v minus 1, even if
 * the atomic variable, v, was not decremented.
 */
static inline long atomic64_dec_if_positive(atomic64_t *v)
{
	long c, old, dec;
	c = atomic64_read(v);
	for (;;) {
		dec = c - 1;
		if (unlikely(dec < 0))
			break;
		old = atomic64_cmpxchg((v), c, dec);
		if (likely(old == c))
			break;
		c = old;
	}
	return dec;
}

#endif /* _ASM_X86_ATOMIC64_64_H */

#else
#ifndef _ASM_X86_ATOMIC64_32_H
#define _ASM_X86_ATOMIC64_32_H

#include <linux/compiler.h>
#include <linux/types.h>
#include <asm/processor.h>
//#include <asm/cmpxchg.h>

/* An 64bit atomic type */

typedef struct {
	u64 __aligned(8) counter;
} atomic64_t;

#define ATOMIC64_INIT(val)	{ (val) }

#define __ATOMIC64_DECL(sym) void atomic64_##sym(atomic64_t *, ...)
#ifndef ATOMIC64_EXPORT
#define ATOMIC64_DECL_ONE __ATOMIC64_DECL
#else
#define ATOMIC64_DECL_ONE(sym) __ATOMIC64_DECL(sym); \
	ATOMIC64_EXPORT(atomic64_##sym)
#endif

#ifdef CONFIG_X86_CMPXCHG64
#define __alternative_atomic64(f, g, out, in...) \
	asm volatile("call %P[func]" \
		     : out : [func] "i" (atomic64_##g##_cx8), ## in)

#define ATOMIC64_DECL(sym) ATOMIC64_DECL_ONE(sym##_cx8)
#else
#define __alternative_atomic64(f, g, out, in...) \
	alternative_call(atomic64_##f##_386, atomic64_##g##_cx8, \
			 X86_FEATURE_CX8, ASM_OUTPUT2(out), ## in)

#define ATOMIC64_DECL(sym) ATOMIC64_DECL_ONE(sym##_cx8); \
	ATOMIC64_DECL_ONE(sym##_386)

ATOMIC64_DECL_ONE(add_386);
ATOMIC64_DECL_ONE(sub_386);
ATOMIC64_DECL_ONE(inc_386);
ATOMIC64_DECL_ONE(dec_386);
#endif

#define alternative_atomic64(f, out, in...) \
	__alternative_atomic64(f, f, ASM_OUTPUT2(out), ## in)

ATOMIC64_DECL(read);
ATOMIC64_DECL(set);
ATOMIC64_DECL(xchg);
ATOMIC64_DECL(add_return);
ATOMIC64_DECL(sub_return);
ATOMIC64_DECL(inc_return);
ATOMIC64_DECL(dec_return);
ATOMIC64_DECL(dec_if_positive);
ATOMIC64_DECL(inc_not_zero);
ATOMIC64_DECL(add_unless);

#undef ATOMIC64_DECL
#undef ATOMIC64_DECL_ONE
#undef __ATOMIC64_DECL
#undef ATOMIC64_EXPORT

/**
 * atomic64_cmpxchg - cmpxchg atomic64 variable
 * @v: pointer to type atomic64_t
 * @o: expected value
 * @n: new value
 *
 * Atomically sets @v to @n if it was equal to @o and returns
 * the old value.
 */

static inline long long atomic64_cmpxchg(atomic64_t *v, long long o, long long n)
{
	return cmpxchg64(&v->counter, o, n);
}

/**
 * atomic64_xchg - xchg atomic64 variable
 * @v: pointer to type atomic64_t
 * @n: value to assign
 *
 * Atomically xchgs the value of @v to @n and returns
 * the old value.
 */
static inline long long atomic64_xchg(atomic64_t *v, long long n)
{
	long long o;
	unsigned high = (unsigned)(n >> 32);
	unsigned low = (unsigned)n;
	alternative_atomic64(xchg, "=&A" (o),
			     "S" (v), "b" (low), "c" (high)
			     : "memory");
	return o;
}

/**
 * atomic64_set - set atomic64 variable
 * @v: pointer to type atomic64_t
 * @i: value to assign
 *
 * Atomically sets the value of @v to @n.
 */
static inline void atomic64_set(atomic64_t *v, long long i)
{
	unsigned high = (unsigned)(i >> 32);
	unsigned low = (unsigned)i;
	alternative_atomic64(set, /* no output */,
			     "S" (v), "b" (low), "c" (high)
			     : "eax", "edx", "memory");
}

/**
 * atomic64_read - read atomic64 variable
 * @v: pointer to type atomic64_t
 *
 * Atomically reads the value of @v and returns it.
 */
static inline long long atomic64_read(const atomic64_t *v)
{
	long long r;
	alternative_atomic64(read, "=&A" (r), "c" (v) : "memory");
	return r;
 }

/**
 * atomic64_add_return - add and return
 * @i: integer value to add
 * @v: pointer to type atomic64_t
 *
 * Atomically adds @i to @v and returns @i + *@v
 */
static inline long long atomic64_add_return(long long i, atomic64_t *v)
{
	alternative_atomic64(add_return,
			     ASM_OUTPUT2("+A" (i), "+c" (v)),
			     ASM_NO_INPUT_CLOBBER("memory"));
	return i;
}

/*
 * Other variants with different arithmetic operators:
 */
static inline long long atomic64_sub_return(long long i, atomic64_t *v)
{
	alternative_atomic64(sub_return,
			     ASM_OUTPUT2("+A" (i), "+c" (v)),
			     ASM_NO_INPUT_CLOBBER("memory"));
	return i;
}

static inline long long atomic64_inc_return(atomic64_t *v)
{
	long long a;
	alternative_atomic64(inc_return, "=&A" (a),
			     "S" (v) : "memory", "ecx");
	return a;
}

static inline long long atomic64_dec_return(atomic64_t *v)
{
	long long a;
	alternative_atomic64(dec_return, "=&A" (a),
			     "S" (v) : "memory", "ecx");
	return a;
}

/**
 * atomic64_add - add integer to atomic64 variable
 * @i: integer value to add
 * @v: pointer to type atomic64_t
 *
 * Atomically adds @i to @v.
 */
static inline long long atomic64_add(long long i, atomic64_t *v)
{
	__alternative_atomic64(add, add_return,
			       ASM_OUTPUT2("+A" (i), "+c" (v)),
			       ASM_NO_INPUT_CLOBBER("memory"));
	return i;
}

/**
 * atomic64_sub - subtract the atomic64 variable
 * @i: integer value to subtract
 * @v: pointer to type atomic64_t
 *
 * Atomically subtracts @i from @v.
 */
static inline long long atomic64_sub(long long i, atomic64_t *v)
{
	__alternative_atomic64(sub, sub_return,
			       ASM_OUTPUT2("+A" (i), "+c" (v)),
			       ASM_NO_INPUT_CLOBBER("memory"));
	return i;
}

/**
 * atomic64_sub_and_test - subtract value from variable and test result
 * @i: integer value to subtract
 * @v: pointer to type atomic64_t
 *
 * Atomically subtracts @i from @v and returns
 * true if the result is zero, or false for all
 * other cases.
 */
static inline int atomic64_sub_and_test(long long i, atomic64_t *v)
{
	return atomic64_sub_return(i, v) == 0;
}

/**
 * atomic64_inc - increment atomic64 variable
 * @v: pointer to type atomic64_t
 *
 * Atomically increments @v by 1.
 */
static inline void atomic64_inc(atomic64_t *v)
{
	__alternative_atomic64(inc, inc_return, /* no output */,
			       "S" (v) : "memory", "eax", "ecx", "edx");
}

/**
 * atomic64_dec - decrement atomic64 variable
 * @v: pointer to type atomic64_t
 *
 * Atomically decrements @v by 1.
 */
static inline void atomic64_dec(atomic64_t *v)
{
	__alternative_atomic64(dec, dec_return, /* no output */,
			       "S" (v) : "memory", "eax", "ecx", "edx");
}

/**
 * atomic64_dec_and_test - decrement and test
 * @v: pointer to type atomic64_t
 *
 * Atomically decrements @v by 1 and
 * returns true if the result is 0, or false for all other
 * cases.
 */
static inline int atomic64_dec_and_test(atomic64_t *v)
{
	return atomic64_dec_return(v) == 0;
}

/**
 * atomic64_inc_and_test - increment and test
 * @v: pointer to type atomic64_t
 *
 * Atomically increments @v by 1
 * and returns true if the result is zero, or false for all
 * other cases.
 */
static inline int atomic64_inc_and_test(atomic64_t *v)
{
	return atomic64_inc_return(v) == 0;
}

/**
 * atomic64_add_negative - add and test if negative
 * @i: integer value to add
 * @v: pointer to type atomic64_t
 *
 * Atomically adds @i to @v and returns true
 * if the result is negative, or false when
 * result is greater than or equal to zero.
 */
static inline int atomic64_add_negative(long long i, atomic64_t *v)
{
	return atomic64_add_return(i, v) < 0;
}

/**
 * atomic64_add_unless - add unless the number is a given value
 * @v: pointer of type atomic64_t
 * @a: the amount to add to v...
 * @u: ...unless v is equal to u.
 *
 * Atomically adds @a to @v, so long as it was not @u.
 * Returns non-zero if the add was done, zero otherwise.
 */
static inline int atomic64_add_unless(atomic64_t *v, long long a, long long u)
{
	unsigned low = (unsigned)u;
	unsigned high = (unsigned)(u >> 32);
	alternative_atomic64(add_unless,
			     ASM_OUTPUT2("+A" (a), "+c" (low), "+D" (high)),
			     "S" (v) : "memory");
	return (int)a;
}


static inline int atomic64_inc_not_zero(atomic64_t *v)
{
	int r;
	alternative_atomic64(inc_not_zero, "=&a" (r),
			     "S" (v) : "ecx", "edx", "memory");
	return r;
}

static inline long long atomic64_dec_if_positive(atomic64_t *v)
{
	long long r;
	alternative_atomic64(dec_if_positive, "=&A" (r),
			     "S" (v) : "ecx", "memory");
	return r;
}

#undef alternative_atomic64
#undef __alternative_atomic64

#endif /* _ASM_X86_ATOMIC64_32_H */

#endif

#endif /* _ASM_X86_ATOMIC_H */
