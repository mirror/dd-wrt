#ifndef SG_PT_H
#define SG_PT_H

/*
 * Copyright (c) 2005-2020 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* This declaration hides the fact that each implementation has its own
 * structure "derived" (using a C++ term) from this one. It compiles
 * because 'struct sg_pt_base' is only referenced (by pointer: 'objp')
 * in this interface. An instance of this structure represents the
 * context of one SCSI command.
 * If an instance of sg_pt_base is shared across several threads then
 * it is up to the application to take care of multi-threaded issues
 * with that instance. */
struct sg_pt_base;


/* The format of the version string is like this: "3.04 20180213".
 * The leading digit will be incremented if this interface changes
 * in a way that may impact backward compatibility. */
const char * scsi_pt_version();
const char * sg_pt_version();   /* both functions give same result */


/* Returns file descriptor or file handle and is >= 0 if successful.
 * If error in Unix returns negated errno. */
int scsi_pt_open_device(const char * device_name, bool read_only,
                        int verbose);

/* Similar to scsi_pt_open_device() but takes Unix style open flags OR-ed
 * together. Returns valid file descriptor or handle ( >= 0 ) if successful,
 * otherwise returns -1 or a negated errno.
 * In Win32 O_EXCL translated to equivalent. */
int scsi_pt_open_flags(const char * device_name, int flags, int verbose);

/* Returns 0 if successful. 'device_fd' should be a value that was previously
 * returned by scsi_pt_open_device() or scsi_pt_open_flags() that has not
 * already been closed. If error in Unix returns negated errno. */
int scsi_pt_close_device(int device_fd);

/* Assumes dev_fd is an "open" file handle associated with device_name. If
 * the implementation (possibly for one OS) cannot determine from dev_fd if
 * a SCSI or NVMe pass-through is referenced, then it might guess based on
 * device_name. Returns 1 if SCSI generic pass-though device, returns 2 if
 * secondary SCSI pass-through device (in Linux a bsg device); returns 3 is
 * char NVMe device (i.e. no NSID); returns 4 if block NVMe device (includes
 * NSID), or 0 if something else (e.g. ATA block device) or dev_fd < 0.
 * If error, returns negated errno (operating system) value. */
int check_pt_file_handle(int dev_fd, const char * device_name, int verbose);


/* Creates an object that can be used to issue one or more SCSI commands
 * (or task management functions). Returns NULL if problem.
 * Once this object has been created it should be destroyed with
 * destruct_scsi_pt_obj() when it is no longer needed. */
struct sg_pt_base * construct_scsi_pt_obj(void);

/* An alternate and preferred way to create an object that can be used to
 * issue one or more SCSI (or NVMe) commands (or task management functions).
 * This variant associates a device file descriptor (handle) with the object
 * and a verbose argument that causes messages to be written to stderr if
 * errors occur. The reason for this is to optionally allow the detection of
 * NVMe devices that will cause pt_device_is_nvme() to return true. Set
 * dev_fd to -1 if no open device file descriptor is available. Caller
 * should additionally call get_scsi_pt_os_err() after this call to check
 * for errors. The dev_fd argument may be -1 to indicate no device file
 * descriptor. */
struct sg_pt_base *
        construct_scsi_pt_obj_with_fd(int dev_fd, int verbose);

/* Forget any previous dev_fd and install the one given. May attempt to
 * find file type (e.g. if pass-though) from OS so there could be an error.
 * Returns 0 for success or the same value as get_scsi_pt_os_err()
 * will return. dev_fd should be >= 0 for a valid file handle or -1 . */
int set_pt_file_handle(struct sg_pt_base * objp, int dev_fd, int verbose);

/* Valid file handles (which is the return value) are >= 0 . Returns -1
 * if there is no valid file handle. */
int get_pt_file_handle(const struct sg_pt_base * objp);

/* Clear state information held in *objp . This allows this object to be
 * used to issue more than one SCSI command. The dev_fd is remembered.
 * Use set_pt_file_handle() to change dev_fd. */
void clear_scsi_pt_obj(struct sg_pt_base * objp);

/* Partially clear state information held in *objp . Any error settings and
 * the data-in and data-out settings are cleared. So dev_fd, cdb and sense
 * settings are kept. */
void partial_clear_scsi_pt_obj(struct sg_pt_base * objp);

/* Set the CDB (command descriptor block). May also be a NVMe Admin command
 * which will be 64 bytes long.
 *
 * Note that the sg_cmds_is_nvme() function found in sg_cmds_basic.h can be
 * called after this function to "guess" which command set the given command
 * belongs to. It is valid to supply a cdb value of NULL. */
void set_scsi_pt_cdb(struct sg_pt_base * objp, const uint8_t * cdb,
                     int cdb_len);

