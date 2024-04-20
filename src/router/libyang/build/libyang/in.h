/**
 * @file in.h
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang input structures and functions
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_IN_H_
#define LY_IN_H_

#include <stdio.h>

#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @page howtoInput Input Processing
 *
 * libyang provides a mechanism to generalize work with the inputs (and [outputs](@ref howtoOutput)) of
 * the different types. The ::ly_in handler can be created providing necessary information connected with the specific
 * input type and then used throughout the parser functions processing the input data. Using a generic input handler avoids
 * need to have a set of functions for each parser functionality and results in simpler API.
 *
 * The API allows to alter the source of the data behind the handler by another source. Also resetting a seekable source
 * input is possible with ::ly_in_reset() to re-read the input.
 *
 * @note
 * Currently, libyang supports only reading data from standard (disk) file, not from sockets, pipes, etc. The reason is
 * that the parsers expects all the data to be present in the file (input data are complete). In future, we would like
 * to change the internal mechanism and support sequential processing of the input data. In XML wording - we have DOM
 * parser, but in future we would like to move to something like a SAX parser.
 *
 * @note
 * This mechanism was introduced in libyang 2.0. To simplify transition from libyang 1.0 to version 2.0 and also for
 * some simple use case where using the input handler would be an overkill, there are some basic parsers functions
 * that do not require input handler. But remember, that functionality of these function can be limited in particular cases
 * in contrast to the functions using input handlers.
 *
 * Functions List
 * --------------
 * - ::ly_in_new_fd()
 * - ::ly_in_new_file()
 * - ::ly_in_new_filepath()
 * - ::ly_in_new_memory()
 *
 * - ::ly_in_fd()
 * - ::ly_in_file()
 * - ::ly_in_filepath()
 * - ::ly_in_memory()
 *
 * - ::ly_in_type()
 * - ::ly_in_parsed()
 *
 * - ::ly_in_reset()
 * - ::ly_in_free()
 *
 * libyang Parsers List
 * --------------------
 * - @subpage howtoSchemaParsers
 * - @subpage howtoDataParsers
 */

/**
 * @struct ly_in
 * @brief Parser input structure specifying where the data are read.
 */
struct ly_in;

/**
 * @brief Types of the parser's inputs
 */
typedef enum LY_IN_TYPE {
    LY_IN_ERROR = -1,  /**< error value to indicate failure of the functions returning LY_IN_TYPE */
    LY_IN_FD,          /**< file descriptor printer */
    LY_IN_FILE,        /**< FILE stream parser */
    LY_IN_FILEPATH,    /**< filepath parser */
    LY_IN_MEMORY       /**< memory parser */
} LY_IN_TYPE;

/**
 * @brief Get input type of the input handler.
 *
 * @param[in] in Input handler.
 * @return Type of the parser's input.
 */
LIBYANG_API_DECL LY_IN_TYPE ly_in_type(const struct ly_in *in);

/**
 * @brief Reset the input medium to read from its beginning, so the following parser function will read from the object's beginning.
 *
 * Note that in case the underlying output is not seekable (stream referring a pipe/FIFO/socket or the callback output type),
 * nothing actually happens despite the function succeeds. Also note that the medium is not returned to the state it was when
 * the handler was created. For example, file is seeked into the offset zero, not to the offset where it was opened when
 * ::ly_in_new_file() was called.
 *
 * @param[in] in Input handler.
 * @return LY_SUCCESS in case of success
 * @return LY_ESYS in case of failure
 */
LIBYANG_API_DECL LY_ERR ly_in_reset(struct ly_in *in);

/**
 * @brief Create input handler using file descriptor.
 *
 * @param[in] fd File descriptor to use.
 * @param[out] in Created input handler supposed to be passed to different ly*_parse() functions.
 * @return LY_SUCCESS in case of success
 * @return LY_ERR value in case of failure.
 */
LIBYANG_API_DECL LY_ERR ly_in_new_fd(int fd, struct ly_in **in);

/**
 * @brief Get or reset file descriptor input handler.
 *
 * @param[in] in Input handler.
 * @param[in] fd Optional value of a new file descriptor for the handler. If -1, only the current file descriptor value is returned.
 * @return Previous value of the file descriptor. Note that caller is responsible for closing the returned file descriptor in case of setting new descriptor @p fd.
 * @return -1 in case of error when setting up the new file descriptor.
 */
LIBYANG_API_DECL int ly_in_fd(struct ly_in *in, int fd);

/**
 * @brief Create input handler using file stream.
 *
 * @param[in] f File stream to use.
 * @param[out] in Created input handler supposed to be passed to different ly*_parse() functions.
 * @return LY_SUCCESS in case of success
 * @return LY_ERR value in case of failure.
 */
