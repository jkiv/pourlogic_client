// See LICENSE.txt for license details.

#ifndef AVR_OO_H
#define AVR_OO_H

/*! \brief Implements some missing OO features. See http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=410870
 */

#include <stdlib.h>

void * operator new(size_t size); 
void operator delete(void * ptr);

void * operator new[](size_t size);
void operator delete[](void * ptr);

__extension__ typedef int __guard __attribute__((mode (__DI__))); 

extern "C" int __cxa_guard_acquire(__guard *); 
extern "C" void __cxa_guard_release (__guard *); 
extern "C" void __cxa_guard_abort (__guard *);
extern "C" void __cxa_pure_virtual(void);

#endif // #ifndef AVR_OO_H