/* Set the sense buffer and the maximum length of that buffer. For NVMe
 * commands this "sense" buffer will receive the 4 DWORDs of from the
 * completion queue. It is valid to supply a sense value of NULL. */
void set_scsi_pt_sense(struct sg_pt_base * objp, uint8_t * sense,
                       int max_sense_len);

/* Set a pointer and length to be used for data transferred from device */
void set_scsi_pt_data_in(struct sg_pt_base * objp,   /* from device */
                         uint8_t * dxferp, int dxfer_ilen);

/* Set a pointer and length to be used for data transferred to device */
void set_scsi_pt_data_out(struct sg_pt_base * objp,    /* to device */
                          const uint8_t * dxferp, int dxfer_olen);

/* Set a pointer and length to be used for metadata transferred to
 * (out_true=true) or from (out_true=false) device (NVMe only) */
void set_pt_metadata_xfer(struct sg_pt_base * objp, uint8_t * mdxferp,
                          uint32_t mdxfer_len, bool out_true);

/* The following "set_"s implementations may be dummies */
void set_scsi_pt_packet_id(struct sg_pt_base * objp, int pack_id);
void set_scsi_pt_tag(struct sg_pt_base * objp, uint64_t tag);
void set_scsi_pt_task_management(struct sg_pt_base * objp, int tmf_code);
void set_scsi_pt_task_attr(struct sg_pt_base * objp, int attribute,
                           int priority);

/* Following is a guard which is defined when set_scsi_pt_flags() is
 * present. Older versions of this library may not have this function. */
#define SCSI_PT_FLAGS_FUNCTION 1
/* If neither QUEUE_AT_HEAD nor QUEUE_AT_TAIL are given, or both
 * are given, use the pass-through default. */
#define SCSI_PT_FLAGS_QUEUE_AT_TAIL 0x10
#define SCSI_PT_FLAGS_QUEUE_AT_HEAD 0x20
/* Set (potentially OS dependent) flags for pass-through mechanism.
 * Apart from contradictions, flags can be OR-ed together. */
void set_scsi_pt_flags(struct sg_pt_base * objp, int flags);

#define SCSI_PT_DO_START_OK 0
#define SCSI_PT_DO_BAD_PARAMS 1
#define SCSI_PT_DO_TIMEOUT 2
#define SCSI_PT_DO_NOT_SUPPORTED 4
#define SCSI_PT_DO_NVME_STATUS 48       /* == SG_LIB_NVME_STATUS */
/* If OS error prior to or during command submission then returns negated
 * error value (e.g. Unix '-errno'). This includes interrupted system calls
 * (e.g. by a signal) in which case -EINTR would be returned. Note that
 * system call errors also can be fetched with get_scsi_pt_os_err().
 * Return 0 if okay (i.e. at the very least: command sent). Positive
 * return values are errors (see SCSI_PT_DO_* defines). If a file descriptor
 * has already been provided by construct_scsi_pt_obj_with_fd() then the
 * given 'fd' can be -1 or the same value as given to the constructor. */
int do_scsi_pt(struct sg_pt_base * objp, int fd, int timeout_secs,
               int verbose);

/* NVMe Admin commands can be sent directly to do_scsi_pt(). Unfortunately
 * NVMe has at least one other command set: "NVM" to access user data and
 * the opcodes in the NVM command set overlap with the Admin command set.
 * So NVMe Admin commands should be sent do_scsi_pt() while NVMe "NVM"
 * commands should be sent to this function. No SCSI commands should be
 * sent to this function. Currently submq is not implemented and all
 * submitted NVM commands are sent on queue 0, the same queue use for
 * Admin commands. The return values follow the same pattern as do_scsi_pt(),
 * with 0 returned being good.  The NVMe device file descriptor must either
 * be given to the obj constructor, or a prior set_pt_file_handle() call. */
int do_nvm_pt(struct sg_pt_base * objp, int submq, int timeout_secs,
              int verbose);

#define SCSI_PT_RESULT_GOOD 0
#define SCSI_PT_RESULT_STATUS 1 /* other than GOOD and CHECK CONDITION */
#define SCSI_PT_RESULT_SENSE 2
#define SCSI_PT_RESULT_TRANSPORT_ERR 3
#define SCSI_PT_RESULT_OS_ERR 4
/* This function, called soon after do_scsi_pt(), returns one of the above
 * result categories. The highest numbered applicable category is returned.
 *
 * Note that the sg_cmds_process_resp() function found in sg_cmds_basic.h
 * is useful for processing SCSI command responses.
 * And the sg_cmds_is_nvme() function found in sg_cmds_basic.h can be called
 * after set_scsi_pt_cdb() to "guess" which command set the given command
 * belongs to. */
