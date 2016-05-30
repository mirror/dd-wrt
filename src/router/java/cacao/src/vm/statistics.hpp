/* src/vm/statistics.hpp - exports global varables for statistics

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

/**
 * @file
 *
 * This file contains the statistics framework.
 *
 * @ingroup statistics
 */

/**
 * @defgroup statistics Statistics framework
 * Statistics framework.
 *
 * The framework provides several facilities to collect information about a
 * virtual machine invocation. The idea is to create objects of specific
 * classes which represent a statistic entity. For different task different
 * classes are provided.
 *
 * ### Variables ###
 *
 * The most general class is cacao::StatVar. It can be used as a wrapper for
 * almost any type. Statistic variables of this type can be altered in various
 * ways although the most common usage is to increment the value. At the end of
 * the virtual machine run the value is emitted.
 *
 * #### Macros ####
 *
 * The following macros should be used to define and declare
 * variables of type cacao::StatVar.
 *
 * - STAT_REGISTER_VAR(type, var, init, name, description)
 * - STAT_DECLARE_VAR(type, var, init)
 * - STAT_REGISTER_VAR_EXTERN(type, var, init, name, description)
 * - STAT_REGISTER_GROUP_VAR(type, var, init, name, description, group)
 * - STAT_REGISTER_GROUP_VAR_EXTERN(type, var, init, name, description, group)
 *
 * #### Example ####
 *
 * @code
 * // somewhere on a file global scope
 * STAT_REGISTER_VAR(int,count_all_methods,0,"all methods","Number of loaded Methods")
 * ..
 *
 * void handle_method() {
 *   // somewhere inside normal code
 *   STATISTICS(count_all_methods++);
 *   ...
 * }
 * @endcode
 *
 * Note: The STATISTICS() macro is used to wrap statistics only code.
 *
 * ### Distribution Tables ###
 *
 * Distribution tables can be used to collect the distribution of a value.
 * There are two different kinds. The cacao::StatDist has a fixed start, step
 * and end value whereas cacao::StatDistRange can be used to construct arbitrary
 * tables. The usage of both classes is the same.
 *
 * #### Macros ####
 *
 * - STAT_REGISTER_DIST(counttype, indextype, var, start, end, step, init, name, description)
 * - STAT_REGISTER_DIST_RANGE(counttype, indextype, var, range, range_size, init, name, description)
 *
 *
 * #### Example ####
 *
 * @code
 * // somewhere on a file global scope
 * STAT_REGISTER_DIST(unsigned int,unsigned int,count_block_stack,0,9,1,0,"stack size dist",
 *	  "Distribution of stack sizes at block boundary")
 *
 * static const unsigned int count_method_bb_distribution_range[] = {5,10,15,20,30,40,50,75};
 * STAT_REGISTER_DIST_RANGE(unsigned int,unsigned int,count_method_bb_distribution,
 *	  count_method_bb_distribution_range,8,0,"method bb dist.","Distribution of basic blocks per method")
 * ...
 *
 * void handle_method() {
 *   // somewhere inside normal code
 *   STATISTICS(count_block_stack[indepth]++);
 *   ...
 *   STATISTICS(count_method_bb_distribution[basicblockcount]++);
 *   ...
 * }
 * @endcode
 *
 * ### Groups ###
 *
 * Groups are used to structure statistics entities.
 *
 * #### Macros ####
 *
 * - STAT_DECLARE_GROUP(var)
 * - STAT_REGISTER_GROUP(var,name,description)
 * - STAT_REGISTER_SUBGROUP(var,name,description,group)
 *
 * #### Example ####
 *
 * @code
 * // somewhere on a file global scope
 * STAT_REGISTER_GROUP(const_pcmd_stat,"const pcmd","Number of Const Pseudocommands")
 * STAT_REGISTER_GROUP_VAR(int,count_pcmd_load,0,"pcmd load","Number of Const Pseudocommands (load)",const_pcmd_stat)
 * ...
 * @endcode
 *
 * ### Summary Groups ###
 *
 * Summary groups are a special form of groups. The only difference is that a
 * summary group sums up the values of all children and prints it.
 *
 * #### Macros ####
 *
 * - STAT_REGISTER_SUM_GROUP(var,name,description)
 * - STAT_REGISTER_SUM_SUBGROUP(var,name,description,group)
 *
 * #### Example ####
 *
 * @code
 * // somewhere on a file global scope
 * STAT_REGISTER_SUM_GROUP(const_pcmd_stat,"const pcmd","Number of Const Pseudocommands")
 * STAT_REGISTER_GROUP_VAR(int,count_pcmd_load,0,"pcmd load","Number of Const Pseudocommands (load)",const_pcmd_stat)
 * STAT_REGISTER_GROUP_VAR(int,count_pcmd_zero,0,"pcmd zero","Number of Const Pseudocommands (zero)",const_pcmd_stat)
 * ...
 * @endcode
 *
 *
 */

