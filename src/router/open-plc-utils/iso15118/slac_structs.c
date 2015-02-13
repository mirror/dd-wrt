/*====================================================================*
 *
 *   Copyright (c) 2013 Qualcomm Atheros, Inc.
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or 
 *   without modification, are permitted (subject to the limitations 
 *   in the disclaimer below) provided that the following conditions 
 *   are met:
 *
 *   * Redistributions of source code must retain the above copyright 
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above 
 *     copyright notice, this list of conditions and the following 
 *     disclaimer in the documentation and/or other materials 
 *     provided with the distribution.
 *
 *   * Neither the name of Qualcomm Atheros nor the names of 
 *     its contributors may be used to endorse or promote products 
 *     derived from this software without specific prior written 
 *     permission.
 *
 *   NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE 
 *   GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
 *   COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 *   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 *   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER 
 *   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 *   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *
 *   void slac_structs ()
 *
 *   slac.h
 *
 *--------------------------------------------------------------------*/

#include <unistd.h>

#include "../iso15118/slac.h"

void slac_structs () 

{ 
	fprintf (stderr, "sizeof struct cm_sta_identity_request %d\n", sizeof (struct cm_sta_identity_request)); 
	fprintf (stderr, "sizeof struct cm_sta_identity_confirm %d\n", sizeof (struct cm_sta_identity_confirm)); 
	fprintf (stderr, "sizeof struct cm_slac_param_request %d\n", sizeof (struct cm_slac_param_request)); 
	fprintf (stderr, "sizeof struct cm_slac_param_confirm %d\n", sizeof (struct cm_slac_param_confirm)); 
	fprintf (stderr, "sizeof struct cm_start_atten_char_indicate %d\n", sizeof (struct cm_start_atten_char_indicate)); 
	fprintf (stderr, "sizeof struct cm_start_atten_char_response %d\n", sizeof (struct cm_start_atten_char_response)); 
	fprintf (stderr, "sizeof struct cm_atten_char_indicate %d\n", sizeof (struct cm_atten_char_indicate)); 
	fprintf (stderr, "sizeof struct cm_atten_char_response %d\n", sizeof (struct cm_atten_char_response)); 
	fprintf (stderr, "sizeof struct cm_mnbc_sound_indicate %d\n", sizeof (struct cm_mnbc_sound_indicate)); 
	fprintf (stderr, "sizeof struct cm_validate_request %d\n", sizeof (struct cm_validate_request)); 
	fprintf (stderr, "sizeof struct cm_validate_confirm %d\n", sizeof (struct cm_validate_confirm)); 
	fprintf (stderr, "sizeof struct cm_slac_match_request %d\n", sizeof (struct cm_slac_match_request)); 
	fprintf (stderr, "sizeof struct cm_slac_match_confirm %d\n", sizeof (struct cm_slac_match_confirm)); 
	return; 
} 

