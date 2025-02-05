        /*
    Copyright (c) 
    (c) 2015-2016 Chintalagiri Shashank, Quazar Technologies Pvt.Ltd.
    (c) 2010-2011 Chintalagiri Shashank, Jugnu, IIT Kanpur
    (c) 2001-2002 Georges Menie (www.menie.org)
    stdarg version contributed by Christian Ettinger

    This file is part of
    Embedded bootstraps : printf library

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


/**
* @file printf.c
* @brief Pared down printf implementation for embedded use.
* 
* See printf.h for usage documentation. Implementation 
* documentation should be added to this file as some point. 
* 
* @see printf.h
*/

#include <math.h>
#include "printf.h"

/* 
* putchar() is needed for implementing garden-variety printf(), 
* and not for anything else. A stub that the compiler should 
* optimise away is retained here, which should be completed or 
* removed if putchar() is to be used.
* 
* #define putchar(c) outbyte(c) *	
*/
int putchar ( int c ) {
    return 1;
}

static int printchar(void **target, const printf_ttype_t ttype, int c)
{
    extern int putchar(int c);
        char **str;
        bytebuf *buf;
    if (ttype == PRINT_TTYPE_STRING){
        str = (char**)target;
        if (str) {
            **str = c;
            ++(*str);
        } 
    }
    else if (ttype == PRINT_TTYPE_BYTEBUF){
        buf = *((bytebuf**)target);
        return bytebuf_cPushByte(buf, c, BYTEBUF_TOKEN_PRINT);
    }
    return putchar(c);
}

#define PAD_RIGHT 1
#define PAD_ZERO  2

static int prints(void **out, const printf_ttype_t ttype, const char *string, int width, int pad)
{
    register int pc = 0, padchar = ' ';

    if (width > 0) {
        register int len = 0;
        register const char *ptr;
        for (ptr = string; *ptr; ++ptr) ++len;
        if (len >= width) width = 0;
        else width -= len;
        if (pad & PAD_ZERO) padchar = '0';
    }
    if (!(pad & PAD_RIGHT)) {
        for ( ; width > 0; --width) {
            pc += printchar (out, ttype, padchar);
        }
    }
    for ( ; *string ; ++string) {
        pc += printchar (out, ttype, *string);
        
    }
    for ( ; width > 0; --width) {
        pc += printchar (out, ttype, padchar);
    }

    return pc;
}

static inline uint8_t _write_number(char ** s, int u, int b, int letbase){
    register uint8_t t, len = 0;
    while (u) {
        t = u % b;
        if( t >= 10 )
            t += letbase - '0' - 10;
        (*s)--;
        **s = t + '0';
        u /= b;
        len ++; 
    }
    return len;
}

static int printl(void **out, const printf_ttype_t ttype, long i, int b, int sg, int width, int pad, int letbase)
{
    char print_buf[PRINT_BUF_LEN];
    char *s;
    register uint8_t neg = 0, pc = 0;
    register unsigned long u = i;

    if (i == 0) {
        print_buf[0] = '0';
        print_buf[1] = '\0';
        return prints (out, ttype, print_buf, width, pad);
    }

    if (sg && b == 10 && i < 0) {
        neg = 1;
        u = -i;
    }

    s = print_buf + PRINT_BUF_LEN-1;
    *s = '\0';

    _write_number(&s, u, b, letbase);

    if (neg) {
        if( width && (pad & PAD_ZERO) ) {
            pc += printchar (out, ttype, '-');
            --width;
        }
        else {
            *--s = '-';
        }
    }

    return pc + prints (out, ttype, s, width, pad);
}

static int printi(void **out, const printf_ttype_t ttype, int i, int b, int sg, int width, int pad, int letbase)
{
    char print_buf[PRINT_BUF_LEN];
    char *s;
    register uint8_t neg = 0, pc = 0;
    register unsigned int u = i;

    if (i == 0) {
        print_buf[0] = '0';
        print_buf[1] = '\0';
        return prints (out, ttype, print_buf, width, pad);
    }

    if (sg && b == 10 && i < 0) {
        neg = 1;
        u = -i;
    }

    s = print_buf + PRINT_BUF_LEN-1;
    *s = '\0';

    _write_number(&s, u, b, letbase);

    if (neg) {
        if( width && (pad & PAD_ZERO) ) {
            pc += printchar (out, ttype, '-');
            --width;
        }
        else {
            *--s = '-';
        }
    }

    return pc + prints (out, ttype, s, width, pad);
}