/**
 * @addtogroup statistics
 * @{
 */

/**
 * @def STAT_REGISTER_VAR(type, var, init, name, description)
 *
 * Register an external statistics variable.
 *
 * This macros defines a static variable of type cacao::StatVar<type,init> and
 * adds it to the root statistics group. All STAT_REGISTER_* macros should only
 * occur in implementation files. If a variable is used in several compilation
 * units STAT_REGISTER_VAR_EXTERN() and STAT_DECLARE_VAR() should be used.
 *
 * @param type Type of the variable. It should support the basic operators
 *   (arithmetic, assigment, etc.) as well as operator<<(OStream&, type) for
 *   printing support.
 * @param var Name of the statistic variable.
 * @param init Initial value for the variable.
 * @param name Printable name of the variable.
 * @param description Printable description of the variable.
 *
 * @see STAT_DECLARE_VAR
 * @see STAT_REGISTER_VAR_EXTERN
 * @see STAT_REGISTER_GROUP_VAR
 **/

/**
 * @def STAT_DECLARE_VAR(type, var, init)
 *
 * Declare an external statistics variable.
 *
 * The definition should be done via the STAT_REGISTER_VAR_EXTERN() macro.
 *
 * @param type Type of the variable. It should support the basic operators
 *   (arithmetic, assigment, etc.) as well as operator<<(OStream&, type) for
 *   printing support.
 * @param var Name of the statistic variable.
 * @param init Initial value for the variable.
 *
 * @see STAT_REGISTER_VAR_EXTERN
 **/

/**
 * @def STAT_REGISTER_VAR_EXTERN(type, var, init, name, description)
 *
 * Register an external statistics variable.
 *
 * In contrast to STAT_REGISTER_VAR() variables defined via
 * STAT_REGISTER_VAR_EXTERN() are not declared static static and therefore
 * allows external binding. If a statistics variable is only used in one
 * compilation unit, STAT_REGISTER_VAR() is preferable.
 *
 * @param type Type of the variable. It should support the basic operators
 *   (arithmetic, assigment, etc.) as well as operator<<(OStream&, type) for
 *   printing support.
 * @param var Name of the statistic variable.
 * @param init Initial value for the variable.
 * @param name Printable name of the variable.
 * @param description Printable description of the variable.
 *
 * @see STAT_DECLARE_VAR
 * @see STAT_REGISTER_VAR
 **/

/**
 * @def STAT_REGISTER_GROUP_VAR(type, var, init, name, description, group)
 *
 * Register an statistics variable and add it to a group.
 *
 * This is the same as STAT_REGISTER_VAR() but the variable is added to group.
 *
 * @param type Type of the variable. It should support the basic operators
 *   (arithmetic, assigment, etc.) as well as operator<<(OStream&, type) for
 *   printing support.
 * @param var Name of the statistic variable.
 * @param init Initial value for the variable.
 * @param name Printable name of the variable.
 * @param description Printable description of the variable.
 * @param group Parent group of this variable.
 *
 * @see STAT_REGISTER_VAR
 * @see STAT_REGISTER_GROUP_VAR_EXTERN
 **/

/**
 * @def STAT_REGISTER_GROUP_VAR_EXTERN(type, var, init, name, description, group)
 *
 * Register an external statistics variable and add it to a group.
 *
 * The difference to STAT_REGISTER_GROUP_VAR() is the same as between
 * STAT_REGISTER_VAR_EXTERN and STAT_REGISTER_VAR().
 *
 * @param type Type of the variable. It should support the basic operators
 *   (arithmetic, assigment, etc.) as well as operator<<(OStream&, type) for
 *   printing support.
 * @param var Name of the statistic variable.
 * @param init Initial value for the variable.
 * @param name Printable name of the variable.
 * @param description Printable description of the variable.
 * @param group Parent group of this variable.
 *
 * @see STAT_DECLARE_VAR
 * @see STAT_REGISTER_VAR
 **/

