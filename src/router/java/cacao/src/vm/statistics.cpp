/* src/vm/statistics.cpp - global variables for statistics

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/


#include <stdint.h>

#include "vm/rt-timing.hpp"
#include "vm/statistics.hpp"
#include "vm/types.hpp"


/* global variables ***********************************************************/

static s4 loadingtime_recursion = 0;
static s4 compilingtime_recursion = 0;

RT_REGISTER_GROUP(legacy_group,"legacy group","legacy group")
RT_REGISTER_GROUP_TIMER(loadingtime,"loading time", "Time for loading classes",legacy_group)
RT_REGISTER_GROUP_TIMER(compilingtime,"compiling time", "Time for compiling code",legacy_group)


/* loadingtime_stop ************************************************************

   XXX

*******************************************************************************/

void loadingtime_start(void)
{
	loadingtime_recursion++;

	if (loadingtime_recursion == 1) {
		RT_TIMER_START(loadingtime);
	}
}


/* loadingtime_stop ************************************************************

   XXX

*******************************************************************************/

void loadingtime_stop(void)
{
	if (loadingtime_recursion == 1) {
		RT_TIMER_STOP(loadingtime);
	}

	loadingtime_recursion--;
}


/* compilingtime_stop **********************************************************

   XXX

*******************************************************************************/

void compilingtime_start(void)
{
	compilingtime_recursion++;

	if (compilingtime_recursion == 1) {
		RT_TIMER_START(compilingtime);
	}
}

/* compilingtime_stop **********************************************************

   XXX

*******************************************************************************/

void compilingtime_stop(void)
{
	if (compilingtime_recursion == 1) {
		RT_TIMER_STOP(compilingtime);
	}

	compilingtime_recursion--;
}

/* print_stats *****************************************************************

   outputs detailed compiler statistics

*******************************************************************************/

