#
# Makefile for the video capture/playback device drivers.
#

obj-$(CPTCFG_VIDEO_M32R_AR_M64278) += arv.o

obj-$(CPTCFG_VIDEO_VIA_CAMERA) += via-camera.o
obj-$(CPTCFG_VIDEO_CAFE_CCIC) += marvell-ccic/
obj-$(CPTCFG_VIDEO_MMP_CAMERA) += marvell-ccic/

obj-$(CPTCFG_VIDEO_OMAP3)	+= omap3isp/
obj-$(CPTCFG_VIDEO_PXA27x)	+= pxa_camera.o

obj-$(CPTCFG_VIDEO_VIU) += fsl-viu.o

obj-$(CPTCFG_VIDEO_VIVID)		+= vivid/
obj-$(CPTCFG_VIDEO_VIM2M)		+= vim2m.o

obj-$(CPTCFG_VIDEO_TI_VPE)		+= ti-vpe/

obj-$(CPTCFG_VIDEO_TI_CAL)		+= ti-vpe/

obj-$(CPTCFG_VIDEO_MX2_EMMAPRP)		+= mx2_emmaprp.o
obj-$(CPTCFG_VIDEO_CODA) 		+= coda/

obj-$(CPTCFG_VIDEO_SH_VEU)		+= sh_veu.o

obj-$(CPTCFG_VIDEO_MEM2MEM_DEINTERLACE)	+= m2m-deinterlace.o

obj-$(CPTCFG_VIDEO_S3C_CAMIF) 		+= s3c-camif/
obj-$(CPTCFG_VIDEO_SAMSUNG_EXYNOS4_IS) 	+= exynos4-is/
obj-$(CPTCFG_VIDEO_SAMSUNG_S5P_JPEG)	+= s5p-jpeg/
obj-$(CPTCFG_VIDEO_SAMSUNG_S5P_MFC)	+= s5p-mfc/

obj-$(CPTCFG_VIDEO_SAMSUNG_S5P_G2D)	+= s5p-g2d/
obj-$(CPTCFG_VIDEO_SAMSUNG_EXYNOS_GSC)	+= exynos-gsc/

obj-$(CPTCFG_VIDEO_STI_BDISP)		+= sti/bdisp/
obj-$(CPTCFG_VIDEO_STI_HVA)		+= sti/hva/
obj-$(CPTCFG_DVB_C8SECTPFE)		+= sti/c8sectpfe/

obj-$(CONFIG_BLACKFIN)                  += blackfin/

obj-$(CONFIG_ARCH_DAVINCI)		+= davinci/

obj-$(CPTCFG_VIDEO_SH_VOU)		+= sh_vou.o

obj-$(CPTCFG_SOC_CAMERA)		+= soc_camera/

obj-$(CPTCFG_VIDEO_RENESAS_FCP) 	+= rcar-fcp.o
obj-$(CPTCFG_VIDEO_RENESAS_FDP1)	+= rcar_fdp1.o
obj-$(CPTCFG_VIDEO_RENESAS_JPU) 	+= rcar_jpu.o
obj-$(CPTCFG_VIDEO_RENESAS_VSP1)	+= vsp1/

obj-y	+= omap/

obj-$(CPTCFG_VIDEO_AM437X_VPFE)		+= am437x/

obj-$(CPTCFG_VIDEO_XILINX)		+= xilinx/

obj-$(CPTCFG_VIDEO_RCAR_VIN)		+= rcar-vin/

obj-$(CPTCFG_VIDEO_ATMEL_ISC)		+= atmel/

ccflags-y += -I$(backport_srctree)/drivers/media/i2c

obj-$(CPTCFG_VIDEO_MEDIATEK_VPU)	+= mtk-vpu/

obj-$(CPTCFG_VIDEO_MEDIATEK_VCODEC)	+= mtk-vcodec/

obj-$(CPTCFG_VIDEO_MEDIATEK_MDP)	+= mtk-mdp/