/**
 * @def STAT_REGISTER_DIST(counttype, indextype, var, start, end, step, init, name, description)
 *
 * Define a distribution table (steps).
 *
 * This macro creates a distribution table for some statistic values (e.g.
 * number of basic blocks per method). In principle a static cacao::StatDist
 * object is created which can be used to collect value distributions.
 *
 * The "width of the table columns" are fixed by the step parameter. All columns
 * have the same width (besides the first and last one).
 *
 * <b>Example:</b>
 * @code
 * STAT_REGISTER_DIST(unsigned int,unsigned int,count_block_stack,0,9,1,0,"stack size dist",
 *			"Distribution of stack sizes at block boundary")
 * @endcode
 *
 * <b>Result:</b>
@verbatim
Distribution of stack sizes at block boundary(stack size dist):
     0]    1]    2]    3]    4]    5]    6]    7]    8]    9](   9
  4017   325    20     8     7     4     1     0     0     0     0
  0.92  0.07  0.00  0.00  0.00  0.00  0.00  0.00  0.00  0.00  0.00  ratio
  0.92  0.99  1.00  1.00  1.00  1.00  1.00  1.00  1.00  1.00  1.00  cumulated ratio
@endverbatim
 *
 *
 * @param counttype Type of the "table entries".
 *   In most cases unsigned is the right choice.
 * @param indextype Type of the "table index".
 *   This is the type of the things you want to count. It should support the
 *   basic arithmetic operations as well as operator<<(OStream&).
 * @param var Name of the statistic distribution variable.
 * @param start Start of the table.
 * @param end End of the table.
 * @param step Step size.
 * @param init Initial value for the variable.
 * @param name Printable name of the variable.
 * @param description Printable description of the variable.
 *
 * @see cacao::StatDist
 * @see STAT_REGISTER_DIST_RANGE
 **/

/**
 * @def STAT_REGISTER_DIST_RANGE(counttype, indextype, var, range, range_size, init, name, description)
 *
 * Define a distribution table (range).
 *
 * In contrast to STAT_REGISTER_DIST() the "width of the columns" can be set arbitrary.
 *
 * <b>Example:</b>
 * @code
 * static const unsigned int count_method_bb_distribution_range[] = {5,10,15,20,30,40,50,75};
 * STAT_REGISTER_DIST_RANGE(unsigned int,unsigned int,count_method_bb_distribution,
 *    count_method_bb_distribution_range,8,0,"method bb dist.","Distribution of basic blocks per method")
 * @endcode
 *
 * <b>Result:</b>
@verbatim
Distribution of basic blocks per method(method bb dist.):
     5]   10]   15]   20]   30]   40]   50]   75](  75
   774    96    45    17    15     3     5     3     2
  0.81  0.10  0.05  0.02  0.02  0.00  0.01  0.00  0.00  ratio
  0.81  0.91  0.95  0.97  0.99  0.99  0.99  1.00  1.00  cumulated ratio
@endverbatim
 *
 * @param counttype Type of the "table entries".
 *   In most cases unsigned is the right choice.
 * @param indextype Type of the "table index".
 *   This is the type of the things you want to count. It should support the
 *   basic arithmetic operations as well as operator<<(OStream&).
 * @param var Name of the statistic distribution variable.
 * @param range Array of table values.
 * @param range_size Size of the range array.
 * @param init Initial value for the variable.
 * @param name Printable name of the variable.
 * @param description Printable description of the variable.
 *
 * @see cacao::StatDistRange
 * @see STAT_REGISTER_DIST
 **/

/**
 * @def STAT_REGISTER_GROUP(var,name,description)
 *
 * Register a statistics group.
 *
 * Create a new statistics group and add it to the root group.
 * Groups are used to organize statistics entries (variables or groups).
 *
 * @param var Name of the statistic group.
 * @param name Printable name of the group.
 * @param description Printable description of the group.
 *
 * @see STAT_DECLARE_GROUP
 * @see STAT_REGISTER_SUBGROUP
 **/

/**
 * @def STAT_REGISTER_SUBGROUP(var,name,description,group)
 *
 * Register a statistics group and add it to a group.
 *
 * Create a new statistics group and add it to a specified group.
 *
 * @param var Name of the statistic group.
 * @param name Printable name of the group.
 * @param description Printable description of the group.
 * @param group Parent group of this variable.
 *
 * @see STAT_DECLARE_GROUP
 * @see STAT_REGISTER_GROUP
 **/

