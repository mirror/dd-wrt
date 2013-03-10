#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "at_parse.h"			/* at_parse_*() */
#include "mutils.h"			/* ITEMS_OF() */


int ok = 0;
int faults = 0;

#/* */
void test_parse_cnum()
{
	static const struct test_case {
		const char	* input;
		const char	* result;
	} cases[] = {
		{ "+CNUM: \"*Subscriber Number\",\"+79139131234\",145", "+79139131234" },
		{ "+CNUM: \"Subscriber Number\",\"\",145", "" },
		{ "+CNUM: \"Subscriber Number\",,145", "" },
		{ "+CNUM: \"\",\"+79139131234\",145", "+79139131234" },
		{ "+CNUM: ,\"\",145", "" },
		{ "+CNUM: ,,145", "" },
		{ "+CNUM: \"\",+79139131234\",145", "+79139131234" },
		{ "+CNUM: \"\",+79139131234,145", "+79139131234" },
	};
	unsigned idx = 0;
	char * input;
	const char * res;
	const char * msg;
	
	for(; idx < ITEMS_OF(cases); ++idx) {
		input = strdup(cases[idx].input);
		fprintf(stderr, "%s(\"%s\")...", "at_parse_cnum", input);
		res = at_parse_cnum(input);
		if(strcmp(res, cases[idx].result) == 0) {
			msg = "OK";
			ok++;
		} else {
			msg = "FAIL";
			faults++;
		}
		fprintf(stderr, " = \"%s\"\t%s\n", res, msg);
		free(input);
	}
	fprintf(stderr, "\n");
}

#/* */
void test_parse_cops()
{
	static const struct test_case {
		const char	* input;
		const char	* result;
	} cases[] = {
		{ "+COPS: 0,0,\"TELE2\",0", "TELE2" },
		{ "+COPS: 0,0,\"TELE2,0", "TELE2" },
		{ "+COPS: 0,0,TELE2,0", "TELE2" },
	};
	unsigned idx = 0;
	char * input;
	const char * res;
	const char * msg;
	
	for(; idx < ITEMS_OF(cases); ++idx) {
		input = strdup(cases[idx].input);
		fprintf(stderr, "%s(\"%s\")...", "at_parse_cops", input);
		res = at_parse_cops(input);
		if(strcmp(res, cases[idx].result) == 0) {
			msg = "OK";
			ok++;
		} else {
			msg = "FAIL";
			faults++;
		}
		fprintf(stderr, " = \"%s\"\t%s\n", res, msg);
		free(input);
	}
	fprintf(stderr, "\n");
}

#/* */
void test_parse_creg()
{
	struct result {
		int	res;
		int	gsm_reg;
		int	gsm_reg_status;
		char 	* lac;
		char	* ci;
	};
	static const struct test_case {
		const char	* input;
		struct result 	result;
	} cases[] = {
		{ "+CREG: 2,1,9110,7E6", { 0, 1, 1, "9110", "7E6"} },
		{ "+CREG: 2,1,XXXX,AAAA", { 0, 1, 1, "XXXX", "AAAA"} },
	};
	unsigned idx = 0;
	char * input;
	struct result result;
	const char * msg;
	
	for(; idx < ITEMS_OF(cases); ++idx) {
		input = strdup(cases[idx].input);
		fprintf(stderr, "%s(\"%s\")...", "at_parse_creg", input);
		result.res = at_parse_creg(input, strlen(input), &result.gsm_reg, &result.gsm_reg_status, &result.lac, &result.ci);
		if(result.res == cases[idx].result.res
			&&
		   result.gsm_reg == cases[idx].result.gsm_reg
			&&
		   result.gsm_reg_status == cases[idx].result.gsm_reg_status
			&&
		   strcmp(result.lac, cases[idx].result.lac) == 0
			&&
		   strcmp(result.ci, cases[idx].result.ci) == 0
			) {
			msg = "OK";
			ok++;
		} else {
			msg = "FAIL";
			faults++;
		}
		fprintf(stderr, " = %d (%d,%d,\"%s\",\"%s\")\t%s\n", result.res, result.gsm_reg, result.gsm_reg_status, result.lac, result.ci, msg);
		free(input);
	}
	fprintf(stderr, "\n");
}

#/* */
void test_parse_cmti()
{
	static const struct test_case {
		const char	* input;
		int		result;
	} cases[] = {
		{ "+CMTI: \"ME\",41", 41 },
		{ "+CMTI: 0,111", 111 },
		{ "+CMTI: ", -1 },
	};
	unsigned idx = 0;
	char * input;
	int result;
	const char * msg;
	
	for(; idx < ITEMS_OF(cases); ++idx) {
		input = strdup(cases[idx].input);
		fprintf(stderr, "%s(\"%s\")...", "at_parse_cmti", input);
		result = at_parse_cmti(input);
		if(result == cases[idx].result) {
			msg = "OK";
			ok++;
		} else {
			msg = "FAIL";
			faults++;
		}
		fprintf(stderr, " = %d\t%s\n", result, msg);
		free(input);
	}
	fprintf(stderr, "\n");
}

