// src/stddef.h

#ifndef STDDEF_H
#define STDDEF_H

// 1. size_t (тип для размеров и счетчиков, обычно unsigned int на 32-битных системах)
#include "stdint.h"



// 2. NULL (макрос для нулевого указателя)
#define NULL 0


// 3. ptrdiff_t (для разницы указателей, обычно signed int)
typedef int32_t ptrdiff_t;


#endif