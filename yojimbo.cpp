/*
    Yojimbo Client/Server Network Library.

    Copyright © 2016 - 2019, The Network Protocol Company, Inc.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer 
           in the documentation and/or other materials provided with the distribution.

        3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived 
           from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
    USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "yojimbo.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if YOJIMBO_DEBUG_MEMORY_LEAKS
#include <map>
#endif // YOJIMBO_DEBUG_MEMORY_LEAKS

//#if YOJIMBO_DEBUG_MEMORY_LEAKS
//#include <stdio.h>
//#endif // #if YOJIMBO_DEBUG_MEMORY_LEAKS

#include "tlsf/tlsf.h"


static yojimbo::Allocator * g_defaultAllocator = NULL;

namespace yojimbo
{
    Allocator & GetDefaultAllocator()
    {
        yojimbo_assert( g_defaultAllocator );
        return *g_defaultAllocator;
    }
}

bool InitializeYojimbo()
{
    g_defaultAllocator = new yojimbo::DefaultAllocator();

    return true;
}

void ShutdownYojimbo()
{
    yojimbo_assert( g_defaultAllocator );
    delete g_defaultAllocator;
    g_defaultAllocator = NULL;
}

// ---------------------------------------------------------------------------------

namespace yojimbo
{

    void print_bytes( const char * label, const uint8_t * data, int data_bytes )
    {
        printf( "%s: ", label );
        for ( int i = 0; i < data_bytes; ++i )
        {
            printf( "0x%02x,", (int) data[i] );
        }
        printf( " (%d bytes)\n", data_bytes );
    }

}

// ---------------------------------------------------------------------------------


namespace yojimbo
{
    Allocator::Allocator()
    {
        m_errorLevel = ALLOCATOR_ERROR_NONE;
    }

    Allocator::~Allocator()
    {
#if YOJIMBO_DEBUG_MEMORY_LEAKS
        if ( m_alloc_map.size() )
        {
            printf( "you leaked memory!\n\n" );
            typedef std::map<void*,AllocatorEntry>::iterator itor_type;
            for ( itor_type i = m_alloc_map.begin(); i != m_alloc_map.end(); ++i )
            {
                void * p = i->first;
                AllocatorEntry entry = i->second;
                printf( "leaked block %p (%d bytes) - %s:%d\n", p, (int) entry.size, entry.file, entry.line );
            }
            printf( "\n" );
            exit(1);
        }
#endif // #if YOJIMBO_DEBUG_MEMORY_LEAKS
    }

    void Allocator::SetErrorLevel( AllocatorErrorLevel errorLevel )
    {
        if ( m_errorLevel == ALLOCATOR_ERROR_NONE && errorLevel != ALLOCATOR_ERROR_NONE )
        {
            yojimbo_printf( YOJIMBO_LOG_LEVEL_ERROR, "allocator went into error state: %s\n", GetAllocatorErrorString( errorLevel ) );
        }
        m_errorLevel = errorLevel;
    }

    void Allocator::TrackAlloc( void * p, size_t size, const char * file, int line )
    {
#if YOJIMBO_DEBUG_MEMORY_LEAKS

        yojimbo_assert( m_alloc_map.find( p ) == m_alloc_map.end() );

        AllocatorEntry entry;
        entry.size = size;
        entry.file = file;
        entry.line = line;
        m_alloc_map[p] = entry;

#else // #if YOJIMBO_DEBUG_MEMORY_LEAKS

        (void) p;
        (void) size;
        (void) file;
        (void) line;

#endif // #if YOJIMBO_DEBUG_MEMORY_LEAKS
    }

    void Allocator::TrackFree( void * p, const char * file, int line )
    {
        (void) p;
        (void) file;
        (void) line;
#if YOJIMBO_DEBUG_MEMORY_LEAKS
        yojimbo_assert( m_alloc_map.find( p ) != m_alloc_map.end() );
        m_alloc_map.erase( p );
#endif // #if YOJIMBO_DEBUG_MEMORY_LEAKS
    }

    // =============================================

    void * DefaultAllocator::Allocate( size_t size, const char * file, int line )
    {
        void * p = malloc( size );

        if ( !p )
        {
            SetErrorLevel( ALLOCATOR_ERROR_OUT_OF_MEMORY );
            return NULL;
        }

        TrackAlloc( p, size, file, line );

        return p;
    }

    void DefaultAllocator::Free( void * p, const char * file, int line )
    {
        if ( !p )
            return;

        TrackFree( p, file, line );

        free( p );
    }

    // =============================================

    static void * AlignPointerUp( void * memory, int align )
    {
        yojimbo_assert( ( align & ( align - 1 ) ) == 0 );
        uintptr_t p = (uintptr_t) memory;
        return (void*) ( ( p + ( align - 1 ) ) & ~( align - 1 ) );
    }

    static void * AlignPointerDown( void * memory, int align )
    {
        yojimbo_assert( ( align & ( align - 1 ) ) == 0 );
        uintptr_t p = (uintptr_t) memory;
        return (void*) ( p - ( p & ( align - 1 ) ) );
    }

    TLSF_Allocator::TLSF_Allocator( void * memory, size_t size )
    {
        yojimbo_assert( size > 0 );

        SetErrorLevel( ALLOCATOR_ERROR_NONE );

        const int AlignBytes = 8;

        uint8_t * aligned_memory_start = (uint8_t*) AlignPointerUp( memory, AlignBytes );
        uint8_t * aligned_memory_finish = (uint8_t*) AlignPointerDown( ( (uint8_t*) memory ) + size, AlignBytes );

        yojimbo_assert( aligned_memory_start < aligned_memory_finish );

        size_t aligned_memory_size = aligned_memory_finish - aligned_memory_start;

        m_tlsf = tlsf_create_with_pool( aligned_memory_start, aligned_memory_size );
    }

    TLSF_Allocator::~TLSF_Allocator()
    {
        tlsf_destroy( m_tlsf );
    }

    void * TLSF_Allocator::Allocate( size_t size, const char * file, int line )
    {
        void * p = tlsf_malloc( m_tlsf, size );

        if ( !p )
        {
            SetErrorLevel( ALLOCATOR_ERROR_OUT_OF_MEMORY );
            return NULL;
        }

        TrackAlloc( p, size, file, line );

        return p;
    }

    void TLSF_Allocator::Free( void * p, const char * file, int line )
    {
        if ( !p )
            return;

        TrackFree( p, file, line );

        tlsf_free( m_tlsf, p );
    }
}

// ---------------------------------------------------------------------------------


static void default_assert_handler( const char * condition, const char * function, const char * file, int line )
{
    printf( "assert failed: ( %s ), function %s, file %s, line %d\n", condition, function, file, line );
    #if defined( __GNUC__ )
    __builtin_trap();
    #elif defined( _MSC_VER )
    __debugbreak();
    #endif
}

static int log_level = 0;
static int (*printf_function)( const char *, ... ) = printf;
void (*yojimbo_assert_function)( const char *, const char *, const char * file, int line ) = default_assert_handler;

void yojimbo_log_level( int level )
{
    log_level = level;
}

void yojimbo_set_printf_function( int (*function)( const char *, ... ) )
{
    yojimbo_assert( function );
    printf_function = function;
}

void yojimbo_set_assert_function( void (*function)( const char *, const char *, const char * file, int line ) )
{
    yojimbo_assert_function = function;
}

#if YOJIMBO_ENABLE_LOGGING

void yojimbo_printf( int level, const char * format, ... )
{
    if ( level > log_level )
        return;
    va_list args;
    va_start( args, format );
    char buffer[4*1024];
    vsprintf( buffer, format, args );
    printf_function( "%s", buffer );
    va_end( args );
}

#else // #if YOJIMBO_ENABLE_LOGGING

void yojimbo_printf( int level, const char * format, ... )
{
    (void) level;
    (void) format;
}

#endif // #if YOJIMBO_ENABLE_LOGGING

#if __APPLE__

// ===============================
//              MacOS
// ===============================

#include <unistd.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

void yojimbo_sleep( double time )
{
    usleep( (int) ( time * 1000000 ) );
}

double yojimbo_time()
{
    static uint64_t start = 0;

    static mach_timebase_info_data_t timebase_info;

    if ( start == 0 )
    {
        mach_timebase_info( &timebase_info );
        start = mach_absolute_time();
        return 0.0;
    }

    uint64_t current = mach_absolute_time();

    if ( current < start )
        current = start;

    return ( double( current - start ) * double( timebase_info.numer ) / double( timebase_info.denom ) ) / 1000000000.0;
}

#elif __linux

// ===============================
//              Linux
// ===============================

#include <unistd.h>
#include <time.h>

void yojimbo_sleep( double time )
{
    usleep( (int) ( time * 1000000 ) );
}

double yojimbo_time()
{
    static double start = -1;

    if ( start == -1 )
    {
        timespec ts;
        clock_gettime( CLOCK_MONOTONIC_RAW, &ts );
        start = ts.tv_sec + double( ts.tv_nsec ) / 1000000000.0;
        return 0.0;
    }

    timespec ts;
    clock_gettime( CLOCK_MONOTONIC_RAW, &ts );
    double current = ts.tv_sec + double( ts.tv_nsec ) / 1000000000.0;
    if ( current < start )
        current = start;
    return current - start;
}

#elif defined(_WIN32)

// ===============================
//             Windows
// ===============================

#define NOMINMAX
#include <windows.h>

void yojimbo_sleep( double time )
{
    const int milliseconds = time * 1000;
    Sleep( milliseconds );
}

static bool timer_initialized = false;
static LARGE_INTEGER timer_frequency;
static LARGE_INTEGER timer_start;

double yojimbo_time()
{
    if ( !timer_initialized )
    {
        QueryPerformanceFrequency( &timer_frequency );
        QueryPerformanceCounter( &timer_start );
        timer_initialized = true;
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter( &now );
    if ( now.QuadPart < timer_start.QuadPart )
        now.QuadPart = timer_start.QuadPart;
    return double( now.QuadPart - timer_start.QuadPart ) / double( timer_frequency.QuadPart );
}

#else

#error unsupported platform!

#endif

// ---------------------------------------------------------------------------------

namespace yojimbo {

    /*
    ** The variable-length integer encoding is as follows:
    **
    ** KEY:
    **         A = 0xxxxxxx    7 bits of data and one flag bit
    **         B = 1xxxxxxx    7 bits of data and one flag bit
    **         C = xxxxxxxx    8 bits of data
    **
    **  7 bits - A
    ** 14 bits - BA
    ** 21 bits - BBA
    ** 28 bits - BBBA
    ** 35 bits - BBBBA
    ** 42 bits - BBBBBA
    ** 49 bits - BBBBBBA
    ** 56 bits - BBBBBBBA
    ** 64 bits - BBBBBBBBC
    */

    /*
    ** Write a 64-bit variable-length integer to memory starting at p[0].
    ** The length of data write will be between 1 and 9 bytes.  The number
    ** of bytes written is returned.
    **
    ** A variable-length integer consists of the lower 7 bits of each byte
    ** for all bytes that have the 8th bit set and one byte with the 8th
    ** bit clear.  Except, if we get to the 9th byte, it stores the full
    ** 8 bits and is the last byte.
    */
    static int put_varint64(unsigned char *p, uint64_t v)
    {
        int i, j, n;
        uint8_t buf[10];
        if (v & (((uint64_t)0xff000000) << 32)) {
            p[8] = (uint8_t)v;
            v >>= 8;
            for (i = 7; i >= 0; i--) {
                p[i] = (uint8_t)((v & 0x7f) | 0x80);
                v >>= 7;
            }
            return 9;
        }
        n = 0;
        do {
            buf[n++] = (uint8_t)((v & 0x7f) | 0x80);
            v >>= 7;
        } while (v != 0);
        buf[0] &= 0x7f;
        yojimbo_assert(n <= 9);
        for (i = 0, j = n - 1; j >= 0; j--, i++) {
            p[i] = buf[j];
        }
        return n;
    }
    int yojimbo_put_varint(unsigned char *p, uint64_t v) {
        if (v <= 0x7f) {
            p[0] = v & 0x7f;
            return 1;
        }
        if (v <= 0x3fff) {
            p[0] = ((v >> 7) & 0x7f) | 0x80;
            p[1] = v & 0x7f;
            return 2;
        }
        return put_varint64(p, v);
    }

    /*
    ** Bitmasks used by yojimbo_get_varint().  These precomputed constants
    ** are defined here rather than simply putting the constant expressions
    ** inline in order to work around bugs in the RVT compiler.
    **
    ** SLOT_2_0     A mask for  (0x7f<<14) | 0x7f
    **
    ** SLOT_4_2_0   A mask for  (0x7f<<28) | SLOT_2_0
    */
