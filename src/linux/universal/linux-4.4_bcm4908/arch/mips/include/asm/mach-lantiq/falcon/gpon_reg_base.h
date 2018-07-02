/******************************************************************************

                               Copyright (c) 2010
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _gpon_reg_base_h
#define _gpon_reg_base_h

/** \addtogroup GPON_BASE
   @{
*/

#ifndef KSEG1
#define KSEG1 0xA0000000
#endif

/** address range for ebu
    0x18000000--0x180000FF */
#define GPON_EBU_BASE		(KSEG1 | 0x18000000)
#define GPON_EBU_END		(KSEG1 | 0x180000FF)
#define GPON_EBU_SIZE		0x00000100
/** address range for gpearb
    0x1D400100--0x1D4001FF */
#define GPON_GPEARB_BASE		(KSEG1 | 0x1D400100)
#define GPON_GPEARB_END		(KSEG1 | 0x1D4001FF)
#define GPON_GPEARB_SIZE		0x00000100
/** address range for tmu
    0x1D404000--0x1D404FFF */
#define GPON_TMU_BASE		(KSEG1 | 0x1D404000)
#define GPON_TMU_END		(KSEG1 | 0x1D404FFF)
#define GPON_TMU_SIZE		0x00001000
/** address range for iqm
    0x1D410000--0x1D41FFFF */
#define GPON_IQM_BASE		(KSEG1 | 0x1D410000)
#define GPON_IQM_END		(KSEG1 | 0x1D41FFFF)
#define GPON_IQM_SIZE		0x00010000
/** address range for octrlg
    0x1D420000--0x1D42FFFF */
#define GPON_OCTRLG_BASE		(KSEG1 | 0x1D420000)
#define GPON_OCTRLG_END		(KSEG1 | 0x1D42FFFF)
#define GPON_OCTRLG_SIZE		0x00010000
/** address range for octrll0
    0x1D440000--0x1D4400FF */
#define GPON_OCTRLL0_BASE		(KSEG1 | 0x1D440000)
#define GPON_OCTRLL0_END		(KSEG1 | 0x1D4400FF)
#define GPON_OCTRLL0_SIZE		0x00000100
/** address range for octrll1
    0x1D440100--0x1D4401FF */
#define GPON_OCTRLL1_BASE		(KSEG1 | 0x1D440100)
#define GPON_OCTRLL1_END		(KSEG1 | 0x1D4401FF)
#define GPON_OCTRLL1_SIZE		0x00000100
/** address range for octrll2
    0x1D440200--0x1D4402FF */
#define GPON_OCTRLL2_BASE		(KSEG1 | 0x1D440200)
#define GPON_OCTRLL2_END		(KSEG1 | 0x1D4402FF)
#define GPON_OCTRLL2_SIZE		0x00000100
/** address range for octrll3
    0x1D440300--0x1D4403FF */
#define GPON_OCTRLL3_BASE		(KSEG1 | 0x1D440300)
#define GPON_OCTRLL3_END		(KSEG1 | 0x1D4403FF)
#define GPON_OCTRLL3_SIZE		0x00000100
/** address range for octrlc
    0x1D441000--0x1D4410FF */
#define GPON_OCTRLC_BASE		(KSEG1 | 0x1D441000)
#define GPON_OCTRLC_END		(KSEG1 | 0x1D4410FF)
#define GPON_OCTRLC_SIZE		0x00000100
/** address range for ictrlg
    0x1D450000--0x1D45FFFF */
#define GPON_ICTRLG_BASE		(KSEG1 | 0x1D450000)
#define GPON_ICTRLG_END		(KSEG1 | 0x1D45FFFF)
#define GPON_ICTRLG_SIZE		0x00010000
/** address range for ictrll0
    0x1D460000--0x1D4601FF */
#define GPON_ICTRLL0_BASE		(KSEG1 | 0x1D460000)
#define GPON_ICTRLL0_END		(KSEG1 | 0x1D4601FF)
#define GPON_ICTRLL0_SIZE		0x00000200
/** address range for ictrll1
    0x1D460200--0x1D4603FF */
#define GPON_ICTRLL1_BASE		(KSEG1 | 0x1D460200)
#define GPON_ICTRLL1_END		(KSEG1 | 0x1D4603FF)
#define GPON_ICTRLL1_SIZE		0x00000200
/** address range for ictrll2
    0x1D460400--0x1D4605FF */
