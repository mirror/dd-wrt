#include "usr/nl/json.h"

#include <errno.h>

struct jool_result type_mismatch(char const *field, cJSON *json,
		char const *expected)
{
	if (!field)
		field = "<unnamed>";

	switch (json->type) {
	case cJSON_False:
		return result_from_error(
			-EINVAL,
			"The '%s' element 'false' is not a valid %s.",
			field, expected
		);
	case cJSON_True:
		return result_from_error(
			-EINVAL,
			"The '%s' element 'true' is not a valid %s.",
			field, expected
		);
	case cJSON_NULL:
		return result_from_error(
			-EINVAL,
			"The '%s' element 'null' is not a valid %s.",
			field, expected
		);
	case cJSON_Number:
		if (json->numflags & VALUENUM_UINT) {
			return result_from_error(
				-EINVAL,
				"The '%s' element '%u' is not a valid %s.",
				field, json->valueuint, expected
			);
		}

		if (json->numflags & VALUENUM_INT) {
			return result_from_error(
				-EINVAL,
				"The '%s' element '%d' is not a valid %s.",
				field, json->valueint, expected
			);
		}

		return result_from_error(
			-EINVAL,
			"The '%s' element '%f' is not a valid %s.",
			field, json->valuedouble, expected
		);

	case cJSON_String:
		return result_from_error(
			-EINVAL,
			"The '%s' element '%s' is not a valid %s.",
			field, json->valuestring, expected
		);
	case cJSON_Array:
		return result_from_error(
			-EINVAL,
			"The '%s' element appears to be an array, not a '%s'.",
			field, expected
		);
	case cJSON_Object:
		return result_from_error(
			-EINVAL,
			"The '%s' element appears to be an object, not a '%s'.",
			field, expected
		);
	}

	return result_from_error(
		-EINVAL,
		"The '%s' element has unknown type. (Expected a '%s'.)",
		field, expected
	);
}

struct jool_result validate_uint(char const *field_name, cJSON *node,
		__u64 min, __u64 max)
{
	if (node->type != cJSON_Number || !(node->numflags & VALUENUM_UINT))
		return type_mismatch(field_name, node, "unsigned integer");

	if (node->valueuint < min || max < node->valueuint) {
		return result_from_error(
			-EINVAL,
			"%s %u is out of range (%llu-%llu).",
			field_name, node->valueuint, min, max
		);
	}

	return result_success();
}