#define SLOT_2_0     0x001fc07f
#define SLOT_4_2_0   0xf01fc07f

    /*
    ** Read a 64-bit variable-length integer from memory starting at p[0].
    ** Return the number of bytes read.  The value is stored in *v.
    */
    uint8_t yojimbo_get_varint(const unsigned char *p, uint64_t *v)
    {
        uint32_t a, b, s;

        if (((signed char*)p)[0] >= 0) {
            *v = *p;
            return 1;
        }
        if (((signed char*)p)[1] >= 0) {
            *v = ((uint32_t)(p[0] & 0x7f) << 7) | p[1];
            return 2;
        }

        /* Verify that constants are precomputed correctly */
        yojimbo_assert(SLOT_2_0 == ((0x7f << 14) | (0x7f)));
        yojimbo_assert(SLOT_4_2_0 == ((0xfU << 28) | (0x7f << 14) | (0x7f)));

        a = ((uint32_t)p[0]) << 14;
        b = p[1];
        p += 2;
        a |= *p;
        /* a: p0<<14 | p2 (unmasked) */
        if (!(a & 0x80))
        {
            a &= SLOT_2_0;
            b &= 0x7f;
            b = b << 7;
            a |= b;
            *v = a;
            return 3;
        }

        /* CSE1 from below */
        a &= SLOT_2_0;
        p++;
        b = b << 14;
        b |= *p;
        /* b: p1<<14 | p3 (unmasked) */
        if (!(b & 0x80))
        {
            b &= SLOT_2_0;
            /* moved CSE1 up */
            /* a &= (0x7f<<14)|(0x7f); */
            a = a << 7;
            a |= b;
            *v = a;
            return 4;
        }

        /* a: p0<<14 | p2 (masked) */
        /* b: p1<<14 | p3 (unmasked) */
        /* 1:save off p0<<21 | p1<<14 | p2<<7 | p3 (masked) */
        /* moved CSE1 up */
        /* a &= (0x7f<<14)|(0x7f); */
        b &= SLOT_2_0;
        s = a;
        /* s: p0<<14 | p2 (masked) */

        p++;
        a = a << 14;
        a |= *p;
        /* a: p0<<28 | p2<<14 | p4 (unmasked) */
        if (!(a & 0x80))
        {
            /* we can skip these cause they were (effectively) done above
            ** while calculating s */
            /* a &= (0x7f<<28)|(0x7f<<14)|(0x7f); */
            /* b &= (0x7f<<14)|(0x7f); */
            b = b << 7;
            a |= b;
            s = s >> 18;
            *v = ((uint64_t)s) << 32 | a;
            return 5;
        }

        /* 2:save off p0<<21 | p1<<14 | p2<<7 | p3 (masked) */
        s = s << 7;
        s |= b;
        /* s: p0<<21 | p1<<14 | p2<<7 | p3 (masked) */

        p++;
        b = b << 14;
        b |= *p;
        /* b: p1<<28 | p3<<14 | p5 (unmasked) */
        if (!(b & 0x80))
        {
            /* we can skip this cause it was (effectively) done above in calc'ing s */
            /* b &= (0x7f<<28)|(0x7f<<14)|(0x7f); */
            a &= SLOT_2_0;
            a = a << 7;
            a |= b;
            s = s >> 18;
            *v = ((uint64_t)s) << 32 | a;
            return 6;
        }

        p++;
        a = a << 14;
        a |= *p;
        /* a: p2<<28 | p4<<14 | p6 (unmasked) */
        if (!(a & 0x80))
        {
            a &= SLOT_4_2_0;
            b &= SLOT_2_0;
            b = b << 7;
            a |= b;
            s = s >> 11;
            *v = ((uint64_t)s) << 32 | a;
            return 7;
        }

        /* CSE2 from below */
        a &= SLOT_2_0;
        p++;
        b = b << 14;
        b |= *p;
        /* b: p3<<28 | p5<<14 | p7 (unmasked) */
        if (!(b & 0x80))
        {
            b &= SLOT_4_2_0;
            /* moved CSE2 up */
            /* a &= (0x7f<<14)|(0x7f); */
            a = a << 7;
            a |= b;
            s = s >> 4;
            *v = ((uint64_t)s) << 32 | a;
            return 8;
        }

        p++;
        a = a << 15;
        a |= *p;
        /* a: p4<<29 | p6<<15 | p8 (unmasked) */

        /* moved CSE2 up */
        /* a &= (0x7f<<29)|(0x7f<<15)|(0xff); */
        b &= SLOT_2_0;
        b = b << 8;
        a |= b;

        s = s << 4;
        b = p[-4];
        b &= 0x7f;
        b = b >> 3;
        s |= b;

        *v = ((uint64_t)s) << 32 | a;

        return 9;
    }

    /*
    ** Read a 32-bit variable-length integer from memory starting at p[0].
    ** Return the number of bytes read.  The value is stored in *v.
    **
    ** If the varint stored in p[0] is larger than can fit in a 32-bit unsigned
    ** integer, then set *v to 0xffffffff.
    **
    ** A MACRO version, getVarint32, is provided which inlines the
    ** single-byte case.  All code should use the MACRO version as
    ** this function assumes the single-byte case has already been handled.
    */
    uint8_t yojimbo_get_varint32(const unsigned char *p, uint32_t *v)
    {
        uint32_t a, b;

        /* The 1-byte case.  Overwhelmingly the most common.  Handled inline
        ** by the getVarin32() macro */
        a = *p;
        /* a: p0 (unmasked) */
#ifndef yojimbo_getvarint32
        if (!(a & 0x80))
        {
            /* Values between 0 and 127 */
            *v = a;
            return 1;
        }
#endif

        /* The 2-byte case */
        p++;
        b = *p;
        /* b: p1 (unmasked) */
        if (!(b & 0x80))
        {
            /* Values between 128 and 16383 */
            a &= 0x7f;
            a = a << 7;
            *v = a | b;
            return 2;
        }

        /* The 3-byte case */
        p++;
        a = a << 14;
        a |= *p;
        /* a: p0<<14 | p2 (unmasked) */
        if (!(a & 0x80))
        {
            /* Values between 16384 and 2097151 */
            a &= (0x7f << 14) | (0x7f);
            b &= 0x7f;
            b = b << 7;
            *v = a | b;
            return 3;
        }

        /* A 32-bit varint is used to store size information in btrees.
        ** Objects are rarely larger than 2MiB limit of a 3-byte varint.
        ** A 3-byte varint is sufficient, for example, to record the size
        ** of a 1048569-byte BLOB or string.
        **
        ** We only unroll the first 1-, 2-, and 3- byte cases.  The very
        ** rare larger cases can be handled by the slower 64-bit varint
        ** routine.
        */
#if 1
        {
            uint64_t v64;
            uint8_t n;

            p -= 2;
            n = yojimbo_get_varint(p, &v64);
            yojimbo_assert(n > 3 && n <= 9);
            if ((v64 & UINT32_MAX) != v64) {
                *v = 0xffffffff;
            }
            else {
                *v = (uint32_t)v64;
            }
            return n;
        }

#else
        /* For following code (kept for historical record only) shows an
        ** unrolling for the 3- and 4-byte varint cases.  This code is
        ** slightly faster, but it is also larger and much harder to test.
        */
        p++;
        b = b << 14;
        b |= *p;
        /* b: p1<<14 | p3 (unmasked) */
        if (!(b & 0x80))
        {
            /* Values between 2097152 and 268435455 */
            b &= (0x7f << 14) | (0x7f);
            a &= (0x7f << 14) | (0x7f);
            a = a << 7;
            *v = a | b;
            return 4;
        }

        p++;
        a = a << 14;
        a |= *p;
        /* a: p0<<28 | p2<<14 | p4 (unmasked) */
        if (!(a & 0x80))
        {
            /* Values  between 268435456 and 34359738367 */
            a &= SLOT_4_2_0;
            b &= SLOT_4_2_0;
            b = b << 7;
            *v = a | b;
            return 5;
        }

        /* We can only reach this point when reading a corrupt database
        ** file.  In that case we are not in any hurry.  Use the (relatively
        ** slow) general-purpose yojimbo_get_varint() routine to extract the
        ** value. */
        {
            uint64_t v64;
            uint8_t n;

            p -= 4;
            n = yojimbo_get_varint(p, &v64);
            yojimbo_assert(n > 5 && n <= 9);
            *v = (uint32_t)v64;
            return n;
        }
#endif
    }

    /*
    ** Return the number of bytes that will be needed to store the given
    ** 64-bit integer.
    */
    int yojimbo_measure_varint(uint64_t v)
    {
        int i;
        for (i = 1; (v >>= 7) != 0; i++) { yojimbo_assert(i < 10); }
        return i;
    }
} // yojimbo