#/* */
void test_parse_cmgr()
{
	struct result {
		const char	* res;
		char		* str;
		char 		* oa;
		str_encoding_t	oa_enc;
		char		* msg;
		str_encoding_t	msg_enc;
	};
	static const struct test_case {
		const char	* input;
		struct result 	result;
	} cases[] = {
		{ "+CMGR: \"REC READ\",\"+79139131234\",,\"10/12/05,22:00:04+12\"\r\n041F04400438043204350442", 
			{
				NULL,
				"\"REC READ\",\"+79139131234",
				"+79139131234",
				STR_ENCODING_7BIT,
				"041F04400438043204350442",
				STR_ENCODING_UNKNOWN
			}
		},
		{ "+CMGR: \"REC READ\",\"002B00370039003500330037003600310032003000350032\",,\"10/12/05,22:00:04+12\"\r\n041F04400438043204350442", 
			{
				NULL, 
				"\"REC READ\",\"002B00370039003500330037003600310032003000350032",
				"002B00370039003500330037003600310032003000350032", 
				STR_ENCODING_UNKNOWN,
				"041F04400438043204350442",
				STR_ENCODING_UNKNOWN
			}
		},
		{ "+CMGR: 0,,106\r\n07911111111100F3040B911111111111F200000121702214952163B1582C168BC562B1984C2693C96432994C369BCD66B3D96C369BD168341A8D46A3D168B55AAD56ABD56AB59ACD66B3D96C369BCD76BBDD6EB7DBED76BBE170381C0E87C3E170B95C2E97CBE572B91C0C0683C16030180C",
			{
				NULL,
				"B1582C168BC562B1984C2693C96432994C369BCD66B3D96C369BD168341A8D46A3D168B55AAD56ABD56AB59ACD66B3D96C369BCD76BBDD6EB7DBED76BBE170381C0E87C3E170B95C2E97CBE572B91C0C0683C16030180C",
				"+11111111112",
				STR_ENCODING_7BIT,
				"B1582C168BC562B1984C2693C96432994C369BCD66B3D96C369BD168341A8D46A3D168B55AAD56ABD56AB59ACD66B3D96C369BCD76BBDD6EB7DBED76BBE170381C0E87C3E170B95C2E97CBE572B91C0C0683C16030180C",
				STR_ENCODING_7BIT_HEX
			} 
		},
		{ "+CMGR: 0,,159\r\n07919740430900F3440B912222222220F20008012180004390218C0500030003010031003100310031003100310031003100310031003200320032003200320032003200320032003200330033003300330033003300330033003300330034003400340034003400340034003400340034003500350035003500350035003500350035003500360036003600360036003600360036003600360037003700370037003700370037",
			{
				NULL,
				"0031003100310031003100310031003100310031003200320032003200320032003200320032003200330033003300330033003300330033003300330034003400340034003400340034003400340034003500350035003500350035003500350035003500360036003600360036003600360036003600360037003700370037003700370037",
				"+22222222022",
				STR_ENCODING_7BIT,
				"0031003100310031003100310031003100310031003200320032003200320032003200320032003200330033003300330033003300330033003300330034003400340034003400340034003400340034003500350035003500350035003500350035003500360036003600360036003600360036003600360037003700370037003700370037",
				STR_ENCODING_UCS2_HEX
			} 
		},

	};

	unsigned idx = 0;
	char * input;
	struct result result;
	char oa[200];
	const char * msg;

	result.oa = oa;
	for(; idx < ITEMS_OF(cases); ++idx) {
		result.str = input = strdup(cases[idx].input);
		fprintf(stderr, "%s(\"%s\")...", "at_parse_cmgr", input);
		result.res = at_parse_cmgr(&result.str, strlen(result.str), result.oa, sizeof(oa), &result.oa_enc, &result.msg, &result.msg_enc);
		if( ((result.res == NULL && result.res == cases[idx].result.res) || strcmp(result.res, cases[idx].result.res) == 0)
			&&
		   strcmp(result.str, cases[idx].result.str) == 0
			&&
		   strcmp(result.oa, cases[idx].result.oa) == 0
			&&
		   result.oa_enc == cases[idx].result.oa_enc
			&&
		   strcmp(result.msg, cases[idx].result.msg) == 0
			&&
		   result.msg_enc == cases[idx].result.msg_enc
			) {
			msg = "OK";
			ok++;
		} else {
			msg = "FAIL";
			faults++;
		}
		fprintf(stderr, " = '%s' ('%s','%s',%d,'%s',%d)\t%s\n", result.res, result.str, result.oa, result.oa_enc, result.msg, result.msg_enc, msg);
		free(input);
	}
	fprintf(stderr, "\n");
}