#if 0
void print_stats(void)
{
	s4    i;
	float f;
	s4    sum;


	// DONE
	dolog("Number of JIT compiler calls: %6d", count_jit_calls);
	// DONE
	dolog("Number of compiled methods:   %6d", count_methods);

	// DONE
	dolog("Number of compiled basic blocks:               %6d",
		  count_basic_blocks);
	// DONE
	dolog("Number of max. basic blocks per method:        %6d",
		  count_max_basic_blocks);

	// DONE
	dolog("Number of compiled JavaVM instructions:        %6d",
		  count_javainstr);
	// DONE
	dolog("Number of max. JavaVM instructions per method: %6d",
		  count_max_javainstr);
	// PARTLY DONE
	dolog("Size of compiled JavaVM instructions:          %6d(%d)",
		  count_javacodesize, count_javacodesize - count_methods * 18);

	// DONE
	dolog("Size of compiled Exception Tables:      %d", count_javaexcsize);
	// XXX 4 byte instructions hardcoded oO. I guess this is no longer valid.
	dolog("Number of Machine-Instructions: %d", count_code_len >> 2);
	// DONE
	dolog("Number of Spills (write to memory) <all [i/l/a|flt|dbl]>: %d [%d|%d|%d]",
		count_spills_write_ila + count_spills_write_flt + count_spills_write_dbl,
		count_spills_write_ila, count_spills_write_flt, count_spills_write_dbl);
	// DONE
	dolog("Number of Spills (read from memory) <all [i/l/a|flt|dbl]>: %d [%d|%d|%d]",
		count_spills_read_ila + count_spills_read_flt + count_spills_read_dbl,
		count_spills_read_ila, count_spills_read_flt, count_spills_read_dbl);
	// NOT used?!
	dolog("Number of Activ    Pseudocommands: %6d", count_pcmd_activ);
	// NOT used?!
	dolog("Number of Drop     Pseudocommands: %6d", count_pcmd_drop);
	// DONE
	dolog("Number of Const    Pseudocommands: %6d (zero:%5d)",
		  count_pcmd_load, count_pcmd_zero);
	// NOT used?!
	dolog("Number of ConstAlu Pseudocommands: %6d (cmp: %5d, store:%5d)",
		  count_pcmd_const_alu, count_pcmd_const_bra, count_pcmd_const_store);
	// NOT used?!
	dolog("Number of Move     Pseudocommands: %6d", count_pcmd_move);
	// DONE
	dolog("Number of Load     Pseudocommands: %6d", count_load_instruction);
	// DONE
	// count_pcmd_store_comb NOT used?!
	dolog("Number of Store    Pseudocommands: %6d (combined: %5d)",
		  count_pcmd_store, count_pcmd_store - count_pcmd_store_comb);
	// DONE
	dolog("Number of OP       Pseudocommands: %6d", count_pcmd_op);
	// DONE
	dolog("Number of DUP      Pseudocommands: %6d", count_dup_instruction);
	// DONE
	dolog("Number of Mem      Pseudocommands: %6d", count_pcmd_mem);
	// DONE
	dolog("Number of Method   Pseudocommands: %6d", count_pcmd_met);
	// DONE
	dolog("Number of Branch   Pseudocommands: %6d (rets:%5d, Xrets: %5d)",
		  count_pcmd_bra, count_pcmd_return, count_pcmd_returnx);
	// DONE
	log_println("                resolved branches: %6d", count_branches_resolved);
	// DONE
	log_println("              unresolved branches: %6d", count_branches_unresolved);
	// DONE
	dolog("Number of Table    Pseudocommands: %6d", count_pcmd_table);
	dolog("Number of Useful   Pseudocommands: %6d", count_pcmd_table +
		  count_pcmd_bra + count_pcmd_load + count_pcmd_mem + count_pcmd_op);
	// DONE
	dolog("Number of Null Pointer Checks:     %6d", count_check_null);
	// DONE
	dolog("Number of Array Bound Checks:      %6d", count_check_bound);
	// DONE
	dolog("Number of Try-Blocks: %d", count_tryblocks);

	// DONE
	dolog("Number of branch_emit (total, 8bit/16bit/32bit/64bit offset): %d, %d/%d/%d/%d",
		count_emit_branch,  count_emit_branch_8bit,  count_emit_branch_16bit,
							count_emit_branch_32bit, count_emit_branch_64bit);

	// DONE
	dolog("Maximal count of stack elements:   %d", count_max_new_stack);
	// DONE
	dolog("Upper bound of max stack elements: %d", count_upper_bound_new_stack);
	// DONE
	dolog("Distribution of stack sizes at block boundary");
	dolog("     0     1     2     3     4     5     6     7     8     9  >=10");
	dolog("%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d",
		  count_block_stack[0], count_block_stack[1], count_block_stack[2],
		  count_block_stack[3], count_block_stack[4], count_block_stack[5],
		  count_block_stack[6], count_block_stack[7], count_block_stack[8],
		  count_block_stack[9], count_block_stack[10]);
	// DONE
	dolog("Distribution of store stack depth");
	dolog("     0     1     2     3     4     5     6     7     8     9  >=10");
	dolog("%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d",
		  count_store_depth[0], count_store_depth[1], count_store_depth[2],
		  count_store_depth[3], count_store_depth[4], count_store_depth[5],
		  count_store_depth[6], count_store_depth[7], count_store_depth[8],
		  count_store_depth[9], count_store_depth[10]);
	dolog("Distribution of store creator chains first part");
	dolog("     0     1     2     3     4     5     6     7     8     9");
	// DONE
	dolog("%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d",
		  count_store_length[0], count_store_length[1], count_store_length[2],
		  count_store_length[3], count_store_length[4], count_store_length[5],
		  count_store_length[6], count_store_length[7], count_store_length[8],
		  count_store_length[9]);
	// DONE
	dolog("Distribution of store creator chains second part");
	dolog("    10    11    12    13    14    15    16    17    18    19  >=20");
	dolog("%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d%6d",
		  count_store_length[10], count_store_length[11],
		  count_store_length[12], count_store_length[13],
		  count_store_length[14], count_store_length[15],
		  count_store_length[16], count_store_length[17],
		  count_store_length[18], count_store_length[19],
		  count_store_length[20]);
	// DONE
	dolog("Distribution of analysis iterations");
	dolog("     1     2     3     4   >=5");
	dolog("%6d%6d%6d%6d%6d",
		  count_analyse_iterations[0], count_analyse_iterations[1],
		  count_analyse_iterations[2], count_analyse_iterations[3],
		  count_analyse_iterations[4]);


	/* Distribution of basic blocks per method ********************************/

	// DONE
	log_println("Distribution of basic blocks per method:");
	log_println("   <=5  <=10  <=15  <=20  <=30  <=40  <=50  <=75   >75");

	log_start();
	for (i = 0; i <= 8; i++)
		log_print("%6d", count_method_bb_distribution[i]);
	log_finish();

	/* print ratio */

	f = (float) count_methods;

	log_start();
	for (i = 0; i <= 8; i++)
		log_print("%6.2f", (float) count_method_bb_distribution[i] / f);
	log_finish();

	/* print cumulated ratio */

	log_start();
	for (i = 0, sum = 0; i <= 8; i++) {
		sum += count_method_bb_distribution[i];
		log_print("%6.2f", (float) sum / f);
	}
	log_finish();


	/* Distribution of basic block sizes **************************************/

	// DONE
	log_println("Distribution of basic block sizes:");
	log_println("     0     1     2     3     4     5     6     7     8     9   <13   <15   <17   <19   <21   <26   <31   >30");

	/* print block sizes */

	log_start();
	for (i = 0; i <= 17; i++)
		log_print("%6d", count_block_size_distribution[i]);
	log_finish();

	/* print ratio */

	f = (float) count_basic_blocks;

	log_start();
	for (i = 0; i <= 17; i++)
		log_print("%6.2f", (float) count_block_size_distribution[i] / f);
	log_finish();

	/* print cumulated ratio */

	log_start();
	for (i = 0, sum = 0; i <= 17; i++) {
		sum += count_block_size_distribution[i];
		log_print("%6.2f", (float) sum / f);
	}
	log_finish();

	// DONE
	statistics_print_memory_usage();

	// DONE
	dolog("Number of class loads:    %6d", count_class_loads);
	// DONE
	dolog("Number of class inits:    %6d", count_class_inits);
	// DONE
	dolog("Number of loaded Methods: %6d\n", count_all_methods);

	// DONE
	dolog("Calls of utf_new:                 %6d", count_utf_new);
	// NOT used?!
	dolog("Calls of utf_new (element found): %6d\n", count_utf_new_found);


	/* LSRA statistics ********************************************************/

	// DONE
	dolog("Moves reg -> reg:     %6d", count_mov_reg_reg);
	// DONE
	dolog("Moves mem -> reg:     %6d", count_mov_mem_reg);
	// DONE
	dolog("Moves reg -> mem:     %6d", count_mov_reg_mem);
	// DONE
	dolog("Moves mem -> mem:     %6d", count_mov_mem_mem);

	// DONE
	dolog("Methods allocated by LSRA:         %6d",
		  count_methods_allocated_by_lsra);
	// DONE
	dolog("Conflicts between local Variables: %6d", count_locals_conflicts);
	// DONE
	dolog("Local Variables held in Memory:    %6d", count_locals_spilled);
	// DONE
	dolog("Local Variables held in Registers: %6d", count_locals_register);
	// DONE
	dolog("Stackslots held in Memory:         %6d", count_ss_spilled);
	// DONE
	dolog("Stackslots held in Registers:      %6d", count_ss_register);
	// not used!?
	dolog("Memory moves at BB Boundaries:     %6d", count_mem_move_bb);
	// DONE
	dolog("Number of interface slots:         %6d\n", count_interface_size);
	// DONE
	dolog("Number of Argument stack slots in register:  %6d",
		  count_argument_reg_ss);
	// DONE
	dolog("Number of Argument stack slots in memory:    %6d\n",
		  count_argument_mem_ss);
	// DONE
	dolog("Number of Methods kept in registers:         %6d\n",
		  count_method_in_register);


	/* instruction scheduler statistics ***************************************/

#if defined(USE_SCHEDULER)
	dolog("Instruction scheduler statistics:");
	dolog("Number of basic blocks:       %7d", count_schedule_basic_blocks);
	dolog("Number of nodes:              %7d", count_schedule_nodes);
	dolog("Number of leaders nodes:      %7d", count_schedule_leaders);
	dolog("Number of max. leaders nodes: %7d", count_schedule_max_leaders);
	dolog("Length of critical path:      %7d\n", count_schedule_critical_path);
#endif


	/* call statistics ********************************************************/

	dolog("Function call statistics:");
	// DONE
	dolog("Number of jni->CallXMethod function invokations: %ld",
		  count_jni_callXmethod_calls);
	// DONE
	dolog("Overall number of jni invokations:               %ld",
		  count_jni_calls);

	// DONE
	log_println("java-to-native calls:   %10ld", count_calls_java_to_native);
	// DONE
	log_println("native-to-java calls:   %10ld", count_calls_native_to_java);


	/* now print other statistics ********************************************/

#if defined(ENABLE_INTRP)
	print_dynamic_super_statistics();
#endif
}

