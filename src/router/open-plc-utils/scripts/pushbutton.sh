#!/bin/sh
# file: scripts/pushbutton.sh

# This script illustrates how to use MS_PB_ENC to break and make a
# network for testing purposes;
  

# ====================================================================
# host symbols;
# --------------------------------------------------------------------

. /etc/environment
. ${SCRIPTS}/hardware.sh

# ====================================================================
# leave network;
# --------------------------------------------------------------------

int6k -i ${ETH1} -B 2
int6kwait -i ${ETH1} -rs
int6k -i ${ETH2} -B 2
int6kwait -i${ETH2} -rs
int6k -i ${ETH1} -I
int6k -i ${ETH2} -I

# ====================================================================
# create network;
# --------------------------------------------------------------------

int6k -i ${ETH1} -B 1
int6k -i ${ETH2} -B 1
int6kwait -i ${ETH1} -rs
int6kwait -i ${ETH2} -rs
int6k -i ${ETH1} -I
int6k -i ${ETH2} -I

