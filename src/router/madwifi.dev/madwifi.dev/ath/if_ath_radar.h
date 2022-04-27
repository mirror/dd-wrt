/*
 * This software is distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 *
 * $Id: if_ath_radar.h 2464 2007-06-15 22:51:56Z mtaylor $
 */
/*
 * Defintions for the Atheros Wireless LAN controller driver.
 */
#ifndef _IF_ATH_RADAR_H
#define _IF_ATH_RADAR_H

/* AR5K_AR5212_PHY_ERR_FIL register definition taken from openhal */
#define AR5K_AR5212_PHY_ERR_FIL		    0x810c
#define AR5K_AR5212_PHY_ERR_FIL_RADAR	0x00000020

/* AR5K_PHY_RADAR register definition reverse engineered with 
 * ATH_REVERSE_ENGINEERING. */

/* PHY radar detection register [5111+] */
#define	AR5K_PHY_RADAR			0x9954

/* Radar enable 			........ ........ ........ .......1 */
#define	AR5K_PHY_RADAR_ENABLE		0x00000001
#define	AR5K_PHY_RADAR_ENABLE_S		0

/* This is the value found on the card  .1.111.1 .1.1.... 111....1 1...1...
at power on. */
#define	AR5K_PHY_RADAR_PWONDEF_AR5213	0x5d50e188

/* This is the value found on the card 	.1.1.111 ..11...1 .1...1.1 1...11.1
after DFS is enabled */
#define	AR5K_PHY_RADAR_ENABLED_AR5213	0x5731458d

/* Finite Impulse Response (FIR) filter .1111111 ........ ........ ........ 
 * power out threshold.
 * 7-bits signed integer, standard power range {-63..63} in 1 dBm units. */
#define AR5K_PHY_RADAR_FIRPWROUTTHR    	0x7f000000
#define AR5K_PHY_RADAR_FIRPWROUTTHR_S	24

/* Radar RSSI/SNR threshold.		........ 111111.. ........ ........ 
 * 6-bits unsigned integer, 1 dBm, range {0..63} in dBm units. */
#define AR5K_PHY_RADAR_RADARRSSITHR    	0x00fc0000
#define AR5K_PHY_RADAR_RADARRSSITHR_S	18

/* Pulse height threshold 		........ ......11 1111.... ........ 
 * 6-bits unsigned integer, 1 dBm, range {0..63} in dBm units. */
#define AR5K_PHY_RADAR_PULSEHEIGHTTHR   0x0003f000
#define AR5K_PHY_RADAR_PULSEHEIGHTTHR_S	12

/* Pulse RSSI/SNR threshold		........ ........ ....1111 11...... 
 * 6-bits unsigned integer, 1 dBm, range {0..63} in dBm units. */
#define AR5K_PHY_RADAR_PULSERSSITHR    	0x00000fc0
#define AR5K_PHY_RADAR_PULSERSSITHR_S	6

/* Inband threshold  			........ ........ ........ ..11111. 
 * 5-bits unsigned integer, 1/2 dBm, range {0..31} in 0.5 dBm units */
#define AR5K_PHY_RADAR_INBANDTHR    	0x0000003e
#define AR5K_PHY_RADAR_INBANDTHR_S	1

/* This struct defines the supported PHY error detection parameters for radar
 * pulse detection logic.  Reference US patent US6891496 B2 for pseudocode for 
 * the chips' operations and pseudocode for how the parameters are used. */
