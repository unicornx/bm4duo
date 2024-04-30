#ifndef _TRACE_H_
#define _TRACE_H_

#include "config.h"
#include <stdio.h>

#ifdef CONFIG_ENABLE_DEBUG
#define debug(fmt, args...)	printf(fmt, ##args)
#else
#define debug(...)
#endif

#endif /* _TRACE_H_ */
