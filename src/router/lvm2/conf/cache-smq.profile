# Demo configuration 'smq' cache policy
#
# The stochastic multi-queue (smq) policy addresses some of the problems
# with the multiqueue (mq) policy and uses less memory.
#

allocation {
	cache_pool_chunk_size = 64
	cache_mode = "writethrough"
	cache_policy = "smq"
	cache_settings {
	        # currently no settings for "smq" policy
	}
}