#define GPON_ICTRLL2_BASE		(KSEG1 | 0x1D460400)
#define GPON_ICTRLL2_END		(KSEG1 | 0x1D4605FF)
#define GPON_ICTRLL2_SIZE		0x00000200
/** address range for ictrll3
    0x1D460600--0x1D4607FF */
#define GPON_ICTRLL3_BASE		(KSEG1 | 0x1D460600)
#define GPON_ICTRLL3_END		(KSEG1 | 0x1D4607FF)
#define GPON_ICTRLL3_SIZE		0x00000200
/** address range for ictrlc0
    0x1D461000--0x1D4610FF */
#define GPON_ICTRLC0_BASE		(KSEG1 | 0x1D461000)
#define GPON_ICTRLC0_END		(KSEG1 | 0x1D4610FF)
#define GPON_ICTRLC0_SIZE		0x00000100
/** address range for ictrlc1
    0x1D461100--0x1D4611FF */
#define GPON_ICTRLC1_BASE		(KSEG1 | 0x1D461100)
#define GPON_ICTRLC1_END		(KSEG1 | 0x1D4611FF)
#define GPON_ICTRLC1_SIZE		0x00000100
/** address range for fsqm
    0x1D500000--0x1D5FFFFF */
#define GPON_FSQM_BASE		(KSEG1 | 0x1D500000)
#define GPON_FSQM_END		(KSEG1 | 0x1D5FFFFF)
#define GPON_FSQM_SIZE		0x00100000
/** address range for pctrl
    0x1D600000--0x1D6001FF */
#define GPON_PCTRL_BASE		(KSEG1 | 0x1D600000)
#define GPON_PCTRL_END		(KSEG1 | 0x1D6001FF)
#define GPON_PCTRL_SIZE		0x00000200
/** address range for link0
    0x1D600200--0x1D6002FF */
#define GPON_LINK0_BASE		(KSEG1 | 0x1D600200)
#define GPON_LINK0_END		(KSEG1 | 0x1D6002FF)
#define GPON_LINK0_SIZE		0x00000100
/** address range for link1
    0x1D600300--0x1D6003FF */
#define GPON_LINK1_BASE		(KSEG1 | 0x1D600300)
#define GPON_LINK1_END		(KSEG1 | 0x1D6003FF)
#define GPON_LINK1_SIZE		0x00000100
/** address range for link2
    0x1D600400--0x1D6004FF */
#define GPON_LINK2_BASE		(KSEG1 | 0x1D600400)
#define GPON_LINK2_END		(KSEG1 | 0x1D6004FF)
#define GPON_LINK2_SIZE		0x00000100
/** address range for disp
    0x1D600500--0x1D6005FF */
#define GPON_DISP_BASE		(KSEG1 | 0x1D600500)
#define GPON_DISP_END		(KSEG1 | 0x1D6005FF)
#define GPON_DISP_SIZE		0x00000100
/** address range for merge
    0x1D600600--0x1D6006FF */
#define GPON_MERGE_BASE		(KSEG1 | 0x1D600600)
#define GPON_MERGE_END		(KSEG1 | 0x1D6006FF)
#define GPON_MERGE_SIZE		0x00000100
/** address range for tbm
    0x1D600700--0x1D6007FF */
#define GPON_TBM_BASE		(KSEG1 | 0x1D600700)
#define GPON_TBM_END		(KSEG1 | 0x1D6007FF)
#define GPON_TBM_SIZE		0x00000100
/** address range for pe0
    0x1D610000--0x1D61FFFF */
#define GPON_PE0_BASE		(KSEG1 | 0x1D610000)
#define GPON_PE0_END		(KSEG1 | 0x1D61FFFF)
#define GPON_PE0_SIZE		0x00010000
/** address range for pe1
    0x1D620000--0x1D62FFFF */
#define GPON_PE1_BASE		(KSEG1 | 0x1D620000)
#define GPON_PE1_END		(KSEG1 | 0x1D62FFFF)
#define GPON_PE1_SIZE		0x00010000
/** address range for pe2
    0x1D630000--0x1D63FFFF */
