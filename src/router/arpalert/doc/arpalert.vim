" Vim syntax file
" Language:    arpalert
" Maintainer:  Bruno Michel <brmichel@free.fr>
" Last Change: Mar 30, 2007
" Version:     0.2
" URL:         http://www.arpalert.org/

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
	syntax clear
elseif exists("b:current_syntax")
	finish
endif

if version >= 600
	setlocal iskeyword=_,-,a-z,A-Z,48-57
else
	set iskeyword=_,-,a-z,A-Z,48-57
endif


" Values
syn keyword arpBool      true false oui non yes no
syn match   arpNumber    /\d\+/
syn region  arpString    start=+"+ end=+"+
syn match   arpEqual     +=+
syn keyword arpAlert     arpalert

" Comments
syn match   arpComment   /#.*$/ contains=arpTodo
syn keyword arpTodo      contained TODO FIXME XXX

" Keys
syn match   arpKey       /\(log\|alert on\|mod on\) \(\(mac\|ip\) change\|\(referenced\|deny\|new\|new mac\) address\|request abus\|mac error\|unauth request\|flood\)/
syn match   arpKey       /\(log\|alert\|mod\) mac vendor/
syn match   arpKey       /\(maclist\( alert\| leases\)\?\|log\|lock\|auth request\|mac vendor\) file/
syn match   arpKey       /use syslog\|log level\|catch only arp\|mod config\|execution timeout\|promiscuous\|reload interval/
syn match   arpKey       /max request\|anti flood \(interval\|global\)\|max entry\|mac timeout\|max alert\|\(action\|mod\) on detect/
syn match   arpKey       /dump \(black list\|white list\|new address\|inter\|paquet\)/
syn match   arpKey       /ignore \(unknown sender\|me\|self test\)\|unauth ignore time method/
syn keyword arpKey       user umask daemon interface
syn case    ignore


" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version < 508
	command -nargs=+ HiLink hi link <args>
else
	command -nargs=+ HiLink hi def link <args>
endif

HiLink      arpComment   Comment
HiLink      arpTodo      Todo
HiLink      arpBool      Boolean
HiLink      arpNumber    Number
HiLink      arpString    String
HiLink      arpAlert     Define
HiLink      arpEqual     Operator
HiLink      arpKey       Identifier

delcommand HiLink

let b:current_syntax = "arpalert"
" vim: ts=8
