#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-error.h>
#include <asm/octeon/cvmx-error-custom.h>
#else
#include "cvmx.h"
#include "cvmx-error.h"
#include "cvmx-error-custom.h"
#include "cvmx-helper.h"
#include "cvmx-helper-cfg.h"
#endif
/**
 * @INTERNAL
 * XAUI interfaces need to be reset whenever a local or remote fault
 * is detected. Calling autoconf takes the link through a reset.
 *
 * @param info
 *
 * @return
 */
static int __cvmx_error_handle_gmxx_rxx_int_reg(const struct cvmx_error_info *info)
{
	if(cvmx_enable_helper_flag) {
		int ipd_port = info->group_index;
		switch (ipd_port) {
			case 0x800:
				ipd_port = 0x840;
				break;
			case 0xa00:
				ipd_port = 0xa40;
				break;
			case 0xb00:
				ipd_port = 0xb40;
				break;
			case 0xc00:
				ipd_port = 0xc40;
				break;
		}
		cvmx_helper_link_autoconf(ipd_port);
	}
	cvmx_write_csr(info->status_addr, info->status_mask);
	return 1;
}

static void cvmx_install_gmx_error_handler(int i)
{
	cvmx_error_change_handler(CVMX_ERROR_REGISTER_IO64,
				  CVMX_GMXX_RXX_INT_REG(0, i),
				  1ull << 21 /* rem_fault */ ,
				  __cvmx_error_handle_gmxx_rxx_int_reg, 0, NULL, NULL);
	cvmx_error_change_handler(CVMX_ERROR_REGISTER_IO64, CVMX_GMXX_RXX_INT_REG(0, i),
				  1ull << 20 /* loc_fault */ ,
				  __cvmx_error_handle_gmxx_rxx_int_reg, 0, NULL, NULL);
}

void __cvmx_install_gmx_error_handler_for_xaui(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX)
	    || OCTEON_IS_OCTEON2()) {
		int i;
		/* Install special handler for all the interfaces, these are
		   specific to XAUI interface */
		for (i = 0; i < CVMX_HELPER_MAX_GMX; i++) {
			if ((OCTEON_IS_MODEL(OCTEON_CN63XX)
			     || OCTEON_IS_MODEL(OCTEON_CN52XX)
			     || OCTEON_IS_MODEL(OCTEON_CNF71XX))
			    && i == 1)
				continue;
			cvmx_install_gmx_error_handler(i);
		}
	}
}