/**
 * @def STAT_DECLARE_GROUP(var)
 *
 * Declare an external group (or subgroup).
 *
 * In contrast to variables are groups always defined at a global scope. This
 * can be prevented by e.g. an anonymous namespace.
 *
 * @param var Variable name of the statistic group.
 *
 * @see STAT_REGISTER_GROUP
 * @see STAT_REGISTER_SUBGROUP
 **/

/**
 * @def STAT_REGISTER_SUM_GROUP(var,name,description)
 *
 * Register a statistics summary group.
 *
 * Create a new statistics group and add it to the root group.
 * Summary groups are used to calculate the sum of several dependant statistic
 * entries e.g. memory usage of the compiler
 *
 * <b>Example:</b>
 * @code
 * STAT_REGISTER_SUM_GROUP(info_struct_stat,"info structs","info struct usage")
 * STAT_REGISTER_GROUP_VAR(int,size_classinfo,0,"size classinfo","classinfo",info_struct_stat)
 * STAT_REGISTER_GROUP_VAR(int,size_fieldinfo,0,"size fieldinfo","fieldinfo",info_struct_stat)
 * STAT_REGISTER_GROUP_VAR(int,size_methodinfo,0,"size methodinfo","methodinfo",info_struct_stat)
 * STAT_REGISTER_GROUP_VAR(int,size_codeinfo,0,"size codeinfo","codeinfo",info_struct_stat)
 * STAT_REGISTER_GROUP_VAR(int,size_lineinfo,0,"size lineinfo","lineinfo",info_struct_stat)
 * @endcode
 *
 * <b>Result:</b>
@verbatim
                size classinfo    147656 : classinfo
                size fieldinfo     78272 : fieldinfo
               size methodinfo    830024 : methodinfo
                 size lineinfo     91192 : lineinfo
                 size codeinfo    158400 : codeinfo
                                 -------
                           sum   1305544 : info struct usage
@endverbatim
 * @param var Name of the statistic group.
 * @param name Printable name of the group.
 * @param description Printable description of the group.
 *
 * @see STAT_REGISTER_SUM_SUBGROUP
 **/

/**
 * @def STAT_REGISTER_SUM_SUBGROUP(var,name,description,group)
 *
 * Register a statistics summary group.
 *
 * Create a new statistics group and add it to a specified group.
 *
 * @param var Name of the statistic group.
 * @param name Printable name of the group.
 * @param description Printable description of the group.
 * @param group Parent group of this group.
 *
 * @see STAT_REGISTER_SUM_GROUP
 **/

/**
 * @def STATISTICS(x)
 *
 * Wrapper for statistics only code.
 **/

/** @} */

#ifndef STATISTICS_HPP_
#define STATISTICS_HPP_ 1

#include "config.h"

#if defined(ENABLE_STATISTICS)

#include <assert.h>
#include <vector>

#include "toolbox/OStream.hpp"

extern bool opt_stat;

namespace cacao {

/**
 * @addtogroup statistics
 * @{
 */

/**
 * Superclass of statistic group entries.
 */
class StatEntry {
protected:
	const char* name;
	const char* description;
public:
	/**
	 * Constructor.
	 */
	StatEntry(const char* name, const char* description) : name(name), description(description) {}
	/**
	 * Print the statistics information to an OStream.
	 * @param[in,out] O     output stream
	 */
	virtual void print(OStream &O) const = 0;

	/**
	 * Get the value of a stat entry.
	 */
	virtual unsigned long get_value() const { return 0; }

	virtual ~StatEntry() {}

};

/**
 * Statistics group.
 * Used to collect a set of StatEntry's
 */
class StatGroup : public StatEntry {
protected:
	typedef std::vector<StatEntry*> StatEntryList;
	/**
	 * List of members.
	 * @note: this must be a pointer. If it is changes to a normal variable strange
	 * things will happen!
	 */
	StatEntryList *members;

	StatGroup(const char* name, const char* description) : StatEntry(name, description) {
		members = new StatEntryList();
	}
public:
	/**
	 * Get the root group
	 */
	static StatGroup& root() {
		static StatGroup rg("root stat group","Statistics");
		return rg;
	}

