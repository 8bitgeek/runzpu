/****************************************************************************
 * Copyright (c) 2020 Mike Sharkey <mike@pikeaero.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <zpu.h>

#define MEM_SEG_TEXT_BASE   0x00000
#define MEM_SEG_STACK_BASE  0x1F800

#define MEM_SEG_TEXT_SZ     (1024*32)
#define MEM_SEG_STACK_SZ    (1024*2)

static uint32_t mem_seg_text[MEM_SEG_TEXT_SZ/4];
static uint32_t mem_seg_stack[MEM_SEG_STACK_SZ/4];

static zpu_mem_t zpu_mem_seg_text;
static zpu_mem_t zpu_mem_seg_stack;
static zpu_t zpu;

static int debug_trace=(-1);
static bool debug=false;
static uint32_t opcode_count=0;

static void command_line (int argc, char **argv);
static void usage        (const char* exec_name);
static void registers   (zpu_t* zpu);

int main(int argc, char *argv[])
{
    command_line( argc, argv );

    memset(mem_seg_text,0,MEM_SEG_TEXT_SZ);
    memset(mem_seg_stack,0,MEM_SEG_STACK_SZ);
    
    zpu_mem_init( (zpu_mem_t*)NULL, 
                  &zpu_mem_seg_text, 
                  "text", 
                  mem_seg_text, 
                  MEM_SEG_TEXT_BASE, 
                  MEM_SEG_TEXT_SZ,
                  ZPU_MEM_ATTR_RD | ZPU_MEM_ATTR_EX );

    zpu_mem_init( &zpu_mem_seg_text,
                  &zpu_mem_seg_stack,
                  "stack",
                  mem_seg_stack,
                  MEM_SEG_STACK_BASE,
                  MEM_SEG_STACK_SZ,
                  ZPU_MEM_ATTR_RD | ZPU_MEM_ATTR_WR );

    zpu_set_mem(&zpu,&zpu_mem_seg_text);
    
    zpu_reset(&zpu,0x1fff8);
    zpu_execute(&zpu);
}

static void command_line(int argc, char **argv)
{
    int index=1; 

	while (index < argc) {
		if (argv[index][0] != '-') break;

		if (!strcmp(argv[index], "-d")) {
			debug=true;
			++index;
			continue;
		}
        else if (!strcmp(argv[index], "-t")) {
			debug_trace=0;
			++index;
            if ( index < argc && isdigit(*argv[index]) )
            {
                debug_trace=atoi(argv[index]);
                ++index;
            }
			continue;
		}
        else if (!strcmp(argv[index], "-?")) {
			usage(argv[0]);
			exit(0);
		}

		fprintf( stderr, "Invalid: option '%s'\n", argv[index] );
		usage(argv[0]);
		exit(1);
	}
}

static void usage(const char* exec_name)
{
	fprintf( stderr, "Usage: %s [-d] [-t [clks]] <somefile>.s19\n", exec_name );
}

static void registers(zpu_t* zpu)
{
    printf ("PC=%08x SP=%08x TOS=%08x OP=%02x DM=%02x debug=%08x\n", zpu_get_pc(zpu), zpu_get_sp(zpu), zpu_get_tos(zpu), zpu->instruction, zpu->decode_mask, 0);
    fflush(0);
}

extern void zpu_opcode_fetch_notify( zpu_mem_t* zpu_mem, uint32_t va )
{
    ++opcode_count;
}

extern void zpu_breakpoint_handler(zpu_t* zpu)
{
    fprintf( stderr, "breakpoint\n" );
    registers(zpu);
    exit(1);
}

extern void zpu_divzero_handler(zpu_t* zpu)
{
    fprintf( stderr, "divide by zero\n" );
    registers(zpu);
    exit(1);
}

extern void zpu_config_handler(zpu_t* zpu)
{
    fprintf( stderr, "CPU %d\n", zpu_get_cpu(zpu) );
}

extern void zpu_illegal_opcode_handler(zpu_t* zpu)
{
    fprintf( stderr, "illegal opcode\n" );
    registers(zpu);
    exit(1);
}

void zpu_segv_handler(zpu_mem_t* zpu_mem, uint32_t va)
{
    fprintf( stderr, "segment violation 0x%08X\n", va );
    exit(1);
}

extern bool zpu_mem_override_get_uint32 ( zpu_mem_t* zpu_mem, uint32_t va, uint32_t* value )
{
    // The PHI platform UART status port
    if (va == 0x80000024 || va == 0x080A000C)
    {
	    *value = 0x100;
        return true;
    }
    return false;
}

extern bool zpu_mem_override_get_uint16 ( zpu_mem_t* zpu_mem, uint32_t va, uint16_t* value )
{
    /* NOP */
    return false;
}

extern bool zpu_mem_override_get_uint8  ( zpu_mem_t* zpu_mem, uint32_t va, uint8_t* value )
{
    /* NOP */
    return false;
}

extern bool zpu_mem_override_set_uint32 ( zpu_mem_t* zpu_mem, uint32_t va, uint32_t w )
{
    // The PHI platform UART port
    if (va == 0x80000024 || va == 0x080A000C)
    {
    	printf("%c", (char)w);
    	return true;
    }
    return false;
}

extern bool zpu_mem_override_set_uint16 ( zpu_mem_t* zpu_mem, uint32_t va, uint16_t w )
{
    /* NOP */
    return false;
}

extern bool zpu_mem_override_set_uint8  ( zpu_mem_t* zpu_mem, uint32_t va, uint8_t w )
{
    /* NOP */
    return false;
}


