#include "usr/util/file.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Remember to free @result when you're done.
 */
struct jool_result file_to_string(char const *file_name, char **out)
{
	FILE *file;
	long int length;
	long int total_read;
	size_t current_read;
	char *buffer;
	int error;

	file = fopen(file_name, "rb");
	if (!file) {
		error = errno;
		return result_from_error(
			error,
			"Could not open file \"%s\": %s",
			file_name, strerror(error)
		);
	}

	error = fseek(file, 0, SEEK_END);
	if (error) {
		error = errno;
		fclose(file);
		return result_from_error(
			error,
			"Could not seek the end of file \"%s\" (1): %s",
			file_name, strerror(error)
		);
	}

	length = ftell(file);
	if (length == -1) {
		error = errno;
		fclose(file);
		return result_from_error(
			error,
			"Could not ftell on file \"%s\": %s",
			file_name, strerror(error)
		);
	}

	error = fseek(file, 0, SEEK_SET);
	if (error) {
		error = errno;
		fclose(file);
		return result_from_error(
			error,
			"Could not fseek on file \"%s\" (2): %s",
			file_name, strerror(error)
		);
	}

	buffer = malloc(length + 1);
	if (!buffer) {
		fclose(file);
		return result_from_enomem();
	}

	total_read = 0;
	while (total_read < length) {
		current_read = fread(&buffer[total_read], 1, length, file);
		if (current_read == 0 && ferror(file)) {
			free(buffer);
			fclose(file);
			/* There's literally no way to get an error code. */
			return result_from_error(
				-EINVAL,
				"File read error. (Sorry; there's no error message.)"
			);
		}

		total_read += current_read;
	}

	fclose(file);

	buffer[total_read] = '\0';
	*out = buffer;
	return result_success();
}
