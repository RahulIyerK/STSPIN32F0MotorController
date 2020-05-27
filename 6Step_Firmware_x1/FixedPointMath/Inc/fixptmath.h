#ifndef FIX_PT_MATH_H
#define FIX_PT_MATH_H
#include <stdint.h>

//saturating unsigned-signed multiplier
static inline int32_t MUL32_Q0_UFIX_SFIX (uint32_t a, int32_t b)
{
    int32_t res = a * b;
    if (b < 0 && res >=0)
    {
        return INT32_MIN;
    }
    else if (b > 0 && res <=0)
    {
        return INT32_MAX;
    }
    else
    {
        return res;
    }
    
}

//saturating unsigned-signed adder
static inline int32_t ADD32_Q0_SFIX_SFIX (int32_t a, int32_t b)
{
    int32_t res = a + b;
    if (a > 0 && b > 0 && res < 0)
    {
        return INT32_MAX;
    }
    else if (a < 0 && b < 0 && res > 0)
    {
        return INT32_MIN;
    }
    else
    {
        return res;
    }
    
}



#endif //FIX_PT_MATH_H