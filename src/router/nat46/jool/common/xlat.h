#ifndef SRC_COMMON_XLAT_H_
#define SRC_COMMON_XLAT_H_

#define JOOL_LICENSE "GPL v2"

/**
 * These defines are read in from dkms.conf. If you change their syntax or
 * relocate them, please make sure to also update dkms.conf accordingly.
 */
#define JOOL_VERSION_MAJOR 4
#define JOOL_VERSION_MINOR 1
#define JOOL_VERSION_REV 11
#define JOOL_VERSION_DEV 0

/** See http://stackoverflow.com/questions/195975 */
#define STR_VALUE(arg) #arg
#define VALUE_TO_STR(name) STR_VALUE(name)
#define JOOL_VERSION_STR \
	VALUE_TO_STR(JOOL_VERSION_MAJOR) "." \
	VALUE_TO_STR(JOOL_VERSION_MINOR) "." \
	VALUE_TO_STR(JOOL_VERSION_REV) "." \
	VALUE_TO_STR(JOOL_VERSION_DEV)

static inline unsigned int xlat_version(void)
{
	return (JOOL_VERSION_MAJOR << 24)
			| (JOOL_VERSION_MINOR << 16)
			| (JOOL_VERSION_REV << 8)
			| JOOL_VERSION_DEV;
}

#endif /* SRC_COMMON_XLAT_H_ */
