#ifndef _BITS_H_
#define _BITS_H_

#define BITS_PER_REG 32

#define BIT_MASK(nr)	(1U << ((nr) % BITS_PER_REG))

// FIXME: BIT may should be merged with BIT_MASK
#define BIT(nr)				(1UL << (nr))

#endif /* _BITS_H_ */
