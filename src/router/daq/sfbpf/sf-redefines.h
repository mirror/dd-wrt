#ifndef _SF_REDEFINES
#define _SF_REDEFINES

#define gen_loadi sf_gen_loadi
#define gen_load sf_gen_load
#define gen_loadlen sf_gen_loadlen
#define gen_neg sf_gen_neg
#define gen_arth sf_gen_arth

#define gen_and sf_gen_and
#define gen_or sf_gen_or
#define gen_not sf_gen_not

#define gen_scode sf_gen_scode
#define gen_ecode sf_gen_ecode
#define gen_acode sf_gen_acode
#define gen_mcode sf_gen_mcode
#ifdef INET6
#define gen_mcode6 sf_gen_mcode6
#endif
#define gen_ncode sf_gen_ncode
#define gen_proto_abbrev sf_gen_proto_abbrev
#define gen_relation sf_gen_relation
#define gen_less sf_gen_less
#define gen_greater sf_gen_greater
#define gen_byteop sf_gen_byteop
#define gen_broadcast sf_gen_broadcast
#define gen_multicast sf_gen_multicast
#define gen_inbound sf_gen_inbound

#define gen_vlan sf_gen_vlan
#define gen_mpls sf_gen_mpls

#define gen_pppoed sf_gen_pppoed
#define gen_pppoes sf_gen_pppoes

#define gen_atmfield_code sf_gen_atmfield_code
#define gen_atmtype_abbrev sf_gen_atmtype_abbrev
#define gen_atmmulti_abbrev sf_gen_atmmulti_abbrev

#define gen_mtp2type_abbrev sf_gen_mtp2type_abbrev
#define gen_mtp3field_code sf_gen_mtp3field_code

#define gen_pf_ifname sf_gen_pf_ifname
#define gen_pf_rnr sf_gen_pf_rnr
#define gen_pf_srnr sf_gen_pf_srnr
#define gen_pf_ruleset sf_gen_pf_ruleset
#define gen_pf_reason sf_gen_pf_reason
#define gen_pf_action sf_gen_pf_action
#define gen_pf_dir sf_gen_pf_dir

#define gen_p80211_type sf_gen_p80211_type
#define gen_p80211_fcdir sf_gen_p80211_fcdir

#define gen_portop sf_gen_portop
#define gen_portop6 sf_gen_portop6
#define gen_portrangeop sf_gen_portrangeop
#define gen_portrangeop6 sf_gen_portrangeop6
#define n_errors sf_n_errors

#define bpf_optimize sf_bpf_optimize
#define bpf_error sf_bpf_error

#define finish_parse sf_finish_parse
#define sdup sf_sdup

#define icode_to_fcode sf_icode_to_fcode
#define lex_init sf_lex_init
#define lex_cleanup sf_lex_cleanup
#define sappend sf_append

#define pcap_parse sfbpf_parse

#define pcap_lval sfbpf_lval

#define bpf_int32 sfbpf_int32
#define bpf_u_int32 sfbpf_u_int32
#define bpf_insn sfbpf_insn
#define bpf_program sfbpf_program

#define bpf_filter sfbpf_filter
#define bpf_validate sfbpf_validate

#define BPF_ALIGNMENT SFBPF_ALIGNMENT
#define BPF_WORDALIGN SFBPF_WORDALIGN

#define BPF_MAXBUFSIZE SFBPF_MAXBUFSIZE
#define BPF_MINBUFSIZE SFBPF_MINBUFSIZE

#define BPF_CLASS SFBPF_CLASS
#define BPF_LD SFBPF_LD
#define BPF_LDX SFBPF_LDX
#define BPF_ST SFBPF_ST
#define BPF_STX SFBPF_STX
#define BPF_ALU SFBPF_ALU
#define BPF_JMP SFBPF_JMP
#define BPF_RET SFBPF_RET
#define BPF_MISC SFBPF_MISC

#define BPF_SIZE SFBPF_SIZE
#define BPF_W SFBPF_W
#define BPF_H SFBPF_H
#define BPF_B SFBPF_B

#define BPF_MODE SFBPF_MODE
#define BPF_IMM SFBPF_IMM
#define BPF_ABS SFBPF_ABS
#define BPF_IND SFBPF_IND
#define BPF_MEM SFBPF_MEM
#define BPF_LEN SFBPF_LEN
#define BPF_MSH SFBPF_MSH

#define BPF_OP SFBPF_OP
#define BPF_ADD SFBPF_ADD
#define BPF_SUB SFBPF_SUB
#define BPF_MUL SFBPF_MUL
#define BPF_DIV SFBPF_DIV
#define BPF_OR SFBPF_OR
#define BPF_AND SFBPF_AND
#define BPF_LSH SFBPF_LSH
#define BPF_RSH SFBPF_RSH
#define BPF_NEG SFBPF_NEG
#define BPF_JA SFBPF_JA
#define BPF_JEQ SFBPF_JEQ
#define BPF_JGT SFBPF_JGT
#define BPF_JGE SFBPF_JGE
#define BPF_JSET SFBPF_JSET

#define BPF_SRC SFBPF_SRC
#define BPF_K SFBPF_K
#define BPF_X SFBPF_X

#define BPF_RVAL SFBPF_RVAL
#define BPF_A SFBPF_A

#define BPF_MISCOP SFBPF_MISCOP
#define BPF_TAX SFBPF_TAX
#define BPF_TXA SFBPF_TXA

#define BPF_STMT SFBPF_STMT
#define BPF_JUMP SFBPF_JUMP

#define BPF_MEMWORDS SFBPF_MEMWORDS

#define PCAP_NETMASK_UNKNOWN SFBPF_NETMASK_UNKNOWN

#define pcap_compile sfbpf_compile
#define pcap_compile_unsafe sfbpf_compile_unsafe
#define pcap_freecode sfbpf_freecode

#endif
