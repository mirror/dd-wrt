# Demo configuration 'mq' cache policy
#
# Note: This policy has been deprecated in favor of the smq policy
# keyword "default" means, setting is left with kernel defaults.
#

allocation {
	cache_pool_chunk_size = 64
	cache_mode = "writethrough"
	cache_policy = "mq"
	cache_settings {
		mq {
			sequential_threshold = "default"	#  #nr_sequential_ios
			random_threshold = "default"		#  #nr_random_ios
			read_promote_adjustment = "default"
			write_promote_adjustment = "default"
			discard_promote_adjustment = "default"
		}
	}
}