/* statistics_print_date *******************************************************

   Print current date and time.

*******************************************************************************/

void statistics_print_date(void)
{
  time_t t;
  struct tm tm;

#if defined(HAVE_TIME)
  time(&t);
#else
# error !HAVE_TIME
#endif

#if defined(HAVE_LOCALTIME_R)
  localtime_r(&t, &tm);
#else
# error !HAVE_LOCALTIME_R
#endif

  log_println("%d-%02d-%02d %02d:%02d:%02d",
			  1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday,
			  tm.tm_hour, tm.tm_min, tm.tm_sec);
}


/* statistics_print_memory_usage ***********************************************

   Print current memory usage.

*******************************************************************************/

void statistics_print_memory_usage(void)
{
	s4 sum;

	log_println("memory usage ----------------------");
	log_println("");
	log_println("code:                      %10d", count_code_len);
	log_println("data:                      %10d", count_data_len);
	log_println("                            ----------");

	sum =
		count_code_len +
		count_data_len;

	log_println("                           %10d", sum);

	log_println("");

	log_println("classinfo  (%3d B):        %10d", (int) sizeof(classinfo), size_classinfo);
	log_println("fieldinfo  (%3d B):        %10d", (int) sizeof(fieldinfo), size_fieldinfo);
	log_println("methodinfo (%3d B):        %10d", (int) sizeof(methodinfo), size_methodinfo);
	log_println("lineinfo   (%3d B):        %10d", (int) sizeof(lineinfo), size_lineinfo);
	log_println("codeinfo   (%3d B):        %10d", (int) sizeof(codeinfo), size_codeinfo);
	log_println("                            ----------");

	sum =
		size_classinfo +
		size_fieldinfo +
		size_methodinfo +
		size_lineinfo +
		size_codeinfo;

	log_println("                           %10d", sum);

	log_println("");

	log_println("linenumber tables (%5d): %10d", count_linenumbertable, size_linenumbertable);
	log_println("exception tables:          %10d", count_extable_len);
	log_println("patcher references:        %10d", size_patchref);
	log_println("                            ----------");

	sum =
		size_linenumbertable +
		count_extable_len +
		size_patchref;

	log_println("                           %10d", sum);

	log_println("");

	log_println("constant pool:             %10d", count_const_pool_len);
	log_println("classref:                  %10d", count_classref_len);
	log_println("parsed descriptors:        %10d", count_parsed_desc_len);
	log_println("vftbl:                     %10d", count_vftbl_len);
	log_println("compiler stubs:            %10d", count_cstub_len);
	log_println("native stubs:              %10d", size_stub_native);
	log_println("utf:                       %10d", count_utf_len);
	log_println("vmcode:                    %10d", count_vmcode_len);
	log_println("stack map:                 %10d", size_stack_map);
	log_println("string:                    %10d", size_string);
	log_println("threadobject:              %10d", size_threadobject);
	log_println("thread index:              %10d", size_thread_index_t);
	log_println("stack size:                %10d", size_stacksize);
	log_println("lock record:               %10d", size_lock_record);
	log_println("lock hashtable:            %10d", size_lock_hashtable);
	log_println("lock waiter:               %10d", size_lock_waiter);
	log_println("                            ----------");

	sum =
		count_const_pool_len +
		count_classref_len +
		count_parsed_desc_len +
		count_vftbl_len +
		count_cstub_len +
		size_stub_native +
		count_utf_len +
		count_vmcode_len +
		size_stack_map +
		size_string +
		size_threadobject +
		size_thread_index_t +
		size_stacksize +
		size_lock_record +
		size_lock_hashtable +
		size_lock_waiter;

	log_println("                           %10d", sum);

	log_println("");

	log_println("max. code memory:          %10d", maxcodememusage);
	log_println("max. heap memory:          %10d", maxmemusage);
	log_println("max. dump memory:          %10d", maxdumpsize);
	log_println("");
	log_println("heap memory not freed:     %10d", (int32_t) memoryusage);
	log_println("dump memory not freed:     %10d", (int32_t) globalallocateddumpsize);

	log_println("");
}


