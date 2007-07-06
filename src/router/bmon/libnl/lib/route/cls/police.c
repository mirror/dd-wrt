
/**
 * \name Policer Type
 * \{
 */

static struct trans_tbl police_types[] = {
	__ADD(TC_POLICE_UNSPEC,unspec)
	__ADD(TC_POLICE_OK,ok)
	__ADD(TC_POLICE_RECLASSIFY,reclassify)
	__ADD(TC_POLICE_SHOT,shot)
#ifdef TC_POLICE_PIPE
	__ADD(TC_POLICE_PIPE,pipe)
#endif
};

/**
 * Transform a policer type number into a character string (Reentrant).
 * @arg type		policer type
 * @arg buf		destination buffer
 * @arg len		buffer length
 *
 * Transforms a policer type number into a character string and stores
 * it in the provided buffer.
 *
 * \return The destination buffer or the type encoded in hex if no match was found.
 */
char * nl_police2str_r(int type, char *buf, size_t len)
{
	return __type2str_r(type, buf, len, police_types,
	    ARRAY_SIZE(police_types));
}

/**
 * Transform a policer type number into a character string
 * @arg type		policer type
 *
 * Transforms a policer type number into a character string and stores
 * it in a static buffer.
 *
 * \return A static buffer or the type encoded in hex if no match was found.
 * \attention This funnction is NOT thread safe.
 */
char * nl_police2str(int type)
{
	static char buf[32];
	memset(buf, 0, sizeof(buf));
	return __type2str_r(type, buf, sizeof(buf), police_types,
	    ARRAY_SIZE(police_types));
}


/**
 * nl_str2police - Converts a police name to police type
 *
 * @arg name   police type name
 *
 * Converts a police name to its correpsnding police type. Returns
 * the type or -1 if no match was found.
 */
/**
 * Transform a character string into a policer type number
 * @arg name		policer type name
 *
 * Transform the provided character string specifying a policer
 * type into the corresponding numeric value
 *
 * \return Policer type number or a negative value.
 */
int nl_str2police(const char *name)
{
	return __str2type(name, police_types, ARRAY_SIZE(police_types));
}

/** \} */
