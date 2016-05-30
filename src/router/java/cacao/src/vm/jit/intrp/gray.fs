\ recursive descent parser generator )

\ Copyright (C) 1995,1996,1997,2000,2003 Free Software Foundation, Inc.
\ Copyright 1990, 1991, 1994 Martin Anton Ertl

\     This program is free software; you can redistribute it and/or modify
\     it under the terms of the GNU General Public License as published by
\     the Free Software Foundation; either version 2 of the License, or
\     (at your option) any later version.

\     This program is distributed in the hope that it will be useful,
\     but WITHOUT ANY WARRANTY; without even the implied warranty of
\     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
\     GNU General Public License for more details.

\     You should have received a copy of the GNU General Public License
\     along with this program; if not, write to the Free Software
\     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.

\ ANS FORTH prolog

: defined? ( "word" -- flag )  bl word find nip ;
defined? WARNINGS 0=
[IF]
Variable warnings
warnings on
[THEN]

\ end of ANS FORTH prolog

warnings @ [IF]
.( Loading Gray ... Copyright 1990-1994 Martin Anton Ertl; NO WARRANTY ) cr
[THEN]

\ misc )
: noop ;

1 cells constant cell
s" address-unit-bits" environment? 0=
[IF]
  warnings @ [IF]
     cr .( environmental attribute address-units-bits unknown, computing... ) cr
  [THEN]
  \ if your machine has more bits/au, this assumption wastes space
  \ if your machine has fewer bits/au, gray will not work
  : (bits/cell)  ( -- n )  1 0 invert dup 1 rshift xor
    BEGIN  dup 1 = 0=  WHILE  1 rshift  swap 1+ swap  REPEAT  drop ;
  (bits/cell)
  warnings @ [IF]
    .( You seem to have ) dup 1 cells / . .( bits/address unit) cr
  [THEN]
[ELSE]
  cells
[THEN]
constant bits/cell \ !! implementation dependent )

: ?not? ( f -- f )
 postpone 0= ; immediate

: 2, ( w1 w2 -- )
 here 2 cells allot 2! ;

: endif postpone then ; immediate

: ?pairs ( n1 n2 -- )
 ( aborts, if the numbers are not equal )
 = ?not? abort" mismatched parenthesis" ;
 
: ', \ -- ) ( use: ', name )
 ' , ;

1 0= constant false
0 0= constant true

\ stack administration )
\ this implementation is completely unsafe )

: stack \ n -- )
\ use: n stack word )
\ creates a stack called word with n cells )
\ the first cell is the stackpointer )
 create here , cells allot ;

: push \ n stack -- )
 cell over +! @ ! ;

: top \ stack -- n )
 @ @ ;

: pop \ stack -- )
 [ -1 cells ] literal swap +! ;

: clear? \ stack -- f )
 dup @ = ;

: clear \ stack -- )
 dup ! ;


\ sets - represented as bit arrays )
\ bits that represent no elements, must be 0 )
\ all operations assume valid parameters )
\ elements must be unsigned numbers )
\ the max. element size must be declared with max-member )
\ no checking is performed )
\ set operations allot memory )

: decode \ u -- w )
\ returns a cell with bit# u set and everyting else clear )
 1 swap lshift ;

variable cells/set 0 cells/set !
variable empty-ptr 0 empty-ptr ! \ updatd by max-member )
: empty \ -- set )
 empty-ptr @ ;

: max-member \ u -- )
\ declares u to be the maximum member of sets generated afterwards )
\ must be called before using any set word except member?, add-member )
 bits/cell / 1+
 dup cells/set !
 here empty-ptr ! \ make empty set )
 0 do 0 , loop ;

: copy-set \ set1 -- set2 )
\ makes a copy of set1 )
 here swap
 cells/set @ 0 do
  dup @ ,
  cell+ loop
 drop ;

: normalize-bit-addr \ addr1 u1 -- addr2 u2 )
\ addr1*bits/cell+u1=addr2*bits/cell+u2, u2<bits/cell )
 bits/cell /mod
 cells rot +
 swap ;
\ the /mod could be optimized into a RSHIFT and an AND, if bits/cell is
\ a power of 2, but in an interpreted implementation this would only be
\ faster if the machine has very slow division and in a native code
\ implementation the compiler should be intelligent enough to optimize
\ without help.