/* statistics_print_gc_memory_usage ********************************************

   Print current GC memory usage.

*******************************************************************************/

void statistics_print_gc_memory_usage(void)
{
	static int64_t count = 0;
	int64_t max;
	int64_t size;
	int64_t free;
	int64_t used;
	int64_t total;

	count++;

	max   = gc_get_max_heap_size();
	size  = gc_get_heap_size();
	free  = gc_get_free_bytes();
	used  = size - free;
	total = gc_get_total_bytes();

	if (opt_ProfileMemoryUsageGNUPlot) {
		if (count == 1)
			fprintf(opt_ProfileMemoryUsageGNUPlot, "plot \"profile.dat\" using 1:2 with lines title \"max. Java heap size\", \"profile.dat\" using 1:3 with lines title \"Java heap size\", \"profile.dat\" using 1:4 with lines title \"used\", \"profile.dat\" using 1:5 with lines title \"free\"\n");

		fprintf(opt_ProfileMemoryUsageGNUPlot,
				"%" PRId64 " %" PRId64 " %" PRId64 " %" PRId64 " %" PRId64 "\n",
				count, max, size, used, free);

		fflush(opt_ProfileMemoryUsageGNUPlot);
	}
	else {
		log_println("GC memory usage -------------------");
		log_println("");
		log_println("max. Java heap size: %10lld", max);
		log_println("");
		log_println("Java heap size:      %10lld", size);
		log_println("used:                %10lld", used);
		log_println("free:                %10lld", free);
		log_println("totally used:        %10lld", total);
		log_println("");
	}
}
#endif


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
