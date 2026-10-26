#ifndef __KERN_MM_SLUB_H__
#define __KERN_MM_SLUB_H__

#include <defs.h>

/* lab2 SLUB challenge 2413995 */

void slub_init(void);
void *slub_alloc(size_t size);
void slub_free(void *ptr);
void slub_check(void);

#endif /* !__KERN_MM_SLUB_H__ */