	/**
	 * Create a new statistics group.
	 * @param[in] name name of the group
	 * @param[in] description description of the group
	 * @param[in] group parent group.
	 */
	StatGroup(const char* name, const char* description, StatGroup &group) : StatEntry(name, description) {
		members = new StatEntryList();
		group.add(this);
	}

	~StatGroup() {
		delete members;
	}

	/**
	 * Add an entry to the group
	 */
	void add(StatEntry *re) {
		members->push_back(re);
	}

	virtual void print(OStream &O) const {
		O << nl << description << ':' << nl;
		//O << indent;
		for(StatEntryList::const_iterator i = members->begin(), e= members->end(); i != e; ++i) {
			StatEntry* re = *i;
			re->print(O);
		}
		//O << dedent;
		O << nl;
	}
};

/**
 * Summary group.
 * Used to collect a set of StatEntry's
 */
class StatSumGroup : public StatGroup {
public:

	/**
	 * Create a new statistics group.
	 * @param[in] name name of the group
	 * @param[in] description description of the group
	 * @param[in] group parent group.
	 */
	StatSumGroup(const char* name, const char* description, StatGroup &group)
			: StatGroup(name,description,group) {}

	virtual void print(OStream &O) const {
		unsigned long sum = 0;
		//O << setw(10) << left << name << right <<"   " << description << nl;
		O << nl;
		//O << indent;
		for(StatEntryList::const_iterator i = members->begin(), e= members->end(); i != e; ++i) {
			StatEntry* se = *i;
			se->print(O);
			sum += se->get_value();
		}
		//O << dedent;
		O << setw(40) << "-------" << nl;
		O << setw(30) << "sum"
		  << setw(10) << sum
		  << " : " << description << nl;
	}

	virtual unsigned long get_value() const {
		unsigned long sum = 0;
		for(StatEntryList::const_iterator i = members->begin(), e= members->end(); i != e; ++i) {
			StatEntry* se = *i;
			sum += se->get_value();
		}
		return sum;
	}
};

/**
 * Statistics Distribution (abstract base).
 */
template<typename _COUNT_TYPE, typename _INDEX_TYPE>
class StatDistAbst : public StatEntry {
protected:
	_COUNT_TYPE  *table;       //< table
	const std::size_t table_size;      //< size of the table

	StatDistAbst(const char* name, const char* description, StatGroup &parent,
			_INDEX_TYPE table_size, _COUNT_TYPE init)
			: StatEntry(name, description), table_size(table_size) {
		parent.add(this);
		table = new _COUNT_TYPE[table_size + 1];

		for(std::size_t i = 0; i <= table_size; ++i ) {
			table[i] = init;
		}
	}

	virtual ~StatDistAbst() {
		delete table;
	}

	virtual void print_header(OStream &O, std::size_t first, std::size_t last) const = 0;

public:
	virtual void print(OStream &O) const {
		_COUNT_TYPE sum = 0;
		_COUNT_TYPE cumulated = 0;

		for(int first = 0, end = table_size +1, last = end;
				first != end; first = last) {
			if (end - first > 15) {
				last = first + 10;
			} else {
				last = end;
			}
			O << description << '(' << name << (first == 0 ? "):" : ") (cont):") << nl;

			print_header(O,first,last);

			// ratio
			for(int i = first; i < last; ++i ) {
				O << setw(6) << table[i];
				sum += table[i];
			}
			O << nl;
			for(int i = first; i < last; ++i ) {
				O << setw(6) << setprecision(2) << float(table[i])/sum;
			}
			O << "  ratio" << nl;
			// cumulated ratio
			for(int i = first; i < last; ++i ) {
				cumulated += table[i];
				O << setw(6) << setprecision(2) << float(cumulated)/sum;
			}
			O << "  cumulated ratio" << nl;
		}
		O << nl;
	}
};

/**
 * Statistics Distribution.
 *
 * @note: step is a template variable to allow the compiler to remove the /step calculation
 * in case step equals 1 (most common case).
 */
template<typename _COUNT_TYPE, typename _INDEX_TYPE, _INDEX_TYPE step>
class StatDist : public StatDistAbst<_COUNT_TYPE,_INDEX_TYPE> {
private:
	const _INDEX_TYPE start;
	const _INDEX_TYPE end;

protected:
	virtual void print_header(OStream &O, std::size_t first, std::size_t last) const {
		O << ' ';
		std::size_t i;
		for(i = first; i < last && i < this->table_size; ++i ) {
			O << setw(5) << start + i * step << ']';
		}
		if (i == this->table_size) {
			O << '(' << setw(4) << start + (this->table_size-1)*step;
		}
		O << nl;
	}

public:
	/**
	 * Create a new statistics distribution.
	 * @param[in] name name of the variable
	 * @param[in] description description of the variable
	 * @param[in] parent parent group.
	 * @param[in] start first entry in the distribution table
	 * @param[in] end last entry in the distribution table
	 * @param[in] init initial value of the distribution table entries
	 */
	StatDist(const char* name, const char* description, StatGroup &parent,
			_INDEX_TYPE start, _INDEX_TYPE end, _COUNT_TYPE init)
			: StatDistAbst<_COUNT_TYPE,_INDEX_TYPE>(name, description, parent,
			                                        (end - start)/step + 1, init),
			start(start), end(end) {
		assert(step);
	}

