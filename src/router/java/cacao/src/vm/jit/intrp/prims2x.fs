\ converts primitives to, e.g., C code 

\ Copyright (C) 1995,1996,1997,1998,2000,2003,2004 Free Software Foundation, Inc.

\ This file is part of Gforth.

\ Gforth is free software; you can redistribute it and/or
\ modify it under the terms of the GNU General Public License
\ as published by the Free Software Foundation; either version 2
\ of the License, or (at your option) any later version.

\ This program is distributed in the hope that it will be useful,
\ but WITHOUT ANY WARRANTY; without even the implied warranty of
\ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
\ GNU General Public License for more details.

\ You should have received a copy of the GNU General Public License
\ along with this program; if not, write to the Free Software
\ Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.


\ This is not very nice (hard limits, no checking, assumes 1 chars = 1).
\ And it grew even worse when it aged.

\ Optimizations:
\ superfluous stores are removed. GCC removes the superfluous loads by itself
\ TOS and FTOS can be kept in register( variable)s.
\ 
\ Problems:
\ The TOS optimization is somewhat hairy. The problems by example:
\ 1) dup ( w -- w w ): w=TOS; sp-=1; sp[1]=w; TOS=w;
\    The store is not superfluous although the earlier opt. would think so
\    Alternatively:    sp[0]=TOS; w=TOS; sp-=1; TOS=w;
\ 2) ( -- .. ): sp[0] = TOS; ... /* This additional store is necessary */
\ 3) ( .. -- ): ... TOS = sp[0]; /* as well as this load */
\ 4) ( -- ): /* but here they are unnecessary */
\ 5) Words that call NEXT themselves have to be done very carefully.
\
\ To do:
\ add the store optimization for doubles
\ regarding problem 1 above: It would be better (for over) to implement
\ 	the alternative
\ store optimization for combined instructions.

\ Design Uglyness:

\ - global state (values, variables) in connection with combined instructions.

\ - index computation is different for instruction-stream and the
\ stacks; there are two mechanisms for dealing with that
\ (stack-in-index-xt and a test for stack==instruction-stream); there
\ should be only one.


\ for backwards compatibility, jaw
require compat/strcomp.fs

warnings off

\ redefinitions of kernel words not present in gforth-0.6.1
: latestxt lastcfa @ ;
: latest last @ ;

[IFUNDEF] try
include startup.fs
[THEN]

: struct% struct ; \ struct is redefined in gray

warnings off
\ warnings on

include ./gray.fs
128 constant max-effect \ number of things on one side of a stack effect
4 constant max-stacks  \ the max. number of stacks (including inst-stream).
255 constant maxchar
maxchar 1+ constant eof-char
#tab constant tab-char
#lf constant nl-char

variable rawinput \ pointer to next character to be scanned
variable endrawinput \ pointer to the end of the input (the char after the last)
variable cookedinput \ pointer to the next char to be parsed
variable line \ line number of char pointed to by input
variable line-start \ pointer to start of current line (for error messages)
0 line !
2variable filename \ filename of original input file
0 0 filename 2!
2variable out-filename \ filename of the output file (for sync lines)
0 0 out-filename 2!
2variable f-comment
0 0 f-comment 2!
variable skipsynclines \ are sync lines ("#line ...") invisible to the parser?
skipsynclines on
variable out-nls \ newlines in output (for output sync lines)
0 out-nls !
variable store-optimization \ use store optimization?
store-optimization off

variable include-skipped-insts
\ does the threaded code for a combined instruction include the cells
\ for the component instructions (true) or only the cells for the
\ inline arguments (false)
include-skipped-insts off

2variable threaded-code-pointer-type \ type used for geninst etc.
s" Inst **" threaded-code-pointer-type 2!

variable immarg \ values for immediate arguments (to be used in IMM_ARG macros)
$12340000 immarg !

: th ( addr1 n -- addr2 )
    cells + ;

: holds ( addr u -- )
    \ like HOLD, but for a string
    tuck + swap 0 +do
	1- dup c@ hold
    loop
    drop ;

: insert-wordlist { c-addr u wordlist xt -- }
    \ adds name "addr u" to wordlist using defining word xt
    \ xt may cause additional stack effects
    get-current >r wordlist set-current
    c-addr u nextname xt execute
    r> set-current ;

: start ( -- addr )
 cookedinput @ ;

: end ( addr -- addr u )
 cookedinput @ over - ;

: print-error-line ( -- )
    \ print the current line and position
    line-start @ endrawinput @ over - 2dup nl-char scan drop nip ( start end )
    over - type cr
    line-start @ rawinput @ over - typewhite ." ^" cr ;

: ?print-error { f addr u -- }
    f ?not? if
	outfile-id >r try
	    stderr to outfile-id
	    filename 2@ type ." :" line @ 0 .r ." : " addr u type cr
	    print-error-line
	    0
	recover endtry
	r> to outfile-id throw
	1 (bye) \ abort
    endif ;

: quote ( -- )
    [char] " emit ;

\ count output lines to generate sync lines for output

: count-nls ( addr u -- )
    bounds u+do
	i c@ nl-char = negate out-nls +!
    loop ;

:noname ( addr u -- )
    2dup count-nls
    defers type ;
is type

variable output          \ xt ( -- ) of output word for simple primitives
variable output-combined \ xt ( -- ) of output word for combined primitives

struct%
    cell%    field stack-number \ the number of this stack
    cell% 2* field stack-pointer \ stackpointer name
    cell%    field stack-type \ name for default type of stack items
    cell%    field stack-in-index-xt \ ( in-size item -- in-index )
    cell%    field stack-access-transform \ ( nitem -- index )
end-struct stack%

struct%
 cell% 2* field item-name   \ name, excluding stack prefixes
 cell%    field item-stack  \ descriptor for the stack used, 0 is default
 cell%    field item-type   \ descriptor for the item type
 cell%    field item-offset \ offset in stack items, 0 for the deepest element
 cell%	  field item-first  \ true if this is the first occurence of the item
end-struct item%

struct%
    cell% 2* field type-c-name
    cell%    field type-stack \ default stack
    cell%    field type-size  \ size of type in stack items
    cell%    field type-fetch \ xt of fetch code generator ( item -- )
    cell%    field type-store \ xt of store code generator ( item -- )
end-struct type%

struct%
    cell%    field register-number
    cell%    field register-type \ pointer to type
    cell% 2* field register-name \ c name
end-struct register%

struct%
    cell% 2* field ss-registers  \ addr u; ss-registers[0] is TOS
                                 \ 0 means: use memory
    cell%    field ss-offset     \ stack pointer offset: sp[-offset] is TOS
end-struct ss% \ stack-state

struct%
    cell%              field state-number
    cell% max-stacks * field state-sss
end-struct state%

variable next-stack-number 0 next-stack-number !
create stacks max-stacks cells allot \ array of stacks
256 constant max-registers
create registers max-registers cells allot \ array of registers
variable nregisters 0 nregisters ! \ number of registers
variable next-state-number 0 next-state-number ! \ next state number

: stack-in-index ( in-size item -- in-index )
    item-offset @ - 1- ;

: inst-in-index ( in-size item -- in-index )
    nip dup item-offset @ swap item-type @ type-size @ + 1- ;

