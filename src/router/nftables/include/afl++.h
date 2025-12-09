#ifndef _NFT_AFLPLUSPLUS_H_
#define _NFT_AFLPLUSPLUS_H_

#include <nftables/libnftables.h>

/*
 * enum nft_afl_fuzzer_stage - current fuzzer stage
 *
 * @NFT_AFL_FUZZER_DISABLED: running without --fuzzer
 * @NFT_AFL_FUZZER_PARSER: only fuzz the parser, do not run eval step.
 * @NFT_AFL_FUZZER_EVALUATION: continue to evaluation step, if possible.
 * @NFT_AFL_FUZZER_NETLINK_RO: convert internal representation to netlink buffer but don't send any changes to the kernel.
 * @NFT_AFL_FUZZER_NETLINK_RW: send the netlink message to kernel for processing.
 */
enum nft_afl_fuzzer_stage {
	NFT_AFL_FUZZER_DISABLED,
	NFT_AFL_FUZZER_PARSER,
	NFT_AFL_FUZZER_EVALUATION,
	NFT_AFL_FUZZER_NETLINK_RO,
	NFT_AFL_FUZZER_NETLINK_RW,
};

#endif