	/// index operator
	inline _COUNT_TYPE& operator[](const _INDEX_TYPE i) {
		if (i <= start) {
			return this->table[0];
		}
		if (i > end) {
			return this->table[this->table_size];
		}
		return this->table[( (i-start)%step == 0 ? (i-start)/step :  (i-start+step)/step )];
	}
};

/**
 * Statistics Distribution (Range).
 *
 * @note: step is a template variable to allow the compiler to remove the /step calculation
 * in case step equals 1 (most common case).
 */
template<typename _COUNT_TYPE, typename _INDEX_TYPE>
class StatDistRange : public StatDistAbst<_COUNT_TYPE,_INDEX_TYPE> {
private:
	const _INDEX_TYPE *range;  //< range table

protected:
	virtual void print_header(OStream &O, std::size_t first, std::size_t last) const {
		O << ' ';
		std::size_t i;
		for(i = first; i < last && i < this->table_size; ++i ) {
			O << setw(5) << range[i] << ']';
		}
		if (i == this->table_size) {
			O << '(' << setw(4) << range[this->table_size-1];
		}
		O << nl;
	}

public:
	/**
	 * Create a new statistics distribution.
	 * @param[in] name name of the variable
	 * @param[in] description description of the variable
	 * @param[in] parent parent group.
	 * @param[in] range range header
	 * @param[in] range_size size of the range array
	 * @param[in] init initial value of the distribution table entries
	 */
	StatDistRange(const char* name, const char* description, StatGroup &parent,
			const _INDEX_TYPE range[], const int range_size, _COUNT_TYPE init)
			: StatDistAbst<_COUNT_TYPE,_INDEX_TYPE>(name, description, parent,
			                                        range_size, init),
			range(range) {}

	/// index operator
	inline _COUNT_TYPE& operator[](const _INDEX_TYPE v) {
		for (std::size_t i = 0; i < this->table_size; ++i ) {
			if (v <= range[i])
				return this->table[i];
		}
		return this->table[this->table_size];
	}
};

/**
 * Statistics Variable.
 */
template<typename _T, _T init>
class StatVar : public StatEntry {
private:
	_T  var;    //< variable
public:
	/**
	 * Create a new statistics variable.
	 * @param[in] name name of the variable
	 * @param[in] description description of the variable
	 * @param[in] group parent group.
	 */
	StatVar(const char* name, const char* description, StatGroup &parent)
			: StatEntry(name, description), var(init) {
		parent.add(this);
	}

	virtual unsigned long get_value() const {
		return (unsigned long)var;
	}

	_T get() const {
		return var;
	}

	void print(OStream &O) const {
		O << setw(30) << name
		  << setw(10) << var
		  << " : " << description << nl;
	}
	/// maximum
	inline StatVar& max(const _T& i) {
	  if (i > var)
		var = i;
	  return *this;
	}

	/// mininum
	inline StatVar& min(const _T& i) {
	  if (i < var)
		var = i;
	  return *this;
	}

	/// prefix operator
	inline StatVar& operator++() {
	  ++var;
	  return *this;
	}
	/// postfix operator
	inline StatVar operator++(int) {
	  StatVar copy(*this);
	  ++var;
	  return copy;
	}
	/// increment operator
	inline StatVar& operator+=(const _T& i) {
	  var+=i;
	  return *this;
	}
	/// prefix operator
	inline StatVar& operator--() {
	  --var;
	  return *this;
	}
	/// postfix operator
	inline StatVar operator--(int) {
	  StatVar copy(*this);
	  --var;
	  return copy;
	}
	/// decrement operator
	inline StatVar& operator-=(const _T& i) {
	  var-=i;
	  return *this;
	}