#if PRINT_SUPPORT_FLOAT
static int printfloat(void **out, const printf_ttype_t ttype, float f, int width, int pad, int precision) {
    char print_buf[PRINT_BUF_LEN], localbuf[4];
    char * s;
    char * sl;
    register uint8_t neg = 0, pc = 0, l8 = 0;
    int8_t exponent = 0;

    if (f < 0) {
        f = -f;
        neg = 1;
    }   

    if (f == 0.0) {
        exponent = 0;
    } else {
        // Normalize the number to scientific notation (1.0 <= mantissa < 10.0)
        while (f >= 10.0) {
            f /= 10.0;
            exponent ++;
        }
        while (f < 1.0) {
            f *= 10.0;
            exponent --;
        }
    }
    
    // Apply rounding based on precision
    int scale = 1;
    for (l8 = 0; l8 < precision; l8++){
        scale *= 10;
    }

    // This is useful, but very expensive.
    // mantissa = round(mantissa * scale) / scale;

    uint32_t digits = (uint32_t)f;
    uint32_t fraction = (uint32_t)((f - digits) * scale);

    s = print_buf + PRINT_BUF_LEN-1;
    *s = '\0';

    // Output the exponent if necessary
    if (exponent){
        if (exponent < 0){
            l8 = 1;
            exponent = -exponent;
        }
        _write_number(&s, exponent, 10, 'a');
        if (l8) {
            *--s = '-';
        }
        *--s = 'e';
    }
    
    // Output the fractional part
    sl = &localbuf[0];
    l8 = printi((void **)&sl, PRINT_TTYPE_STRING, fraction, 10, 0, precision, PAD_ZERO, 'a');
    while(l8){
        l8 --;
        *--s = localbuf[l8];
    }
    *--s = '.';
    
    // Output the integer part
    if (digits) {
        _write_number(&s, digits, 10, 'a');
        if (neg) {
            if( width && (pad & PAD_ZERO) ) {
                pc += printchar (out, ttype, '-');
                --width;
            }
            else {
                *--s = '-';
            }
        }
    } else {
        *--s = '0';
    }
    
    return pc + prints (out, ttype, s, width, pad);
}
#endif

int print(void **out, const printf_ttype_t ttype, const char *format, va_list args )
{
    register int width, pad;
    register int pc = 0;
    char scr[2], lng=0;
    char** outp;
        
    for (; *format != 0; ++format) {
        if (*format == '%') {
            ++format;
            width = pad = 0;
            #if PRINT_SUPPORT_FLOAT
            int precision = -1;
            #endif
            if (*format == '\0') break;
            if (*format == '%') goto out;
            if (*format == '-') {
                ++format;
                pad = PAD_RIGHT;
            }
            while (*format == '0') {
                ++format;
                pad |= PAD_ZERO;
            }
            for ( ; *format >= '0' && *format <= '9'; ++format) {
                width *= 10;
                width += *format - '0';
            }
            #if PRINT_SUPPORT_FLOAT
            if (*format == '.') {
                ++format;
                precision = 0;
                for (; *format >= '0' && *format <= '9'; ++format) {
                    precision *= 10;
                    precision += *format - '0';
                }
            }
            #endif
            if( *format == 'l' ) {
                lng=1;
                ++format;
            }
            #if PRINT_SUPPORT_FLOAT
            if (*format == 'f') {
                double fval = va_arg(args, double);
                if (precision == -1) precision = PRINT_DEFAULT_FLOAT_PRECISION;
                pc += printfloat(out, ttype, fval, width, pad, precision);
                continue;
            }
            #endif
            if( *format == 's' ) {
                register char *s = (char *)va_arg( args, int );
                pc += prints (out, ttype, s?s:"(null)", width, pad);
                continue;
            }
            if( *format == 'd' ) {
                if(!lng)
                    pc += printi (out, ttype, va_arg( args, int ), 10, 1, width, pad, 'a');
                else 
                    pc += printl (out, ttype, va_arg( args, long ), 10, 1, width, pad, 'a');
                continue;
            }
            if( *format == 'x' ) {
                pc += printi (out, ttype, va_arg( args, int ), 16, 0, width, pad, 'a');
                continue;
            }
            if( *format == 'X' ) {
                pc += printi (out, ttype, va_arg( args, int ), 16, 0, width, pad, 'A');
                continue;
            }
            if( *format == 'u' ) {
                pc += printi (out, ttype, va_arg( args, int ), 10, 0, width, pad, 'a');
                continue;
            }
            if( *format == 'c' ) {
                /* char are converted to int then pushed on the stack */
                scr[0] = (char)va_arg( args, int );
                scr[1] = '\0';
                pc += prints (out, ttype, scr, width, pad);
                continue;
            }
        }
        else {
            out:
                pc += printchar (out, ttype, *format);
        }
    }
    if (ttype == PRINT_TTYPE_STRING){   
        outp = (char **) out;
        **outp = '\0';
    }
    va_end( args );
    return pc;
}