int get_scsi_pt_result_category(const struct sg_pt_base * objp);

/* If not available return 0 which implies there is no residual value. If
 * supported it is the number of bytes requested to transfer less the
 * number actually transferred. This it typically important for data-in
 * transfers. For data-out (only) transfers, the 'dout_req_len -
 * dout_act_len' is returned. For bidi transfer the data-in residual is
 * returned. */
int get_scsi_pt_resid(const struct sg_pt_base * objp);

/* Returns SCSI status value (from device that received the command). If an
 * NVMe command was issued directly (i.e. through do_scsi_pt() then return
 * NVMe status (i.e. ((SCT << 8) | SC)). If problem returns -1. */
int get_scsi_pt_status_response(const struct sg_pt_base * objp);

/* Returns SCSI status value or, if NVMe command given to do_scsi_pt(),
 * then returns NVMe result (i.e. DWord(0) from completion queue). If
 * 'objp' is NULL then returns 0xffffffff. */
uint32_t get_pt_result(const struct sg_pt_base * objp);

/* These two get functions should just echo what has been given to
 * set_scsi_pt_cdb(). If it has not been called or clear_scsi_pt_obj()
 * has been called then return 0 and NULL respectively. */
int get_scsi_pt_cdb_len(const struct sg_pt_base * objp);
uint8_t * get_scsi_pt_cdb_buf(const struct sg_pt_base * objp);

/* Actual sense length returned. If sense data is present but
   actual sense length is not known, return 'max_sense_len' */
int get_scsi_pt_sense_len(const struct sg_pt_base * objp);
uint8_t * get_scsi_pt_sense_buf(const struct sg_pt_base * objp);

/* If not available return 0 (for success). */
int get_scsi_pt_os_err(const struct sg_pt_base * objp);
char * get_scsi_pt_os_err_str(const struct sg_pt_base * objp, int max_b_len,
                              char * b);

/* If not available return 0 (for success) */
int get_scsi_pt_transport_err(const struct sg_pt_base * objp);
void set_scsi_pt_transport_err(struct sg_pt_base * objp, int err);
char * get_scsi_pt_transport_err_str(const struct sg_pt_base * objp,
                                     int max_b_len, char * b);

/* If not available return -1 otherwise return number of milliseconds
 * that the lower layers (and hardware) took to execute the previous
 * command. */
int get_scsi_pt_duration_ms(const struct sg_pt_base * objp);

/* If not available return 0 otherwise return number of nanoseconds that the
 * lower layers (and hardware) took to execute the command just completed. */
uint64_t get_pt_duration_ns(const struct sg_pt_base * objp);

/* The two functions yield requested and actual data transfer lengths in
 * bytes. The second argument is a pointer to the data-in length; the third
 * argument is a pointer to the data-out length. The pointers may be NULL.
 * The _actual_ values are related to resid (residual count from DMA) */
void get_pt_req_lengths(const struct sg_pt_base * objp, int * req_dinp,
                        int * req_doutp);
void get_pt_actual_lengths(const struct sg_pt_base * objp, int * act_dinp,
                           int * act_doutp);

/* Return true if device associated with 'objp' uses NVMe command set. To
 * be useful (in modifying the type of command sent (SCSI or NVMe) then
 * construct_scsi_pt_obj_with_fd() should be used followed by an invocation
 * of this function. */
bool pt_device_is_nvme(const struct sg_pt_base * objp);

/* If a NVMe block device (which includes the NSID) handle is associated
 * with 'objp', then its NSID is returned (values range from 0x1 to
 * 0xffffffe). Otherwise 0 is returned. */
uint32_t get_pt_nvme_nsid(const struct sg_pt_base * objp);


/* Should be invoked once per objp after other processing is complete in
 * order to clean up resources. For ever successful construct_scsi_pt_obj()
 * call there should be one destruct_scsi_pt_obj(). If the
 * construct_scsi_pt_obj_with_fd() function was used to create this object
 * then the dev_fd provided to that constructor is not altered by this
 * destructor. So the user should still close dev_fd (perhaps with
 * scsi_pt_close_device() ).  */
void destruct_scsi_pt_obj(struct sg_pt_base * objp);

#ifdef SG_LIB_WIN32
#define SG_LIB_WIN32_DIRECT 1

/* Request SPT direct interface when state_direct is 1, state_direct set
 * to 0 for the SPT indirect interface. Default setting selected by build
 * (i.e. library compile time) and is usually indirect. */
void scsi_pt_win32_direct(int state_direct);

/* Returns current SPT interface state, 1 for direct, 0 for indirect */
int scsi_pt_win32_spt_state(void);

#endif

#ifdef __cplusplus
}
#endif

#endif          /* SG_PT_H */