: add-member \ u set -- )
\ changes set to include u )
 swap normalize-bit-addr
 decode
 over @ or swap ! ;

: singleton \ u -- set )
\ makes a set that contains u and nothing else )
 empty copy-set swap over add-member ;

: member? \ set u -- f )
\ returns true if u is in set )
 normalize-bit-addr
 decode
 swap @ and
 0= ?not? ;

: binary-set-operation \ set1 set2 [w1 w2 -- w3] -- set )
\ creates set from set1 and set2 by applying [w1 w2 -- w3] on members )
\ e.g. ' or binary-set-operation  is the union operation )
 here >r
 cells/set @ 0 do >r
  over @ over @ r@ execute ,
  cell+ swap cell+ swap
 r> loop
 drop 2drop r> ;

: union1 \ set1 set2 -- set )
 ['] or binary-set-operation ;

: intersection \ set1 set2 -- set )
 ['] and binary-set-operation ;

: binary-set-test? \ set1 set2 [w1 w2 -- w3] -- f )
\ returns true, if [w1 w2 -- w3] binary-set-operation returns empty )
\ e.g. set1 set2 ' and binary-set-test?  is true, if set1 and set2
\ are disjoint, i.e. they contain no common members )
 >r true rot rot r>
 cells/set @ 0 do >r
  over @ over @ r@ execute 0= ?not? if
   rot drop false rot rot
  endif
  cell+ swap cell+ swap
 r> loop
 drop 2drop ;

: notb&and \ w1 w2 -- w3 )
 -1 xor and ;

: subset? \ set1 set2 -- f )
\ returns true if every member of set1 is in set2 )
 ['] notb&and binary-set-test? ;

: disjoint? \ set1 set2 -- f )
\ returns true if set1 and set2 heve no common members )
 ['] and binary-set-test? ;

: apply-to-members \ set [ u -- ] -- )
\ executes [ u -- ] for every member of set )
 cells/set @ bits/cell * 0 do
  over i member? if
   i over execute
  endif
 loop
 2drop ;

: union \ set1 set2 -- set )
\ just a little more space-efficient ) 
 2dup subset? if
  swap drop
 else 2dup swap subset? if
  drop
 else
  union1
 endif endif ;


\ tests )
variable test-vector ' abort test-vector !
\ here you should store the execution address of a word ( set -- f )
\ that returns true if the token of the current symbol is in set )

: compile-test \ set -- )
 postpone literal
 test-vector @ compile, ;


\ context management )
500 stack context-stack
\ this stack holds the syntax-exprs currently being treated )
\ enlarge it, if your grammar is large and complex )
context-stack clear

: this \ -- syntax-expr )
\ get current syntax-expr )
 context-stack top ;

: new-context \ syntax-expr -- )
 context-stack push ;

: old-context \ -- )
 context-stack pop ;


\ structures )
: <builds-field \ n1 n2 -- n3 ) ( defining-word )
\ n1 is the offset of the field, n2 its length, n3 the offset of the
\ next field; creates a word that contains the offset )
 create over , + ;

0 constant struct
\ initial offset

: context-var \ use: < offset > size context-var name < offset2 > )
\ name returns the address of the offset field of "this" )
 <builds-field \ n1 n2 -- n3 )
 does> \ -- addr )
  @ this + ;

: context-const \ use: < offset > context-const name < offset2 > )
\ name returns the contents of the field of this at offset )
 cell <builds-field \ n1 -- n2 )
 does> \ -- n )
  @ this + @ ;


\ syntax-exprs )
struct
 aligned context-const methods
        \ table of words applicable to the syntax-expr (a map)
 1 context-var mark-propagate \ used to ensure that "propagate" is
        \ called at least once for each syntax-expr )
 1 context-var mark-pass2
        \ make sure pass2 is called exactly once )
 aligned cell context-var first-set
        \ all tokens a nonempty path may begin with )
        \ if it's equal to 0, the first-set has not been computed yet )
 1 context-var maybe-empty
        \ true if the syntax-expr can derive eps )
 aligned cell context-var follow-set
	\ the tokens of the terminals that can follow the syntax-expr )
s" gforth" environment?
[IF]  2drop \ clear gforth's version numbers )
 aligned 2 cells context-var source-location \ for error msgs )