#define GPON_PE2_BASE		(KSEG1 | 0x1D630000)
#define GPON_PE2_END		(KSEG1 | 0x1D63FFFF)
#define GPON_PE2_SIZE		0x00010000
/** address range for pe3
    0x1D640000--0x1D64FFFF */
#define GPON_PE3_BASE		(KSEG1 | 0x1D640000)
#define GPON_PE3_END		(KSEG1 | 0x1D64FFFF)
#define GPON_PE3_SIZE		0x00010000
/** address range for pe4
    0x1D650000--0x1D65FFFF */
#define GPON_PE4_BASE		(KSEG1 | 0x1D650000)
#define GPON_PE4_END		(KSEG1 | 0x1D65FFFF)
#define GPON_PE4_SIZE		0x00010000
/** address range for pe5
    0x1D660000--0x1D66FFFF */
#define GPON_PE5_BASE		(KSEG1 | 0x1D660000)
#define GPON_PE5_END		(KSEG1 | 0x1D66FFFF)
#define GPON_PE5_SIZE		0x00010000
/** address range for sys_gpe
    0x1D700000--0x1D7000FF */
#define GPON_SYS_GPE_BASE		(KSEG1 | 0x1D700000)
#define GPON_SYS_GPE_END		(KSEG1 | 0x1D7000FF)
#define GPON_SYS_GPE_SIZE		0x00000100
/** address range for eim
    0x1D800000--0x1D800FFF */
#define GPON_EIM_BASE		(KSEG1 | 0x1D800000)
#define GPON_EIM_END		(KSEG1 | 0x1D800FFF)
#define GPON_EIM_SIZE		0x00001000
/** address range for sxgmii
    0x1D808800--0x1D8088FF */
#define GPON_SXGMII_BASE		(KSEG1 | 0x1D808800)
#define GPON_SXGMII_END		(KSEG1 | 0x1D8088FF)
#define GPON_SXGMII_SIZE		0x00000100
/** address range for sgmii
    0x1D808C00--0x1D808CFF */
#define GPON_SGMII_BASE		(KSEG1 | 0x1D808C00)
#define GPON_SGMII_END		(KSEG1 | 0x1D808CFF)
#define GPON_SGMII_SIZE		0x00000100
/** address range for gpio0
    0x1D810000--0x1D81007F */
#define GPON_GPIO0_BASE		(KSEG1 | 0x1D810000)
#define GPON_GPIO0_END		(KSEG1 | 0x1D81007F)
#define GPON_GPIO0_SIZE		0x00000080
/** address range for gpio2
    0x1D810100--0x1D81017F */
#define GPON_GPIO2_BASE		(KSEG1 | 0x1D810100)
#define GPON_GPIO2_END		(KSEG1 | 0x1D81017F)
#define GPON_GPIO2_SIZE		0x00000080
/** address range for sys_eth
    0x1DB00000--0x1DB000FF */
#define GPON_SYS_ETH_BASE		(KSEG1 | 0x1DB00000)
#define GPON_SYS_ETH_END		(KSEG1 | 0x1DB000FF)
#define GPON_SYS_ETH_SIZE		0x00000100
/** address range for padctrl0
    0x1DB01000--0x1DB010FF */
#define GPON_PADCTRL0_BASE		(KSEG1 | 0x1DB01000)
#define GPON_PADCTRL0_END		(KSEG1 | 0x1DB010FF)
#define GPON_PADCTRL0_SIZE		0x00000100
/** address range for padctrl2
    0x1DB02000--0x1DB020FF */
#define GPON_PADCTRL2_BASE		(KSEG1 | 0x1DB02000)
#define GPON_PADCTRL2_END		(KSEG1 | 0x1DB020FF)
#define GPON_PADCTRL2_SIZE		0x00000100
/** address range for gtc
    0x1DC05000--0x1DC052D4 */
#define GPON_GTC_BASE		(KSEG1 | 0x1DC05000)
#define GPON_GTC_END		(KSEG1 | 0x1DC052D4)
#define GPON_GTC_SIZE		0x000002D5
/** address range for pma
    0x1DD00000--0x1DD003FF */
#define GPON_PMA_BASE		(KSEG1 | 0x1DD00000)
#define GPON_PMA_END		(KSEG1 | 0x1DD003FF)
#define GPON_PMA_SIZE		0x00000400
/** address range for fcsic
    0x1DD00600--0x1DD0061F */
