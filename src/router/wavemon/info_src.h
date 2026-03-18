
#ifndef __INFO_SRC__
#define __INFO_SRC__
/*
 * 80211ah conversions definitions.
 */

#include "dot11ah_channel.h"

/**
 * @brief Returns 80211ah values from 5g frequency
 *
 * @param freq 5G frequncy
 * @param country_channel_map map to the channel information for the respective country
 * @param halow_vals 80211ah channel, freq and bw values
 */
void convert_to_80211ah_values_from_freq(int freq, const country_channel_map_t *country_channel_map, channel_to_halow_freq_t *halow_vals);

/**
 * @brief Returns 80211ah values from 5g channel
 *
 * @param freq 5G frequncy
 * @param country_channel_map map to the channel information for the respective country
 * @param halow_vals 80211ah channel, freq and bw values
 */
void convert_to_80211ah_values_from_channel(int channel, const country_channel_map_t *country_channel_map, channel_to_halow_freq_t *halow_vals);
#endif