	/// assignment operator
	inline StatVar& operator=(const _T& i) {
	  var=i;
	  return *this;
	}
};

/** @} */

} // end namespace cacao

/* statistic macros ***********************************************************/

#define STAT_DECLARE_VAR(type, var, init) \
	extern cacao::StatVar<type,init> var;

#define STAT_REGISTER_VAR_EXTERN(type, var, init, name, description) \
	cacao::StatVar<type,init> var(name,description,cacao::StatGroup::root());

#define STAT_REGISTER_GROUP_VAR_EXTERN(type, var, init, name, description, group) \
	cacao::StatVar<type,init> var(name,description,group##_group());

#define STAT_REGISTER_VAR(type, var, init, name, description) \
	static cacao::StatVar<type,init> var(name,description,cacao::StatGroup::root());

#define STAT_REGISTER_GROUP_VAR(type, var, init, name, description, group) \
	static cacao::StatVar<type,init> var(name,description,group##_group());

#define STAT_REGISTER_DIST(counttype, indextype, var, start, end, step, init, name, description) \
	static cacao::StatDist<counttype,indextype,step> var(name,description,cacao::StatGroup::root(),start,end,init);

#define STAT_REGISTER_DIST_RANGE(counttype, indextype, var, range, range_size, init, name, description) \
	static cacao::StatDistRange<counttype,indextype> var(name,description,cacao::StatGroup::root(),range,range_size,init);

#define STAT_DECLARE_GROUP(var) \
	cacao::StatGroup& var##_group();

#define STAT_REGISTER_GROUP(var,name,description)                              \
	cacao::StatGroup& var##_group() {                                          \
		static cacao::StatGroup var(name,description,cacao::StatGroup::root());\
		return var;                                                            \
	}

#define STAT_REGISTER_SUM_GROUP(var,name,description)                          \
	cacao::StatSumGroup& var##_group() {                                       \
		static cacao::StatSumGroup var(name,description,                       \
		                               cacao::StatGroup::root());              \
		return var;                                                            \
	}

#define STAT_REGISTER_SUM_SUBGROUP(var,name,description,group)                 \
	cacao::StatSumGroup& var##_group() {                                       \
		static cacao::StatSumGroup var(name,description, group##_group());     \
		return var;                                                            \
	}

#define STAT_REGISTER_SUBGROUP(var,name,description,group)                     \
	cacao::StatGroup& var##_group() {                                          \
		static cacao::StatGroup var(name,description, group##_group());        \
		return var;                                                            \
	}

#define STATISTICS(x)  do { x; } while (0)

#else
#define STAT_DECLARE_VAR(type, var, init)
#define STAT_REGISTER_VAR_EXTERN(type, var, init, name, description)
#define STAT_REGISTER_GROUP_VAR_EXTERN(type, var, init, name, description, group)
#define STAT_REGISTER_VAR(type, var, init, name, description)
#define STAT_REGISTER_GROUP_VAR(type, var, init, name, description, group)
#define STAT_REGISTER_DIST(counttype, indextype, var, start, end, step, init, name, description)
#define STAT_REGISTER_DIST_RANGE(counttype, indextype, var, range, range_size, init, name, description)
#define STAT_DECLARE_GROUP(var)
#define STAT_REGISTER_GROUP(var,name,description)
#define STAT_REGISTER_SUM_GROUP(var,name,description)
#define STAT_REGISTER_SUM_SUBGROUP(var,name,description,group)
#define STAT_REGISTER_SUBGROUP(var,name,description,group)
#define STATISTICS(x)    /* nothing */
#endif

#ifdef ENABLE_STATISTICS

/// @cond
// exclude from doxygen

/* function prototypes ********************************************************/

void loadingtime_start(void);
void loadingtime_stop(void);
void compilingtime_start(void);
void compilingtime_stop(void);

#if 0
void statistics_print_date(void);
void statistics_print_memory_usage(void);
void statistics_print_gc_memory_usage(void);
#endif

/// @endcond
// end exclude from doxygen

#endif /* ENABLE_STATISTICS */

#endif // STATISTICS_HPP_


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
