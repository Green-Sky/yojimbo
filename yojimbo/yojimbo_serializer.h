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

#ifndef YOJIMBO_SERIALIZER_H
#define YOJIMBO_SERIALIZER_H

/** @file */

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#define YOJIMBO_MAJOR_VERSION 1
#define YOJIMBO_MINOR_VERSION 0
#define YOJIMBO_PATCH_VERSION 0

#if !defined (YOJIMBO_LITTLE_ENDIAN ) && !defined( YOJIMBO_BIG_ENDIAN )

  #ifdef __BYTE_ORDER__
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      #define YOJIMBO_LITTLE_ENDIAN 1
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
      #define YOJIMBO_BIG_ENDIAN 1
    #else
      #error Unknown machine endianess detected. User needs to define YOJIMBO_LITTLE_ENDIAN or YOJIMBO_BIG_ENDIAN.
    #endif // __BYTE_ORDER__

  // Detect with GLIBC's endian.h
  #elif defined(__GLIBC__)
    #include <endian.h>
    #if (__BYTE_ORDER == __LITTLE_ENDIAN)
      #define YOJIMBO_LITTLE_ENDIAN 1
    #elif (__BYTE_ORDER == __BIG_ENDIAN)
      #define YOJIMBO_BIG_ENDIAN 1
    #else
      #error Unknown machine endianess detected. User needs to define YOJIMBO_LITTLE_ENDIAN or YOJIMBO_BIG_ENDIAN.
    #endif // __BYTE_ORDER

  // Detect with _LITTLE_ENDIAN and _BIG_ENDIAN macro
  #elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
    #define YOJIMBO_LITTLE_ENDIAN 1
  #elif defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
    #define YOJIMBO_BIG_ENDIAN 1

  // Detect with architecture macros
  #elif    defined(__sparc)     || defined(__sparc__)                           \
        || defined(_POWER)      || defined(__powerpc__)                         \
        || defined(__ppc__)     || defined(__hpux)      || defined(__hppa)      \
        || defined(_MIPSEB)     || defined(_POWER)      || defined(__s390__)
    #define YOJIMBO_BIG_ENDIAN 1
  #elif    defined(__i386__)    || defined(__alpha__)   || defined(__ia64)      \
        || defined(__ia64__)    || defined(_M_IX86)     || defined(_M_IA64)     \
        || defined(_M_ALPHA)    || defined(__amd64)     || defined(__amd64__)   \
        || defined(_M_AMD64)    || defined(__x86_64)    || defined(__x86_64__)  \
        || defined(_M_X64)      || defined(__bfin__)
    #define YOJIMBO_LITTLE_ENDIAN 1
  #elif defined(_MSC_VER) && defined(_M_ARM)
    #define YOJIMBO_LITTLE_ENDIAN 1
  #else
    #error Unknown machine endianess detected. User needs to define YOJIMBO_LITTLE_ENDIAN or YOJIMBO_BIG_ENDIAN.
  #endif
#endif

#ifndef YOJIMBO_LITTLE_ENDIAN
#define YOJIMBO_LITTLE_ENDIAN 0
#endif

#ifndef YOJIMBO_BIG_ENDIAN
#define YOJIMBO_BIG_ENDIAN 0
#endif

#ifndef YOJIMBO_DEFAULT_TIMEOUT
#define YOJIMBO_DEFAULT_TIMEOUT 5
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4127 )
#pragma warning( disable : 4244 )
#endif // #ifdef _MSC_VER

#define YOJIMBO_PLATFORM_WINDOWS                    1
#define YOJIMBO_PLATFORM_MAC                        2
#define YOJIMBO_PLATFORM_UNIX                       3

#if defined(_WIN32)
#define YOJIMBO_PLATFORM YOJIMBO_PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define YOJIMBO_PLATFORM YOJIMBO_PLATFORM_MAC
#else
#define YOJIMBO_PLATFORM YOJIMBO_PLATFORM_UNIX
#endif

#define YOJIMBO_SERIALIZE_CHECKS                    1

#ifndef NDEBUG

#define YOJIMBO_DEBUG_MEMORY_LEAKS                  1
#define YOJIMBO_DEBUG_MESSAGE_LEAKS                 1
#define YOJIMBO_DEBUG_MESSAGE_BUDGET                1

#else // #ifndef NDEBUG

#define YOJIMBO_DEBUG_MEMORY_LEAKS                  0
#define YOJIMBO_DEBUG_MESSAGE_LEAKS                 0
#define YOJIMBO_DEBUG_MESSAGE_BUDGET                0

#endif // #ifndef NDEBUG

#define YOJIMBO_ENABLE_LOGGING                      1

#include <inttypes.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <memory.h>

#include <new>

#if YOJIMBO_DEBUG_MESSAGE_LEAKS
#include <map>
#endif // #if YOJIMBO_DEBUG_MESSAGE_LEAKS


/// The library namespace.

namespace yojimbo {
    const uint32_t SerializeCheckValue = 0x12345678;                ///< The value written to the stream for serialize checks. See WriteStream::SerializeCheck and ReadStream::SerializeCheck.
}

/**
    Initialize the yojimbo library.
    Call this before calling any yojimbo library functions.
    @returns True if the library was successfully initialized, false otherwise.
 */

bool InitializeYojimbo();

/**
    Shutdown the yojimbo library.
    Call this after you finish using the library and it will run some checks for you (for example, checking for memory leaks in debug build).

 */

void ShutdownYojimbo();

/**
    Template function to get the minimum of two values.
    @param a The first value.
    @param b The second value.
    @returns The minimum of a and b.
 */

template <typename T> const T & yojimbo_min( const T & a, const T & b )
{
    return ( a < b ) ? a : b;
}

/**
    Template function to get the maximum of two values.
    @param a The first value.
    @param b The second value.
    @returns The maximum of a and b.
 */

template <typename T> const T & yojimbo_max( const T & a, const T & b )
{
    return ( a > b ) ? a : b;
}

/**
    Template function to clamp a value.
    @param value The value to be clamped.
    @param a The minimum value.
    @param b The minimum value.
    @returns The clamped value in [a,b].
 */

template <typename T> T yojimbo_clamp( const T & value, const T & a, const T & b )
{
    if ( value < a )
        return a;
    else if ( value > b )
        return b;
    else
        return value;
}

/**
    Swap two values.
    @param a First value.
    @param b Second value.
 */

template <typename T> void yojimbo_swap( T & a, T & b )
{
    T tmp = a;
    a = b;
    b = tmp;
}

/**
    Get the absolute value.

    @param value The input value.

    @returns The absolute value.
 */

template <typename T> T yojimbo_abs( const T & value )
{
    return ( value < 0 ) ? -value : value;
}

/**
    Sleep for approximately this number of seconds.
    @param time number of seconds to sleep for.
 */

//void yojimbo_sleep( double time );

/**
    Get a high precision time in seconds since the application has started.
    Please store time in doubles so you retain sufficient precision as time increases.
    @returns Time value in seconds.
 */

//double yojimbo_time();

#define YOJIMBO_LOG_LEVEL_NONE      0
#define YOJIMBO_LOG_LEVEL_ERROR     1
#define YOJIMBO_LOG_LEVEL_INFO      2
#define YOJIMBO_LOG_LEVEL_DEBUG     3

/**
    Set the yojimbo log level.
    Valid log levels are: YOJIMBO_LOG_LEVEL_NONE, YOJIMBO_LOG_LEVEL_ERROR, YOJIMBO_LOG_LEVEL_INFO and YOJIMBO_LOG_LEVEL_DEBUG
    @param level The log level to set. Initially set to YOJIMBO_LOG_LEVEL_NONE.
 */

void yojimbo_log_level( int level );

/**
    Printf function used by yojimbo to emit logs.
    This function internally calls the printf callback set by the user.
    @see yojimbo_set_printf_function
 */

void yojimbo_printf( int level, const char * format, ... );

extern void (*yojimbo_assert_function)( const char *, const char *, const char * file, int line );

/**
    Assert function used by yojimbo.
    This assert function lets the user override the assert presentation.
    @see yojimbo_set_assert_functio
 */

