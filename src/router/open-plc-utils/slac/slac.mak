# ====================================================================
# programs;
# --------------------------------------------------------------------

evse.o: evse.c channel.h config.h error.h flags.h getoptv.h memory.h number.h putoptv.h slac.h types.h
pev.o: pev.c channel.h config.h error.h files.h flags.h getoptv.h memory.h number.h putoptv.h slac.h types.h

# ====================================================================
# functions;
# --------------------------------------------------------------------

evse_cm_atten_char.o: evse_cm_atten_char.c channel.h error.h memory.h slac.h
evse_cm_atten_profile.o: evse_cm_atten_profile.c channel.h error.h memory.h slac.h types.h
evse_cm_mnbc_sound.o: evse_cm_mnbc_sound.c channel.h error.h memory.h slac.h
evse_cm_slac_match.o: evse_cm_slac_match.c channel.h error.h memory.h slac.h types.h
evse_cm_slac_param.o: evse_cm_slac_param.c channel.h error.h memory.h slac.h types.h
evse_cm_start_atten_char.o: evse_cm_start_atten_char.c channel.h error.h memory.h slac.h
pev_cm_atten_char.o: pev_cm_atten_char.c channel.h error.h memory.h slac.h
pev_cm_mnbc_sound.o: pev_cm_mnbc_sound.c channel.h error.h memory.h slac.h types.h
pev_cm_slac_match.o: pev_cm_slac_match.c channel.h error.h memory.h slac.h types.h
pev_cm_slac_param.o: pev_cm_slac_param.c channel.h error.h memory.h slac.h types.h
pev_cm_start_atten_char.o: pev_cm_start_atten_char.c channel.h error.h slac.h types.h
slac_session.o: slac_session.c memory.h slac.h
slac_structs.o: slac_structs.c slac.h

# ====================================================================
# headers;
# --------------------------------------------------------------------

slac.h: types.h endian.h ether.h mme.h