LIBYANG_API_DECL LY_ERR ly_in_new_file(FILE *f, struct ly_in **in);

/**
 * @brief Get or reset file stream input handler.
 *
 * @param[in] in Input handler.
 * @param[in] f Optional new file stream for the handler. If NULL, only the current file stream is returned.
 * @return NULL in case of invalid argument or an error when setting up the new input file, original input handler @p in is untouched in this case.
 * @return Previous file stream of the handler. Note that caller is responsible for closing the returned stream in case of setting new stream @p f.
 */
LIBYANG_API_DECL FILE *ly_in_file(struct ly_in *in, FILE *f);

/**
 * @brief Create input handler using memory to read data.
 *
 * @param[in] str Pointer where to start reading data. The input data are expected to be NULL-terminated.
 * Note that in case the destroy argument of ::ly_in_free() is used, the input string is passed to free(),
 * so if it is really a static string, do not use the destroy argument!
 * @param[out] in Created input handler supposed to be passed to different ly*_parse() functions.
 * @return LY_SUCCESS in case of success
 * @return LY_ERR value in case of failure.
 */
LIBYANG_API_DECL LY_ERR ly_in_new_memory(const char *str, struct ly_in **in);

/**
 * @brief Get or change memory where the data are read from.
 *
 * @param[in] in Input handler.
 * @param[in] str String containing the data to read. The input data are expected to be NULL-terminated.
 * Note that in case the destroy argument of ::ly_in_free() is used, the input string is passed to free(),
 * so if it is really a static string, do not use the destroy argument!
 * @return Previous starting address to read data from. Note that the caller is responsible to free
 * the data in case of changing string pointer @p str.
 */
LIBYANG_API_DECL const char *ly_in_memory(struct ly_in *in, const char *str);

/**
 * @brief Create input handler file of the given filename.
 *
 * @param[in] filepath Path of the file where to read data.
 * @param[in] len Optional number of bytes to use from @p filepath. If 0, the @p filepath is considered to be NULL-terminated and
 * the whole string is taken into account.
 * @param[out] in Created input handler supposed to be passed to different ly*_parse() functions.
 * @return LY_SUCCESS in case of success
 * @return LY_ERR value in case of failure.
 */
LIBYANG_API_DECL LY_ERR ly_in_new_filepath(const char *filepath, size_t len, struct ly_in **in);

/**
 * @brief Get or change the filepath of the file where the parser reads the data.
 *
 * Note that in case of changing the filepath, the current file is closed and a new one is
 * created/opened instead of renaming the previous file. Also note that the previous filepath
 * string is returned only in case of not changing it's value.
 *
 * @param[in] in Input handler.
 * @param[in] filepath Optional new filepath for the handler. If and only if NULL, the current filepath string is returned.
 * @param[in] len Optional number of bytes to use from @p filepath. If 0, the @p filepath is considered to be NULL-terminated and
 * the whole string is taken into account.
 * @return Previous filepath string in case the @p filepath argument is NULL.
 * @return NULL if changing filepath succeedes and ((void *)-1) otherwise.
 */
LIBYANG_API_DECL const char *ly_in_filepath(struct ly_in *in, const char *filepath, size_t len);

/**
 * @brief Get the number of parsed bytes by the last function.
 *
 * @param[in] in In structure used.
 * @return Number of parsed bytes.
 */
LIBYANG_API_DECL size_t ly_in_parsed(const struct ly_in *in);

/**
 * @brief Free the input handler.
 *
 * @param[in] in Input handler to free.
 * @param[in] destroy Flag to free the input data buffer (for LY_IN_MEMORY) or to
 * close stream/file descriptor (for LY_IN_FD and LY_IN_FILE)
 */
LIBYANG_API_DECL void ly_in_free(struct ly_in *in, ly_bool destroy);

/**
 * @brief Read bytes from an input.
 *
 * Does not count new lines, which is expected from the caller who has better idea about
 * the content of the read data and can better optimize counting.
 *
 * @param[in] in Input structure.
 * @param[in] buf Destination buffer.
 * @param[in] count Number of bytes to read.
 * @return LY_SUCCESS on success,
 * @return LY_EDENIED on EOF.
 */
LIBYANG_API_DECL LY_ERR ly_in_read(struct ly_in *in, void *buf, size_t count);

/**
 * @brief Just skip bytes in an input.
 *
 * Does not count new lines, which is expected from the caller who has better idea about
 * the content of the skipped data and can better optimize counting.
 *
 * @param[in] in Input structure.
 * @param[in] count Number of bytes to skip.
 * @return LY_SUCCESS on success,
 * @return LY_EDENIED on EOF.
 */
LIBYANG_API_DECL LY_ERR ly_in_skip(struct ly_in *in, size_t count);

#ifdef __cplusplus
}
#endif

#endif /* LY_IN_H_ */
