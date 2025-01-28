

#ifndef PRINTF_CONFIG_H
#define PRINTF_CONFIG_H


/* Length of internal buffers for sting construction. 
 * It should be enough for 32 bit int */
#ifndef PRINT_BUF_LEN
    #define PRINT_BUF_LEN                   20
#endif

#ifndef PRINT_SUPPORT_FLOAT
    #define PRINT_SUPPORT_FLOAT             1
#endif

#ifndef PRINT_DEFAULT_FLOAT_PRECISION
    #define PRINT_DEFAULT_FLOAT_PRECISION   4
#endif



#endif