#/* */
void test_parse_cusd()
{
	struct result {
		int	res;
		int	type;
		char 	* cusd;
		int	dcs;
	};
	static const struct test_case {
		const char	* input;
		struct result 	result;
	} cases[] = {
		{ "+CUSD: 0,\"CF2135487D2E4130572D0682BB1A\",0", { 0, 0, "CF2135487D2E4130572D0682BB1A", 0} },
		{ "+CUSD: 1,\"CF2135487D2E4130572D0682BB1A\",1", { 0, 1, "CF2135487D2E4130572D0682BB1A", 1} },
		{ "+CUSD: 5", { 0, 5, "", -1} },
	};
	unsigned idx = 0;
	char * input;
	struct result result;
	const char * msg;
	
	for(; idx < ITEMS_OF(cases); ++idx) {
		input = strdup(cases[idx].input);
		fprintf(stderr, "%s(\"%s\")...", "at_parse_cusd", input);
		result.res = at_parse_cusd(input, &result.type, &result.cusd, &result.dcs);
		if(result.res == cases[idx].result.res
			&&
		   result.type == cases[idx].result.type
			&&
		   result.dcs == cases[idx].result.dcs
			&&
		   strcmp(result.cusd, cases[idx].result.cusd) == 0
			) {
			msg = "OK";
			ok++;
		} else {
			msg = "FAIL";
			faults++;
		}
		fprintf(stderr, " = %d (%d,\"%s\",%d)\t%s\n", result.res, result.type, result.cusd, result.dcs, msg);
		free(input);
	}
	fprintf(stderr, "\n");
}

#/* */
void test_parse_cpin()
{
}

#/* */
void test_parse_csq()
{
}

#/* */
void test_parse_rssi()
{
}

#/* */
void test_parse_mode()
{
}

#/* */
void test_parse_csca()
{
}

#/* */
void test_parse_clcc()
{
	struct result {
		int		res;

		unsigned	index;
		unsigned	dir;
		unsigned	stat;
		unsigned	mode;
		unsigned	mpty;
		char		* number;
		unsigned	toa;
	};
	static const struct test_case {
		const char	* input;
		struct result	result;
	} cases[] = {
		{ "+CLCC: 1,1,4,0,0,\"\",145", { 0, 1, 1, 4, 0, 0, "", 145} },
		{ "+CLCC: 1,1,4,0,0,\"+79139131234\",145", { 0, 1, 1, 4, 0, 0, "+79139131234", 145} },
		{ "+CLCC: 1,1,4,0,0,\"+7913913ABCA\",145", { 0, 1, 1, 4, 0, 0, "+7913913ABCA", 145} },
		{ "+CLCC: 1,1,4,0,0,\"+7913913ABCA\"", { -1, 0, 0, 0, 0, 0, "", 0} },
	};
	unsigned idx = 0;
	char * input;
	struct result result;
	const char * msg;
	
	for(; idx < ITEMS_OF(cases); ++idx) {
		input = strdup(cases[idx].input);
		fprintf(stderr, "%s(\"%s\")...", "at_parse_clcc", input);
		result.res = at_parse_clcc(input, &result.index, &result.dir, &result.stat, &result.mode, &result.mpty, &result.number, &result.toa);
		if(result.res == cases[idx].result.res
			&&
		   result.index == cases[idx].result.index
			&&
		   result.dir == cases[idx].result.dir
			&&
		   result.stat == cases[idx].result.stat
			&&
		   result.mode == cases[idx].result.mode
			&&
		   result.mpty == cases[idx].result.mpty
			&&
		   strcmp(result.number, cases[idx].result.number) == 0
			&&
		   result.toa == cases[idx].result.toa
			) {
			msg = "OK";
			ok++;
		} else {
			msg = "FAIL";
			faults++;
		}
		fprintf(stderr, " = %d (%d,%d,%d,%d,%d,\"%s\",%d)\t%s\n", result.res, result.index, result.dir, result.stat, result.mode, result.mpty, result.number, result.toa, msg);
		free(input);
	}
	fprintf(stderr, "\n");
}

#/* */
void test_parse_ccwa()
{
}

#/* */
int main()
{
	test_parse_cnum();
	test_parse_cops();
	test_parse_creg();
	test_parse_cmti();
	test_parse_cmgr();
	test_parse_cusd();
	test_parse_cpin();
	test_parse_csq();
	test_parse_rssi();
	test_parse_mode();
	test_parse_csca();
	test_parse_clcc();
	test_parse_ccwa();
	
	fprintf(stderr, "done %d tests: %d OK %d FAILS\n", ok + faults, ok, faults);
	return 0;
}