#define GPON_FCSIC_BASE		(KSEG1 | 0x1DD00600)
#define GPON_FCSIC_END		(KSEG1 | 0x1DD0061F)
#define GPON_FCSIC_SIZE		0x00000020
/** address range for pma_int200
    0x1DD00700--0x1DD0070F */
#define GPON_PMA_INT200_BASE		(KSEG1 | 0x1DD00700)
#define GPON_PMA_INT200_END		(KSEG1 | 0x1DD0070F)
#define GPON_PMA_INT200_SIZE		0x00000010
/** address range for pma_inttx
    0x1DD00720--0x1DD0072F */
#define GPON_PMA_INTTX_BASE		(KSEG1 | 0x1DD00720)
#define GPON_PMA_INTTX_END		(KSEG1 | 0x1DD0072F)
#define GPON_PMA_INTTX_SIZE		0x00000010
/** address range for pma_intrx
    0x1DD00740--0x1DD0074F */
#define GPON_PMA_INTRX_BASE		(KSEG1 | 0x1DD00740)
#define GPON_PMA_INTRX_END		(KSEG1 | 0x1DD0074F)
#define GPON_PMA_INTRX_SIZE		0x00000010
/** address range for gtc_pma
    0x1DEFFF00--0x1DEFFFFF */
#define GPON_GTC_PMA_BASE		(KSEG1 | 0x1DEFFF00)
#define GPON_GTC_PMA_END		(KSEG1 | 0x1DEFFFFF)
#define GPON_GTC_PMA_SIZE		0x00000100
/** address range for sys
    0x1DF00000--0x1DF000FF */
#define GPON_SYS_BASE		(KSEG1 | 0x1DF00000)
#define GPON_SYS_END		(KSEG1 | 0x1DF000FF)
#define GPON_SYS_SIZE		0x00000100
/** address range for asc1
    0x1E100B00--0x1E100BFF */
#define GPON_ASC1_BASE		(KSEG1 | 0x1E100B00)
#define GPON_ASC1_END		(KSEG1 | 0x1E100BFF)
#define GPON_ASC1_SIZE		0x00000100
/** address range for asc0
    0x1E100C00--0x1E100CFF */
#define GPON_ASC0_BASE		(KSEG1 | 0x1E100C00)
#define GPON_ASC0_END		(KSEG1 | 0x1E100CFF)
#define GPON_ASC0_SIZE		0x00000100
/** address range for i2c
    0x1E200000--0x1E20FFFF */
#define GPON_I2C_BASE		(KSEG1 | 0x1E200000)
#define GPON_I2C_END		(KSEG1 | 0x1E20FFFF)
#define GPON_I2C_SIZE		0x00010000
/** address range for gpio1
    0x1E800100--0x1E80017F */
#define GPON_GPIO1_BASE		(KSEG1 | 0x1E800100)
#define GPON_GPIO1_END		(KSEG1 | 0x1E80017F)
#define GPON_GPIO1_SIZE		0x00000080
/** address range for gpio3
    0x1E800200--0x1E80027F */
#define GPON_GPIO3_BASE		(KSEG1 | 0x1E800200)
#define GPON_GPIO3_END		(KSEG1 | 0x1E80027F)
#define GPON_GPIO3_SIZE		0x00000080
/** address range for gpio4
    0x1E800300--0x1E80037F */
#define GPON_GPIO4_BASE		(KSEG1 | 0x1E800300)
#define GPON_GPIO4_END		(KSEG1 | 0x1E80037F)
#define GPON_GPIO4_SIZE		0x00000080
/** address range for padctrl1
    0x1E800400--0x1E8004FF */
#define GPON_PADCTRL1_BASE		(KSEG1 | 0x1E800400)
#define GPON_PADCTRL1_END		(KSEG1 | 0x1E8004FF)
#define GPON_PADCTRL1_SIZE		0x00000100
/** address range for padctrl3
    0x1E800500--0x1E8005FF */
#define GPON_PADCTRL3_BASE		(KSEG1 | 0x1E800500)
#define GPON_PADCTRL3_END		(KSEG1 | 0x1E8005FF)
#define GPON_PADCTRL3_SIZE		0x00000100
/** address range for padctrl4
    0x1E800600--0x1E8006FF */