#ifndef NDEBUG
#define yojimbo_assert( condition )                                                         \
do                                                                                          \
{                                                                                           \
    if ( !(condition) )                                                                     \
    {                                                                                       \
        yojimbo_assert_function( #condition, __FUNCTION__, __FILE__, __LINE__ );            \
        exit(1);                                                                            \
    }                                                                                       \
} while(0)
#else
#define yojimbo_assert( ignore ) ((void)0)
#endif

/**
    Call this to set the printf function to use for logging.
    @param function The printf callback function.
 */

void yojimbo_set_printf_function( int (*function)( const char * /*format*/, ... ) );

/**
    Call this to set the function to call when an assert triggers.
    @param function The assert callback function.
 */

void yojimbo_set_assert_function( void (*function)( const char * /*condition*/, const char * /*function*/, const char * /*file*/, int /*line*/ ) );


typedef void* tlsf_t;

namespace yojimbo {
    /**
        Get the default allocator.
        Use this allocator when you just want to use malloc/free, but in the form of a yojimbo allocator.
        This allocator instance is created inside InitializeYojimbo and destroyed in ShutdownYojimbo.
        In debug build, it will automatically check for memory leaks and print them out for you when you shutdown the library.
        @returns The default allocator instances backed by malloc and free.
     */

    class Allocator & GetDefaultAllocator();

    /// Macro for creating a new object instance with a yojimbo allocator.
    #define YOJIMBO_NEW( a, T, ... ) ( new ( (a).Allocate( sizeof(T), __FILE__, __LINE__ ) ) T(__VA_ARGS__) )

    /// Macro for deleting an object created with a yojimbo allocator.
    #define YOJIMBO_DELETE( a, T, p ) do { if (p) { (p)->~T(); (a).Free( p, __FILE__, __LINE__ ); p = NULL; } } while (0)

    /// Macro for allocating a block of memory with a yojimbo allocator.
    #define YOJIMBO_ALLOCATE( a, bytes ) (a).Allocate( (bytes), __FILE__, __LINE__ )

    /// Macro for freeing a block of memory created with a yojimbo allocator.
    #define YOJIMBO_FREE( a, p ) do { if ( p ) { (a).Free( p, __FILE__, __LINE__ ); p = NULL; } } while(0)

    /// Allocator error level.
    enum AllocatorErrorLevel
    {
        ALLOCATOR_ERROR_NONE = 0,                               ///< No error. All is well.
        ALLOCATOR_ERROR_OUT_OF_MEMORY                           ///< The allocator is out of memory!
    };

    /// Helper function to convert an allocator error to a user friendly string.
    inline const char * GetAllocatorErrorString( AllocatorErrorLevel error )
    {
        switch ( error )
        {
            case ALLOCATOR_ERROR_NONE:                  return "none";
            case ALLOCATOR_ERROR_OUT_OF_MEMORY:         return "out of memory";
            default:
                yojimbo_assert( false );
                return "(unknown)";
        }
    }

#if YOJIMBO_DEBUG_MEMORY_LEAKS

    /**
        Debug structure used to track allocations and find memory leaks.
        Active in debug build only. Disabled in release builds for performance reasons.
     */

    struct AllocatorEntry
    {
        size_t size;                        ///< The size of the allocation in bytes.
        const char * file;                  ///< Filename of the source code file that made the allocation.
        int line;                           ///< Line number in the source code where the allocation was made.
    };

#endif // #if YOJIMBO_DEBUG_MEMORY_LEAKS

    /**
        Functionality common to all allocators.
        Extend this class to hook up your own allocator to yojimbo.
        IMPORTANT: This allocator is not yet thread safe. Only call it from one thread!
     */

    class Allocator
    {
    public:

        /**
            Allocator constructor.
            Sets the error level to ALLOCATOR_ERROR_NONE.
         */

        Allocator();

        /**
            Allocator destructor.
            Make sure all allocations made from this allocator are freed before you destroy this allocator.
            In debug build, validates this is true walks the map of allocator entries. Any outstanding entries are considered memory leaks and printed to stdout.
         */

        virtual ~Allocator();

        /**
            Allocate a block of memory.
            IMPORTANT: Don't call this directly. Use the YOJIMBO_NEW or YOJIMBO_ALLOCATE macros instead, because they automatically pass in the source filename and line number for you.
            @param size The size of the block of memory to allocate (bytes).
            @param file The source code filename that is performing the allocation. Used for tracking allocations and reporting on memory leaks.
            @param line The line number in the source code file that is performing the allocation.
            @returns A block of memory of the requested size, or NULL if the allocation could not be performed. If NULL is returned, the error level is set to ALLOCATION_ERROR_FAILED_TO_ALLOCATE.
            @see Allocator::Free
            @see Allocator::GetErrorLevel
         */

        virtual void * Allocate( size_t size, const char * file, int line ) = 0;

        /**
            Free a block of memory.
            IMPORTANT: Don't call this directly. Use the YOJIMBO_DELETE or YOJIMBO_FREE macros instead, because they automatically pass in the source filename and line number for you.
            @param p Pointer to the block of memory to free. Must be non-NULL block of memory that was allocated with this allocator. Will assert otherwise.
            @param file The source code filename that is performing the free. Used for tracking allocations and reporting on memory leaks.
            @param line The line number in the source code file that is performing the free.
            @see Allocator::Allocate
            @see Allocator::GetErrorLevel
         */

        virtual void Free( void * p, const char * file, int line ) = 0;

        /**
            Get the allocator error level.
            Use this function to check if an allocation has failed. This is used in the client/server to disconnect a client with a failed allocation.
            @returns The allocator error level.
         */

        AllocatorErrorLevel GetErrorLevel() const { return m_errorLevel; }

        /**
            Clear the allocator error level back to default.
         */

        void ClearError() { m_errorLevel = ALLOCATOR_ERROR_NONE; }

    protected:

        /**
            Set the error level.
            For correct client/server behavior when an allocation fails, please make sure you call this method to set the error level to ALLOCATOR_ERROR_FAILED_TO_ALLOCATE.
            @param error The allocator error level to set.
         */

        void SetErrorLevel( AllocatorErrorLevel errorLevel );

        /**
            Call this function to track an allocation made by your derived allocator class.
            In debug build, tracked allocations are automatically checked for leaks when the allocator is destroyed.
            @param p Pointer to the memory that was allocated.
            @param size The size of the allocation in bytes.
            @param file The source code file that performed the allocation.
            @param line The line number in the source file where the allocation was performed.
         */

        void TrackAlloc( void * p, size_t size, const char * file, int line );

        /**
            Call this function to track a free made by your derived allocator class.
            In debug build, any allocation tracked without a corresponding free is considered a memory leak when the allocator is destroyed.
            @param p Pointer to the memory that was allocated.
            @param file The source code file that is calling in to free the memory.
            @param line The line number in the source file where the free is being called from.
         */

        void TrackFree( void * p, const char * file, int line );

        AllocatorErrorLevel m_errorLevel;                                       ///< The allocator error level.

#if YOJIMBO_DEBUG_MEMORY_LEAKS
        std::map<void*,AllocatorEntry> m_alloc_map;                             ///< Debug only data structure used to find and report memory leaks.
#endif // #if YOJIMBO_DEBUG_MEMORY_LEAKS

    private:

        Allocator( const Allocator & other );

        Allocator & operator = ( const Allocator & other );
    };

    /**
        The default allocator implementation based around malloc and free.
     */

    class DefaultAllocator : public Allocator
    {
    public:

        /**
            Default constructor.
         */

        DefaultAllocator() {}

        /**
            Allocates a block of memory using "malloc".
            IMPORTANT: Don't call this directly. Use the YOJIMBO_NEW or YOJIMBO_ALLOCATE macros instead, because they automatically pass in the source filename and line number for you.
            @param size The size of the block of memory to allocate (bytes).
            @param file The source code filename that is performing the allocation. Used for tracking allocations and reporting on memory leaks.
            @param line The line number in the source code file that is performing the allocation.
            @returns A block of memory of the requested size, or NULL if the allocation could not be performed. If NULL is returned, the error level is set to ALLOCATION_ERROR_FAILED_TO_ALLOCATE.
         */

        void * Allocate( size_t size, const char * file, int line );

        /**
            Free a block of memory by calling "free".
            IMPORTANT: Don't call this directly. Use the YOJIMBO_DELETE or YOJIMBO_FREE macros instead, because they automatically pass in the source filename and line number for you.
            @param p Pointer to the block of memory to free. Must be non-NULL block of memory that was allocated with this allocator. Will assert otherwise.
            @param file The source code filename that is performing the free. Used for tracking allocations and reporting on memory leaks.
            @param line The line number in the source code file that is performing the free.
         */

        void Free( void * p, const char * file, int line );

    private:

        DefaultAllocator( const DefaultAllocator & other );

        DefaultAllocator & operator = ( const DefaultAllocator & other );
    };

    /**
        An allocator built on the TLSF allocator implementation by Matt Conte. Thanks Matt!
        This is a fast allocator that supports multiple heaps. It's used inside the yojimbo server to silo allocations for each client to their own heap.
        See https://github.com/mattconte/tlsf for details on this allocator implementation.
     */

    class TLSF_Allocator : public Allocator
    {
    public:

        /**
            TLSF allocator constructor.
            If you want to integrate your own allocator with yojimbo for use with the client and server, this class is a good template to start from.
            Make sure your constructor has the same signature as this one, and it will work with the YOJIMBO_SERVER_ALLOCATOR and YOJIMBO_CLIENT_ALLOCATOR helper macros.
            @param memory Block of memory in which the allocator will work. This block must remain valid while this allocator exists. The allocator does not assume ownership of it, you must free it elsewhere, if necessary.
            @param bytes The size of the block of memory (bytes). The maximum amount of memory you can allocate will be less, due to allocator overhead.
         */

        TLSF_Allocator( void * memory, size_t bytes );

        /**
            TLSF allocator destructor.
            Checks for memory leaks in debug build. Free all memory allocated by this allocator before destroying.
         */

        ~TLSF_Allocator();

        /**
            Allocates a block of memory using TLSF.
            IMPORTANT: Don't call this directly. Use the YOJIMBO_NEW or YOJIMBO_ALLOCATE macros instead, because they automatically pass in the source filename and line number for you.
            @param size The size of the block of memory to allocate (bytes).
            @param file The source code filename that is performing the allocation. Used for tracking allocations and reporting on memory leaks.
            @param line The line number in the source code file that is performing the allocation.
            @returns A block of memory of the requested size, or NULL if the allocation could not be performed. If NULL is returned, the error level is set to ALLOCATION_ERROR_FAILED_TO_ALLOCATE.
         */

        void * Allocate( size_t size, const char * file, int line );

        /**
            Free a block of memory using TLSF.
            IMPORTANT: Don't call this directly. Use the YOJIMBO_DELETE or YOJIMBO_FREE macros instead, because they automatically pass in the source filename and line number for you.
            @param p Pointer to the block of memory to free. Must be non-NULL block of memory that was allocated with this allocator. Will assert otherwise.
            @param file The source code filename that is performing the free. Used for tracking allocations and reporting on memory leaks.
            @param line The line number in the source code file that is performing the free.
            @see Allocator::Allocate
            @see Allocator::GetError
         */

        void Free( void * p, const char * file, int line );

    private:

        tlsf_t m_tlsf;              ///< The TLSF allocator instance backing this allocator.

        TLSF_Allocator( const TLSF_Allocator & other );
        TLSF_Allocator & operator = ( const TLSF_Allocator & other );
    };

    /**
        Calculates the population count of an unsigned 32 bit integer at compile time.
        Population count is the number of bits in the integer that set to 1.
        See "Hacker's Delight" and http://www.hackersdelight.org/hdcodetxt/popArrayHS.c.txt
        @see yojimbo::Log2
        @see yojimbo::BitsRequired
     */

    template <uint32_t x> struct PopCount
    {
        enum {   a = x - ( ( x >> 1 )       & 0x55555555 ),
                 b =   ( ( ( a >> 2 )       & 0x33333333 ) + ( a & 0x33333333 ) ),
                 c =   ( ( ( b >> 4 ) + b ) & 0x0f0f0f0f ),
                 d =   c + ( c >> 8 ),
                 e =   d + ( d >> 16 ),

            result = e & 0x0000003f
        };
    };

    /**
        Calculates the log 2 of an unsigned 32 bit integer at compile time.
        @see yojimbo::Log2
        @see yojimbo::BitsRequired
     */

    template <uint32_t x> struct Log2
    {
        enum {   a = x | ( x >> 1 ),
                 b = a | ( a >> 2 ),
                 c = b | ( b >> 4 ),
                 d = c | ( c >> 8 ),
                 e = d | ( d >> 16 ),
                 f = e >> 1,

            result = PopCount<f>::result
        };
    };

    /**
        Calculates the number of bits required to serialize an integer value in [min,max] at compile time.
        @see Log2
        @see PopCount
     */

    template <int64_t min, int64_t max> struct BitsRequired
    {
        static const uint32_t result = ( min == max ) ? 0 : ( Log2<uint32_t(max-min)>::result + 1 );
    };

    /**
        Calculates the population count of an unsigned 32 bit integer.
        The population count is the number of bits in the integer set to 1.
        @param x The input integer value.
        @returns The number of bits set to 1 in the input value.
     */

    inline uint32_t popcount( uint32_t x )
    {
#ifdef __GNUC__
        return __builtin_popcount( x );
#else // #ifdef __GNUC__
        const uint32_t a = x - ( ( x >> 1 )       & 0x55555555 );
        const uint32_t b =   ( ( ( a >> 2 )       & 0x33333333 ) + ( a & 0x33333333 ) );
        const uint32_t c =   ( ( ( b >> 4 ) + b ) & 0x0f0f0f0f );
        const uint32_t d =   c + ( c >> 8 );
        const uint32_t e =   d + ( d >> 16 );
        const uint32_t result = e & 0x0000003f;
        return result;
#endif // #ifdef __GNUC__
    }

    /**
        Calculates the log base 2 of an unsigned 32 bit integer.
        @param x The input integer value.
        @returns The log base 2 of the input.
     */

    inline uint32_t log2( uint32_t x )
    {
        const uint32_t a = x | ( x >> 1 );
        const uint32_t b = a | ( a >> 2 );
        const uint32_t c = b | ( b >> 4 );
        const uint32_t d = c | ( c >> 8 );
        const uint32_t e = d | ( d >> 16 );
        const uint32_t f = e >> 1;
        return popcount( f );
    }

    /**
        Calculates the number of bits required to serialize an integer in range [min,max].
        @param min The minimum value.
        @param max The maximum value.
        @returns The number of bits required to serialize the integer.
     */

    inline int bits_required( uint32_t min, uint32_t max )
    {
#ifdef __GNUC__
        return ( min == max ) ? 0 : 32 - __builtin_clz( max - min );
#else // #ifdef __GNUC__
        return ( min == max ) ? 0 : log2( max - min ) + 1;
#endif // #ifdef __GNUC__
    }

    /**
        Reverse the order of bytes in a 64 bit integer.
        @param value The input value.
        @returns The input value with the byte order reversed.
     */

    inline uint64_t bswap( uint64_t value )
    {
#ifdef __GNUC__
        return __builtin_bswap64( value );
#else // #ifdef __GNUC__
        value = ( value & 0x00000000FFFFFFFF ) << 32 | ( value & 0xFFFFFFFF00000000 ) >> 32;
        value = ( value & 0x0000FFFF0000FFFF ) << 16 | ( value & 0xFFFF0000FFFF0000 ) >> 16;
        value = ( value & 0x00FF00FF00FF00FF ) << 8  | ( value & 0xFF00FF00FF00FF00 ) >> 8;
        return value;
#endif // #ifdef __GNUC__
    }

    /**
        Reverse the order of bytes in a 32 bit integer.
        @param value The input value.
        @returns The input value with the byte order reversed.
     */

    inline uint32_t bswap( uint32_t value )
    {
#ifdef __GNUC__
        return __builtin_bswap32( value );
#else // #ifdef __GNUC__
        return ( value & 0x000000ff ) << 24 | ( value & 0x0000ff00 ) << 8 | ( value & 0x00ff0000 ) >> 8 | ( value & 0xff000000 ) >> 24;
#endif // #ifdef __GNUC__
    }

    /**
        Reverse the order of bytes in a 16 bit integer.
        @param value The input value.
        @returns The input value with the byte order reversed.
     */

    inline uint16_t bswap( uint16_t value )
    {
        return ( value & 0x00ff ) << 8 | ( value & 0xff00 ) >> 8;
    }

    /**
        Template to convert an integer value from local byte order to network byte order.
        IMPORTANT: Because most machines running yojimbo are little endian, yojimbo defines network byte order to be little endian.
        @param value The input value in local byte order. Supported integer types: uint64_t, uint32_t, uint16_t.
        @returns The input value converted to network byte order. If this processor is little endian the output is the same as the input. If the processor is big endian, the output is the input byte swapped.
        @see yojimbo::bswap
     */

    template <typename T> T host_to_network( T value )
    {
#if YOJIMBO_BIG_ENDIAN
        return bswap( value );
#else // #if YOJIMBO_BIG_ENDIAN
        return value;
#endif // #if YOJIMBO_BIG_ENDIAN
    }

    /**
        Template to convert an integer value from network byte order to local byte order.
        IMPORTANT: Because most machines running yojimbo are little endian, yojimbo defines network byte order to be little endian.
        @param value The input value in network byte order. Supported integer types: uint64_t, uint32_t, uint16_t.
        @returns The input value converted to local byte order. If this processor is little endian the output is the same as the input. If the processor is big endian, the output is the input byte swapped.
        @see yojimbo::bswap
     */

    template <typename T> T network_to_host( T value )
    {
#if YOJIMBO_BIG_ENDIAN
        return bswap( value );
#else // #if YOJIMBO_BIG_ENDIAN
        return value;
#endif // #if YOJIMBO_BIG_ENDIAN
    }

    /**
        Compares two 16 bit sequence numbers and returns true if the first one is greater than the second (considering wrapping).
        IMPORTANT: This is not the same as s1 > s2!
        Greater than is defined specially to handle wrapping sequence numbers.
        If the two sequence numbers are close together, it is as normal, but they are far apart, it is assumed that they have wrapped around.
        Thus, sequence_greater_than( 1, 0 ) returns true, and so does sequence_greater_than( 0, 65535 )!
        @param s1 The first sequence number.
        @param s2 The second sequence number.
        @returns True if the s1 is greater than s2, with sequence number wrapping considered.
     */

    inline bool sequence_greater_than( uint16_t s1, uint16_t s2 )
    {
        return ( ( s1 > s2 ) && ( s1 - s2 <= 32768 ) ) ||
               ( ( s1 < s2 ) && ( s2 - s1  > 32768 ) );
    }

    /**
        Compares two 16 bit sequence numbers and returns true if the first one is less than the second (considering wrapping).
        IMPORTANT: This is not the same as s1 < s2!
        Greater than is defined specially to handle wrapping sequence numbers.
        If the two sequence numbers are close together, it is as normal, but they are far apart, it is assumed that they have wrapped around.
        Thus, sequence_less_than( 0, 1 ) returns true, and so does sequence_greater_than( 65535, 0 )!
        @param s1 The first sequence number.
        @param s2 The second sequence number.
        @returns True if the s1 is less than s2, with sequence number wrapping considered.
     */

    inline bool sequence_less_than( uint16_t s1, uint16_t s2 )
    {
        return sequence_greater_than( s2, s1 );
    }

    /**
        Convert a signed integer to an unsigned integer with zig-zag encoding.
        0,-1,+1,-2,+2... becomes 0,1,2,3,4 ...
        @param n The input value.
        @returns The input value converted from signed to unsigned with zig-zag encoding.
     */

    inline int signed_to_unsigned( int n )
    {
        return ( n << 1 ) ^ ( n >> 31 );
    }

    /**
        Convert an unsigned integer to as signed integer with zig-zag encoding.
        0,1,2,3,4... becomes 0,-1,+1,-2,+2...
        @param n The input value.
        @returns The input value converted from unsigned to signed with zig-zag encoding.
     */

    inline int unsigned_to_signed( uint32_t n )
    {
        return ( n >> 1 ) ^ ( -int32_t( n & 1 ) );
    }

    /**
        A simple bit array class.
        You can create a bit array with a number of bits, set, clear and test if each bit is set.
     */

    class BitArray
    {
    public:

        /**
            The bit array constructor.
            @param allocator The allocator to use.
            @param size The number of bits in the bit array.
            All bits are initially set to zero.
         */

        BitArray( Allocator & allocator, int size )
        {
            yojimbo_assert( size > 0 );
            m_allocator = &allocator;
            m_size = size;
            m_bytes = 8 * ( ( size / 64 ) + ( ( size % 64 ) ? 1 : 0 ) );
            yojimbo_assert( m_bytes > 0 );
            m_data = (uint64_t*) YOJIMBO_ALLOCATE( allocator, m_bytes );
            Clear();
        }

        /**
            The bit array destructor.
         */

        ~BitArray()
        {
            yojimbo_assert( m_data );
            yojimbo_assert( m_allocator );
            YOJIMBO_FREE( *m_allocator, m_data );
            m_allocator = NULL;
        }

        /**
            Clear all bit values to zero.
         */

        void Clear()
        {
            yojimbo_assert( m_data );
            memset( m_data, 0, m_bytes );
        }

        /**
            Set a bit to 1.
            @param index The index of the bit.
         */

        void SetBit( int index )
        {
            yojimbo_assert( index >= 0 );
            yojimbo_assert( index < m_size );
            const int data_index = index >> 6;
            const int bit_index = index & ( (1<<6) - 1 );
            yojimbo_assert( bit_index >= 0 );
            yojimbo_assert( bit_index < 64 );
            m_data[data_index] |= uint64_t(1) << bit_index;
        }

        /**
            Clear a bit to 0.
            @param index The index of the bit.
         */

        void ClearBit( int index )
        {
            yojimbo_assert( index >= 0 );
            yojimbo_assert( index < m_size );
            const int data_index = index >> 6;
            const int bit_index = index & ( (1<<6) - 1 );
            m_data[data_index] &= ~( uint64_t(1) << bit_index );
        }

        /**
            Get the value of the bit.
            Returns 1 if the bit is set, 0 if the bit is not set.
            @param index The index of the bit.
         */

        uint64_t GetBit( int index ) const
        {
            yojimbo_assert( index >= 0 );
            yojimbo_assert( index < m_size );
            const int data_index = index >> 6;
            const int bit_index = index & ( (1<<6) - 1 );
            yojimbo_assert( bit_index >= 0 );
            yojimbo_assert( bit_index < 64 );
            return ( m_data[data_index] >> bit_index ) & 1;
        }

        /**
            Gets the size of the bit array, in number of bits.
            @returns The number of bits.
         */

        int GetSize() const
        {
            return m_size;
        }

    private:

        Allocator * m_allocator;                            ///< Allocator passed in to the constructor.
        int m_size;                                         ///< The size of the bit array in bits.
        int m_bytes;                                        ///< The size of the bit array in bytes.
        uint64_t * m_data;                                  ///< The data backing the bit array is an array of 64 bit integer values.

        BitArray( const BitArray & other );
        BitArray & operator = ( const BitArray & other );
    };

    /**
        A simple templated queue.
        This is a FIFO queue. First entry in, first entry out.
     */

    template <typename T> class Queue
    {
    public:

        /**
            Queue constructor.
            @param allocator The allocator to use.
            @param size The maximum number of entries in the queue.
         */

        Queue( Allocator & allocator, int size )
        {
            yojimbo_assert( size > 0 );
            m_arraySize = size;
            m_startIndex = 0;
            m_numEntries = 0;
            m_allocator = &allocator;
            m_entries = (T*) YOJIMBO_ALLOCATE( allocator, sizeof(T) * size );
            memset( m_entries, 0, sizeof(T) * size );
        }

        /**
            Queue destructor.
         */

        ~Queue()
        {
            yojimbo_assert( m_allocator );

            YOJIMBO_FREE( *m_allocator, m_entries );

            m_arraySize = 0;
            m_startIndex = 0;
            m_numEntries = 0;

            m_allocator = NULL;
        }

        /**
            Clear all entries in the queue and reset back to default state.
         */

        void Clear()
        {
            m_numEntries = 0;
            m_startIndex = 0;
        }

        /**
            Pop a value off the queue.
            IMPORTANT: This will assert if the queue is empty. Check Queue::IsEmpty or Queue::GetNumEntries first!
            @returns The value popped off the queue.
         */

        T Pop()
        {
            yojimbo_assert( !IsEmpty() );
            const T & entry = m_entries[m_startIndex];
            m_startIndex = ( m_startIndex + 1 ) % m_arraySize;
            m_numEntries--;
            return entry;
        }

        /**
            Push a value on to the queue.
            @param value The value to push onto the queue.
            IMPORTANT: Will assert if the queue is already full. Check Queue::IsFull before calling this!
         */

        void Push( const T & value )
        {
            yojimbo_assert( !IsFull() );
            const int index = ( m_startIndex + m_numEntries ) % m_arraySize;
            m_entries[index] = value;
            m_numEntries++;
        }

        /**
            Random access for entries in the queue.
            @param index The index into the queue. 0 is the oldest entry, Queue::GetNumEntries() - 1 is the newest.
            @returns The value in the queue at the index.
         */

        T & operator [] ( int index )
        {
            yojimbo_assert( !IsEmpty() );
            yojimbo_assert( index >= 0 );
            yojimbo_assert( index < m_numEntries );
            return m_entries[ ( m_startIndex + index ) % m_arraySize ];
        }

        /**
            Random access for entries in the queue (const version).
            @param index The index into the queue. 0 is the oldest entry, Queue::GetNumEntries() - 1 is the newest.
            @returns The value in the queue at the index.
         */

        const T & operator [] ( int index ) const
        {
            yojimbo_assert( !IsEmpty() );
            yojimbo_assert( index >= 0 );
            yojimbo_assert( index < m_numEntries );
            return m_entries[ ( m_startIndex + index ) % m_arraySize ];
        }

        /**
            Get the size of the queue.
            This is the maximum number of values that can be pushed on the queue.
            @returns The size of the queue.
         */

        int GetSize() const
        {
            return m_arraySize;
        }

        /**
            Is the queue currently full?
            @returns True if the queue is full. False otherwise.
         */

        bool IsFull() const
        {
            return m_numEntries == m_arraySize;
        }

        /**
            Is the queue currently empty?
            @returns True if there are no entries in the queue.
         */

        bool IsEmpty() const
        {
            return m_numEntries == 0;
        }

        /**
            Get the number of entries in the queue.
            @returns The number of entries in the queue in [0,GetSize()].
         */

        int GetNumEntries() const
        {
            return m_numEntries;
        }

    private:


        Allocator * m_allocator;                        ///< The allocator passed in to the constructor.
        T * m_entries;                                  ///< Array of entries backing the queue (circular buffer).
        int m_arraySize;                                ///< The size of the array, in number of entries. This is the "size" of the queue.
        int m_startIndex;                               ///< The start index for the queue. This is the next value that gets popped off.
        int m_numEntries;                               ///< The number of entries currently stored in the queue.
    };

    /**
        Data structure that stores data indexed by sequence number.
        Entries may or may not exist. If they don't exist the sequence value for the entry at that index is set to 0xFFFFFFFF.
        This provides a constant time lookup for an entry by sequence number. If the entry at sequence modulo buffer size doesn't have the same sequence number, that sequence number is not stored.
        This is incredibly useful and is used as the foundation of the packet level ack system and the reliable message send and receive queues.
        @see Connection
     */

    template <typename T> class SequenceBuffer
    {
    public:

        /**
            Sequence buffer constructor.
            @param allocator The allocator to use.
            @param size The size of the sequence buffer.
         */

        SequenceBuffer( Allocator & allocator, int size )
        {
            yojimbo_assert( size > 0 );
            m_size = size;
            m_sequence = 0;
            m_allocator = &allocator;
            m_entry_sequence = (uint32_t*) YOJIMBO_ALLOCATE( allocator, sizeof( uint32_t ) * size );
            m_entries = (T*) YOJIMBO_ALLOCATE( allocator, sizeof(T) * size );
            Reset();
        }

        /**
            Sequence buffer destructor.
         */

        ~SequenceBuffer()
        {
            yojimbo_assert( m_allocator );
            YOJIMBO_FREE( *m_allocator, m_entries );
            YOJIMBO_FREE( *m_allocator, m_entry_sequence );
            m_allocator = NULL;
        }

        /**
            Reset the sequence buffer.
            Removes all entries from the sequence buffer and restores it to initial state.
         */

        void Reset()
        {
            m_sequence = 0;
            memset( m_entry_sequence, 0xFF, sizeof( uint32_t ) * m_size );
        }

        /**
            Insert an entry in the sequence buffer.
            IMPORTANT: If another entry exists at the sequence modulo buffer size, it is overwritten.
            @param sequence The sequence number.
            @returns The sequence buffer entry, which you must fill with your data. NULL if a sequence buffer entry could not be added for your sequence number (if the sequence number is too old for example).
         */

        T * Insert( uint16_t sequence )
        {
            if ( sequence_greater_than( sequence + 1, m_sequence ) )
            {
                RemoveEntries( m_sequence, sequence );
                m_sequence = sequence + 1;
            }
            else if ( sequence_less_than( sequence, m_sequence - m_size ) )
            {
                return NULL;
            }
            const int index = sequence % m_size;
            m_entry_sequence[index] = sequence;
            return &m_entries[index];
        }

        /**
            Remove an entry from the sequence buffer.
            @param sequence The sequence number of the entry to remove.
         */

        void Remove( uint16_t sequence )
        {
            m_entry_sequence[ sequence % m_size ] = 0xFFFFFFFF;
        }

        /**
            Is the entry corresponding to the sequence number available? eg. Currently unoccupied.
            This works because older entries are automatically set back to unoccupied state as the sequence buffer advances forward.
            @param sequence The sequence number.
            @returns True if the sequence buffer entry is available, false if it is already occupied.
         */

        bool Available( uint16_t sequence ) const
        {
            return m_entry_sequence[ sequence % m_size ] == 0xFFFFFFFF;
        }

        /**
            Does an entry exist for a sequence number?
            @param sequence The sequence number.
            @returns True if an entry exists for this sequence number.
         */

        bool Exists( uint16_t sequence ) const
        {
            return m_entry_sequence[ sequence % m_size ] == uint32_t( sequence );
        }

        /**
            Get the entry corresponding to a sequence number.
            @param sequence The sequence number.
            @returns The entry if it exists. NULL if no entry is in the buffer for this sequence number.
         */

        T * Find( uint16_t sequence )
        {
            const int index = sequence % m_size;
            if ( m_entry_sequence[index] == uint32_t( sequence ) )
                return &m_entries[index];
            else
                return NULL;
        }

        /**
            Get the entry corresponding to a sequence number (const version).
            @param sequence The sequence number.
            @returns The entry if it exists. NULL if no entry is in the buffer for this sequence number.
         */

        const T * Find( uint16_t sequence ) const
        {
            const int index = sequence % m_size;
            if ( m_entry_sequence[index] == uint32_t( sequence ) )
                return &m_entries[index];
            else
                return NULL;
        }

        /**
            Get the entry at the specified index.
            Use this to iterate across entries in the sequence buffer.
            @param index The entry index in [0,GetSize()-1].
            @returns The entry if it exists. NULL if no entry is in the buffer at the specified index.
         */

        T * GetAtIndex( int index )
        {
            yojimbo_assert( index >= 0 );
            yojimbo_assert( index < m_size );
            return m_entry_sequence[index] != 0xFFFFFFFF ? &m_entries[index] : NULL;
        }

        /**
            Get the entry at the specified index (const version).
            Use this to iterate across entries in the sequence buffer.
            @param index The entry index in [0,GetSize()-1].
            @returns The entry if it exists. NULL if no entry is in the buffer at the specified index.
         */

        const T * GetAtIndex( int index ) const
        {
            yojimbo_assert( index >= 0 );
            yojimbo_assert( index < m_size );
            return m_entry_sequence[index] != 0xFFFFFFFF ? &m_entries[index] : NULL;
        }

        /**
            Get the most recent sequence number added to the buffer.
            This sequence number can wrap around, so if you are at 65535 and add an entry for sequence 0, then 0 becomes the new "most recent" sequence number.
            @returns The most recent sequence number.
            @see yojimbo::sequence_greater_than
            @see yojimbo::sequence_less_than
         */

        uint16_t GetSequence() const
        {
            return m_sequence;
        }

        /**
            Get the entry index for a sequence number.
            This is simply the sequence number modulo the sequence buffer size.
            @param sequence The sequence number.
            @returns The sequence buffer index corresponding of the sequence number.
         */

        int GetIndex( uint16_t sequence ) const
        {
            return sequence % m_size;
        }

        /**
            Get the size of the sequence buffer.
            @returns The size of the sequence buffer (number of entries).
         */

        int GetSize() const
        {
            return m_size;
        }

    protected:

        /**
            Helper function to remove entries.
            This is used to remove old entries as we advance the sequence buffer forward.
            Otherwise, if when entries are added with holes (eg. receive buffer for packets or messages, where not all sequence numbers are added to the buffer because we have high packet loss),
            and we are extremely unlucky, we can have old sequence buffer entries from the previous sequence # wrap around still in the buffer, which corrupts our internal connection state.
            This actually happened in the soak test at high packet loss levels (>90%). It took me days to track it down :)
         */

        void RemoveEntries( int start_sequence, int finish_sequence )
        {
            if ( finish_sequence < start_sequence )
                finish_sequence += 65535;
            yojimbo_assert( finish_sequence >= start_sequence );
            if ( finish_sequence - start_sequence < m_size )
            {
                for ( int sequence = start_sequence; sequence <= finish_sequence; ++sequence )
                    m_entry_sequence[sequence % m_size] = 0xFFFFFFFF;
            }
            else
            {
                for ( int i = 0; i < m_size; ++i )
                    m_entry_sequence[i] = 0xFFFFFFFF;
            }
        }

    private:

        Allocator * m_allocator;                   ///< The allocator passed in to the constructor.
        int m_size;                                ///< The size of the sequence buffer.
        uint16_t m_sequence;                       ///< The most recent sequence number added to the buffer.
        uint32_t * m_entry_sequence;               ///< Array of sequence numbers corresponding to each sequence buffer entry for fast lookup. Set to 0xFFFFFFFF if no entry exists at that index.
        T * m_entries;                             ///< The sequence buffer entries. This is where the data is stored per-entry. Separate from the sequence numbers for fast lookup (hot/cold split) when the data per-sequence number is relatively large.

        SequenceBuffer( const SequenceBuffer<T> & other );

        SequenceBuffer<T> & operator = ( const SequenceBuffer<T> & other );
    };

    /**
        Bitpacks unsigned integer values to a buffer.
        Integer bit values are written to a 64 bit scratch value from right to left.
        Once the low 32 bits of the scratch is filled with bits it is flushed to memory as a dword and the scratch value is shifted right by 32.
        The bit stream is written to memory in little endian order, which is considered network byte order for this library.
        @see BitReader
     */

    class BitWriter
    {
    public:

        /**
            Bit writer constructor.
            Creates a bit writer object to write to the specified buffer.
            @param data The pointer to the buffer to fill with bitpacked data.
            @param bytes The size of the buffer in bytes. Must be a multiple of 4, because the bitpacker reads and writes memory as dwords, not bytes.
         */

        BitWriter( void * data, int bytes ) : m_data( (uint32_t*) data ), m_numWords( bytes / 4 )
        {
            yojimbo_assert( data );
            yojimbo_assert( ( bytes % 4 ) == 0 );
            m_numBits = m_numWords * 32;
            m_bitsWritten = 0;
            m_wordIndex = 0;
            m_scratch = 0;
            m_scratchBits = 0;
        }

        /**
            Write bits to the buffer.
            Bits are written to the buffer as-is, without padding to nearest byte. Will assert if you try to write past the end of the buffer.
            A boolean value writes just 1 bit to the buffer, a value in range [0,31] can be written with just 5 bits and so on.
            IMPORTANT: When you have finished writing to your buffer, take care to call BitWrite::FlushBits, otherwise the last dword of data will not get flushed to memory!
            @param value The integer value to write to the buffer. Must be in [0,(1<<bits)-1].
            @param bits The number of bits to encode in [1,32].
            @see BitReader::ReadBits
         */

        void WriteBits( uint32_t value, int bits )
        {
            yojimbo_assert( bits > 0 );
            yojimbo_assert( bits <= 32 );
            yojimbo_assert( m_bitsWritten + bits <= m_numBits );
            yojimbo_assert( uint64_t( value ) <= ( ( 1ULL << bits ) - 1 ) );

            m_scratch |= uint64_t( value ) << m_scratchBits;

            m_scratchBits += bits;

            if ( m_scratchBits >= 32 )
            {
                yojimbo_assert( m_wordIndex < m_numWords );
                m_data[m_wordIndex] = host_to_network( uint32_t( m_scratch & 0xFFFFFFFF ) );
                m_scratch >>= 32;
                m_scratchBits -= 32;
                m_wordIndex++;
            }

            m_bitsWritten += bits;
        }

        /**
            Write an alignment to the bit stream, padding zeros so the bit index becomes is a multiple of 8.
            This is useful if you want to write some data to a packet that should be byte aligned. For example, an array of bytes, or a string.
            IMPORTANT: If the current bit index is already a multiple of 8, nothing is written.
            @see BitReader::ReadAlign
         */

        void WriteAlign()
        {
            const int remainderBits = m_bitsWritten % 8;

            if ( remainderBits != 0 )
            {
                uint32_t zero = 0;
                WriteBits( zero, 8 - remainderBits );
                yojimbo_assert( ( m_bitsWritten % 8 ) == 0 );
            }
        }

        /**
            Write an array of bytes to the bit stream.
            Use this when you have to copy a large block of data into your bitstream.
            Faster than just writing each byte to the bit stream via BitWriter::WriteBits( value, 8 ), because it aligns to byte index and copies into the buffer without bitpacking.
            @param data The byte array data to write to the bit stream.
            @param bytes The number of bytes to write.
            @see BitReader::ReadBytes
         */

        void WriteBytes( const uint8_t * data, int bytes )
        {
            yojimbo_assert( GetAlignBits() == 0 );
            yojimbo_assert( m_bitsWritten + bytes * 8 <= m_numBits );
            yojimbo_assert( ( m_bitsWritten % 32 ) == 0 || ( m_bitsWritten % 32 ) == 8 || ( m_bitsWritten % 32 ) == 16 || ( m_bitsWritten % 32 ) == 24 );

            int headBytes = ( 4 - ( m_bitsWritten % 32 ) / 8 ) % 4;
            if ( headBytes > bytes )
                headBytes = bytes;
            for ( int i = 0; i < headBytes; ++i )
                WriteBits( data[i], 8 );
            if ( headBytes == bytes )
                return;

            FlushBits();

            yojimbo_assert( GetAlignBits() == 0 );

            int numWords = ( bytes - headBytes ) / 4;
            if ( numWords > 0 )
            {
                yojimbo_assert( ( m_bitsWritten % 32 ) == 0 );
                memcpy( &m_data[m_wordIndex], data + headBytes, numWords * 4 );
                m_bitsWritten += numWords * 32;
                m_wordIndex += numWords;
                m_scratch = 0;
            }

            yojimbo_assert( GetAlignBits() == 0 );

            int tailStart = headBytes + numWords * 4;
            int tailBytes = bytes - tailStart;
            yojimbo_assert( tailBytes >= 0 && tailBytes < 4 );
            for ( int i = 0; i < tailBytes; ++i )
                WriteBits( data[tailStart+i], 8 );

            yojimbo_assert( GetAlignBits() == 0 );

            yojimbo_assert( headBytes + numWords * 4 + tailBytes == bytes );
        }

        /**
            Flush any remaining bits to memory.
            Call this once after you've finished writing bits to flush the last dword of scratch to memory!
            @see BitWriter::WriteBits
         */

        void FlushBits()
        {
            if ( m_scratchBits != 0 )
            {
                yojimbo_assert( m_scratchBits <= 32 );
                yojimbo_assert( m_wordIndex < m_numWords );
                m_data[m_wordIndex] = host_to_network( uint32_t( m_scratch & 0xFFFFFFFF ) );
                m_scratch >>= 32;
                m_scratchBits = 0;
                m_wordIndex++;
            }
        }

        /**
            How many align bits would be written, if we were to write an align right now?
            @returns Result in [0,7], where 0 is zero bits required to align (already aligned) and 7 is worst case.
         */

        int GetAlignBits() const
        {
            return ( 8 - ( m_bitsWritten % 8 ) ) % 8;
        }

        /**
            How many bits have we written so far?
            @returns The number of bits written to the bit buffer.
         */

        int GetBitsWritten() const
        {
            return m_bitsWritten;
        }

        /**
            How many bits are still available to write?
            For example, if the buffer size is 4, we have 32 bits available to write, if we have already written 10 bytes then 22 are still available to write.
            @returns The number of bits available to write.
         */

        int GetBitsAvailable() const
        {
            return m_numBits - m_bitsWritten;
        }

        /**
            Get a pointer to the data written by the bit writer.
            Corresponds to the data block passed in to the constructor.
            @returns Pointer to the data written by the bit writer.
         */

        const uint8_t * GetData() const
        {
            return (uint8_t*) m_data;
        }

        /**
            The number of bytes flushed to memory.
            This is effectively the size of the packet that you should send after you have finished bitpacking values with this class.
            The returned value is not always a multiple of 4, even though we flush dwords to memory. You won't miss any data in this case because the order of bits written is designed to work with the little endian memory layout.
            IMPORTANT: Make sure you call BitWriter::FlushBits before calling this method, otherwise you risk missing the last dword of data.
         */

        int GetBytesWritten() const
        {
            return ( m_bitsWritten + 7 ) / 8;
        }

    private:

        uint32_t * m_data;              ///< The buffer we are writing to, as a uint32_t * because we're writing dwords at a time.
        uint64_t m_scratch;             ///< The scratch value where we write bits to (right to left). 64 bit for overflow. Once # of bits in scratch is >= 32, the low 32 bits are flushed to memory.
        int m_numBits;                  ///< The number of bits in the buffer. This is equivalent to the size of the buffer in bytes multiplied by 8. Note that the buffer size must always be a multiple of 4.
        int m_numWords;                 ///< The number of words in the buffer. This is equivalent to the size of the buffer in bytes divided by 4. Note that the buffer size must always be a multiple of 4.
        int m_bitsWritten;              ///< The number of bits written so far.
        int m_wordIndex;                ///< The current word index. The next word flushed to memory will be at this index in m_data.
        int m_scratchBits;              ///< The number of bits in scratch. When this is >= 32, the low 32 bits of scratch is flushed to memory as a dword and scratch is shifted right by 32.
    };

    /**
        Reads bit packed integer values from a buffer.
        Relies on the user reconstructing the exact same set of bit reads as bit writes when the buffer was written. This is an unattributed bitpacked binary stream!
        Implementation: 32 bit dwords are read in from memory to the high bits of a scratch value as required. The user reads off bit values from the scratch value from the right, after which the scratch value is shifted by the same number of bits.
     */

    class BitReader
    {
    public:

        /**
            Bit reader constructor.
            Non-multiples of four buffer sizes are supported, as this naturally tends to occur when packets are read from the network.
            However, actual buffer allocated for the packet data must round up at least to the next 4 bytes in memory, because the bit reader reads dwords from memory not bytes.
            @param data Pointer to the bitpacked data to read.
            @param bytes The number of bytes of bitpacked data to read.
            @see BitWriter
         */

#ifndef NDEBUG
        BitReader( const void * data, int bytes ) : m_data( (const uint32_t*) data ), m_numBytes( bytes ), m_numWords( ( bytes + 3 ) / 4)
#else // #ifndef NDEBUG
        BitReader( const void * data, int bytes ) : m_data( (const uint32_t*) data ), m_numBytes( bytes )
#endif // #ifndef NDEBUG
        {
            yojimbo_assert( data );
            m_numBits = m_numBytes * 8;
            m_bitsRead = 0;
            m_scratch = 0;
            m_scratchBits = 0;
            m_wordIndex = 0;
        }

        /**
            Would the bit reader would read past the end of the buffer if it read this many bits?
            @param bits The number of bits that would be read.
            @returns True if reading the number of bits would read past the end of the buffer.
         */

        bool WouldReadPastEnd( int bits ) const
        {
            return m_bitsRead + bits > m_numBits;
        }

        /**
            Read bits from the bit buffer.
            This function will assert in debug builds if this read would read past the end of the buffer.
            In production situations, the higher level ReadStream takes care of checking all packet data and never calling this function if it would read past the end of the buffer.
            @param bits The number of bits to read in [1,32].
            @returns The integer value read in range [0,(1<<bits)-1].
            @see BitReader::WouldReadPastEnd
            @see BitWriter::WriteBits
         */

        uint32_t ReadBits( int bits )
        {
            yojimbo_assert( bits > 0 );
            yojimbo_assert( bits <= 32 );
            yojimbo_assert( m_bitsRead + bits <= m_numBits );

            m_bitsRead += bits;

            yojimbo_assert( m_scratchBits >= 0 && m_scratchBits <= 64 );

            if ( m_scratchBits < bits )
            {
                yojimbo_assert( m_wordIndex < m_numWords );
                m_scratch |= uint64_t( network_to_host( m_data[m_wordIndex] ) ) << m_scratchBits;
                m_scratchBits += 32;
                m_wordIndex++;
            }

            yojimbo_assert( m_scratchBits >= bits );

            const uint32_t output = m_scratch & ( (uint64_t(1)<<bits) - 1 );

            m_scratch >>= bits;
            m_scratchBits -= bits;

            return output;
        }

        /**
            Read an align.
            Call this on read to correspond to a WriteAlign call when the bitpacked buffer was written.
            This makes sure we skip ahead to the next aligned byte index. As a safety check, we verify that the padding to next byte is zero bits and return false if that's not the case.
            This will typically abort packet read. Just another safety measure...
            @returns True if we successfully read an align and skipped ahead past zero pad, false otherwise (probably means, no align was written to the stream).
            @see BitWriter::WriteAlign
         */

        bool ReadAlign()
        {
            const int remainderBits = m_bitsRead % 8;
            if ( remainderBits != 0 )
            {
                uint32_t value = ReadBits( 8 - remainderBits );
                yojimbo_assert( m_bitsRead % 8 == 0 );
                if ( value != 0 )
                    return false;
            }
            return true;
        }

        /**
            Read bytes from the bitpacked data.
            @see BitWriter::WriteBytes
         */

        void ReadBytes( uint8_t * data, int bytes )
        {
            yojimbo_assert( GetAlignBits() == 0 );
            yojimbo_assert( m_bitsRead + bytes * 8 <= m_numBits );
            yojimbo_assert( ( m_bitsRead % 32 ) == 0 || ( m_bitsRead % 32 ) == 8 || ( m_bitsRead % 32 ) == 16 || ( m_bitsRead % 32 ) == 24 );

            int headBytes = ( 4 - ( m_bitsRead % 32 ) / 8 ) % 4;
            if ( headBytes > bytes )
                headBytes = bytes;
            for ( int i = 0; i < headBytes; ++i )
                data[i] = (uint8_t) ReadBits( 8 );
            if ( headBytes == bytes )
                return;

            yojimbo_assert( GetAlignBits() == 0 );

            int numWords = ( bytes - headBytes ) / 4;
            if ( numWords > 0 )
            {
                yojimbo_assert( ( m_bitsRead % 32 ) == 0 );
                memcpy( data + headBytes, &m_data[m_wordIndex], numWords * 4 );
                m_bitsRead += numWords * 32;
                m_wordIndex += numWords;
                m_scratchBits = 0;
            }

            yojimbo_assert( GetAlignBits() == 0 );

            int tailStart = headBytes + numWords * 4;
            int tailBytes = bytes - tailStart;
            yojimbo_assert( tailBytes >= 0 && tailBytes < 4 );
            for ( int i = 0; i < tailBytes; ++i )
                data[tailStart+i] = (uint8_t) ReadBits( 8 );

            yojimbo_assert( GetAlignBits() == 0 );

            yojimbo_assert( headBytes + numWords * 4 + tailBytes == bytes );
        }

        /**
            How many align bits would be read, if we were to read an align right now?
            @returns Result in [0,7], where 0 is zero bits required to align (already aligned) and 7 is worst case.
         */

        int GetAlignBits() const
        {
            return ( 8 - m_bitsRead % 8 ) % 8;
        }

        /**
            How many bits have we read so far?
            @returns The number of bits read from the bit buffer so far.
         */

        int GetBitsRead() const
        {
            return m_bitsRead;
        }

        /**
            How many bits are still available to read?
            For example, if the buffer size is 4, we have 32 bits available to read, if we have already written 10 bytes then 22 are still available.
            @returns The number of bits available to read.
         */

        int GetBitsRemaining() const
        {
            return m_numBits - m_bitsRead;
        }

    private:

        const uint32_t * m_data;            ///< The bitpacked data we're reading as a dword array.
        uint64_t m_scratch;                 ///< The scratch value. New data is read in 32 bits at a top to the left of this buffer, and data is read off to the right.
        int m_numBits;                      ///< Number of bits to read in the buffer. Of course, we can't *really* know this so it's actually m_numBytes * 8.
        int m_numBytes;                     ///< Number of bytes to read in the buffer. We know this, and this is the non-rounded up version.
#ifndef NDEBUG
        int m_numWords;                     ///< Number of words to read in the buffer. This is rounded up to the next word if necessary.
#endif // #ifndef NDEBUG
        int m_bitsRead;                     ///< Number of bits read from the buffer so far.
        int m_scratchBits;                  ///< Number of bits currently in the scratch value. If the user wants to read more bits than this, we have to go fetch another dword from memory.
        int m_wordIndex;                    ///< Index of the next word to read from memory.
    };


    /**
        Routines to read and write variable-length integers.
    */

    int yojimbo_put_varint(unsigned char *p, uint64_t v);
    uint8_t yojimbo_get_varint(const unsigned char *p, uint64_t *v);
    uint8_t yojimbo_get_varint32(const unsigned char *p, uint32_t *v);
    int yojimbo_measure_varint(uint64_t v);

    /**
        The common case is for a varint to be a single byte.  They following macros handle the common case without a procedure call, but then call the procedure for larger varints.
    */
    // #define yojimbo_getvarint32(A,B) (uint8_t)((*(A)<(uint8_t)0x80)?((B)=(uint32_t)*(A)),1:yojimbo_get_varint32((A),(uint32_t *)&(B)))
    // #define yojimbo_putvarint32(A,B) (uint8_t)(((uint32_t)(B)<(uint32_t)0x80)?(*(A)=(unsigned char)(B)),1:yojimbo_put_varint((A),(B)))
    // #define yojimbo_getvarint yojimbo_get_varint
    // #define yojimbo_putvarint yojimbo_put_varint

    /**
        Functionality common to all stream classes.
     */

    class BaseStream
    {
    public:

        /**
            Base stream constructor.
            @param allocator The allocator to use for stream allocations. This lets you dynamically allocate memory as you read and write packets.
         */

        explicit BaseStream( Allocator & allocator ) : m_allocator( &allocator ), m_context( NULL ) {}

        /**
            Set a context on the stream.
            Contexts are used by the library supply data that is needed to read and write packets.
            Specifically, this context is used by the connection to supply data needed to read and write connection packets.
            If you are using the yojimbo client/server or connection classes you should NOT set this manually. It's already taken!
            However, if you are using only the low-level parts of yojimbo, feel free to take this over and use it for whatever you want.
            @see ConnectionContext
            @see ConnectionPacket
         */

        void SetContext( void * context )
        {
            m_context = context;
        }

        /**
            Get the context pointer set on the stream.

            @returns The context pointer. May be NULL.
         */

        void * GetContext() const
        {
            return m_context;
        }

        /**
            Get the allocator set on the stream.
            You can use this allocator to dynamically allocate memory while reading and writing packets.
            @returns The stream allocator.
         */

        Allocator & GetAllocator()
        {
            return *m_allocator;
        }

    private:

        Allocator * m_allocator;                    ///< The allocator passed into the constructor.
        void * m_context;                           ///< The context pointer set on the stream. May be NULL.
    };

    /**
        Stream class for writing bitpacked data.
        This class is a wrapper around the bit writer class. Its purpose is to provide unified interface for reading and writing.
        You can determine if you are writing to a stream by calling Stream::IsWriting inside your templated serialize method.
        This is evaluated at compile time, letting the compiler generate optimized serialize functions without the hassle of maintaining separate read and write functions.
        IMPORTANT: Generally, you don't call methods on this class directly. Use the serialize_* macros instead. See test/shared.h for some examples.
        @see BitWriter
     */

    class WriteStream : public BaseStream
    {
    public:

        enum { IsWriting = 1 };
        enum { IsReading = 0 };

        /**
            Write stream constructor.
            @param buffer The buffer to write to.
            @param bytes The number of bytes in the buffer. Must be a multiple of four.
            @param allocator The allocator to use for stream allocations. This lets you dynamically allocate memory as you read and write packets.
         */

        WriteStream( Allocator & allocator, uint8_t * buffer, int bytes ) : BaseStream( allocator ), m_writer( buffer, bytes ) {}

        /**
            Serialize an integer (write).
            @param value The integer value in [min,max].
            @param min The minimum value.
            @param max The maximum value.
            @returns Always returns true. All checking is performed by debug asserts only on write.
         */

        bool SerializeInteger( int32_t value, int32_t min, int32_t max )
        {
            yojimbo_assert( min < max );
            yojimbo_assert( value >= min );
            yojimbo_assert( value <= max );
            const int bits = bits_required( min, max );
            uint32_t unsigned_value = value - min;
            m_writer.WriteBits( unsigned_value, bits );
            return true;
        }

        /**
            Serialize an varint (write).
            @param value The integer value.
            @returns Always returns true. All checking is performed by debug asserts only on write.
         */

        bool SerializeVarint32( uint32_t value )
        {
            uint8_t data[5];
            const int bytes = yojimbo_put_varint( data, value );
            yojimbo_assert( bytes >= 0 );
            for ( int i = 0; i < bytes; ++i )
                m_writer.WriteBits( data[i], 8 );
            return true;
        }

        /**
            Serialize an varint64 (write).
            @param value The integer value.
            @returns Always returns true. All checking is performed by debug asserts only on write.
        */

        bool SerializeVarint64( uint64_t value )
        {
            uint8_t data[9];
            const int bytes = yojimbo_put_varint( data, value );
            yojimbo_assert( bytes >= 0 );
            for ( int i = 0; i < bytes; ++i )
                m_writer.WriteBits( data[i], 8 );
            return true;
        }

        /**
            Serialize a number of bits (write).
            @param value The unsigned integer value to serialize. Must be in range [0,(1<<bits)-1].
            @param bits The number of bits to write in [1,32].
            @returns Always returns true. All checking is performed by debug asserts on write.
         */

        bool SerializeBits( uint32_t value, int bits )
        {
            yojimbo_assert( bits > 0 );
            yojimbo_assert( bits <= 32 );
            m_writer.WriteBits( value, bits );
            return true;
        }

        /**
            Serialize an array of bytes (write).
            @param data Array of bytes to be written.
            @param bytes The number of bytes to write.
            @returns Always returns true. All checking is performed by debug asserts on write.
         */

        bool SerializeBytes( const uint8_t * data, int bytes )
        {
            yojimbo_assert( data );
            yojimbo_assert( bytes >= 0 );
            SerializeAlign();
            m_writer.WriteBytes( data, bytes );
            return true;
        }

        /**
            Serialize an align (write).
            @returns Always returns true. All checking is performed by debug asserts on write.
         */

        bool SerializeAlign()
        {
            m_writer.WriteAlign();
            return true;
        }

        /**
            If we were to write an align right now, how many bits would be required?
            @returns The number of zero pad bits required to achieve byte alignment in [0,7].
         */

        int GetAlignBits() const
        {
            return m_writer.GetAlignBits();
        }

        /**
            Serialize a safety check to the stream (write).
            Safety checks help track down desyncs. A check is written to the stream, and on the other side if the check is not present it asserts and fails the serialize.
            @returns Always returns true. All checking is performed by debug asserts on write.
         */

        bool SerializeCheck()
        {
#if YOJIMBO_SERIALIZE_CHECKS
            SerializeAlign();
            SerializeBits( SerializeCheckValue, 32 );
#else // #if YOJIMBO_SERIALIZE_CHECKS
            (void)string;
#endif // #if YOJIMBO_SERIALIZE_CHECKS
            return true;
        }

        /**
            Flush the stream to memory after you finish writing.
            Always call this after you finish writing and before you call WriteStream::GetData, or you'll potentially truncate the last dword of data you wrote.
            @see BitWriter::FlushBits
         */

        void Flush()
        {
            m_writer.FlushBits();
        }

        /**
            Get a pointer to the data written by the stream.
            IMPORTANT: Call WriteStream::Flush before you call this function!
            @returns A pointer to the data written by the stream
         */

        const uint8_t * GetData() const
        {
            return m_writer.GetData();
        }

        /**
            How many bytes have been written so far?
            @returns Number of bytes written. This is effectively the packet size.
         */

        int GetBytesProcessed() const
        {
            return m_writer.GetBytesWritten();
        }

        /**
            Get number of bits written so far.
            @returns Number of bits written.
         */

        int GetBitsProcessed() const
        {
            return m_writer.GetBitsWritten();
        }

    private:

        BitWriter m_writer;                 ///< The bit writer used for all bitpacked write operations.
    };

    /**
        Stream class for reading bitpacked data.
        This class is a wrapper around the bit reader class. Its purpose is to provide unified interface for reading and writing.
        You can determine if you are reading from a stream by calling Stream::IsReading inside your templated serialize method.
        This is evaluated at compile time, letting the compiler generate optimized serialize functions without the hassle of maintaining separate read and write functions.
        IMPORTANT: Generally, you don't call methods on this class directly. Use the serialize_* macros instead. See test/shared.h for some examples.
        @see BitReader
     */

    class ReadStream : public BaseStream
    {
    public:

        enum { IsWriting = 0 };
        enum { IsReading = 1 };

        /**
            Read stream constructor.
            @param buffer The buffer to read from.
            @param bytes The number of bytes in the buffer. May be a non-multiple of four, however if it is, the underlying buffer allocated should be large enough to read the any remainder bytes as a dword.
            @param allocator The allocator to use for stream allocations. This lets you dynamically allocate memory as you read and write packets.
         */

        ReadStream( Allocator & allocator, const uint8_t * buffer, int bytes ) : BaseStream( allocator ), m_reader( buffer, bytes ) {}

        /**
            Serialize an integer (read).
            @param value The integer value read is stored here. It is guaranteed to be in [min,max] if this function succeeds.
            @param min The minimum allowed value.
            @param max The maximum allowed value.
            @returns Returns true if the serialize succeeded and the value is in the correct range. False otherwise.
         */

        bool SerializeInteger( int32_t & value, int32_t min, int32_t max )
        {
            yojimbo_assert( min < max );
            const int bits = bits_required( min, max );
            if ( m_reader.WouldReadPastEnd( bits ) )
                return false;
            uint32_t unsigned_value = m_reader.ReadBits( bits );
            value = (int32_t) unsigned_value + min;
            return true;
        }

        /**
            Serialize a varint32 (read).
            @param value The integer value read is stored here.
            @returns Returns true if the serialize succeeded and the value is in the correct range. False otherwise.
        */

        bool SerializeVarint32( uint32_t & value )
        {
            int i = 0;
            uint8_t data[6];
            uint32_t read_value;
            do {
                if ( m_reader.WouldReadPastEnd( 8 ) )
                    return false;
                read_value = m_reader.ReadBits( 8 );
                data[ i++ ] = read_value;
            } while (i < 5 && (read_value >> 7) != 0 );
            data[i] = 0;
            yojimbo_get_varint32( data, &value );
            return true;
        }

        /**
            Serialize a varint64 (read).
            @param value The integer value read is stored here.
            @returns Returns true if the serialize succeeded and the value is in the correct range. False otherwise.
        */

        bool SerializeVarint64( uint64_t & value )
        {
            int i = 0;
            uint8_t data[10];
            uint32_t read_value;
            do {
                if ( m_reader.WouldReadPastEnd( 8 ) )
                    return false;
                read_value = m_reader.ReadBits( 8 );
                data[ i++ ] = read_value;
            } while (i < 9 && (read_value >> 7) != 0 );
            data[i] = 0;
            yojimbo_get_varint( data, &value );
            return true;
        }

        /**
            Serialize a number of bits (read).
            @param value The integer value read is stored here. Will be in range [0,(1<<bits)-1].
            @param bits The number of bits to read in [1,32].
            @returns Returns true if the serialize read succeeded, false otherwise.
         */

        bool SerializeBits( uint32_t & value, int bits )
        {
            yojimbo_assert( bits > 0 );
            yojimbo_assert( bits <= 32 );
            if ( m_reader.WouldReadPastEnd( bits ) )
                return false;
            uint32_t read_value = m_reader.ReadBits( bits );
            value = read_value;
            return true;
        }

        /**
            Serialize an array of bytes (read).
            @param data Array of bytes to read.
            @param bytes The number of bytes to read.
            @returns Returns true if the serialize read succeeded. False otherwise.
         */

        bool SerializeBytes( uint8_t * data, int bytes )
        {
            if ( !SerializeAlign() )
                return false;
            if ( m_reader.WouldReadPastEnd( bytes * 8 ) )
                return false;
            m_reader.ReadBytes( data, bytes );
            return true;
        }

        /**
            Serialize an align (read).
            @returns Returns true if the serialize read succeeded. False otherwise.
         */

        bool SerializeAlign()
        {
            const int alignBits = m_reader.GetAlignBits();
            if ( m_reader.WouldReadPastEnd( alignBits ) )
                return false;
            if ( !m_reader.ReadAlign() )
                return false;
            return true;
        }

        /**
            If we were to read an align right now, how many bits would we need to read?
            @returns The number of zero pad bits required to achieve byte alignment in [0,7].
         */

        int GetAlignBits() const
        {
            return m_reader.GetAlignBits();
        }

        /**
            Serialize a safety check from the stream (read).
            Safety checks help track down desyncs. A check is written to the stream, and on the other side if the check is not present it asserts and fails the serialize.
            @returns Returns true if the serialize check passed. False otherwise.
         */

        bool SerializeCheck()
        {
#if YOJIMBO_SERIALIZE_CHECKS
            if ( !SerializeAlign() )
                return false;
            uint32_t value = 0;
            if ( !SerializeBits( value, 32 ) )
                return false;
            if ( value != SerializeCheckValue )
            {
                yojimbo_printf( YOJIMBO_LOG_LEVEL_DEBUG, "serialize check failed: expected %x, got %x\n", SerializeCheckValue, value );
            }
            return value == SerializeCheckValue;
#else // #if YOJIMBO_SERIALIZE_CHECKS
            return true;
#endif // #if YOJIMBO_SERIALIZE_CHECKS
        }

        /**
            Get number of bits read so far.
            @returns Number of bits read.
         */

        int GetBitsProcessed() const
        {
            return m_reader.GetBitsRead();
        }

        /**
            How many bytes have been read so far?
            @returns Number of bytes read. Effectively this is the number of bits read, rounded up to the next byte where necessary.
         */

        int GetBytesProcessed() const
        {
            return ( m_reader.GetBitsRead() + 7 ) / 8;
        }

    private:

        BitReader m_reader;             ///< The bit reader used for all bitpacked read operations.
    };

    /**
        Stream class for estimating how many bits it would take to serialize something.
        This class acts like a bit writer (IsWriting is 1, IsReading is 0), but instead of writing data, it counts how many bits would be written.
        It's used by the connection channel classes to work out how many messages will fit in the channel packet budget.
        Note that when the serialization includes alignment to byte (see MeasureStream::SerializeAlign), this is an estimate and not an exact measurement. The estimate is guaranteed to be conservative.
        @see BitWriter
        @see BitReader
     */

    class MeasureStream : public BaseStream
    {
    public:

        enum { IsWriting = 1 };
        enum { IsReading = 0 };

        /**
            Measure stream constructor.
            @param allocator The allocator to use for stream allocations. This lets you dynamically allocate memory as you read and write packets.
         */

        explicit MeasureStream( Allocator & allocator ) : BaseStream( allocator ), m_bitsWritten(0) {}

        /**
            Serialize an integer (measure).
            @param value The integer value to write. Not actually used or checked.
            @param min The minimum value.
            @param max The maximum value.
            @returns Always returns true. All checking is performed by debug asserts only on measure.
         */

        bool SerializeInteger( int32_t value, int32_t min, int32_t max )
        {
            (void) value;
            yojimbo_assert( min < max );
            yojimbo_assert( value >= min );
            yojimbo_assert( value <= max );
            const int bits = bits_required( min, max );
            m_bitsWritten += bits;
            return true;
        }

        /**
            Serialize an varint32 (measure).
            @param value The integer value to write. Not actually used or checked.
            @returns Always returns true. All checking is performed by debug asserts only on measure.
         */

        bool SerializeVarint32( int32_t value )
        {
            const int bits = yojimbo_measure_varint( value ) * 8;
            m_bitsWritten += bits;
            return true;
        }

        /**
            Serialize an varint64 (measure).
            @param value The integer value to write. Not actually used or checked.
            @returns Always returns true. All checking is performed by debug asserts only on measure.
        */

        bool SerializeVarint64( int64_t value )
        {
            const int bits = yojimbo_measure_varint( value ) * 8;
            m_bitsWritten += bits;
            return true;
        }

        /**
            Serialize a number of bits (write).
            @param value The unsigned integer value to serialize. Not actually used or checked.
            @param bits The number of bits to write in [1,32].
            @returns Always returns true. All checking is performed by debug asserts on write.
         */

        bool SerializeBits( uint32_t value, int bits )
        {
            (void) value;
            yojimbo_assert( bits > 0 );
            yojimbo_assert( bits <= 32 );
            m_bitsWritten += bits;
            return true;
        }

        /**
            Serialize an array of bytes (measure).
            @param data Array of bytes to 'write'. Not actually used.
            @param bytes The number of bytes to 'write'.
            @returns Always returns true. All checking is performed by debug asserts on write.
         */

        bool SerializeBytes( const uint8_t * data, int bytes )
        {
            (void) data;
            SerializeAlign();
            m_bitsWritten += bytes * 8;
            return true;
        }

        /**
            Serialize an align (measure).
            @returns Always returns true. All checking is performed by debug asserts on write.
         */

        bool SerializeAlign()
        {
            const int alignBits = GetAlignBits();
            m_bitsWritten += alignBits;
            return true;
        }

        /**
            If we were to write an align right now, how many bits would be required?
            IMPORTANT: Since the number of bits required for alignment depends on where an object is written in the final bit stream, this measurement is conservative.
            @returns Always returns worst case 7 bits.
         */

        int GetAlignBits() const
        {
            return 7;
        }

        /**
            Serialize a safety check to the stream (measure).
            @returns Always returns true. All checking is performed by debug asserts on write.
         */

        bool SerializeCheck()
        {
#if YOJIMBO_SERIALIZE_CHECKS
            SerializeAlign();
            m_bitsWritten += 32;
#endif // #if YOJIMBO_SERIALIZE_CHECKS
            return true;
        }

        /**
            Get number of bits written so far.
            @returns Number of bits written.
         */

        int GetBitsProcessed() const
        {
            return m_bitsWritten;
        }

        /**
            How many bytes have been written so far?
            @returns Number of bytes written.
         */

        int GetBytesProcessed() const
        {
            return ( m_bitsWritten + 7 ) / 8;
        }

    private:

        int m_bitsWritten;              ///< Counts the number of bits written.
    };

    /**
        Serialize integer value (read/write/measure).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
        @param value The integer value to serialize in [min,max].
        @param min The minimum value.
        @param max The maximum value.
     */

    #define serialize_int( stream, value, min, max )                    \
        do                                                              \
        {                                                               \
            yojimbo_assert( min < max );                                \
            int32_t int32_value = 0;                                    \
            if ( Stream::IsWriting )                                    \
            {                                                           \
                yojimbo_assert( int64_t(value) >= int64_t(min) );       \
                yojimbo_assert( int64_t(value) <= int64_t(max) );       \
                int32_value = (int32_t) value;                          \
            }                                                           \
            if ( !stream.SerializeInteger( int32_value, min, max ) )    \
            {                                                           \
                return false;                                           \
            }                                                           \
            if ( Stream::IsReading )                                    \
            {                                                           \
                value = int32_value;                                    \
                if ( int64_t(value) < int64_t(min) ||                   \
                     int64_t(value) > int64_t(max) )                    \
                {                                                       \
                    return false;                                       \
                }                                                       \
            }                                                           \
        } while (0)


    /**
         Serialize variable integer value to the stream (read/write/measure).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
        @param value The integer value to serialize.
    */

    #define serialize_varint32( stream, value )                         \
        do                                                              \
        {                                                               \
            uint32_t int32_value = 0;                                   \
            if ( Stream::IsWriting )                                    \
            {                                                           \
                int32_value = (uint32_t) value;                         \
            }                                                           \
            if ( !stream.SerializeVarint32( int32_value ) )             \
            {                                                           \
                return false;                                           \
            }                                                           \
            if ( Stream::IsReading )                                    \
            {                                                           \
                value = int32_value;                                    \
            }                                                           \
        } while (0)

    /**
         Serialize variable integer value to the stream (read/write/measure).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
        @param value The 64 bit integer value to serialize.
    */

    #define serialize_varint64( stream, value )                         \
        do                                                              \
        {                                                               \
            uint64_t int64_value = 0;                                   \
            if ( Stream::IsWriting )                                    \
            {                                                           \
                int64_value = (uint64_t) value;                         \
            }                                                           \
            if ( !stream.SerializeVarint64( int64_value ) )             \
            {                                                           \
                return false;                                           \
            }                                                           \
            if ( Stream::IsReading )                                    \
            {                                                           \
                value = int64_value;                                    \
            }                                                           \
        } while (0)

    /**
        Serialize bits to the stream (read/write/measure).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
        @param value The unsigned integer value to serialize.
        @param bits The number of bits to serialize in [1,32].
     */

    #define serialize_bits( stream, value, bits )                       \
        do                                                              \
        {                                                               \
            yojimbo_assert( bits > 0 );                                 \
            yojimbo_assert( bits <= 32 );                               \
            uint32_t uint32_value = 0;                                  \
            if ( Stream::IsWriting )                                    \
            {                                                           \
                uint32_value = (uint32_t) value;                        \
            }                                                           \
            if ( !stream.SerializeBits( uint32_value, bits ) )          \
            {                                                           \
                return false;                                           \
            }                                                           \
            if ( Stream::IsReading )                                    \
            {                                                           \
                value = uint32_value;                                   \
            }                                                           \
        } while (0)

    /**
        Serialize a boolean value to the stream (read/write/measure).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
        @param value The boolean value to serialize.
     */

    #define serialize_bool( stream, value )                             \
        do                                                              \
        {                                                               \
            uint32_t uint32_bool_value = 0;                             \
            if ( Stream::IsWriting )                                    \
            {                                                           \
                uint32_bool_value = value ? 1 : 0;                      \
            }                                                           \
            serialize_bits( stream, uint32_bool_value, 1 );             \
            if ( Stream::IsReading )                                    \
            {                                                           \
                value = uint32_bool_value ? true : false;               \
            }                                                           \
        } while (0)

    template <typename Stream> bool serialize_float_internal( Stream & stream, float & value )
    {
        uint32_t int_value;
        if ( Stream::IsWriting )
        {
            memcpy( &int_value, &value, 4 );
        }
        bool result = stream.SerializeBits( int_value, 32 );
        if ( Stream::IsReading )
        {
            memcpy( &value, &int_value, 4 );
        }
        return result;
    }

    /**
        Serialize floating point value (read/write/measure).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
        @param value The float value to serialize.
     */

    #define serialize_float( stream, value )                                        \
        do                                                                          \
        {                                                                           \
            if ( !yojimbo::serialize_float_internal( stream, value ) )              \
            {                                                                       \
                return false;                                                       \
            }                                                                       \
        } while (0)

    /**
        Serialize a 32 bit unsigned integer to the stream (read/write/measure).
        This is a helper macro to make unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
        @param value The unsigned 32 bit integer value to serialize.
     */

    #define serialize_uint32( stream, value ) serialize_bits( stream, value, 32 );

    template <typename Stream> bool serialize_uint64_internal( Stream & stream, uint64_t & value )
    {
        uint32_t hi = 0, lo = 0;
        if ( Stream::IsWriting )
        {
            lo = value & 0xFFFFFFFF;
            hi = value >> 32;
        }
        serialize_bits( stream, lo, 32 );
        serialize_bits( stream, hi, 32 );
        if ( Stream::IsReading )
        {
            value = ( uint64_t(hi) << 32 ) | lo;
        }
        return true;
    }

    /**
        Serialize a 64 bit unsigned integer to the stream (read/write/measure).
        This is a helper macro to make unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
        @param value The unsigned 64 bit integer value to serialize.
     */

    #define serialize_uint64( stream, value )                                       \
        do                                                                          \
        {                                                                           \
            if ( !yojimbo::serialize_uint64_internal( stream, value ) )             \
                return false;                                                       \
        } while (0)

    template <typename Stream> bool serialize_double_internal( Stream & stream, double & value )
    {
        union DoubleInt
        {
            double double_value;
            uint64_t int_value;
        };
        DoubleInt tmp = { 0 };
        if ( Stream::IsWriting )
        {
            tmp.double_value = value;
        }
        serialize_uint64( stream, tmp.int_value );
        if ( Stream::IsReading )
        {
            value = tmp.double_value;
        }
        return true;
    }

    /**
        Serialize double precision floating point value to the stream (read/write/measure).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
        @param value The double precision floating point value to serialize.
     */

    #define serialize_double( stream, value )                                       \
        do                                                                          \
        {                                                                           \
            if ( !yojimbo::serialize_double_internal( stream, value ) )             \
            {                                                                       \
                return false;                                                       \
            }                                                                       \
        } while (0)

    template <typename Stream> bool serialize_bytes_internal( Stream & stream, uint8_t * data, int bytes )
    {
        return stream.SerializeBytes( data, bytes );
    }

    /**
        Serialize an array of bytes to the stream (read/write/measure).
        This is a helper macro to make unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
        @param data Pointer to the data to be serialized.
        @param bytes The number of bytes to serialize.
     */

    #define serialize_bytes( stream, data, bytes )                                  \
        do                                                                          \
        {                                                                           \
            if ( !yojimbo::serialize_bytes_internal( stream, data, bytes ) )        \
            {                                                                       \
                return false;                                                       \
            }                                                                       \
        } while (0)

    template <typename Stream> bool serialize_string_internal( Stream & stream, char * string, int buffer_size )
    {
        int length = 0;
        if ( Stream::IsWriting )
        {
            length = (int) strlen( string );
            yojimbo_assert( length < buffer_size );
        }
        serialize_int( stream, length, 0, buffer_size - 1 );
        serialize_bytes( stream, (uint8_t*)string, length );
        if ( Stream::IsReading )
        {
            string[length] = '\0';
        }
        return true;
    }

    /**
        Serialize a string to the stream (read/write/measure).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
        @param string The string to serialize write/measure. Pointer to buffer to be filled on read.
        @param buffer_size The size of the string buffer. String with terminating null character must fit into this buffer.
     */

    #define serialize_string( stream, string, buffer_size )                                 \
        do                                                                                  \
        {                                                                                   \
            if ( !yojimbo::serialize_string_internal( stream, string, buffer_size ) )       \
            {                                                                               \
                return false;                                                               \
            }                                                                               \
        } while (0)

    template <typename Stream> bool serialize_stl_string_internal( Stream & stream, std::string& string )
    {
		const int max_length = 1024; // TODO: this is super arbitrary
        int length = 0;
        if ( Stream::IsWriting ) {
            length = string.size();
            yojimbo_assert( length <= max_length );
        }

        serialize_int( stream, length, 0, length );

        if ( Stream::IsReading ) {
			string.resize(length);
        }
        serialize_bytes( stream, (uint8_t*)string.data(), length );
        return true;
    }

    /**
        Serialize a standard library string to the stream (read/write/measure).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
        @param string The string to serialize write/measure. Pointer to buffer to be filled on read.
        @param buffer_size The size of the string buffer. String with terminating null character must fit into this buffer.
     */

    #define serialize_stl_string( stream, string )                                          \
        do                                                                                  \
        {                                                                                   \
            if ( !yojimbo::serialize_stl_string_internal( stream, string ) )                \
            {                                                                               \
                return false;                                                               \
            }                                                                               \
        } while (0)

    /**
        Serialize an alignment to the stream (read/write/measure).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
     */

    #define serialize_align( stream )                                                       \
        do                                                                                  \
        {                                                                                   \
            if ( !stream.SerializeAlign() )                                                 \
            {                                                                               \
                return false;                                                               \
            }                                                                               \
        } while (0)

    /**
        Serialize a safety check to the stream (read/write/measure).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
     */

    #define serialize_check( stream )                                                       \
        do                                                                                  \
        {                                                                                   \
            if ( !stream.SerializeCheck() )                                                 \
            {                                                                               \
                return false;                                                               \
            }                                                                               \
        } while (0)

    /**
        Serialize an object to the stream (read/write/measure).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
        @param object The object to serialize. Must have a serialize method on it.
     */

    #define serialize_object( stream, object )                                              \
        do                                                                                  \
        {                                                                                   \
            if ( !object.Serialize( stream ) )                                              \
            {                                                                               \
                return false;                                                               \
            }                                                                               \
        }                                                                                   \
        while(0)


    template <typename Stream, typename T> bool serialize_int_relative_internal( Stream & stream, T previous, T & current )
    {
        uint32_t difference = 0;
        if ( Stream::IsWriting )
        {
            yojimbo_assert( previous < current );
            difference = current - previous;
        }

        bool oneBit = false;
        if ( Stream::IsWriting )
        {
            oneBit = difference == 1;
        }
        serialize_bool( stream, oneBit );
        if ( oneBit )
        {
            if ( Stream::IsReading )
            {
                current = previous + 1;
            }
            return true;
        }

        bool twoBits = false;
        if ( Stream::IsWriting )
        {
            twoBits = difference <= 6;
        }
        serialize_bool( stream, twoBits );
        if ( twoBits )
        {
            serialize_int( stream, difference, 2, 6 );
            if ( Stream::IsReading )
            {
                current = previous + difference;
            }
            return true;
        }

        bool fourBits = false;
        if ( Stream::IsWriting )
        {
            fourBits = difference <= 23;
        }
        serialize_bool( stream, fourBits );
        if ( fourBits )
        {
            serialize_int( stream, difference, 7, 23 );
            if ( Stream::IsReading )
            {
                current = previous + difference;
            }
            return true;
        }

        bool eightBits = false;
        if ( Stream::IsWriting )
        {
            eightBits = difference <= 280;
        }
        serialize_bool( stream, eightBits );
        if ( eightBits )
        {
            serialize_int( stream, difference, 24, 280 );
            if ( Stream::IsReading )
            {
                current = previous + difference;
            }
            return true;
        }

        bool twelveBits = false;
        if ( Stream::IsWriting )
        {
            twelveBits = difference <= 4377;
        }
        serialize_bool( stream, twelveBits );
        if ( twelveBits )
        {
            serialize_int( stream, difference, 281, 4377 );
            if ( Stream::IsReading )
            {
                current = previous + difference;
            }
            return true;
        }

        bool sixteenBits = false;
        if ( Stream::IsWriting )
        {
            sixteenBits = difference <= 69914;
        }
        serialize_bool( stream, sixteenBits );
        if ( sixteenBits )
        {
            serialize_int( stream, difference, 4378, 69914 );
            if ( Stream::IsReading )
            {
                current = previous + difference;
            }
            return true;
        }

        uint32_t value = current;
        serialize_uint32( stream, value );
        if ( Stream::IsReading )
        {
            current = value;
        }

        return true;
    }

    /**
        Serialize an integer value relative to another (read/write/measure).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
        @param previous The previous integer value.
        @param current The current integer value.
     */

    #define serialize_int_relative( stream, previous, current )                             \
        do                                                                                  \
        {                                                                                   \
            if ( !yojimbo::serialize_int_relative_internal( stream, previous, current ) )   \
            {                                                                               \
                return false;                                                               \
            }                                                                               \
        } while (0)

    template <typename Stream> bool serialize_ack_relative_internal( Stream & stream, uint16_t sequence, uint16_t & ack )
    {
        int ack_delta = 0;
        bool ack_in_range = false;
        if ( Stream::IsWriting )
        {
            if ( ack < sequence )
            {
                ack_delta = sequence - ack;
            }
            else
            {
                ack_delta = (int)sequence + 65536 - ack;
            }
            yojimbo_assert( ack_delta > 0 );
            yojimbo_assert( uint16_t( sequence - ack_delta ) == ack );
            ack_in_range = ack_delta <= 64;
        }
        serialize_bool( stream, ack_in_range );
        if ( ack_in_range )
        {
            serialize_int( stream, ack_delta, 1, 64 );
            if ( Stream::IsReading )
            {
                ack = sequence - ack_delta;
            }
        }
        else
        {
            serialize_bits( stream, ack, 16 );
        }
        return true;
    }

    /**
        Serialize an ack relative to the current sequence number (read/write/measure).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
        @param sequence The current sequence number.
        @param ack The ack sequence number, which is typically near the current sequence number.
     */

    #define serialize_ack_relative( stream, sequence, ack  )                                        \
        do                                                                                          \
        {                                                                                           \
            if ( !yojimbo::serialize_ack_relative_internal( stream, sequence, ack ) )               \
            {                                                                                       \
                return false;                                                                       \
            }                                                                                       \
        } while (0)

    template <typename Stream> bool serialize_sequence_relative_internal( Stream & stream, uint16_t sequence1, uint16_t & sequence2 )
    {
        if ( Stream::IsWriting )
        {
            uint32_t a = sequence1;
            uint32_t b = sequence2 + ( ( sequence1 > sequence2 ) ? 65536 : 0 );
            serialize_int_relative( stream, a, b );
        }
        else
        {
            uint32_t a = sequence1;
            uint32_t b = 0;
            serialize_int_relative( stream, a, b );
            if ( b >= 65536 )
            {
                b -= 65536;
            }
            sequence2 = uint16_t( b );
        }

        return true;
    }

    /**
        Serialize a sequence number relative to another (read/write/measure).
        This is a helper macro to make writing unified serialize functions easier.
        Serialize macros returns false on error so we don't need to use exceptions for error handling on read. This is an important safety measure because packet data comes from the network and may be malicious.
        IMPORTANT: This macro must be called inside a templated serialize function with template \<typename Stream\>. The serialize method must have a bool return value.
        @param stream The stream object. May be a read, write or measure stream.
        @param sequence1 The first sequence number to serialize relative to.
        @param sequence2 The second sequence number to be encoded relative to the first.
     */

    #define serialize_sequence_relative( stream, sequence1, sequence2 )                             \
        do                                                                                          \
        {                                                                                           \
            if ( !yojimbo::serialize_sequence_relative_internal( stream, sequence1, sequence2 ) )   \
            {                                                                                       \
                return false;                                                                       \
            }                                                                                       \
        } while (0)

    // read macros corresponding to each serialize_*. useful when you want separate read and write functions.

    #define read_bits( stream, value, bits )                                                \
    do                                                                                      \
    {                                                                                       \
        yojimbo_assert( bits > 0 );                                                         \
        yojimbo_assert( bits <= 32 );                                                       \
        uint32_t uint32_value= 0;                                                           \
        if ( !stream.SerializeBits( uint32_value, bits ) )                                  \
        {                                                                                   \
            return false;                                                                   \
        }                                                                                   \
        value = uint32_value;                                                               \
    } while (0)

    #define read_int( stream, value, min, max )                                             \
        do                                                                                  \
        {                                                                                   \
            yojimbo_assert( min < max );                                                    \
            int32_t int32_value = 0;                                                        \
            if ( !stream.SerializeInteger( int32_value, min, max ) )                        \
            {                                                                               \
                return false;                                                               \
            }                                                                               \
            value = int32_value;                                                            \
            if ( value < min || value > max )                                               \
            {                                                                               \
                return false;                                                               \
            }                                                                               \
        } while (0)

    #define read_bool( stream, value ) read_bits( stream, value, 1 )

    #define read_float                  serialize_float
    #define read_uint32                 serialize_uint32
    #define read_uint64                 serialize_uint64
    #define read_double                 serialize_double
    #define read_bytes                  serialize_bytes
    #define read_string                 serialize_string
    #define read_align                  serialize_align
    #define read_check                  serialize_check
    #define read_object                 serialize_object
    #define read_address                serialize_address
    #define read_int_relative           serialize_int_relative
    #define read_ack_relative           serialize_ack_relative
    #define read_sequence_relative      serialize_sequence_relative

    // write macros corresponding to each serialize_*. useful when you want separate read and write functions for some reason.

    #define write_bits( stream, value, bits )                                               \
        do                                                                                  \
        {                                                                                   \
            yojimbo_assert( bits > 0 );                                                     \
            yojimbo_assert( bits <= 32 );                                                   \
            uint32_t uint32_value = (uint32_t) value;                                       \
            if ( !stream.SerializeBits( uint32_value, bits ) )                              \
            {                                                                               \
                return false;                                                               \
            }                                                                               \
        } while (0)

    #define write_int( stream, value, min, max )                                            \
        do                                                                                  \
        {                                                                                   \
            yojimbo_assert( min < max );                                                    \
            yojimbo_assert( value >= min );                                                 \
            yojimbo_assert( value <= max );                                                 \
            int32_t int32_value = (int32_t) value;                                          \
            if ( !stream.SerializeInteger( int32_value, min, max ) )                        \
                return false;                                                               \
        } while (0)

    #define write_float                 serialize_float
    #define write_uint32                serialize_uint32
    #define write_uint64                serialize_uint64
    #define write_double                serialize_double
    #define write_bytes                 serialize_bytes
    #define write_string                serialize_string
    #define write_align                 serialize_align
    #define write_check                 serialize_check
    #define write_object                serialize_object
    #define write_address               serialize_address
    #define write_int_relative          serialize_int_relative
    #define write_ack_relative          serialize_ack_relative
    #define write_sequence_relative     serialize_sequence_relative

    /**
        Interface for an object that knows how to read, write and measure how many bits it would take up in a bit stream.
        IMPORTANT: Instead of overriding the serialize virtual methods method directly, use the YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS macro in your derived class to override and redirect them to your templated serialize method.
        This way you can implement read and write for your messages in a single method and the C++ compiler takes care of generating specialized read, write and measure implementations for you.
        See tests/shared.h for some examples of this.
        @see ReadStream
        @see WriteStream
        @see MeasureStream
     */

    class Serializable
    {
    public:

        virtual ~Serializable() {}

        /**
            Virtual serialize function (read).
            Reads the object in from a bitstream.
            @param stream The stream to read from.
         */

        virtual bool SerializeInternal( class ReadStream & stream ) = 0;

        /**
            Virtual serialize function (write).
            Writes the object to a bitstream.
            @param stream The stream to write to.
         */

        virtual bool SerializeInternal( class WriteStream & stream ) = 0;

        /**
            Virtual serialize function (measure).
            Quickly measures how many bits the object would take if it were written to a bit stream.
            @param stream The read stream.
         */

        virtual bool SerializeInternal( class MeasureStream & stream ) = 0;
    };

    /**
        Helper macro to define virtual serialize functions for read, write and measure that call into the templated serialize function.
        This helps avoid writing boilerplate code, which is nice when you have lots of hand coded message types.
        See tests/shared.h for examples of usage.
     */

    #define YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS()                                                               \
        bool SerializeInternal( class yojimbo::ReadStream & stream ) { return Serialize( stream ); }            \
        bool SerializeInternal( class yojimbo::WriteStream & stream ) { return Serialize( stream ); }           \
        bool SerializeInternal( class yojimbo::MeasureStream & stream ) { return Serialize( stream ); }


} // yojimbo

#endif // #ifndef YOJIMBO_SERIALIZER_H