: make-stack ( addr-ptr u1 type "stack-name" -- )
    next-stack-number @ max-stacks < s" too many stacks" ?print-error
    create stack% %allot >r
    r@ stacks next-stack-number @ th !
    next-stack-number @ r@ stack-number !
    1 next-stack-number +!
    r@ stack-type !
    save-mem r@ stack-pointer 2! 
    ['] stack-in-index r@ stack-in-index-xt !
    ['] noop r@ stack-access-transform !
    rdrop ;

: map-stacks { xt -- }
    \ perform xt ( stack -- ) for all stacks
    next-stack-number @ 0 +do
	stacks i th @ xt execute
    loop ;

: map-stacks1 { xt -- }
    \ perform xt ( stack -- ) for all stacks except inst-stream
    next-stack-number @ 1 +do
	stacks i th @ xt execute
    loop ;

\ stack items

: init-item ( addr u addr1 -- )
    \ initialize item at addr1 with name addr u
    \ the stack prefix is removed by the stack-prefix
    dup item% %size erase
    item-name 2! ;

: map-items { addr end xt -- }
    \ perform xt for all items in array addr...end
    end addr ?do
	i xt execute
    item% %size +loop ;

\ types

: print-type-prefix ( type -- )
    body> >head name>string type ;

\ various variables for storing stuff of one primitive

struct%
    cell% 2* field prim-name
    cell% 2* field prim-wordset
    cell% 2* field prim-c-name
    cell% 2* field prim-c-name-orig \ for reprocessed prims, the original name
    cell% 2* field prim-doc
    cell% 2* field prim-c-code
    cell% 2* field prim-forth-code
    cell% 2* field prim-stack-string
    cell%    field prim-num            \ ordinal number
    cell%    field prim-items-wordlist \ unique items
    item% max-effect * field prim-effect-in
    item% max-effect * field prim-effect-out
    cell%    field prim-effect-in-end
    cell%    field prim-effect-out-end
    cell% max-stacks * field prim-stacks-in  \ number of in items per stack
    cell% max-stacks * field prim-stacks-out \ number of out items per stack
    cell% max-stacks * field prim-stacks-sync \ sync flag per stack
end-struct prim%

: make-prim ( -- prim )
    prim% %alloc { p }
    s" " p prim-doc 2! s" " p prim-forth-code 2! s" " p prim-wordset 2!
    p ;

0 value prim     \ in combined prims either combined or a part
0 value combined \ in combined prims the combined prim
variable in-part \ true if processing a part
 in-part off
0 value state-in  \ state on entering prim
0 value state-out \ state on exiting prim
0 value state-default  \ canonical state at bb boundaries

: prim-context ( ... p xt -- ... )
    \ execute xt with prim set to p
    prim >r
    swap to prim
    catch
    r> to prim
    throw ;

: prim-c-name-2! ( c-addr u -- )
    2dup prim prim-c-name 2! prim prim-c-name-orig 2! ;

1000 constant max-combined
create combined-prims max-combined cells allot
variable num-combined
variable part-num \ current part number during process-combined

: map-combined { xt -- }
    \ perform xt for all components of the current combined instruction
    num-combined @ 0 +do
	combined-prims i th @ xt execute
    loop ;

table constant combinations
  \ the keys are the sequences of pointers to primitives

create current-depth max-stacks cells allot
create max-depth     max-stacks cells allot
create min-depth     max-stacks cells allot

create sp-update-in max-stacks cells allot
\ where max-depth occured the first time
create max-depths max-stacks max-combined 1+ * cells allot
\ maximum depth at start of each part: array[parts] of array[stack]
create max-back-depths max-stacks max-combined 1+ * cells allot
\ maximun depth from end of the combination to the start of the each part

: s-c-max-depth ( nstack ncomponent -- addr )
    max-stacks * + cells max-depths + ;

: s-c-max-back-depth ( nstack ncomponent -- addr )
    max-stacks * + cells max-back-depths + ;

wordlist constant primitives

: create-prim ( prim -- )
    dup prim-name 2@ primitives ['] constant insert-wordlist ;

: stack-in ( stack -- addr )
    \ address of number of stack items in effect in
    stack-number @ cells prim prim-stacks-in + ;

: stack-out ( stack -- addr )
    \ address of number of stack items in effect out
    stack-number @ cells prim prim-stacks-out + ;

: stack-prim-stacks-sync ( stack -- addr )
    prim prim-stacks-sync swap stack-number @ th ;

\ global vars
variable c-line
2variable c-filename
variable name-line
2variable name-filename
2variable last-name-filename
Variable function-number 0 function-number !
Variable function-old 0 function-old !
: function-diff ( n -- )
    ." GROUPADD(" function-number @ function-old @ - 0 .r ." )" cr
    function-number @ function-old ! ;
: forth-fdiff ( -- )
    function-number @ function-old @ - 0 .r ."  groupadd" cr
    function-number @ function-old ! ;

\ a few more set ops

: bit-equivalent ( w1 w2 -- w3 )
 xor invert ;

: complement ( set1 -- set2 )
 empty ['] bit-equivalent binary-set-operation ;

\ forward declaration for inst-stream (breaks cycle in definitions)
defer inst-stream-f ( -- stack )

\ stack access stuff

: normal-stack-access0 { n stack -- }
    \ n has the ss-offset already applied (see ...-access1)
    n stack stack-access-transform @ execute ." [" 0 .r ." ]" ;

: state-ss { stack state -- ss }
    state state-sss stack stack-number @ th @ ;

: stack-reg { n stack state -- reg }
    \ n is the index (TOS=0); reg is 0 if the access is to memory
    stack state state-ss ss-registers 2@ n u> if ( addr ) \ in ss-registers?
	n th @
    else
	drop 0
    endif ;

: .reg ( reg -- )
    register-name 2@ type ;

: stack-offset ( stack state -- n )
    \ offset for stack in state
    state-ss ss-offset @ ;

: normal-stack-access1 { n stack state -- }
    n stack state stack-reg ?dup-if
	.reg exit
    endif
    stack stack-pointer 2@ type
    n stack state stack-offset - stack normal-stack-access0 ;

: normal-stack-access ( n stack state -- )
    over inst-stream-f = if
	." IMM_ARG(" normal-stack-access1 ." ," immarg ? ." )"
	1 immarg +!
    else
	normal-stack-access1
    endif ;

: stack-depth { stack -- n }
    current-depth stack stack-number @ th @ ;

: part-stack-access { n stack -- }
    \ print _<stack><x>, x=inst-stream? n : maxdepth-currentdepth-n-1
    ." _" stack stack-pointer 2@ type
    stack stack-number @ { stack# }
    stack stack-depth n + { access-depth }
    stack inst-stream-f = if
	access-depth
    else
	combined prim-stacks-in stack# th @
	assert( dup max-depth stack# th @ = )
	access-depth - 1-
    endif
    0 .r ;

: part-stack-read { n stack -- }
    stack stack-depth n + ( ndepth )
    stack stack-number @ part-num @ s-c-max-depth @
\    max-depth stack stack-number @ th @ ( ndepth nmaxdepth )
    over <= if ( ndepth ) \ load from memory
	stack state-in normal-stack-access
    else
	drop n stack part-stack-access
    endif ;

: stack-diff ( stack -- n )
    \ in-out
    dup stack-in @ swap stack-out @ - ;

: part-stack-write { n stack -- }
    stack stack-depth n +
    stack stack-number @ part-num @ s-c-max-back-depth @
    over <= if ( ndepth )
	stack combined ['] stack-diff prim-context -
	stack state-out normal-stack-access
    else
	drop n stack part-stack-access
    endif ;

: stack-read ( n stack -- )
    \ print a stack access at index n of stack
    in-part @ if
	part-stack-read
    else
	state-in normal-stack-access
    endif ;

: stack-write ( n stack -- )
    \ print a stack access at index n of stack
    in-part @ if
	part-stack-write
    else
	state-out normal-stack-access
    endif ;

: item-in-index { item -- n }
    \ n is the index of item (in the in-effect)
    item item-stack @ dup >r stack-in @ ( in-size r:stack )
    item r> stack-in-index-xt @ execute ;

: item-stack-type-name ( item -- addr u )
    item-stack @ stack-type @ type-c-name 2@ ;

: fetch-single ( item -- )
    \ fetch a single stack item from its stack
    >r
    ." vm_" r@ item-stack-type-name type
    ." 2" r@ item-type @ print-type-prefix ." ("
    r@ item-in-index r@ item-stack @ stack-read ." ,"
    r@ item-name 2@ type
    ." );" cr
    rdrop ; 

: fetch-double ( item -- )
    \ fetch a double stack item from its stack
    >r
    ." vm_two"
    r@ item-stack-type-name type ." 2"
    r@ item-type @ print-type-prefix ." ("
    r@ item-in-index r@ item-stack @ 2dup ." (Cell)" stack-read
    ." , "                      -1 under+ ." (Cell)" stack-read
    ." , " r@ item-name 2@ type
    ." )" cr
    rdrop ;

: same-as-in? ( item -- f )
    \ f is true iff the offset and stack of item is the same as on input
    >r
    r@ item-stack @ stack-prim-stacks-sync @ if
	rdrop false exit
    endif
    r@ item-first @ if
	rdrop false exit
    endif
    r@ item-name 2@ prim prim-items-wordlist @ search-wordlist 0= abort" bug"
    execute @
    dup r@ =
    if \ item first appeared in output
	drop false
    else
	dup  item-stack  @ r@ item-stack  @ = 
	swap item-offset @ r@ item-offset @ = and
    endif
    rdrop ;

: item-out-index ( item -- n )
    \ n is the index of item (in the out-effect)
    >r r@ item-stack @ stack-out @ r> item-offset @ - 1- ;

: really-store-single ( item -- )
    >r
    ." vm_"
    r@ item-type @ print-type-prefix ." 2"
    r@ item-stack-type-name type ." ("
    r@ item-name 2@ type ." ,"
    r@ item-out-index r@ item-stack @ stack-write ." );"
    rdrop ;

: store-single { item -- }
    item item-stack @ { stack }
    store-optimization @ in-part @ 0= and item same-as-in? and
    item item-in-index  stack state-in  stack-reg       \  in reg/mem
    item item-out-index stack state-out stack-reg = and \ out reg/mem
    0= if
	item really-store-single cr
    endif ;

: store-double ( item -- )
\ !! store optimization is not performed, because it is not yet needed
 >r
 ." vm_"
 r@ item-type @ print-type-prefix ." 2two"
 r@ item-stack-type-name type ." ("
 r@ item-name 2@ type ." , "
 r@ item-out-index r@ item-stack @ 2dup stack-write
 ." , "                       -1 under+ stack-write
 ." )" cr
 rdrop ;

: single ( -- xt1 xt2 n )
    ['] fetch-single ['] store-single 1 ;

: double ( -- xt1 xt2 n )
    ['] fetch-double ['] store-double 2 ;

: s, ( addr u -- )
\ allocate a string
 here swap dup allot move ;

wordlist constant prefixes

: declare ( addr "name" -- )
\ remember that there is a stack item at addr called name
 create , ;

: !default ( w addr -- )
    dup @ if
	2drop \ leave nonzero alone
    else
	!
    endif ;

: create-type { addr u xt1 xt2 n stack -- } ( "prefix" -- )
    \ describes a type
    \ addr u specifies the C type name
    \ stack effect entries of the type start with prefix
    create type% %allot >r
    addr u save-mem r@ type-c-name 2!
    xt1   r@ type-fetch !
    xt2   r@ type-store !
    n     r@ type-size !
    stack r@ type-stack !
    rdrop ;

: type-prefix ( addr u xt1 xt2 n stack "prefix" -- )
    get-current >r prefixes set-current
    create-type r> set-current
does> ( item -- )
    \ initialize item
    { item typ }
    typ item item-type !
    typ type-stack @ item item-stack !default
    item item-name 2@ prim prim-items-wordlist @ search-wordlist 0= if
	item item-name 2@ nextname item declare
	item item-first on
	\ typ type-c-name 2@ type space type  ." ;" cr
    else
	drop
	item item-first off
    endif ;

: execute-prefix ( item addr1 u1 -- )
    \ execute the word ( item -- ) associated with the longest prefix
    \ of addr1 u1
    0 swap ?do
	dup i prefixes search-wordlist
	if \ ok, we have the type ( item addr1 xt )
	    nip execute
	    UNLOOP EXIT
	endif
	-1 s+loop
	\ we did not find a type, abort
	abort
    false s" unknown prefix" ?print-error ;

: declaration ( item -- )
    dup item-name 2@ execute-prefix ;

: declaration-list ( addr1 addr2 -- )
    ['] declaration map-items ;

: declarations ( -- )
 wordlist dup prim prim-items-wordlist ! set-current
 prim prim-effect-in prim prim-effect-in-end @ declaration-list
 prim prim-effect-out prim prim-effect-out-end @ declaration-list ;

: print-declaration { item -- }
    item item-first @ if
	item item-type @ type-c-name 2@ type space
	item item-name 2@ type ." ;" cr
    endif ;

: print-declarations ( -- )
    prim prim-effect-in  prim prim-effect-in-end  @ ['] print-declaration map-items
    prim prim-effect-out prim prim-effect-out-end @ ['] print-declaration map-items ;
    
: stack-prefix ( stack "prefix" -- )
    get-current >r prefixes set-current
    name tuck nextname create ( stack length ) 2,
    r> set-current
does> ( item -- )
    2@ { item stack prefix-length }
    item item-name 2@ prefix-length /string item item-name 2!
    stack item item-stack !
    item declaration ;

: set-prim-stacks-sync ( stack -- )
    stack-prim-stacks-sync on ;

: clear-prim-stacks-sync ( stack -- )
    stack-prim-stacks-sync off ;


get-current prefixes set-current
: ... ( item -- )
    \ this "prefix" ensures that the appropriate stack is synced with memory
    dup item-name 2@ s" ..." str= 0= abort" '...' must end the item name"
    item-stack @ dup if
	set-prim-stacks-sync
    else \ prefixless "..." syncs all stacks
	drop ['] set-prim-stacks-sync map-stacks1
    endif ;
set-current

create ...-item ( -- addr ) \ just used for letting stack-prefixes work on it
item% %allot                \ stores the stack temporarily until used by ...

: init-item1 ( addr1 addr u -- addr2 )
    \ initialize item at addr1 with name addr u, next item is at addr2
    \ !! make sure that any mention of "..." is only stack-prefixed
    2dup s" ..." search nip nip if ( addr1 addr u )
	0 ...-item item-stack ! \ initialize to prefixless
	2dup ...-item item-name 2!
	...-item rot rot execute-prefix ( addr1 )
    else
	2 pick init-item item% %size +
    endif ;

\ types pointed to by stacks for use in combined prims
\ !! output-c-combined shouldn't use these names!
: stack-type-name ( addr u "name" -- )
    single 0 create-type ;

wordlist constant type-names \ this is here just to meet the requirement
                    \ that a type be a word; it is never used for lookup

: define-type ( addr u -- xt )
    \ define single type with name addr u, without stack
    get-current type-names set-current >r
    2dup nextname stack-type-name
    r> set-current
    latestxt ;

: stack ( "name" "stack-pointer" "type" -- )
    \ define stack
    name { d: stack-name }
    name { d: stack-pointer }
    name { d: stack-type }
    stack-type define-type
    stack-pointer rot >body stack-name nextname make-stack ;

stack inst-stream IP Cell
' inst-in-index inst-stream stack-in-index-xt !
' inst-stream <is> inst-stream-f
\ !! initialize stack-in and stack-out

\ registers

: make-register ( type addr u -- )
    \ define register with type TYPE and name ADDR U.
    nregisters @ max-registers < s" too many registers" ?print-error
    2dup nextname create register% %allot >r
    r@ register-name 2!
    r@ register-type !
    nregisters @ r@ register-number !
    1 nregisters +!
    rdrop ;

: register ( "name" "type" -- )
    \ define register
    name { d: reg-name }
    name { d: reg-type }
    reg-type define-type >body
    reg-name make-register ;

\ stack-states

: stack-state ( a-addr u uoffset "name" -- )
    create ss% %allot >r
    r@ ss-offset !
    r@ ss-registers 2!
    rdrop ;

0 0 0 stack-state default-ss

\ state

: state ( "name" -- )
    \ create a state initialized with default-sss
    create state% %allot { s }
    next-state-number @ s state-number ! 1 next-state-number +!
    max-stacks 0 ?do
	default-ss s state-sss i th !
    loop ;

: .state ( state -- )
    0 >body - >name .name ;

: set-ss ( ss stack state -- )
    state-sss swap stack-number @ th ! ;

\ offset computation
\ the leftmost (i.e. deepest) item has offset 0
\ the rightmost item has the highest offset

: compute-offset { item xt -- }
    \ xt specifies in/out; update stack-in/out and set item-offset
    item item-type @ type-size @
    item item-stack @ xt execute dup @ >r +!
    r> item item-offset ! ;

: compute-offset-in ( addr1 addr2 -- )
    ['] stack-in compute-offset ;

: compute-offset-out ( addr1 addr2 -- )
    ['] stack-out compute-offset ;

: compute-offsets ( -- )
    prim prim-stacks-in  max-stacks cells erase
    prim prim-stacks-out max-stacks cells erase
    prim prim-effect-in  prim prim-effect-in-end  @ ['] compute-offset-in  map-items
    prim prim-effect-out prim prim-effect-out-end @ ['] compute-offset-out map-items
    inst-stream stack-out @ 0= s" # can only be on the input side" ?print-error ;

: init-simple { prim -- }
    \ much of the initialization is elsewhere
    ['] clear-prim-stacks-sync map-stacks ;

: process-simple ( -- )
    prim prim { W^ key } key cell
    combinations ['] constant insert-wordlist
    declarations compute-offsets
    output @ execute ;

: stack-state-items ( stack state -- n )
    state-ss ss-registers 2@ nip ;

: unused-stack-items { stack -- n-in n-out }
    \ n-in  are the stack items in state-in  not used    by prim
    \ n-out are the stack items in state-out not written by prim
    stack state-in  stack-state-items stack stack-in  @ - 0 max
    stack state-out stack-state-items stack stack-out @ - 0 max ;

: spill-stack-items { stack -- u }
    \ there are u items to spill in stack
    stack unused-stack-items
    stack stack-prim-stacks-sync @ if
	drop 0
    endif
    swap - ;

: spill-stack { stack -- }
    \ spill regs of state-in that are not used by prim and are not in state-out
    stack state-in stack-offset { offset }
    stack state-in stack-state-items ( items )
    dup stack spill-stack-items + +do
	\ loop through the bottom items
	stack stack-pointer 2@ type
	i offset - stack normal-stack-access0 ."  = "
	i stack state-in normal-stack-access1 ." ;" cr
    loop ;

: spill-state ( -- )
    ['] spill-stack map-stacks1 ;

: fill-stack-items { stack -- u }
    \ there are u items to fill in stack
    stack unused-stack-items
    stack stack-prim-stacks-sync @ if
	swap drop 0 swap
    endif
    - ;

: fill-stack { stack -- }
    stack state-out stack-offset { offset }
    stack state-out stack-state-items ( items )
    dup stack fill-stack-items + +do
	\ loop through the bottom items
	i stack state-out normal-stack-access1 ."  = "
	stack stack-pointer 2@ type
	i offset - stack normal-stack-access0 ." ;" cr
    loop ;

: fill-state ( -- )
    \ !! inst-stream for prefetching?
    ['] fill-stack map-stacks1 ;

: fetch ( addr -- )
    dup item-type @ type-fetch @ execute ;

: fetches ( -- )
    prim prim-effect-in prim prim-effect-in-end @ ['] fetch map-items ;

: reg-reg-move ( reg-from reg-to -- )
    2dup = if
	2drop
    else
	.reg ."  = " .reg ." ;" cr
    endif ;

: stack-bottom-reg { n stack state -- reg }
    stack state stack-state-items n - 1- stack state stack-reg ;

: stack-moves { stack -- }
    \ generate moves between registers in state-in/state-out that are
    \ not spilled or consumed/produced by prim.
    \ !! this works only for a simple stack cache, not e.g., for
    \ rotating stack caches, or registers shared between stacks (the
    \ latter would also require a change in interface)
    \ !! maybe place this after NEXT_P1?
    stack unused-stack-items 2dup < if ( n-in n-out )
	\ move registers from 0..n_in-1 to n_out-n_in..n_out-1
	over - { diff } ( n-in )
	-1 swap 1- -do
	    i stack state-in stack-bottom-reg ( reg-from )
	    i diff + stack state-out stack-bottom-reg reg-reg-move
	1 -loop
    else
	\ move registers from n_in-n_out..n_in-1 to 0..n_out-1
	swap over - { diff } ( n-out )
	0 +do
	    i diff + stack state-in stack-bottom-reg ( reg-from )
	    i stack state-out stack-bottom-reg reg-reg-move
	loop
    endif ;

: stack-update-transform ( n1 stack -- n2 )
    \ n2 is the number by which the stack pointer should be
    \ incremented to pop n1 items
    stack-access-transform @ dup >r execute
    0 r> execute - ;

: update-stack-pointer { stack n -- }
    n if \ this check is not necessary, gcc would do this for us
	stack inst-stream = if
	    ." INC_IP(" n 0 .r ." );" cr
	else
	    stack stack-pointer 2@ type ."  += "
	    n stack stack-update-transform 0 .r ." ;" cr
	endif
    endif ;

: stack-pointer-update { stack -- }
    \ and moves
    \ stacks grow downwards
    stack stack-prim-stacks-sync @ if
	stack stack-in @
	stack state-in  stack-offset -
	stack swap update-stack-pointer
    else
	stack stack-diff ( in-out )
	stack state-in  stack-offset -
	stack state-out stack-offset + ( [in-in_offset]-[out-out_offset] )
	stack swap update-stack-pointer
	stack stack-moves
    endif ;

: stack-pointer-updates ( -- )
    ['] stack-pointer-update map-stacks ;

: stack-pointer-update2 { stack -- }
    stack stack-prim-stacks-sync @ if
	stack state-out stack-offset
	stack stack-out @ -
	stack swap update-stack-pointer
    endif ;

: stack-pointer-updates2 ( -- )
    \ update stack pointers after C code, where necessary
    ['] stack-pointer-update2 map-stacks ;

: store ( item -- )
\ f is true if the item should be stored
\ f is false if the store is probably not necessary
 dup item-type @ type-store @ execute ;

: stores ( -- )
    prim prim-effect-out prim prim-effect-out-end @ ['] store map-items ;

: print-debug-arg { item -- }
    ." fputs(" quote space item item-name 2@ type ." =" quote ." , vm_out); "
    ." printarg_" item item-type @ print-type-prefix
    ." (" item item-name 2@ type ." );" cr ;
    
: print-debug-args ( -- )
    ." #ifdef VM_DEBUG" cr
    ." if (vm_debug) {" cr
    prim prim-effect-in prim prim-effect-in-end @ ['] print-debug-arg map-items
\    ." fputc('\n', vm_out);" cr
    ." }" cr
    ." #endif" cr ;

: print-debug-result { item -- }
    item item-first @ if
	item print-debug-arg
    endif ;

: print-debug-results ( -- )
    cr
    ." #ifdef VM_DEBUG" cr
    ." if (vm_debug) {" cr
    ." fputs(" quote ."  -- " quote ." , vm_out); "
    prim prim-effect-out prim prim-effect-out-end @ ['] print-debug-result map-items
    ." fputc('\n', vm_out);" cr
    ." }" cr
    ." #endif" cr ;

: output-super-end ( -- )
    prim prim-c-code 2@ s" SET_IP" search if
	." SUPER_END;" cr
    endif
    2drop ;


defer output-nextp0
:noname ( -- )
    ." NEXT_P0;" cr ;
is output-nextp0

defer output-nextp1
:noname ( -- )
    ." NEXT_P1;" cr ;
is output-nextp1

: output-nextp2 ( -- )
    ." NEXT_P2;" cr ;

variable tail-nextp2 \ xt to execute for printing NEXT_P2 in INST_TAIL
' output-nextp2 tail-nextp2 !

: output-label2 ( -- )
    ." LABEL2(" prim prim-c-name 2@ type ." )" cr
    ." NEXT_P1_5;" cr
    ." LABEL3(" prim prim-c-name 2@ type ." )" cr
    ." DO_GOTO;" cr ;

: output-c-tail1 { xt -- }
    \ the final part of the generated C code, with xt printing LABEL2 or not.
    output-super-end
    print-debug-results
    output-nextp1
    stack-pointer-updates2
    stores
    fill-state 
    xt execute ;

: output-c-vm-jump-tail ( -- )
    \ !! this functionality not yet implemented for superinstructions
    output-super-end
    print-debug-results
    stores
    fill-state 
    ." LABEL2(" prim prim-c-name 2@ type ." )" cr
    ." LABEL3(" prim prim-c-name 2@ type ." )" cr
    ." DO_GOTO;" cr ;

: output-c-tail1-no-stores { xt -- }
    \ the final part of the generated C code for combinations
    output-super-end
    output-nextp1
    fill-state 
    xt execute ;

: output-c-tail ( -- )
    tail-nextp2 @ output-c-tail1 ;

: output-c-tail2 ( -- )
    prim prim-c-code 2@ s" VM_JUMP(" search nip nip if
	output-c-vm-jump-tail
    else
	['] output-label2 output-c-tail1
    endif ;

: output-c-tail-no-stores ( -- )
    tail-nextp2 @ output-c-tail1-no-stores ;

: output-c-tail2-no-stores ( -- )
    ['] output-label2 output-c-tail1-no-stores ;

: type-c-code ( c-addr u xt -- )
    \ like TYPE, but replaces "INST_TAIL;" with tail code produced by xt
    { xt }
    ." {" cr
    ." #line " c-line @ . quote c-filename 2@ type quote cr
    begin ( c-addr1 u1 )
	2dup s" INST_TAIL;" search
    while ( c-addr1 u1 c-addr3 u3 )
	2dup 2>r drop nip over - type
	xt execute
	2r> 10 /string
	\ !! resync #line missing
    repeat
    2drop type
    ." #line " out-nls @ 2 + . quote out-filename 2@ type quote cr
    ." }" cr ;

: print-entry ( -- )
    ." LABEL(" prim prim-c-name 2@ type ." )" ;

: prim-type ( addr u -- )
    \ print out a primitive, but avoid "*/"
    2dup s" */" search  nip nip  IF
	bounds ?DO  I c@ dup '* = IF  drop 'x  THEN  emit  LOOP
    ELSE  type  THEN ;

: output-c ( -- )
    print-entry ."  /* " prim prim-name 2@ prim-type
    ."  ( " prim prim-stack-string 2@ type ." ) "
    state-in .state ." -- " state-out .state ."  */" cr
    ." /* " prim prim-doc 2@ type ."  */" cr
    ." NAME(" quote prim prim-name 2@ type quote ." )" cr \ debugging
    ." {" cr
    ." DEF_CA" cr
    print-declarations
    output-nextp0
    spill-state
    fetches
    print-debug-args
    stack-pointer-updates
    prim prim-c-code 2@ ['] output-c-tail type-c-code
    output-c-tail2
    ." }" cr
    cr
;

: disasm-arg { item -- }
    item item-stack @ inst-stream = if
	." {" cr
	item print-declaration
	item fetch
	item print-debug-arg
	." }" cr
    endif ;

: disasm-args ( -- )
    prim prim-effect-in prim prim-effect-in-end @ ['] disasm-arg map-items ;

: output-disasm ( -- )
    \ generate code for disassembling VM instructions
    ." if (VM_IS_INST(*ip, " function-number @ 0 .r ." )) {" cr
    ."   fputs(" quote prim prim-name 2@ type quote ." , vm_out);" cr
    disasm-args
    ."   ip += " inst-stream stack-in @ 1+ 0 .r ." ;" cr
    ."   goto _endif_;" cr
    ." }" cr ;

: output-profile ( -- )
    \ generate code for postprocessing the VM block profile stuff
    ." if (VM_IS_INST(*ip, " function-number @ 0 .r ." )) {" cr
    ."   add_inst(b, " quote prim prim-name 2@ type quote ." );" cr
    ."   ip += " inst-stream stack-in @ 1+ 0 .r ." ;" cr
    prim prim-c-code 2@  s" SET_IP"    search nip nip
    prim prim-c-code 2@  s" SUPER_END" search nip nip or if
	."   return;" cr
    else
	."   goto _endif_;" cr
    endif
    ." }" cr ;

: output-profile-part ( p )
    ."   add_inst(b, " quote
    prim-name 2@ type
    quote ." );" cr ;
    
: output-profile-combined ( -- )
    \ generate code for postprocessing the VM block profile stuff
    ." if (VM_IS_INST(*ip, " function-number @ 0 .r ." )) {" cr
    ['] output-profile-part map-combined
    ."   ip += " inst-stream stack-in @ 1+ 0 .r ." ;" cr
    combined-prims num-combined @ 1- th @ prim-c-code 2@  s" SET_IP"    search nip nip
    combined-prims num-combined @ 1- th @ prim-c-code 2@  s" SUPER_END" search nip nip or if
	."   return;" cr
    else
	."   goto _endif_;" cr
    endif
    ." }" cr ;

: prim-branch? { prim -- f }
    \ true if prim is a branch or super-end
    prim prim-c-code 2@  s" SET_IP" search nip nip 0<> ;

: output-superend ( -- )
    \ output flag specifying whether the current word ends a dynamic superinst
    prim prim-branch?
    prim prim-c-code 2@  s" SUPER_END" search nip nip 0<> or
    prim prim-c-code 2@  s" SUPER_CONTINUE" search nip nip 0= and
    negate 0 .r ." , /* " prim prim-name 2@ prim-type ."  */" cr ;

: gen-arg-parm { item -- }
    item item-stack @ inst-stream = if
	." , " item item-type @ type-c-name 2@ type space
	item item-name 2@ type
    endif ;

: gen-args-parm ( -- )
    prim prim-effect-in prim prim-effect-in-end @ ['] gen-arg-parm map-items ;

: gen-arg-gen { item -- }
    item item-stack @ inst-stream = if
	."   genarg_" item item-type @ print-type-prefix
        ." (ctp, " item item-name 2@ type ." );" cr
    endif ;

: gen-args-gen ( -- )
    prim prim-effect-in prim prim-effect-in-end @ ['] gen-arg-gen map-items ;

: output-gen ( -- )
    \ generate C code for generating VM instructions
    ." void gen_" prim prim-c-name 2@ type ." ("
    threaded-code-pointer-type 2@ type ." ctp" gen-args-parm ." )" cr
    ." {" cr
    ."   gen_inst(ctp, " function-number @ 0 .r ." );" cr
    gen-args-gen
    ." }" cr ;

: stack-used? { stack -- f }
    stack stack-in @ stack stack-out @ or 0<> ;

: output-funclabel ( -- )
  ." &I_" prim prim-c-name 2@ type ." ," cr ;

: output-forthname ( -- )
  '" emit prim prim-name 2@ type '" emit ." ," cr ;

\  : output-c-func ( -- )
\  \ used for word libraries
\      ." Cell * I_" prim prim-c-name 2@ type ." (Cell *SP, Cell **FP)      /* " prim prim-name 2@ type
\      ."  ( " prim prim-stack-string 2@ type ."  ) */" cr
\      ." /* " prim prim-doc 2@ type ."  */" cr
\      ." NAME(" quote prim prim-name 2@ type quote ." )" cr
\      \ debugging
\      ." {" cr
\      print-declarations
\      \ !! don't know what to do about that
\      inst-stream  stack-used? IF ." Cell *ip=IP;" cr THEN
\      data-stack   stack-used? IF ." Cell *sp=SP;" cr THEN
\      fp-stack     stack-used? IF ." Cell *fp=*FP;" cr THEN
\      return-stack stack-used? IF ." Cell *rp=*RP;" cr THEN
\      spill-state
\      fetches
\      stack-pointer-updates
\      fp-stack   stack-used? IF ." *FP=fp;" cr THEN
\      ." {" cr
\      ." #line " c-line @ . quote c-filename 2@ type quote cr
\      prim prim-c-code 2@ type
\      ." }" cr
\      stores
\      fill-state
\      ." return (sp);" cr
\      ." }" cr
\      cr ;

: output-label ( -- )  
    ." INST_ADDR(" prim prim-c-name 2@ type ." )," cr ;

: output-alias ( -- ) 
    ( primitive-number @ . ." alias " ) ." Primitive " prim prim-name 2@ type cr ;

defer output-c-prim-num ( -- )

:noname ( -- )
    ." N_" prim prim-c-name 2@ type ." ," cr ;
is output-c-prim-num

: output-forth ( -- )  
    prim prim-forth-code @ 0=
    IF    	\ output-alias
	\ this is bad for ec: an alias is compiled if tho word does not exist!
	\ JAW
    ELSE  ." : " prim prim-name 2@ type ."   ( "
	prim prim-stack-string 2@ type ." )" cr
	prim prim-forth-code 2@ type cr
    THEN ;

: output-tag-file ( -- )
    name-filename 2@ last-name-filename 2@ compare if
	name-filename 2@ last-name-filename 2!
	#ff emit cr
	name-filename 2@ type
	." ,0" cr
    endif ;

: output-tag ( -- )
    output-tag-file
    prim prim-name 2@ 1+ type
    127 emit
    space prim prim-name 2@ type space
    1 emit
    name-line @ 0 .r
    ." ,0" cr ;

: output-vi-tag ( -- )
    name-filename 2@ type #tab emit
    prim prim-name 2@ type #tab emit
    ." /^" prim prim-name 2@ type ."  *(/" cr ;

[IFDEF] documentation
: register-doc ( -- )
    prim prim-name 2@ documentation ['] create insert-wordlist
    prim prim-name 2@ 2,
    prim prim-stack-string 2@ condition-stack-effect 2,
    prim prim-wordset 2@ 2,
    prim prim-c-name 2@ condition-pronounciation 2,
    prim prim-doc 2@ 2, ;
[THEN]


\ combining instructions

\ The input should look like this:

\ lit_+ = lit +

\ The output should look like this:

\  I_lit_+:
\  {
\  DEF_CA
\  Cell _x_ip0;
\  Cell _x_sp0;
\  Cell _x_sp1;
\  NEXT_P0;
\  _x_ip0 = (Cell) IPTOS;
\  _x_sp0 = (Cell) spTOS;
\  INC_IP(1);
\  /* sp += 0; */
\  /* lit ( #w -- w ) */
\  /*  */
\  NAME("lit")
\  {
\  Cell w;
\  w = (Cell) _x_ip0;
\  #ifdef VM_DEBUG
\  if (vm_debug) {
\  fputs(" w=", vm_out); printarg_w (w);
\  fputc('\n', vm_out);
\  }
\  #endif
\  {
\  #line 136 "./prim"
\  }
\  _x_sp1 = (Cell)w;
\  }
\  I_plus:	/* + ( n1 n2 -- n ) */
\  /*  */
\  NAME("+")
\  {
\  DEF_CA
\  Cell n1;
\  Cell n2;
\  Cell n;
\  NEXT_P0;
\  n1 = (Cell) _x_sp0;
\  n2 = (Cell) _x_sp1;
\  #ifdef VM_DEBUG
\  if (vm_debug) {
\  fputs(" n1=", vm_out); printarg_n (n1);
\  fputs(" n2=", vm_out); printarg_n (n2);
\  fputc('\n', vm_out);
\  }
\  #endif
\  {
\  #line 516 "./prim"
\  n = n1+n2;
\  }
\  _x_sp0 = (Cell)n;
\  }
\  NEXT_P1;
\  spTOS = (Cell)_x_sp0;
\  NEXT_P2;

: init-combined ( -- )
    prim to combined
    0 num-combined !
    current-depth max-stacks cells erase
    include-skipped-insts @ current-depth 0 th !
    max-depth     max-stacks cells erase
    min-depth     max-stacks cells erase
    prim prim-effect-in  prim prim-effect-in-end  !
    prim prim-effect-out prim prim-effect-out-end ! ;

: max! ( n addr -- )
    tuck @ max swap ! ;

: min! ( n addr -- )
    tuck @ min swap ! ;

: inst-stream-adjustment ( nstack -- n )
    \ number of stack items to add for each part
    0= include-skipped-insts @ and negate ;

: add-depths { p -- }
    \ combine stack effect of p with *-depths
    max-stacks 0 ?do
	current-depth i th @
	p prim-stacks-in  i th @ + i inst-stream-adjustment +
	dup max-depth i th max!
	p prim-stacks-out i th @ -
	dup min-depth i th min!
	current-depth i th !
    loop ;

: copy-maxdepths ( n -- )
    max-depth max-depths rot max-stacks * th max-stacks cells move ;

: add-prim ( addr u -- )
    \ add primitive given by "addr u" to combined-prims
    primitives search-wordlist s" unknown primitive" ?print-error
    execute { p }
    p combined-prims num-combined @ th !
    num-combined @ copy-maxdepths
    1 num-combined +!
    p add-depths
    num-combined @ copy-maxdepths ;

: compute-effects { q -- }
    \ compute the stack effects of q from the depths
    max-stacks 0 ?do
	max-depth i th @ dup
	q prim-stacks-in i th !
	current-depth i th @ -
	q prim-stacks-out i th !
    loop ;

: make-effect-items { stack# items effect-endp -- }
    \ effect-endp points to a pointer to the end of the current item-array
    \ and has to be updated
    stacks stack# th @ { stack }
    items 0 +do
	effect-endp @ { item }
	i 0 <# #s stack stack-pointer 2@ holds [char] _ hold #> save-mem
	item item-name 2!
	stack item item-stack !
	stack stack-type @ item item-type !
	i item item-offset !
	item item-first on
	item% %size effect-endp +!
    loop ;

: init-effects { q -- }
    \ initialize effects field for FETCHES and STORES
    max-stacks 0 ?do
	i q prim-stacks-in  i th @ q prim-effect-in-end  make-effect-items
	i q prim-stacks-out i th @ q prim-effect-out-end make-effect-items
    loop ;

: compute-stack-max-back-depths ( stack -- )
    stack-number @ { stack# }
    current-depth stack# th @ dup
    dup stack# num-combined @ s-c-max-back-depth !
    -1 num-combined @ 1- -do ( max-depth current-depth )
	combined-prims i th @ { p }
	p prim-stacks-out stack# th @ +
	dup >r max r>
	over stack# i s-c-max-back-depth !
	p prim-stacks-in stack# th @ -
	stack# inst-stream-adjustment -
    1 -loop
    assert( dup stack# inst-stream-adjustment negate = )
    assert( over max-depth stack# th @ = )
    2drop ;

: compute-max-back-depths ( -- )
    \ compute max-back-depths.
    \ assumes that current-depths is correct for the end of the combination
    ['] compute-stack-max-back-depths map-stacks ;

: process-combined ( -- )
    combined combined-prims num-combined @ cells
    combinations ['] constant insert-wordlist
    combined-prims num-combined @ 1- th ( last-part )
    @ prim-c-code 2@ prim prim-c-code 2! \ used by output-super-end
    prim compute-effects
    prim init-effects
    compute-max-back-depths
    output-combined perform ;

\ reprocessing (typically to generate versions for another cache states)
\ !! use prim-context

variable reprocessed-num 0 reprocessed-num !

: new-name ( -- c-addr u )
    reprocessed-num @ 0
    1 reprocessed-num +!
    <# #s 'p hold '_ hold #> save-mem ;

: reprocess-simple ( prim -- )
    to prim
    new-name prim prim-c-name 2!
    output @ execute ;

: lookup-prim ( c-addr u -- prim )
    primitives search-wordlist 0= -13 and throw execute ;

: state-prim1 { in-state out-state prim -- }
    in-state out-state state-default dup d= ?EXIT
    in-state  to state-in
    out-state to state-out
    prim reprocess-simple ;

: state-prim ( in-state out-state "name" -- )
    parse-word lookup-prim state-prim1 ;

\ reprocessing with default states

\ This is a simple scheme and should be generalized
\ assumes we only cache one stack and use simple states for that

0 value cache-stack  \ stack that we cache
2variable cache-states \ states of the cache, starting with the empty state

: compute-default-state-out ( n-in -- n-out )
    \ for the current prim
    cache-stack stack-in @ - 0 max
    cache-stack stack-prim-stacks-sync @ if
	drop 0
    endif
    cache-stack stack-out @ + cache-states 2@ nip 1- min ;

: gen-prim-states ( prim -- )
    to prim
    cache-states 2@ swap { states } ( nstates )
    cache-stack stack-in @ +do
	states i th @
	states i compute-default-state-out th @
	prim state-prim1
    loop ;

: prim-states ( "name" -- )
    parse-word lookup-prim gen-prim-states ;

: gen-branch-states ( prim -- )
    \ generate versions that produce state-default; useful for branches
    to prim
    cache-states 2@ swap { states } ( nstates )
    cache-stack stack-in @ +do
	states i th @ state-default prim state-prim1
    loop ;

: branch-states ( out-state "name" -- )
    parse-word lookup-prim gen-branch-states ;

\ producing state transitions

: gen-transitions ( "name" -- )
    parse-word lookup-prim { prim }
    cache-states 2@ { states nstates }
    nstates 0 +do
	nstates 0 +do
	    i j <> if
		states i th @ states j th @ prim state-prim1
	    endif
	loop
    loop ;

\ C output

: print-item { n stack -- }
    \ print nth stack item name
    stack stack-type @ type-c-name 2@ type space
    ." MAYBE_UNUSED _" stack stack-pointer 2@ type n 0 .r ;

: print-declarations-combined ( -- )
    max-stacks 0 ?do
	max-depth i th @ min-depth i th @ - 0 +do
	    i stacks j th @ print-item ." ;" cr
	loop
    loop ;

: part-fetches ( -- )
    fetches ;

: part-output-c-tail ( -- )
    print-debug-results
    stores ;

: output-combined-tail ( -- )
    part-output-c-tail
    in-part @ >r in-part off
    combined ['] output-c-tail-no-stores prim-context
    r> in-part ! ;

: part-stack-pointer-updates ( -- )
    next-stack-number @ 0 +do
	i part-num @ 1+ s-c-max-depth @ dup
	i num-combined @ s-c-max-depth @ =    \ final depth
	swap i part-num @ s-c-max-depth @ <> \ just reached now
	part-num @ 0= \ first part
	or and if
	    stacks i th @ stack-pointer-update
	endif
    loop ;

: output-part ( p -- )
    to prim
    ." /* " prim prim-name 2@ prim-type ."  ( " prim prim-stack-string 2@ type ." ) */" cr
    ." NAME(" quote prim prim-name 2@ type quote ." )" cr \ debugging
    ." {" cr
    print-declarations
    part-fetches
    print-debug-args
    combined ['] part-stack-pointer-updates prim-context
    1 part-num +!
    prim add-depths \ !! right place?
    prim prim-c-code 2@ ['] output-combined-tail type-c-code
    part-output-c-tail
    ." }" cr ;

: output-parts ( -- )
    prim >r in-part on
    current-depth max-stacks cells erase
    0 part-num !
    ['] output-part map-combined
    in-part off
    r> to prim ;

: output-c-combined ( -- )
    print-entry cr
    \ debugging messages just in parts
    ." {" cr
    ." DEF_CA" cr
    print-declarations-combined
    output-nextp0
    spill-state
    \ fetches \ now in parts
    \ print-debug-args
    \ stack-pointer-updates now in parts
    output-parts
    output-c-tail2-no-stores
    ." }" cr
    cr ;

: output-forth-combined ( -- )
;


\ peephole optimization rules

\ data for a simple peephole optimizer that always tries to combine
\ the currently compiled instruction with the last one.

\ in order for this to work as intended, shorter combinations for each
\ length must be present, and the longer combinations must follow
\ shorter ones (this restriction may go away in the future).
  
: output-peephole ( -- )
    combined-prims num-combined @ 1- cells combinations search-wordlist
    s" the prefix for this superinstruction must be defined earlier" ?print-error
    ." {"
    execute prim-num @ 5 .r ." ,"
    combined-prims num-combined @ 1- th @ prim-num @ 5 .r ." ,"
    combined prim-num @ 5 .r ." }, /* "
    combined prim-c-name 2@ type ."  */"
    cr ;


\ cost and superinstruction data for a sophisticated combiner (e.g.,
\ shortest path)

\ This is intended as initializer for a structure like this

\  struct cost {
\    char loads;       /* number of stack loads */
\    char stores;      /* number of stack stores */
\    char updates;     /* number of stack pointer updates */
\    char branch;      /* is it a branch (SET_IP) */
\    char state_in;    /* state on entry */
\    char state_out;   /* state on exit */
\    short offset;     /* offset into super2 table */
\    char length;      /* number of components */
\  };

\ How do you know which primitive or combined instruction this
\ structure refers to?  By the order of cost structures, as in most
\ other cases.

: super2-length ( -- n )
    combined if
	num-combined @
    else
	1
    endif ;

: compute-costs { p -- nloads nstores nupdates }
    \ compute the number of loads, stores, and stack pointer updates
    \ of a primitive or combined instruction; does not take TOS
    \ caching into account
    0 max-stacks 0 +do
	p prim-stacks-in i th @ +
    loop
    super2-length 1- - \ don't count instruction fetches of subsumed insts
    0 max-stacks 0 +do
	p prim-stacks-out i th @ +
    loop
    0 max-stacks 1 +do \ don't count ip updates, therefore "1 +do"
	p prim-stacks-in i th @ p prim-stacks-out i th @ <> -
    loop ;

: output-num-part ( p -- )
    ." N_" prim-c-name-orig 2@ type ." ," ;
    \ prim-num @ 4 .r ." ," ;

: output-name-comment ( -- )
    ."  /* " prim prim-name 2@ prim-type ."  */" ;

variable offset-super2  0 offset-super2 ! \ offset into the super2 table

: output-costs-prefix ( -- )
    ." {" prim compute-costs
    rot 2 .r ." ," swap 2 .r ." ," 2 .r ." , "
    prim prim-branch? negate . ." ,"
    state-in  state-number @ 2 .r ." ,"
    state-out state-number @ 2 .r ." ,"
    inst-stream stack-in @ 1 .r ." ,"
;

: output-costs-gforth-simple ( -- )
    output-costs-prefix
    prim output-num-part
    1 2 .r ." },"
    output-name-comment
    cr ;

: output-costs-gforth-combined ( -- )
    output-costs-prefix
    ." N_START_SUPER+" offset-super2 @ 5 .r ." ,"
    super2-length dup 2 .r ." }," offset-super2 +!
    output-name-comment
    cr ;

\  : output-costs ( -- )
\      \ description of superinstructions and simple instructions
\      ." {" prim compute-costs
\      rot 2 .r ." ," swap 2 .r ." ," 2 .r ." ,"
\      offset-super2 @ 5 .r ." ,"
\      super2-length dup 2 .r ." ," offset-super2 +!
\      inst-stream stack-in @ 1 .r ." },"
\      output-name-comment
\      cr ;

: output-super2-simple ( -- )
    prim prim-c-name 2@ prim prim-c-name-orig 2@ d= if
	prim output-num-part
	output-name-comment
	cr
    endif ;   
  
: output-super2-combined ( -- )
    ['] output-num-part map-combined 
    output-name-comment
    cr ;   

\ the parser

eof-char max-member \ the whole character set + EOF

: getinput ( -- n )
 rawinput @ endrawinput @ =
 if
   eof-char
 else
   cookedinput @ c@
 endif ;

:noname ( n -- )
 dup bl > if
  emit space
 else
  .
 endif ;
print-token !

: testchar? ( set -- f )
 getinput member? ;
' testchar? test-vector !

: checksynclines ( -- )
    \ when input points to a newline, check if the next line is a
    \ sync line.  If it is, perform the appropriate actions.
    rawinput @ begin >r
	s" #line " r@ over compare if
	    rdrop 1 line +! EXIT
	endif
	0. r> 6 chars + 20 >number drop >r drop line ! r> ( c-addr )
	dup c@ bl = if
	    char+ dup c@ [char] " <> 0= s" sync line syntax" ?print-error
	    char+ dup 100 [char] " scan drop swap 2dup - save-mem filename 2!
	    char+
	endif
	dup c@ nl-char <> 0= s" sync line syntax" ?print-error
	skipsynclines @ if
	    char+ dup rawinput !
	    rawinput @ c@ cookedinput @ c!
	endif
    again ;

: ?nextchar ( f -- )
    s" syntax error, wrong char" ?print-error
    rawinput @ endrawinput @ <> if
	rawinput @ c@
	1 chars rawinput +!
	1 chars cookedinput +!
	nl-char = if
	    checksynclines
	    rawinput @ line-start !
	endif
	rawinput @ c@
	cookedinput @ c!
    endif ;

: charclass ( set "name" -- )
 ['] ?nextchar terminal ;

: .. ( c1 c2 -- set )
 ( creates a set that includes the characters c, c1<=c<=c2 )
 empty copy-set
 swap 1+ rot do
  i over add-member
 loop ;

: ` ( -- terminal ) ( use: ` c )
 ( creates anonymous terminal for the character c )
 char singleton ['] ?nextchar make-terminal ;

char a char z ..  char A char Z ..  union char _ singleton union  charclass letter
char 0 char 9 ..					charclass digit
bl singleton tab-char over add-member			charclass white
nl-char singleton eof-char over add-member complement	charclass nonl
nl-char singleton eof-char over add-member
    char : over add-member complement                   charclass nocolonnl
nl-char singleton eof-char over add-member
    char } over add-member complement                   charclass nobracenl
bl 1+ maxchar .. char \ singleton complement intersection
                                                        charclass nowhitebq
bl 1+ maxchar ..                                        charclass nowhite
char " singleton eof-char over add-member complement	charclass noquote
nl-char singleton					charclass nl
eof-char singleton					charclass eof
nl-char singleton eof-char over add-member		charclass nleof

(( letter (( letter || digit )) **
)) <- c-ident ( -- )

(( ` . ` . ` .
)) <- sync-stack ( -- )

(( ` # ?? (( letter || digit || ` : )) ++ sync-stack ??
|| sync-stack
)) <- stack-ident ( -- )

(( nowhitebq nowhite ** ))
<- forth-ident ( -- )

Variable forth-flag
Variable c-flag

(( (( ` e || ` E )) {{ start }} nonl ** 
   {{ end evaluate }}
)) <- eval-comment ( ... -- ... )

(( (( ` f || ` F )) {{ start }} nonl ** 
   {{ end forth-flag @ IF type cr ELSE 2drop THEN }}
)) <- forth-comment ( -- )

(( (( ` c || ` C )) {{ start }} nonl ** 
   {{ end c-flag @ IF type cr ELSE 2drop THEN }}
)) <- c-comment ( -- )

(( ` - nonl ** {{ 
	forth-flag @ IF forth-fdiff ." [ELSE]" cr THEN
	c-flag @ IF
	    function-diff
	    ." #else /* " function-number @ 0 .r ."  */" cr THEN }}
)) <- else-comment

(( ` + {{ start }} nonl ** {{ end
	dup
	IF	c-flag @
	    IF
		function-diff
		." #ifdef HAS_" bounds ?DO  I c@ toupper emit  LOOP cr
		THEN
		forth-flag @
		IF  forth-fdiff  ." has? " type ."  [IF]"  cr THEN
	ELSE	2drop
	    c-flag @      IF
		function-diff  ." #endif" cr THEN
	    forth-flag @  IF  forth-fdiff  ." [THEN]"  cr THEN
	THEN }}
)) <- if-comment

(( (( ` g || ` G )) {{ start }} nonl **
   {{ end
      forth-flag @ IF  forth-fdiff  ." group " type cr  THEN
      c-flag @     IF  function-diff
	  ." GROUP(" type ." , " function-number @ 0 .r ." )" cr  THEN }}
)) <- group-comment

(( (( eval-comment || forth-comment || c-comment || else-comment || if-comment || group-comment )) ?? nonl ** )) <- comment-body

(( ` \ comment-body nleof )) <- comment ( -- )

(( {{ start }} stack-ident {{ end init-item1 }} white ** )) **
<- stack-items ( addr1 -- addr2 )

(( {{ prim prim-effect-in }}  stack-items {{ prim prim-effect-in-end ! }}
   ` - ` - white **
   {{ prim prim-effect-out }} stack-items {{ prim prim-effect-out-end ! }}
)) <- stack-effect ( -- )

(( {{ prim create-prim prim init-simple }}
   ` ( white ** {{ start }} stack-effect {{ end prim prim-stack-string 2! }} ` ) white **
   (( {{ start }} forth-ident {{ end prim prim-wordset 2! }} white **
      (( {{ start }}  c-ident {{ end 2dup prim-c-name-2! }} )) ??
   )) ??  nleof
   (( ` " ` "  {{ start }} (( noquote ++ ` " )) ++ {{ end 1- prim prim-doc 2! }} ` " white ** nleof )) ??
   {{ skipsynclines off line @ c-line ! filename 2@ c-filename 2! start }}
   (( (( ` { nonl ** nleof (( (( nobracenl {{ line @ drop }} nonl ** )) ?? nleof )) ** ` } white ** nleof white ** ))
   || (( nocolonnl nonl **  nleof white ** )) ** ))
   {{ end prim prim-c-code 2! skipsynclines on }}
   (( ` :  white ** nleof
      {{ start }} (( nonl ++  nleof white ** )) ++ {{ end prim prim-forth-code 2! }}
   )) ?? {{ process-simple }}
   nleof
)) <- simple-primitive ( -- )

(( {{ init-combined }}
   ` = white ** (( {{ start }} forth-ident {{ end add-prim }} white ** )) ++
   nleof {{ process-combined }}
)) <- combined-primitive

(( {{ make-prim to prim 0 to combined
      line @ name-line ! filename 2@ name-filename 2!
      function-number @ prim prim-num !
      start }} [ifdef] vmgen c-ident [else] forth-ident [then] {{ end
      2dup prim prim-name 2! prim-c-name-2! }}  white **
   (( ` / white ** {{ start }} c-ident {{ end prim-c-name-2! }} white ** )) ??
   (( simple-primitive || combined-primitive ))
   {{ 1 function-number +! }}
)) <- primitive ( -- )

(( (( comment || primitive || nl white ** )) ** eof ))
parser primitives2something
warnings @ [IF]
.( parser generated ok ) cr
[THEN]


\ run with gforth-0.5.0 (slurp-file is missing)
[IFUNDEF] slurp-file
: slurp-file ( c-addr1 u1 -- c-addr2 u2 )
    \ c-addr1 u1 is the filename, c-addr2 u2 is the file's contents
    r/o bin open-file throw >r
    r@ file-size throw abort" file too large"
    dup allocate throw swap
    2dup r@ read-file throw over <> abort" could not read whole file"
    r> close-file throw ;
[THEN]

: primfilter ( addr u -- )
    \ process the string at addr u
    over dup rawinput ! dup line-start ! cookedinput !
    + endrawinput !
    checksynclines
    primitives2something ;    

: unixify ( c-addr u1 -- c-addr u2 )
    \ delete crs from the string
    bounds tuck tuck ?do ( c-addr1 )
	i c@ dup #cr <> if
	    over c! char+
	else
	    drop
	endif
    loop
    over - ;

: process-file ( addr u xt-simple x-combined -- )
    output-combined ! output !
    save-mem 2dup filename 2!
    slurp-file unixify
    warnings @ if
	." ------------ CUT HERE -------------" cr  endif
    primfilter ;

\  : process      ( xt -- )
\      bl word count rot
\      process-file ;
