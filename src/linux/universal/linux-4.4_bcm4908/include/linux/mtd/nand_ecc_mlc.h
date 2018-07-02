#ifndef __MTD_NAND_ECC_MLC_H__
#define __MTD_NAND_ECC_MLC_H__

int nand_calculate_ecc_mlc(const u_char *dat, u_char *ecc_code);
int nand_correct_data_mlc(u_char *dat, u_char *read_ecc);

#endif /* __MTD_NAND_ECC_H__ */