typedef struct {

	/* Finite Impulse Response (FIR) filter - power out threshold.
	 * 
	 * If a signal is received with a pulse width that is too short,
	 * the AR chip cannot fine-adjust gain fast enough to get an accurate
	 * reading of RSSI.  If this signal has an power value after filtering
	 * that exceeds a fixed threshold (rp_fir_filter_output_power_thr) 
	 * and then drops by rp_pulse_height_thr then it may be considered a 
	 * radar pulse.
	 *
	 * Reference: 405 in Figure 4A of US patent US6891496 B2.
	 * Default: Default value is -41dBm.
	 * Units: Signed integer, value in dBm {-63..63} in 1 dBm units. */
	int32_t rp_fir_filter_output_power_thr;

	/* Pulse height threshold
	 * This is delta between the max and min RSSI for short pulse radar 
	 * bursts where AGC cannot be adjusted enough times to get an accurate
	 * sizing of the pulse.  
	 * 
	 * If a signal is received with a pulse width that is too short,
	 * the AR chip cannot fine-adjust gain fast enough to get an accurate
	 * reading of RSSI.  If this signal has an power value after filtering
	 * that exceeds a fixed threshold (rp_fir_filter_output_power_thr) 
	 * and then drops by rp_pulse_height_thr then it may be considered a 
	 * radar pulse.
	 *
	 * Refernece: See Figure 4A.
	 * Default: value is 20 dBm.
	 * Units: Unsigned 6-bits, dBm range {0..63} in dBm units. */
	int32_t rp_pulse_height_thr;

	/* Radar RSSI/SNR threshold.
	 *
	 * This is the threshold that RSSI must exceed for the pulse when AGC 
	 * is ok and we actually know the power of the signal (not the chip)
	 * and reach an initial peak greater than this threshold.  
	 * 
	 * For pulses long enough for AGC to be used to detect
	 * pulse width (approximately >3us), we will start counting the pulse 
	 * width if it reaches this level as this may be the rise of a radar 
	 * pulse.
	 *
	 * Reference: See 450 on Figure 4B in US patent US6891496 B2.
	 * Default: value is 12, or 6 dBm.
	 * Units: 6-bits, dBm range {0..63} in dBm units. */
	int32_t rp_radar_rssi_thr;

	/* Pulse RSSI/SNR threshold
	 * 
	 * This is how much the RSSI of the pulse must drop, relative to
	 * rp_radar_rssi_thr before we will stop counting the pulse width.
	 *  
	 * If we see a pulse reach rp_radar_rssi_thr we start thinking it might
	 * be radar, but if it subsequently reached 
	 * (rp_radar_rssi_thr - rp_pulse_height_thr) and it wasn't part of a 
	 * WLAN signal, then we would know it was radar. 
	 *
	 * In other words, we are looking for a 6dBm drop in gain that happens
	 * after a pulse on the down swing as shown in figure 4A and 4B.
	 * 
	 * Default: 22, or 11 dBm.
	 * Units: Unsigned 6-bits, dBm range {0..63} in dBm units. */
	int32_t rp_pulse_rssi_thr;

	/* Inband threshold.
	 * Units: Unsigned 5-bits, dBm range {0..31} in half dBm units. */
	int32_t rp_inband_thr;

} RADAR_PARAM;

/* Any value in RADAR_PARAM can be set to this magic value in order to use
the default for that field */
#define RADAR_PARAM_USE_DEFAULT 0xffff

/* This is called on channel change to enable radar detection for 5211+ chips.  
 * NOTE: AR5210 doesn't have radar pulse detection support. */
int ath_radar_update(struct ath_softc *sc);
/* Returns true if radar detection is enabled. */
int ath_radar_is_enabled(struct ath_softc *sc);
/* Read the radar pulse detection parameters. */
void ath_radar_get_params(struct ath_softc *sc, RADAR_PARAM * rp);
/* Update the radar pulse detection parameters. 
 * If rp is NULL, defaults are used for all fields.
 * If any member of rp is set to RADAR_PARAM_USE_DEFAULT, the default
 * is used for that field. */
void ath_radar_set_params(struct ath_softc *sc, RADAR_PARAM * rp);
/* Update channel's DFS flags based upon whether DFS is reqired */
int ath_radar_correct_dfs_flags(struct ath_softc *sc, HAL_CHANNEL *hchan);
/* Returns true if DFS is required for the regulatory domain, country and 
 * combination in use. */
int ath_radar_is_dfs_required(struct ath_softc *sc, HAL_CHANNEL *hchan);

/* Maximum number of radar pulse recorded */
#define ATH_RADAR_PULSE_NR 100

/* init/done function for radar pulse stuff */
void ath_rp_init(struct ath_softc *sc);
void ath_rp_done(struct ath_softc *sc);

/* Record a radar pulse event in a circular array */
void ath_rp_record(struct ath_softc *sc, u_int64_t tsf, u_int8_t rssi, u_int8_t width, HAL_BOOL is_simulated);

/* Print the content of the radar pulse circular array */
void ath_rp_print(struct ath_softc *sc, int analyzed_pulses_only);
void ath_rp_print_mem(struct ath_softc *sc, int analyzed_pulses_only);

/* Empty the radar pulse circular array */
void ath_rp_flush(struct ath_softc *sc);

#endif				/* #ifndef _IF_ATH_RADAR_H */
