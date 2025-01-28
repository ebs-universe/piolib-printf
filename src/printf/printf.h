/* 
   Copyright (c) 
      (c) 2015-2016 Chintalagiri Shashank, Quazar Technologies Pvt.Ltd.
      (c) 2010-2011 Chintalagiri Shashank, Jugnu, IIT Kanpur
   
   This file is part of
   Embedded bootstraps : printf library
  
   This library is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. 
*/

/**
 * @file printf.h
 * @brief Interface and usage of printf implementation.
 *
 * This printf implementation can be used for embedded applications
 * in small embedded systems. As far as possible, the implementation
 * is pared down to the minimum required to retain core functionality.
 * 
 * The library provides two vprintf-like functions, though not printf
 * itself. 
 *      - vbprintf() : prints to a ::bytebuf, uses ds/bytebuf library 
 *      - vsprintf() : prints to a string
 * 
 * Locking is not implemented here. As long as it is correctly invoked,
 * vbprintf() inherits the locking mechanism of bytebuf.h and no more. 
 * Besides this, the functions should be sufficiently thread safe as long 
 * as the write targets are separate. This has not been tested. 
 * 
 * Due to the fairly heavy usage of the stack during execution, this
 * library should be used with care, and sparingly. 
 * 
 * @warning There may be, somewhere in the vbprintf() implementation, a race 
 * condition that nukes the buffer if it comes in too fast. As far as is 
 * apparant, it only occurs at the rollover boundary, it happens at the 
 * very first rollover, is not recoverable, and disappears entirely at 
 * slower write speeds. It is entirely possible that the error is a result
 * of the producer not checking for a returned error value. Excercise caution.
 * 
 * Allowed format strings : 
 *      - \%d      : signed integer
 *      - \%ld     : signed long integer
 *      - \%s      : string
 *      - \%x, \%X  : hexadecimal
 *      - \%u      : unsigned integer
 *      - \%c      : character
 *      - \%%      : escape
 * 
 * Allowed flags:
 *      - -       : Left justify.
 *      - 0       : Field is padded with 0's instead of blanks.
 * 
 * @see printf.c
 */

#include<stdarg.h>
#include<stdint.h>

/**
 * @name Definitions for internal printf.c use
 */
/**@{*/ 

/* Length of internal buffers for sting construction. 
 * It should be enough for 32 bit int */
#define PRINT_BUF_LEN           20

#define PRINT_TTYPE_STRING      0
#define PRINT_TTYPE_BYTEBUF     1

#ifndef PRINT_SUPPORT_FLOAT
    #define PRINT_SUPPORT_FLOAT     1
#endif

#ifndef PRINT_DEFAULT_FLOAT_PRECISION
    #define PRINT_DEFAULT_FLOAT_PRECISION   4
#endif
/**@}*/

/**
  * Declaration of a generic putchar implementation. Not used.
  * @param c Character to write.
  * @return 0 for failure, 1 for successful.
  */
int putchar(int c);


/**
  * Declaration of the underlying print implementation. This should not be
  * called directly. The call to this function should usually be inlined by one of
  * the proxy functions. 
  * @param out Pointer to the buffer or string to write to.
  * @param ttype Target type, either PRINT_TTYPE_STRING or PRINT_TTYPE_BYTEBUF.
  * @param format format string and args as documented.
  * @param args variable list from stdarg.h
  * @return Number of characters printed.
  */
extern int print(void **out, const char ttype, const char *format, va_list args );

/**
 * Prints to a string. For use by client libraries which intend to provide
 * printf like interfaces.
 * 
 * @param out Pointer to string / array to write to.
 * @param format format string and args as documented.
 * @param args variable list from stdarg.h
 * @return Number of characters printed.
 */
static inline int vsprintf(char *out, const char *format, va_list args);

static inline int vsprintf(char *buf, const char *format, va_list args)
{        
    return print( (void **)&buf, PRINT_TTYPE_STRING, format, args );
}

/**
 * Prints to a string. For direct use, exactly as sprintf.
 * 
 * @param out Pointer to string / array to write to.
 * @param format format string and args as documented.
 * @return Number of characters printed.
 */
static inline int sprintf(char *out, const char *format, ...){
    uint8_t rval;
    va_list args;
    va_start( args, format );
    rval = vsprintf(out, format, args);
    va_end(args);
    return rval;
}


/**
 * @name bytebuf Support
 * This implementation supports ::bytebuf buffers as first-class
 * write targets. If this is undesirable, then the following can 
 * be removed and/or reimplemented as needed.
 */
/**@{*/ 

#include <ds/bytebuf.h>

/**
  * Prints to a ::bytebuf buffer. For use by client libraries which 
  * intend to provide printf like interfaces.
  * @param buf Pointer to ::bytebuf to write to.
  * @param format format string and args as documented.
  * @param args variable list from stdarg.h
  * @return Number of characters printed.
  */
static inline int vbprintf(bytebuf *buf, const char *format, va_list args);

static inline int vbprintf(bytebuf *buf, const char *format, va_list args)
{        
        return print( (void **)&buf, PRINT_TTYPE_BYTEBUF, format, args );
}

/**
 * Prints to a ::bytebuf buffer. For direct use, similarly to sprintf.
 * 
 * @param buf Pointer to ::bytebuf buffer to write to.
 * @param format format string and args as documented.
 * @return Number of characters printed.
 */
static inline int bprintf(bytebuf *buf, const char *format, ...){
    uint8_t rval;
    va_list args;
    va_start( args, format );
    rval = vbprintf(buf, format, args);
    va_end(args);
    return rval;
}

/**@}*/ 