#define GPON_PADCTRL4_BASE		(KSEG1 | 0x1E800600)
#define GPON_PADCTRL4_END		(KSEG1 | 0x1E8006FF)
#define GPON_PADCTRL4_SIZE		0x00000100
/** address range for status
    0x1E802000--0x1E80207F */
#define GPON_STATUS_BASE		(KSEG1 | 0x1E802000)
#define GPON_STATUS_END		(KSEG1 | 0x1E80207F)
#define GPON_STATUS_SIZE		0x00000080
/** address range for dcdc_1v0
    0x1E803000--0x1E8033FF */
#define GPON_DCDC_1V0_BASE		(KSEG1 | 0x1E803000)
#define GPON_DCDC_1V0_END		(KSEG1 | 0x1E8033FF)
#define GPON_DCDC_1V0_SIZE		0x00000400
/** address range for dcdc_ddr
    0x1E804000--0x1E8043FF */
#define GPON_DCDC_DDR_BASE		(KSEG1 | 0x1E804000)
#define GPON_DCDC_DDR_END		(KSEG1 | 0x1E8043FF)
#define GPON_DCDC_DDR_SIZE		0x00000400
/** address range for dcdc_apd
    0x1E805000--0x1E8053FF */
#define GPON_DCDC_APD_BASE		(KSEG1 | 0x1E805000)
#define GPON_DCDC_APD_END		(KSEG1 | 0x1E8053FF)
#define GPON_DCDC_APD_SIZE		0x00000400
/** address range for sys1
    0x1EF00000--0x1EF000FF */
#define GPON_SYS1_BASE		(KSEG1 | 0x1EF00000)
#define GPON_SYS1_END		(KSEG1 | 0x1EF000FF)
#define GPON_SYS1_SIZE		0x00000100
/** address range for sbs0ctrl
    0x1F080000--0x1F0801FF */
#define GPON_SBS0CTRL_BASE		(KSEG1 | 0x1F080000)
#define GPON_SBS0CTRL_END		(KSEG1 | 0x1F0801FF)
#define GPON_SBS0CTRL_SIZE		0x00000200
/** address range for sbs0red
    0x1F080200--0x1F08027F */
#define GPON_SBS0RED_BASE		(KSEG1 | 0x1F080200)
#define GPON_SBS0RED_END		(KSEG1 | 0x1F08027F)
#define GPON_SBS0RED_SIZE		0x00000080
/** address range for sbs0ram
    0x1F200000--0x1F32FFFF */
#define GPON_SBS0RAM_BASE		(KSEG1 | 0x1F200000)
#define GPON_SBS0RAM_END		(KSEG1 | 0x1F32FFFF)
#define GPON_SBS0RAM_SIZE		0x00130000
/** address range for ddrdb
    0x1F701000--0x1F701FFF */
#define GPON_DDRDB_BASE		(KSEG1 | 0x1F701000)
#define GPON_DDRDB_END		(KSEG1 | 0x1F701FFF)
#define GPON_DDRDB_SIZE		0x00001000
/** address range for sbiu
    0x1F880000--0x1F8800FF */
#define GPON_SBIU_BASE		(KSEG1 | 0x1F880000)
#define GPON_SBIU_END		(KSEG1 | 0x1F8800FF)
#define GPON_SBIU_SIZE		0x00000100
/** address range for icu0
    0x1F880200--0x1F8802DF */
#define GPON_ICU0_BASE		(KSEG1 | 0x1F880200)
#define GPON_ICU0_END		(KSEG1 | 0x1F8802DF)
#define GPON_ICU0_SIZE		0x000000E0
/** address range for icu1
    0x1F880300--0x1F8803DF */
#define GPON_ICU1_BASE		(KSEG1 | 0x1F880300)
#define GPON_ICU1_END		(KSEG1 | 0x1F8803DF)
#define GPON_ICU1_SIZE		0x000000E0
/** address range for wdt
    0x1F8803F0--0x1F8803FF */
#define GPON_WDT_BASE		(KSEG1 | 0x1F8803F0)
#define GPON_WDT_END		(KSEG1 | 0x1F8803FF)
#define GPON_WDT_SIZE		0x00000010

/*! @} */ /* GPON_BASE */

#endif /* _gpon_reg_base_h */