[ELSE]
s" bigFORTH" environment?
[IF]  2drop \ clear bigFORTH' version numbers )
 aligned cell context-var source-location
        \ for error msgs
[ELSE]
 \ !! replace the stuff until constant with something working on your system
 aligned 3 cells context-var source-location
        \ for error msgs
 80 chars context-var error-info
        \ string
[THEN] [THEN]
aligned constant syntax-expr   \ length of a syntax-expr )

: make-syntax-expr \ map -- syntax-expr )
\ allocate a syntax-expr and initialize it )
 here swap , false c, false c,
 align 0 , false c, align empty ,
\ source location. !! replace the stuff until `;' with your stuff
\ if you use blocks, use:
\  blk @ >in @ 2,
\ the following is just a dummy
[ s" gforth" environment? ]
[IF]  [ 2drop ]
 0 sourceline# 2,
[ELSE]
[ s" bigFORTH" environment? ]
[IF]  [ 2drop ]
 makeview w, >in @ w,
[ELSE]
 source 80 min >r  here 3 cells + r@ cmove
 here 3 cells + ,  r@ ,  >in @ 80 min ,  r> chars allot align
[THEN] [THEN]
 ;


\ warnings and errors )
: .in \ -- )
\ !! implementation dependent )
\ prints the info stored in source-location in a usable way )
\ prints where the error happened )
[ s" gforth" environment? ]
[IF]  [ 2drop ]
 source-location 2@ ." line" . drop ." :" ;
[ELSE]
[ s" bigFORTH" environment? ]
[IF]  [ 2drop ]
 source-location dup w@ $3FF and scr ! 2+ w@ r# ! ;
[ELSE]
 source-location 2@ swap cr type cr
 error-info @ 2 - spaces ." ^" cr  ." ::: " ;
[THEN] [THEN]
 
: gray-error abort ;

: internal-error
 cr .in ." you found a bug" gray-error ;

variable print-token ' . print-token !
\ contains execution address of a word < token -- > to print a token )

: check-conflict \ set1 set2 -- )
\ print the intersection of set1 and set2 if it isn't empty )
 2dup disjoint? ?not? warnings @ and if
  cr .in ." conflict:"
  intersection print-token @ apply-to-members
 else
  2drop
 endif ;


\ methods and maps )
: method \ use: < offset > method name < offset2 > )
\ executes the word whose execution address is stored in the field
\ at offset of a table pointed to by the "methods" field of "this" ) 
 cell <builds-field \ n1 -- n2 )
 does>
  @ methods + @ execute ;

\ method table for syntax-exprs
struct
 method compute-method
 method propagate-method
 method generate-method
 method pass2-method
constant syntax-expr-methods


\ general routines )
: compute \ syntax-expr -- first-set maybe-empty )
\ compute the first-set and maybe-empty of a syntax-expr )
\ a bit of memoization is used here )
 new-context
 first-set @ 0= if
  compute-method
  maybe-empty c!
  first-set !
 endif
 first-set @ maybe-empty c@
 old-context ;

: get-first \ syntax-expr -- first-set )
 compute drop ;

: check-cycle \ syntax-expr -- )
\ just check for left recursion )
 compute 2drop ;

: propagate \ follow-set syntax-expr -- )
\ add follow-set to the follow set of syntax-expr and its children ) 
 new-context
 dup follow-set @ subset? ?not?  \ would everything stay the same
 mark-propagate c@ ?not? or if   \ and was propagate here already
  true mark-propagate c!       \ NO, do propagate
  follow-set @ union dup follow-set !
  propagate-method
 else
  drop
 endif
 old-context ;

: generate \ syntax-expr -- )
\ this one gets things done )
 new-context generate-method old-context ;

: pass2 \ syntax-expr -- )
\ computes all necessary first sets, checks for left recursions
\ and conflicts and generates code for rules )
 new-context
 mark-pass2 c@ ?not? if
  true mark-pass2 c!
  this check-cycle
  pass2-method
 endif
 old-context ;


\ main routine )
: parser \ syntax-expr -- )
\ use: syntax-expr parser xxx )
 context-stack clear
 empty over propagate
 dup pass2
 \ : should not be immediate
 >r : r> generate postpone ; ;


\ eps - empty syntax-expr )
create eps-map
', internal-error
', drop
', noop
', noop


create eps1
\ the eps syntax-expr proper
 eps-map make-syntax-expr
drop


: eps \ -- syntax-expr )
\ just adjusts eps1 and returns it
 eps1 new-context
 empty first-set ! ( empty changes due to max-member )
 empty follow-set !
 true maybe-empty c!
 old-context
 eps1 ;


\ terminals )
\ a terminal is a syntax-expr with an extra field )
syntax-expr
 context-const check&next
        \ contains address of a word < f -- > that checks
        \ if f is true and reads the next terminal symbol )
constant terminal-syntax-expr

: generate-terminal \ -- )
 this get-first compile-test
 check&next compile, ;

create terminal-map
', internal-error
', drop
', generate-terminal
', noop

: make-terminal \ first-set cfa -- syntax-expr )
 terminal-map make-syntax-expr
 new-context
 ,
 first-set !
 this old-context ;

: terminal \ first-set cfa -- )
 create make-terminal drop ;


\ binary syntax-exprs )
syntax-expr
 context-const operand1
 context-const operand2
constant binary-syntax-expr

: make-binary \ syntax-expr1 syntax-expr2 map -- syntax-expr )
 make-syntax-expr rot , swap , ;

: pass2-binary
 operand1 pass2
 operand2 pass2 ;


\ concatenations )
: compute-concatenation \ -- first maybe-empty )
 operand1 compute dup if
  drop
  operand2 compute
  >r union r>
 endif ;

: propagate-concatenation \ follow-set -- )
 operand2 compute if
  over union
 endif \ follow follow1 )
 operand1 propagate
 operand2 propagate ;

: generate-concatenation \ -- )
 operand1 generate
 operand2 generate ;

create concatenation-map
', compute-concatenation
', propagate-concatenation
', generate-concatenation
', pass2-binary

: concat \ syntax-expr1 syntax-expr2 -- syntax-expr )
 concatenation-map make-binary ;
\ this is the actual concatenation operator )
\ but for safety and readability the parenthesised notation )
\ is preferred )


\ alternatives )
: compute-alternative \ -- first maybe-empty )
 operand1 compute
 operand2 compute
 rot 2dup and warnings @ and if
  cr .in ." warning: two branches may be empty" endif
 or >r union r> ;

: propagate-alternative \ follow -- )
 dup operand1 propagate
 operand2 propagate ;

: generate-alternative1 \ -- )
 operand1 get-first compile-test
 postpone if
 operand1 generate
 postpone else
 operand2 generate
 postpone endif ;

: generate-alternative2 \ -- )
 operand1 get-first compile-test postpone ?not?
 operand2 get-first compile-test postpone and
 postpone if
 operand2 generate
 postpone else
 operand1 generate
 postpone endif ;

: generate-alternative \ -- )
 operand1 compute if
  generate-alternative2
 else
  generate-alternative1
 endif
 drop ;

: pass2-alternative \ -- )
 this compute if
  follow-set @ check-conflict
 else
  drop
 endif
 operand1 get-first operand2 get-first check-conflict
 pass2-binary ;

create alternative-map
', compute-alternative
', propagate-alternative
', generate-alternative
', pass2-alternative

: alt \ syntax-expr1 syntax-expr2 -- syntax-expr )
 alternative-map make-binary ;
\ this is the actual alternative operator )
\ but for safety and readability the parenthesised notation )
\ is preferred )


\ unary syntax-exprs )
syntax-expr
 context-const operand
constant unary-syntax-expr

: make-unary \ syntax-expr1 map -- syntax-expr2 )
 make-syntax-expr swap , ;


\ options and repetitions )
: pass2-option&repetition \ -- )
 follow-set @ operand get-first check-conflict
 operand pass2 ;


\ options )
: compute-option \ -- set f )
 operand compute warnings @ and if
  cr .in ." warning: unnessesary option" endif
 true ;

: propagate-option \ follow -- )
 operand propagate ;

: generate-option \ -- )
 operand get-first compile-test
 postpone if
 operand generate
 postpone endif ;

create option-map
', compute-option
', propagate-option
', generate-option
', pass2-option&repetition

: ?? \ syntax-expr1 -- syntax-expr2 )
 option-map make-unary ;


\ repetitions )
: propagate-repetition \ follow-set -- )
 operand get-first union operand propagate ;


\ *-repetitions )
: compute-*repetition \ -- set f )
 operand compute warnings @ and if
  cr .in ." warning: *repetition of optional term" endif
 true ;

: generate-*repetition \ -- )
 postpone begin
 operand get-first compile-test
 postpone while
 operand generate
 postpone repeat ;

create *repetition-map
', compute-*repetition
', propagate-repetition
', generate-*repetition
', pass2-option&repetition

: ** \ syntax-expr1 -- syntax-expr2 )
 *repetition-map make-unary ;


\ +-repetitions )
: compute-+repetition \ -- set f )
 operand compute ;

: generate-+repetition \ -- )
 postpone begin
 operand generate
 operand get-first compile-test
 postpone ?not? postpone until ;

create +repetition-map
', compute-+repetition
', propagate-repetition
', generate-+repetition
', pass2-option&repetition

: ++ \ syntax-expr1 -- syntax-expr2 )
 +repetition-map make-unary ;


\ actions )
syntax-expr
 context-const action
constant action-syntax-expr

: generate-action \ syntax-expr -- )
 action compile, ;

create action-map
', internal-error
', drop
', generate-action
', noop

: {{ \ -- syntax-expr addr colon-sys )
 action-map make-syntax-expr
 new-context
 empty first-set !
 true maybe-empty c!
 this old-context
 \ ?exec !csp )
 here cell allot
 :noname ;

: }} \ syntax-expr addr colon-sys -- syntax-expr )
 \ ?csp )
 postpone ;
 swap !
; immediate


\ nonterminals )
syntax-expr
 1 context-var mark-compute
 aligned cell context-var rule-body \ in forth left side of rule )
 cell context-var exec            \ cfa of code for rule )
constant nt-syntax-expr

: get-body \ -- syntax-expr )
\ get the body of the rule for the nt in "this" )
  rule-body @ if
   rule-body @
  else
   cr .in ." no rule for nonterminal" gray-error
  endif ;

: compute-nt \ -- set f )
 mark-compute c@ if
  cr .in ." left recursion" gray-error
 else
  true mark-compute c!
  get-body compute
 endif ;

: propagate-nt \ follow-set -- )
  get-body propagate ;

: code-nt \ -- )
\ generates the code for a rule )
 :noname 
 get-body generate
 postpone ;
 exec ! ;

: generate-nt \ -- )
\ generates a call to the code for the rule )
\ since the code needs not be generated yet, an indirect call is used )
 exec postpone literal
 postpone @
 postpone execute ;

: pass2-nt \ -- )
\ apart from the usual duties, this pass2 also has to code-nt )
 get-body pass2
 code-nt ;

create nt-map
', compute-nt
', propagate-nt
', generate-nt
', pass2-nt

: make-nt \ syntax-expr -- nt )
 nt-map make-syntax-expr
 false c, align swap , 0 , ;

: <- \ use: syntax-expr <- xxx )
     \ xxx: -- syntax-expr )
 create make-nt drop ;

: nonterminal \ use: nonterminal xxx )
 0 <- ;       \ forward declaration )

: rule \ syntax-expr nt -- )
\ makes a rule )
 new-context
 rule-body @ if
  .in ." multiple rules for nonterminal" gray-error endif
 rule-body !
 old-context ;


\ syntactic sugar )
: reduce \ 0 x1 ... [x2 x3 -- x4] -- x )
\ e.g. 0 5 6 7 ' + reduce  =  5 6 7 + +  =  18 )
 >r dup 0= if
  ." no operand" abort
 endif
 begin
  over 0= ?not? while
  r@ execute
 repeat \ 0 x )
 swap drop r> drop ;

7 constant concatenation-id
: (- \ -- n 0 )
 concatenation-id 0 ;
: -) \ n 0 syntax-expr1 syntax-expr2 .. -- syntax-expr )
 ['] concat reduce
 swap concatenation-id ?pairs ;

8 constant alternative-id
: (| \ -- n 0 )
 alternative-id 0 ;
: |) \ n 0 syntax-expr1 syntax-expr2 .. -- syntax-expr )
 ['] alt reduce
 swap alternative-id ?pairs ;

: (( (| (- ;
: )) -) |) ;
: || -) (- ;
