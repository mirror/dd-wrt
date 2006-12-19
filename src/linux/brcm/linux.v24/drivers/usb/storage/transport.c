/* Driver for USB Mass Storage compliant devices
 *
 * $Id: transport.c,v 1.44 2002/02/25 00:43:41 mdharm Exp $
 *
 * Current development and maintenance by:
 *   (c) 1999-2002 Matthew Dharm (mdharm-usb@one-eyed-alien.net)
 *
 * Developed with the assistance of:
 *   (c) 2000 David L. Brown, Jr. (usb-storage@davidb.org)
 *   (c) 2000 Stephen J. Gowdy (SGowdy@lbl.gov)
 *   (c) 2002 Alan Stern <stern@rowland.org>
 *
 * Initial work by:
 *   (c) 1999 Michael Gee (michael@linuxspecific.com)
 *
 * This driver is based on the 'USB Mass Storage Class' document. This
 * describes in detail the protocol used to communicate with such
 * devices.  Clearly, the designers had SCSI and ATAPI commands in
 * mind when they created this document.  The commands are all very
 * similar to commands in the SCSI-II and ATAPI specifications.
 *
 * It is important to note that in a number of cases this class
 * exhibits class-specific exemptions from the USB specification.
 * Notably the usage of NAK, STALL and ACK differs from the norm, in
 * that they are used to communicate wait, failed and OK on commands.
 *
 * Also, for certain devices, the interrupt endpoint is used to convey
 * status of a command.
 *
 * Please see http://www.one-eyed-alien.net/~mdharm/linux-usb for more
 * information about this driver.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/config.h>
#include "transport.h"
#include "protocol.h"
#include "usb.h"
#include "debug.h"

#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/pci.h>
#include "../hcd.h"
#include "../../net/ctmisc/ext_io.h"

/* These definitions mirror those in pci.h, so they can be used
 * interchangeably with their PCI_ counterparts */
enum dma_data_direction {
	DMA_BIDIRECTIONAL = 0,
	DMA_TO_DEVICE = 1,
	DMA_FROM_DEVICE = 2,
	DMA_NONE = 3,
};

#define dma_map_sg(d,s,n,dir) pci_map_sg(d,s,n,dir)
#define dma_unmap_sg(d,s,n,dir) pci_unmap_sg(d,s,n,dir)



/***********************************************************************
 * Helper routines
 ***********************************************************************/

/* Calculate the length of the data transfer (not the command) for any
 * given SCSI command
 */
unsigned int usb_stor_transfer_length(Scsi_Cmnd *srb)
{
      	int i;
	int doDefault = 0;
	unsigned int len = 0;
	unsigned int total = 0;
	struct scatterlist *sg;

	/* This table tells us:
	   X = command not supported
	   L = return length in cmnd[4] (8 bits).
	   M = return length in cmnd[8] (8 bits).
	   G = return length in cmnd[3] and cmnd[4] (16 bits)
	   H = return length in cmnd[7] and cmnd[8] (16 bits)
	   I = return length in cmnd[8] and cmnd[9] (16 bits)
	   C = return length in cmnd[2] to cmnd[5] (32 bits)
	   D = return length in cmnd[6] to cmnd[9] (32 bits)
	   B = return length in blocksize so we use buff_len
	   R = return length in cmnd[2] to cmnd[4] (24 bits)
	   S = return length in cmnd[3] to cmnd[5] (24 bits)
	   T = return length in cmnd[6] to cmnd[8] (24 bits)
	   U = return length in cmnd[7] to cmnd[9] (24 bits)
	   0-9 = fixed return length
	   V = 20 bytes
	   W = 24 bytes
	   Z = return length is mode dependant or not in command, use buff_len
	*/

	static char *lengths =

	      /* 0123456789ABCDEF   0123456789ABCDEF */

		"00XLZ6XZBXBBXXXB" "00LBBLG0R0L0GG0X"  /* 00-1F */
		"XXXXT8XXB4B0BBBB" "ZZZ0B00HCSSZTBHH"  /* 20-3F */
		"M0HHB0X000H0HH0X" "XHH0HHXX0TH0H0XX"  /* 40-5F */
		"XXXXXXXXXXXXXXXX" "XXXXXXXXXXXXXXXX"  /* 60-7F */
		"XXXXXXXXXXXXXXXX" "XXXXXXXXXXXXXXXX"  /* 80-9F */
		"X0XXX00XB0BXBXBB" "ZZZ0XUIDU000XHBX"  /* A0-BF */
		"XXXXXXXXXXXXXXXX" "XXXXXXXXXXXXXXXX"  /* C0-DF */
		"XDXXXXXXXXXXXXXX" "XXW00HXXXXXXXXXX"; /* E0-FF */

	/* Commands checked in table:

	   CHANGE_DEFINITION 40
	   COMPARE 39
	   COPY 18
	   COPY_AND_VERIFY 3a
	   ERASE 19
	   ERASE_10 2c
	   ERASE_12 ac
	   EXCHANGE_MEDIUM a6
	   FORMAT_UNIT 04
	   GET_DATA_BUFFER_STATUS 34
	   GET_MESSAGE_10 28
	   GET_MESSAGE_12 a8
	   GET_WINDOW 25   !!! Has more data than READ_CAPACITY, need to fix table
	   INITIALIZE_ELEMENT_STATUS 07 !!! REASSIGN_BLOCKS luckily uses buff_len
	   INQUIRY 12
	   LOAD_UNLOAD 1b
	   LOCATE 2b
	   LOCK_UNLOCK_CACHE 36
	   LOG_SELECT 4c
	   LOG_SENSE 4d
	   MEDIUM_SCAN 38     !!! This was M
	   MODE_SELECT6 15
	   MODE_SELECT_10 55
	   MODE_SENSE_6 1a
	   MODE_SENSE_10 5a
	   MOVE_MEDIUM a5
	   OBJECT_POSITION 31  !!! Same as SEARCH_DATA_EQUAL
	   PAUSE_RESUME 4b
	   PLAY_AUDIO_10 45
	   PLAY_AUDIO_12 a5
	   PLAY_AUDIO_MSF 47
	   PLAY_AUDIO_TRACK_INDEX 48
   	   PLAY_AUDIO_TRACK_RELATIVE_10 49
	   PLAY_AUDIO_TRACK_RELATIVE_12 a9
	   POSITION_TO_ELEMENT 2b
      	   PRE-FETCH 34
	   PREVENT_ALLOW_MEDIUM_REMOVAL 1e
	   PRINT 0a             !!! Same as WRITE_6 but is always in bytes
	   READ_6 08
	   READ_10 28
	   READ_12 a8
	   READ_BLOCK_LIMITS 05
	   READ_BUFFER 3c
	   READ_CAPACITY 25
	   READ_CDROM_CAPACITY 25
	   READ_DEFECT_DATA 37
	   READ_DEFECT_DATA_12 b7
	   READ_ELEMENT_STATUS b8 !!! Think this is in bytes
	   READ_GENERATION 29 !!! Could also be M?
	   READ_HEADER 44     !!! This was L
	   READ_LONG 3e
	   READ_POSITION 34   !!! This should be V but conflicts with PRE-FETCH
	   READ_REVERSE 0f
	   READ_SUB-CHANNEL 42 !!! Is this in bytes?
	   READ_TOC 43         !!! Is this in bytes?
	   READ_UPDATED_BLOCK 2d
	   REASSIGN_BLOCKS 07
	   RECEIVE 08        !!! Same as READ_6 probably in bytes though
	   RECEIVE_DIAGNOSTIC_RESULTS 1c
	   RECOVER_BUFFERED_DATA 14 !!! For PRINTERs this is bytes
	   RELEASE_UNIT 17
	   REQUEST_SENSE 03
	   REQUEST_VOLUME_ELEMENT_ADDRESS b5 !!! Think this is in bytes
	   RESERVE_UNIT 16
	   REWIND 01
	   REZERO_UNIT 01
	   SCAN 1b          !!! Conflicts with various commands, should be L
	   SEARCH_DATA_EQUAL 31
	   SEARCH_DATA_EQUAL_12 b1
	   SEARCH_DATA_LOW 30
	   SEARCH_DATA_LOW_12 b0
	   SEARCH_DATA_HIGH 32
	   SEARCH_DATA_HIGH_12 b2
	   SEEK_6 0b         !!! Conflicts with SLEW_AND_PRINT
	   SEEK_10 2b
	   SEND 0a           !!! Same as WRITE_6, probably in bytes though
	   SEND 2a           !!! Similar to WRITE_10 but for scanners
	   SEND_DIAGNOSTIC 1d
	   SEND_MESSAGE_6 0a   !!! Same as WRITE_6 - is in bytes
	   SEND_MESSAGE_10 2a  !!! Same as WRITE_10 - is in bytes
	   SEND_MESSAGE_12 aa  !!! Same as WRITE_12 - is in bytes
	   SEND_OPC 54
	   SEND_VOLUME_TAG b6 !!! Think this is in bytes
	   SET_LIMITS 33
	   SET_LIMITS_12 b3
	   SET_WINDOW 24
	   SLEW_AND_PRINT 0b !!! Conflicts with SEEK_6
	   SPACE 11
	   START_STOP_UNIT 1b
	   STOP_PRINT 1b
	   SYNCHRONIZE_BUFFER 10
	   SYNCHRONIZE_CACHE 35
	   TEST_UNIT_READY 00
	   UPDATE_BLOCK 3d
	   VERIFY 13
	   VERIFY 2f
	   VERIFY_12 af
	   WRITE_6 0a
	   WRITE_10 2a
	   WRITE_12 aa
	   WRITE_AND_VERIFY 2e
	   WRITE_AND_VERIFY_12 ae
	   WRITE_BUFFER 3b
	   WRITE_FILEMARKS 10
	   WRITE_LONG 3f
	   WRITE_SAME 41
	*/

	if (srb->sc_data_direction == SCSI_DATA_WRITE) {
		doDefault = 1;
	}
	else
		switch (lengths[srb->cmnd[0]]) {
			case 'L':
				len = srb->cmnd[4];
				break;

			case 'M':
				len = srb->cmnd[8];
				break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				len = lengths[srb->cmnd[0]]-'0';
				break;

			case 'G':
				len = (((unsigned int)srb->cmnd[3])<<8) |
					srb->cmnd[4];
				break;

			case 'H':
				len = (((unsigned int)srb->cmnd[7])<<8) |
					srb->cmnd[8];
				break;

			case 'I':
				len = (((unsigned int)srb->cmnd[8])<<8) |
					srb->cmnd[9];
				break;

			case 'R':
				len = (((unsigned int)srb->cmnd[2])<<16) |
					(((unsigned int)srb->cmnd[3])<<8) |
					srb->cmnd[4];
				break;

			case 'S':
				len = (((unsigned int)srb->cmnd[3])<<16) |
					(((unsigned int)srb->cmnd[4])<<8) |
					srb->cmnd[5];
				break;

			case 'T':
				len = (((unsigned int)srb->cmnd[6])<<16) |
					(((unsigned int)srb->cmnd[7])<<8) |
					srb->cmnd[8];
				break;

			case 'U':
				len = (((unsigned int)srb->cmnd[7])<<16) |
					(((unsigned int)srb->cmnd[8])<<8) |
					srb->cmnd[9];
				break;

			case 'C':
				len = (((unsigned int)srb->cmnd[2])<<24) |
					(((unsigned int)srb->cmnd[3])<<16) |
					(((unsigned int)srb->cmnd[4])<<8) |
					srb->cmnd[5];
				break;

			case 'D':
				len = (((unsigned int)srb->cmnd[6])<<24) |
					(((unsigned int)srb->cmnd[7])<<16) |
					(((unsigned int)srb->cmnd[8])<<8) |
					srb->cmnd[9];
				break;

			case 'V':
				len = 20;
				break;

			case 'W':
				len = 24;
				break;

			case 'B':
				/* Use buffer size due to different block sizes */
				doDefault = 1;
				break;

			case 'X':
				US_DEBUGP("Error: UNSUPPORTED COMMAND %02X\n",
						srb->cmnd[0]);
				doDefault = 1;
				break;

			case 'Z':
				/* Use buffer size due to mode dependence */
				doDefault = 1;
				break;

			default:
				US_DEBUGP("Error: COMMAND %02X out of range or table inconsistent (%c).\n",
						srb->cmnd[0], lengths[srb->cmnd[0]] );
				doDefault = 1;
		}
	   
	   if ( doDefault == 1 ) {
		   /* Are we going to scatter gather? */
		   if (srb->use_sg) {
			   /* Add up the sizes of all the sg segments */
			   sg = (struct scatterlist *) srb->request_buffer;
			   for (i = 0; i < srb->use_sg; i++)
				   total += sg[i].length;
			   len = total;
		   }
		   else
			   /* Just return the length of the buffer */
			   len = srb->request_bufflen;
	   }

	return len;
}

/***********************************************************************
 * Data transfer routines
 ***********************************************************************/

/* This is the completion handler which will wake us up when an URB
 * completes.
 */
static void usb_stor_blocking_completion(struct urb *urb)
{
	struct completion *urb_done_ptr = (struct completion *)urb->context;

	complete(urb_done_ptr);
}

/* This is our function to emulate usb_control_msg() but give us enough
 * access to make aborts/resets work
 */
int usb_stor_control_msg(struct us_data *us, unsigned int pipe,
			 u8 request, u8 requesttype, u16 value, u16 index, 
			 void *data, u16 size)
{
	int status;
	struct usb_ctrlrequest *dr;

	/* allocate the device request structure */
	dr = kmalloc(sizeof(struct usb_ctrlrequest), GFP_NOIO);
	if (!dr)
		return -ENOMEM;

	/* fill in the structure */
	dr->bRequestType = requesttype;
	dr->bRequest = request;
	dr->wValue = cpu_to_le16(value);
	dr->wIndex = cpu_to_le16(index);
	dr->wLength = cpu_to_le16(size);

	/* set up data structures for the wakeup system */
	init_completion(&us->current_done);

	/* lock the URB */
	down(&(us->current_urb_sem));

	/* fill the URB */
	FILL_CONTROL_URB(us->current_urb, us->pusb_dev, pipe, 
			 (unsigned char*) dr, data, size, 
			 usb_stor_blocking_completion, &us->current_done);
	us->current_urb->actual_length = 0;
	us->current_urb->error_count = 0;
	us->current_urb->transfer_flags = USB_ASYNC_UNLINK;
	us->current_urb->status = 0;

	/* submit the URB */
	status = usb_submit_urb(us->current_urb);
	if (status) {
		/* something went wrong */
		up(&(us->current_urb_sem));
		kfree(dr);
		return status;
	}

	/* wait for the completion of the URB */
	up(&(us->current_urb_sem));
	wait_for_completion(&us->current_done);
	down(&(us->current_urb_sem));

	/* return the actual length of the data transferred if no error*/
	status = us->current_urb->status;
	if (status == -ENOENT)
		status = -ECONNRESET;
	if (status >= 0)
		status = us->current_urb->actual_length;

	/* release the lock and return status */
	up(&(us->current_urb_sem));
	kfree(dr);
  	return status;
}
extern int usb_led_flag;
extern wait_queue_head_t usb_led_queue;

/* This is our function to emulate usb_bulk_msg() but give us enough
 * access to make aborts/resets work
 */
int usb_stor_bulk_msg(struct us_data *us, void *data, int pipe,
		      unsigned int len, unsigned int *act_len)
{
	int status;

	/* set up data structures for the wakeup system */
	init_completion(&us->current_done);

	/* lock the URB */
	down(&(us->current_urb_sem));

	/* fill the URB */
	FILL_BULK_URB(us->current_urb, us->pusb_dev, pipe, data, len,
		      usb_stor_blocking_completion, &us->current_done);
	us->current_urb->actual_length = 0;
	us->current_urb->error_count = 0;
	us->current_urb->transfer_flags = USB_ASYNC_UNLINK;
	us->current_urb->status = 0;

	/* submit the URB */
	status = usb_submit_urb(us->current_urb);
	if (status) {
		/* something went wrong */
		up(&(us->current_urb_sem));
		return status;
	}

	/* wait for the completion of the URB */
	up(&(us->current_urb_sem));
	wait_for_completion(&us->current_done);
	down(&(us->current_urb_sem));
	if (us->current_urb->status == -ENOENT)
		us->current_urb->status = -ECONNRESET;

	/* return the actual length of the data transferred */
	*act_len = us->current_urb->actual_length;

	/* release the lock and return status */
	up(&(us->current_urb_sem));
	if (usb_led_flag == 1)
		wake_up_interruptible_sync(&usb_led_queue);
	return us->current_urb->status;
}

/* This is a version of usb_clear_halt() that doesn't read the status from
 * the device -- this is because some devices crash their internal firmware
 * when the status is requested after a halt
 */
int usb_stor_clear_halt(struct us_data *us, int pipe)
{
	int result;
	int endp = usb_pipeendpoint(pipe) | (usb_pipein(pipe) << 7);

	result = usb_stor_control_msg(us,
		usb_sndctrlpipe(us->pusb_dev, 0),
		USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
		endp, NULL, 0);		/* note: no 3*HZ timeout */
	US_DEBUGP("usb_stor_clear_halt: result=%d\n", result);

	/* this is a failure case */
	if (result < 0)
		return result;

	/* reset the toggles and endpoint flags */
	usb_endpoint_running(us->pusb_dev, usb_pipeendpoint(pipe),
		usb_pipeout(pipe));
	usb_settoggle(us->pusb_dev, usb_pipeendpoint(pipe),
		usb_pipeout(pipe), 0);

	return 0;
}

/*
 * Transfer one SCSI scatter-gather buffer via bulk transfer
 *
 * Note that this function is necessary because we want the ability to
 * use scatter-gather memory.  Good performance is achieved by a combination
 * of scatter-gather and clustering (which makes each chunk bigger).
 *
 * Note that the lower layer will always retry when a NAK occurs, up to the
 * timeout limit.  Thus we don't have to worry about it for individual
 * packets.
 */
int usb_stor_transfer_partial(struct us_data *us, char *buf, int length)
{
	int result;
	int partial;
	int pipe;

	/* calculate the appropriate pipe information */
	if (us->srb->sc_data_direction == SCSI_DATA_READ)
		pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
	else
		pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);

	/* transfer the data */
	US_DEBUGP("usb_stor_transfer_partial(): xfer %d bytes\n", length);
	result = usb_stor_bulk_msg(us, buf, pipe, length, &partial);
	US_DEBUGP("usb_stor_bulk_msg() returned %d xferred %d/%d\n",
		  result, partial, length);

	/* if we stall, we need to clear it before we go on */
	if (result == -EPIPE) {
		US_DEBUGP("clearing endpoint halt for pipe 0x%x\n", pipe);
		usb_stor_clear_halt(us, pipe);
	}

	/* did we abort this command? */
	if (result == -ECONNRESET) {
		US_DEBUGP("usb_stor_transfer_partial(): transfer aborted\n");
		return US_BULK_TRANSFER_ABORTED;
	}

	/* did we send all the data? */
	if (partial == length) {
		US_DEBUGP("usb_stor_transfer_partial(): transfer complete\n");
		return US_BULK_TRANSFER_GOOD;
	}

	/* NAK - that means we've retried a few times already */
	if (result == -ETIMEDOUT) {
		US_DEBUGP("usb_stor_transfer_partial(): device NAKed\n");
		return US_BULK_TRANSFER_FAILED;
	}

	/* the catch-all error case */
	if (result) {
		US_DEBUGP("usb_stor_transfer_partial(): unknown error\n");
		return US_BULK_TRANSFER_FAILED;
	}

	/* no error code, so we must have transferred some data, 
	 * just not all of it */
	return US_BULK_TRANSFER_SHORT;
}

/*-------------------------------------------------------------------*/
/**
 * usb_buffer_unmap_sg - free DMA mapping(s) for a scatterlist
 * @dev: device to which the scatterlist will be mapped
 * @pipe: endpoint defining the mapping direction
 * @sg: the scatterlist to unmap
 * @n_hw_ents: the positive return value from usb_buffer_map_sg
 *
 * Reverses the effect of usb_buffer_map_sg().
 */
static void usb_buffer_unmap_sg (struct usb_device *dev, unsigned pipe,
		struct scatterlist *sg, int n_hw_ents)
{
	struct usb_bus *bus;
	struct usb_hcd *hcd;
	struct pci_dev *pdev;

	if (!dev
			|| !(bus = dev->bus)
			|| !(hcd = bus->hcpriv)
			|| !(pdev = hcd->pdev)
			|| !pdev->dma_mask)
		return;

	dma_unmap_sg (pdev, sg, n_hw_ents,
			usb_pipein (pipe) ? DMA_FROM_DEVICE : DMA_TO_DEVICE);
}

/**
 * usb_buffer_map_sg - create scatterlist DMA mapping(s) for an endpoint
 * @dev: device to which the scatterlist will be mapped
 * @pipe: endpoint defining the mapping direction
 * @sg: the scatterlist to map
 * @nents: the number of entries in the scatterlist
 *
 * Return value is either < 0 (indicating no buffers could be mapped), or
 * the number of DMA mapping array entries in the scatterlist.
 *
 * The caller is responsible for placing the resulting DMA addresses from
 * the scatterlist into URB transfer buffer pointers, and for setting the
 * URB_NO_TRANSFER_DMA_MAP transfer flag in each of those URBs.
 *
 * Top I/O rates come from queuing URBs, instead of waiting for each one
 * to complete before starting the next I/O.   This is particularly easy
 * to do with scatterlists.  Just allocate and submit one URB for each DMA
 * mapping entry returned, stopping on the first error or when all succeed.
 * Better yet, use the usb_sg_*() calls, which do that (and more) for you.
 *
 * This call would normally be used when translating scatterlist requests,
 * rather than usb_buffer_map(), since on some hardware (with IOMMUs) it
 * may be able to coalesce mappings for improved I/O efficiency.
 *
 * Reverse the effect of this call with usb_buffer_unmap_sg().
 */
static int usb_buffer_map_sg (struct usb_device *dev, unsigned pipe,
		struct scatterlist *sg, int nents)
{
	struct usb_bus		*bus;
	struct usb_hcd *hcd;
	struct pci_dev *pdev;

	if (!dev
			|| usb_pipecontrol (pipe)
			|| !(bus = dev->bus)
			|| !(hcd = bus->hcpriv)
			|| !(pdev = hcd->pdev)
			|| !pdev->dma_mask)
		return -1;

	// FIXME generic api broken like pci, can't report errors
	return dma_map_sg (pdev, sg, nents,
			usb_pipein (pipe) ? DMA_FROM_DEVICE : DMA_TO_DEVICE);
}

static void sg_clean (struct usb_sg_request *io)
{
	struct usb_hcd *hcd = io->dev->bus->hcpriv;
	struct pci_dev *pdev = hcd->pdev;

	if (io->urbs) {
		while (io->entries--)
			usb_free_urb (io->urbs [io->entries]);
		kfree (io->urbs);
		io->urbs = 0;
	}
	if (pdev->dma_mask != 0)
		usb_buffer_unmap_sg (io->dev, io->pipe, io->sg, io->nents);
	io->dev = 0;
}

static void sg_complete (struct urb *urb)
{
	struct usb_sg_request	*io = (struct usb_sg_request *) urb->context;

	spin_lock (&io->lock);

	/* In 2.5 we require hcds' endpoint queues not to progress after fault
	 * reports, until the completion callback (this!) returns.  That lets
	 * device driver code (like this routine) unlink queued urbs first,
	 * if it needs to, since the HC won't work on them at all.  So it's
	 * not possible for page N+1 to overwrite page N, and so on.
	 *
	 * That's only for "hard" faults; "soft" faults (unlinks) sometimes
	 * complete before the HCD can get requests away from hardware,
	 * though never during cleanup after a hard fault.
	 */
	if (io->status
			&& (io->status != -ECONNRESET
				|| urb->status != -ECONNRESET)
			&& urb->actual_length) {
		US_DEBUGP("Error: %s ep%d%s scatterlist error %d/%d\n",
			io->dev->devpath,
			usb_pipeendpoint (urb->pipe),
			usb_pipein (urb->pipe) ? "in" : "out",
			urb->status, io->status);
		// BUG ();
	}

	if (urb->status && urb->status != -ECONNRESET) {
		int		i, found, status;

		io->status = urb->status;

		/* the previous urbs, and this one, completed already.
		 * unlink pending urbs so they won't rx/tx bad data.
		 */
		for (i = 0, found = 0; i < io->entries; i++) {
			if (!io->urbs [i])
				continue;
			if (found) {
				status = usb_unlink_urb (io->urbs [i]);
				if (status != -EINPROGRESS && status != -EBUSY)
					US_DEBUGP("Error: %s, unlink --> %d\n", __FUNCTION__, status);
			} else if (urb == io->urbs [i])
				found = 1;
		}
	}
	urb->dev = 0;

	/* on the last completion, signal usb_sg_wait() */
	io->bytes += urb->actual_length;
	io->count--;
	if (!io->count)
		complete (&io->complete);

	spin_unlock (&io->lock);
}

/**
 * usb_sg_init - initializes scatterlist-based bulk/interrupt I/O request
 * @io: request block being initialized.  until usb_sg_wait() returns,
 *	treat this as a pointer to an opaque block of memory,
 * @dev: the usb device that will send or receive the data
 * @pipe: endpoint "pipe" used to transfer the data
 * @period: polling rate for interrupt endpoints, in frames or
 * 	(for high speed endpoints) microframes; ignored for bulk
 * @sg: scatterlist entries
 * @nents: how many entries in the scatterlist
 * @length: how many bytes to send from the scatterlist, or zero to
 * 	send every byte identified in the list.
 * @mem_flags: SLAB_* flags affecting memory allocations in this call
 *
 * Returns zero for success, else a negative errno value.  This initializes a
 * scatter/gather request, allocating resources such as I/O mappings and urb
 * memory (except maybe memory used by USB controller drivers).
 *
 * The request must be issued using usb_sg_wait(), which waits for the I/O to
 * complete (or to be canceled) and then cleans up all resources allocated by
 * usb_sg_init().
 *
 * The request may be canceled with usb_sg_cancel(), either before or after
 * usb_sg_wait() is called.
 */
int usb_sg_init (
	struct usb_sg_request	*io,
	struct usb_device	*dev,
	unsigned		pipe, 
	unsigned		period,
	struct scatterlist	*sg,
	int			nents,
	size_t			length,
	int			mem_flags
)
{
	int			i;
	int			urb_flags;
	int			dma;
	struct usb_hcd *hcd;

	hcd = dev->bus->hcpriv;

	if (!io || !dev || !sg
			|| usb_pipecontrol (pipe)
			|| usb_pipeisoc (pipe)
			|| nents <= 0)
		return -EINVAL;

	spin_lock_init (&io->lock);
	io->dev = dev;
	io->pipe = pipe;
	io->sg = sg;
	io->nents = nents;

	/* not all host controllers use DMA (like the mainstream pci ones);
	 * they can use PIO (sl811) or be software over another transport.
	 */
	dma = (hcd->pdev->dma_mask != 0);
	if (dma)
		io->entries = usb_buffer_map_sg (dev, pipe, sg, nents);
	else
		io->entries = nents;

	/* initialize all the urbs we'll use */
	if (io->entries <= 0)
		return io->entries;

	io->count = 0;
	io->urbs = kmalloc (io->entries * sizeof *io->urbs, mem_flags);
	if (!io->urbs)
		goto nomem;

	urb_flags = USB_ASYNC_UNLINK | URB_NO_INTERRUPT | URB_NO_TRANSFER_DMA_MAP;
	if (usb_pipein (pipe))
		urb_flags |= URB_SHORT_NOT_OK;

	for (i = 0; i < io->entries; i++, io->count = i) {
		unsigned		len;

		io->urbs [i] = usb_alloc_urb (0);
		if (!io->urbs [i]) {
			io->entries = i;
			goto nomem;
		}

		io->urbs [i]->dev = 0;
		io->urbs [i]->pipe = pipe;
		io->urbs [i]->interval = period;
		io->urbs [i]->transfer_flags = urb_flags;

		io->urbs [i]->complete = sg_complete;
		io->urbs [i]->context = io;
		io->urbs [i]->status = -EINPROGRESS;
		io->urbs [i]->actual_length = 0;

		if (dma) {
			/* hc may use _only_ transfer_dma */
			io->urbs [i]->transfer_dma = sg_dma_address (sg + i);
			len = sg_dma_len (sg + i);
		} else {
			/* hc may use _only_ transfer_buffer */
			io->urbs [i]->transfer_buffer =
				page_address (sg [i].page) + sg [i].offset;
			len = sg [i].length;
		}

		if (length) {
			len = min_t (unsigned, len, length);
			length -= len;
			if (length == 0)
				io->entries = i + 1;
		}
		io->urbs [i]->transfer_buffer_length = len;
	}
	io->urbs [--i]->transfer_flags &= ~URB_NO_INTERRUPT;

	/* transaction state */
	io->status = 0;
	io->bytes = 0;
	init_completion (&io->complete);
	return 0;

nomem:
	sg_clean (io);
	return -ENOMEM;
}

/**
 * usb_sg_cancel - stop scatter/gather i/o issued by usb_sg_wait()
 * @io: request block, initialized with usb_sg_init()
 *
 * This stops a request after it has been started by usb_sg_wait().
 * It can also prevents one initialized by usb_sg_init() from starting,
 * so that call just frees resources allocated to the request.
 */
void usb_sg_cancel (struct usb_sg_request *io)
{
	unsigned long	flags;

	spin_lock_irqsave (&io->lock, flags);

	/* shut everything down, if it didn't already */
	if (!io->status) {
		int	i;

		io->status = -ECONNRESET;
		for (i = 0; i < io->entries; i++) {
			int	retval;

			if (!io->urbs [i]->dev)
				continue;
			retval = usb_unlink_urb (io->urbs [i]);
			if (retval != -EINPROGRESS && retval != -EBUSY)
				US_DEBUGP("WARNING: %s, unlink --> %d\n", __FUNCTION__, retval);
		}
	}
	spin_unlock_irqrestore (&io->lock, flags);
}

/**
 * usb_sg_wait - synchronously execute scatter/gather request
 * @io: request block handle, as initialized with usb_sg_init().
 * 	some fields become accessible when this call returns.
 * Context: !in_interrupt ()
 *
 * This function blocks until the specified I/O operation completes.  It
 * leverages the grouping of the related I/O requests to get good transfer
 * rates, by queueing the requests.  At higher speeds, such queuing can
 * significantly improve USB throughput.
 *
 * There are three kinds of completion for this function.
 * (1) success, where io->status is zero.  The number of io->bytes
 *     transferred is as requested.
 * (2) error, where io->status is a negative errno value.  The number
 *     of io->bytes transferred before the error is usually less
 *     than requested, and can be nonzero.
 * (3) cancelation, a type of error with status -ECONNRESET that
 *     is initiated by usb_sg_cancel().
 *
 * When this function returns, all memory allocated through usb_sg_init() or
 * this call will have been freed.  The request block parameter may still be
 * passed to usb_sg_cancel(), or it may be freed.  It could also be
 * reinitialized and then reused.
 *
 * Data Transfer Rates:
 *
 * Bulk transfers are valid for full or high speed endpoints.
 * The best full speed data rate is 19 packets of 64 bytes each
 * per frame, or 1216 bytes per millisecond.
 * The best high speed data rate is 13 packets of 512 bytes each
 * per microframe, or 52 KBytes per millisecond.
 *
 * The reason to use interrupt transfers through this API would most likely
 * be to reserve high speed bandwidth, where up to 24 KBytes per millisecond
 * could be transferred.  That capability is less useful for low or full
 * speed interrupt endpoints, which allow at most one packet per millisecond,
 * of at most 8 or 64 bytes (respectively).
 */
void usb_sg_wait (struct usb_sg_request *io)
{
	int		i, entries = io->entries;

	/* queue the urbs.  */
	spin_lock_irq (&io->lock);
	for (i = 0; i < entries && !io->status; i++) {
		int	retval;

		io->urbs [i]->dev = io->dev;
		retval = usb_submit_urb (io->urbs [i]);

		/* after we submit, let completions or cancelations fire;
		 * we handshake using io->status.
		 */
		spin_unlock_irq (&io->lock);
		switch (retval) {
			/* maybe we retrying will recover */
		case -ENXIO:	// hc didn't queue this one
		case -EAGAIN:
		case -ENOMEM:
			io->urbs [i]->dev = 0;
			retval = 0;
			i--;
			yield ();
			break;

			/* no error? continue immediately.
			 *
			 * NOTE: to work better with UHCI (4K I/O buffer may
			 * need 3K of TDs) it may be good to limit how many
			 * URBs are queued at once; N milliseconds?
			 */
		case 0:
			cpu_relax ();
			break;

			/* fail any uncompleted urbs */
		default:
			spin_lock_irq (&io->lock);
			io->count -= entries - i;
			if (io->status == -EINPROGRESS)
				io->status = retval;
			if (io->count == 0)
				complete (&io->complete);
			spin_unlock_irq (&io->lock);

			io->urbs [i]->dev = 0;
			io->urbs [i]->status = retval;
			
			US_DEBUGP("%s, submit --> %d\n", __FUNCTION__, retval);
			usb_sg_cancel (io);
		}
		spin_lock_irq (&io->lock);
		if (retval && io->status == -ECONNRESET)
			io->status = retval;
	}
	spin_unlock_irq (&io->lock);

	/* OK, yes, this could be packaged as non-blocking.
	 * So could the submit loop above ... but it's easier to
	 * solve neither problem than to solve both!
	 */
	wait_for_completion (&io->complete);

	sg_clean (io);
}

/*
 * Interpret the results of a URB transfer
 *
 * This function prints appropriate debugging messages, clears halts on
 * non-control endpoints, and translates the status to the corresponding
 * USB_STOR_XFER_xxx return code.
 */
static int interpret_urb_result(struct us_data *us, unsigned int pipe,
		unsigned int length, int result, unsigned int partial)
{
	US_DEBUGP("Status code %d; transferred %u/%u\n",
			result, partial, length);
	switch (result) {

	/* no error code; did we send all the data? */
	case 0:
		if (partial != length) {
			US_DEBUGP("-- short transfer\n");
			return USB_STOR_XFER_SHORT;
		}

		US_DEBUGP("-- transfer complete\n");
		return USB_STOR_XFER_GOOD;

	/* stalled */
	case -EPIPE:
		/* for control endpoints, (used by CB[I]) a stall indicates
		 * a failed command */
		if (usb_pipecontrol(pipe)) {
			US_DEBUGP("-- stall on control pipe\n");
			return USB_STOR_XFER_STALLED;
		}

		/* for other sorts of endpoint, clear the stall */
		US_DEBUGP("clearing endpoint halt for pipe 0x%x\n", pipe);
		if (usb_stor_clear_halt(us, pipe) < 0)
			return USB_STOR_XFER_ERROR;
		return USB_STOR_XFER_STALLED;

	/* timeout or excessively long NAK */
	case -ETIMEDOUT:
		US_DEBUGP("-- timeout or NAK\n");
		return USB_STOR_XFER_ERROR;

	/* babble - the device tried to send more than we wanted to read */
	case -EOVERFLOW:
		US_DEBUGP("-- babble\n");
		return USB_STOR_XFER_LONG;

	/* the transfer was cancelled by abort, disconnect, or timeout */
	case -ECONNRESET:
		US_DEBUGP("-- transfer cancelled\n");
		return USB_STOR_XFER_ERROR;

	/* short scatter-gather read transfer */
	case -EREMOTEIO:
		US_DEBUGP("-- short read transfer\n");
		return USB_STOR_XFER_SHORT;

	/* abort or disconnect in progress */
	case -EIO:
		US_DEBUGP("-- abort or disconnect in progress\n");
		return USB_STOR_XFER_ERROR;

	/* the catch-all error case */
	default:
		US_DEBUGP("-- unknown error\n");
		return USB_STOR_XFER_ERROR;
	}
}

/*
 * Transfer a scatter-gather list via bulk transfer
 *
 * This function does basically the same thing as usb_stor_bulk_msg()
 * above, but it uses the usbcore scatter-gather library.
 */
int usb_stor_bulk_transfer_sglist(struct us_data *us, unsigned int pipe,
		struct scatterlist *sg, int num_sg, unsigned int length,
		unsigned int *act_len)
{
	int result;

	/* don't submit s-g requests during abort/disconnect processing */
	if (us->flags & ABORTING_OR_DISCONNECTING)
		return USB_STOR_XFER_ERROR;

	/* initialize the scatter-gather request block */
	US_DEBUGP("%s: xfer %u bytes, %d entries\n", __FUNCTION__,
			length, num_sg);
	result = usb_sg_init(&us->current_sg, us->pusb_dev, pipe, 0,
			sg, num_sg, length, SLAB_NOIO);
	if (result) {
		US_DEBUGP("usb_sg_init returned %d\n", result);
		return USB_STOR_XFER_ERROR;
	}

	/* since the block has been initialized successfully, it's now
	 * okay to cancel it */
	set_bit(US_FLIDX_SG_ACTIVE, &us->flags);

	/* did an abort/disconnect occur during the submission? */
	if (us->flags & ABORTING_OR_DISCONNECTING) {

		/* cancel the request, if it hasn't been cancelled already */
		if (test_and_clear_bit(US_FLIDX_SG_ACTIVE, &us->flags)) {
			US_DEBUGP("-- cancelling sg request\n");
			usb_sg_cancel(&us->current_sg);
		}
	}

	/* wait for the completion of the transfer */
	usb_sg_wait(&us->current_sg);
	clear_bit(US_FLIDX_SG_ACTIVE, &us->flags);

	result = us->current_sg.status;
	if (act_len)
		*act_len = us->current_sg.bytes;
	return interpret_urb_result(us, pipe, length, result,
			us->current_sg.bytes);
}

/*
 * Transfer an entire SCSI command's worth of data payload over the bulk
 * pipe.
 *
 * Note that this uses usb_stor_transfer_partial to achieve its goals -- this
 * function simply determines if we're going to use scatter-gather or not,
 * and acts appropriately.  For now, it also re-interprets the error codes.
 */
void usb_stor_transfer(Scsi_Cmnd *srb, struct us_data* us)
{
	int i;
	int result = -1;
	struct scatterlist *sg;
	unsigned int total_transferred = 0;
	unsigned int transfer_amount;
	unsigned int partial;
	unsigned int pipe;

	/* calculate how much we want to transfer */
	transfer_amount = usb_stor_transfer_length(srb);

	/* was someone foolish enough to request more data than available
	 * buffer space? */
	if (transfer_amount > srb->request_bufflen)
		transfer_amount = srb->request_bufflen;

	/* are we scatter-gathering? */
	if (srb->use_sg) {

		/* loop over all the scatter gather structures and 
		 * make the appropriate requests for each, until done
		 */
		sg = (struct scatterlist *) srb->request_buffer;
		if (us->pusb_dev->speed == USB_SPEED_HIGH) {
			/* calculate the appropriate pipe information */
			if (us->srb->sc_data_direction == SCSI_DATA_READ)
				pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
			else
				pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);
			/* use the usb core scatter-gather primitives */
			result = usb_stor_bulk_transfer_sglist(us, pipe,
					sg, srb->use_sg, transfer_amount, &partial);
		} else {
			for (i = 0; i < srb->use_sg; i++) {

				/* transfer the lesser of the next buffer or the
				 * remaining data */
				if (transfer_amount - total_transferred >= 
						sg[i].length) {
					result = usb_stor_transfer_partial(us,
							sg[i].address, sg[i].length);
					total_transferred += sg[i].length;
				} else
					result = usb_stor_transfer_partial(us,
							sg[i].address,
							transfer_amount - total_transferred);

				/* if we get an error, end the loop here */
				if (result)
					break;
			}
		}
	}
	else
		/* no scatter-gather, just make the request */
		result = usb_stor_transfer_partial(us, srb->request_buffer, 
					     transfer_amount);

	/* return the result in the data structure itself */
	srb->result = result;
}

/***********************************************************************
 * Transport routines
 ***********************************************************************/

/* Invoke the transport and basic error-handling/recovery methods
 *
 * This is used by the protocol layers to actually send the message to
 * the device and receive the response.
 */
void usb_stor_invoke_transport(Scsi_Cmnd *srb, struct us_data *us)
{
	int need_auto_sense;
	int result;

	/*
	 * Grab device's exclusive access lock to prevent libusb/usbfs from
	 * sending out a command in the middle of ours (if libusb sends a
	 * get_descriptor or something on pipe 0 after our CBW and before
	 * our CSW, and then we get a stall, we have trouble).
	 */
	usb_excl_lock(us->pusb_dev, 3, 0);

	/* send the command to the transport layer */
	result = us->transport(srb, us);
	usb_excl_unlock(us->pusb_dev, 3);

	/* if the command gets aborted by the higher layers, we need to
	 * short-circuit all other processing
	 */
	if (result == USB_STOR_TRANSPORT_ABORTED) {
		US_DEBUGP("-- transport indicates command was aborted\n");
		srb->result = DID_ABORT << 16;

		/* Bulk-only aborts require a device reset */
		if (us->protocol == US_PR_BULK)
			us->transport_reset(us);
		return;
	}

	/* if there is a transport error, reset and don't auto-sense */
	if (result == USB_STOR_TRANSPORT_ERROR) {
		US_DEBUGP("-- transport indicates error, resetting\n");
		us->transport_reset(us);
		srb->result = DID_ERROR << 16;
		return;
	}

	/* Determine if we need to auto-sense
	 *
	 * I normally don't use a flag like this, but it's almost impossible
	 * to understand what's going on here if I don't.
	 */
	need_auto_sense = 0;

	/*
	 * If we're running the CB transport, which is incapable
	 * of determining status on it's own, we need to auto-sense almost
	 * every time.
	 */
	if (us->protocol == US_PR_CB || us->protocol == US_PR_DPCM_USB) {
		US_DEBUGP("-- CB transport device requiring auto-sense\n");
		need_auto_sense = 1;

		/* There are some exceptions to this.  Notably, if this is
		 * a UFI device and the command is REQUEST_SENSE or INQUIRY,
		 * then it is impossible to truly determine status.
		 */
		if (us->subclass == US_SC_UFI &&
		    ((srb->cmnd[0] == REQUEST_SENSE) ||
		     (srb->cmnd[0] == INQUIRY))) {
			US_DEBUGP("** no auto-sense for a special command\n");
			need_auto_sense = 0;
		}
	}

	/*
	 * If we have a failure, we're going to do a REQUEST_SENSE 
	 * automatically.  Note that we differentiate between a command
	 * "failure" and an "error" in the transport mechanism.
	 */
	if (result == USB_STOR_TRANSPORT_FAILED) {
		US_DEBUGP("-- transport indicates command failure\n");
		need_auto_sense = 1;
	}

	/*
	 * Also, if we have a short transfer on a command that can't have
	 * a short transfer, we're going to do this.
	 */
	if ((srb->result == US_BULK_TRANSFER_SHORT) &&
	    !((srb->cmnd[0] == REQUEST_SENSE) ||
	      (srb->cmnd[0] == INQUIRY) ||
	      (srb->cmnd[0] == MODE_SENSE) ||
	      (srb->cmnd[0] == LOG_SENSE) ||
	      (srb->cmnd[0] == MODE_SENSE_10))) {
		US_DEBUGP("-- unexpectedly short transfer\n");
		need_auto_sense = 1;
	}

	/* Now, if we need to do the auto-sense, let's do it */
	if (need_auto_sense) {
		int temp_result;
		void* old_request_buffer;
		unsigned short old_sg;
		unsigned old_request_bufflen;
		unsigned char old_sc_data_direction;
		unsigned char old_cmd_len;
		unsigned char old_cmnd[MAX_COMMAND_SIZE];

		US_DEBUGP("Issuing auto-REQUEST_SENSE\n");

		/* save the old command */
		memcpy(old_cmnd, srb->cmnd, MAX_COMMAND_SIZE);
		old_cmd_len = srb->cmd_len;

		/* set the command and the LUN */
		memset(srb->cmnd, 0, MAX_COMMAND_SIZE);
		srb->cmnd[0] = REQUEST_SENSE;
		srb->cmnd[1] = old_cmnd[1] & 0xE0;
		srb->cmnd[4] = 18;

		/* FIXME: we must do the protocol translation here */
		if (us->subclass == US_SC_RBC || us->subclass == US_SC_SCSI)
			srb->cmd_len = 6;
		else
			srb->cmd_len = 12;

		/* set the transfer direction */
		old_sc_data_direction = srb->sc_data_direction;
		srb->sc_data_direction = SCSI_DATA_READ;

		/* use the new buffer we have */
		old_request_buffer = srb->request_buffer;
		srb->request_buffer = srb->sense_buffer;

		/* set the buffer length for transfer */
		old_request_bufflen = srb->request_bufflen;
		srb->request_bufflen = 18;

		/* set up for no scatter-gather use */
		old_sg = srb->use_sg;
		srb->use_sg = 0;

		/* issue the auto-sense command */
		usb_excl_lock(us->pusb_dev, 3, 0);
		temp_result = us->transport(us->srb, us);
		usb_excl_unlock(us->pusb_dev, 3);

		/* let's clean up right away */
		srb->request_buffer = old_request_buffer;
		srb->request_bufflen = old_request_bufflen;
		srb->use_sg = old_sg;
		srb->sc_data_direction = old_sc_data_direction;
		srb->cmd_len = old_cmd_len;
		memcpy(srb->cmnd, old_cmnd, MAX_COMMAND_SIZE);

		if (temp_result == USB_STOR_TRANSPORT_ABORTED) {
			US_DEBUGP("-- auto-sense aborted\n");
			srb->result = DID_ABORT << 16;
			return;
		}
		if (temp_result != USB_STOR_TRANSPORT_GOOD) {
			US_DEBUGP("-- auto-sense failure\n");

			/* we skip the reset if this happens to be a
			 * multi-target device, since failure of an
			 * auto-sense is perfectly valid
			 */
			if (!(us->flags & US_FL_SCM_MULT_TARG)) {
				us->transport_reset(us);
			}
			srb->result = DID_ERROR << 16;
			return;
		}

		US_DEBUGP("-- Result from auto-sense is %d\n", temp_result);
		US_DEBUGP("-- code: 0x%x, key: 0x%x, ASC: 0x%x, ASCQ: 0x%x\n",
			  srb->sense_buffer[0],
			  srb->sense_buffer[2] & 0xf,
			  srb->sense_buffer[12], 
			  srb->sense_buffer[13]);
#ifdef CONFIG_USB_STORAGE_DEBUG
		usb_stor_show_sense(
			  srb->sense_buffer[2] & 0xf,
			  srb->sense_buffer[12], 
			  srb->sense_buffer[13]);
#endif

		/* set the result so the higher layers expect this data */
		srb->result = CHECK_CONDITION << 1;

		/* If things are really okay, then let's show that */
		if ((srb->sense_buffer[2] & 0xf) == 0x0)
			srb->result = GOOD << 1;
	} else /* if (need_auto_sense) */
		srb->result = GOOD << 1;

	/* Regardless of auto-sense, if we _know_ we have an error
	 * condition, show that in the result code
	 */
	if (result == USB_STOR_TRANSPORT_FAILED)
		srb->result = CHECK_CONDITION << 1;

	/* If we think we're good, then make sure the sense data shows it.
	 * This is necessary because the auto-sense for some devices always
	 * sets byte 0 == 0x70, even if there is no error
	 */
	if ((us->protocol == US_PR_CB || us->protocol == US_PR_DPCM_USB) && 
	    (result == USB_STOR_TRANSPORT_GOOD) &&
	    ((srb->sense_buffer[2] & 0xf) == 0x0))
		srb->sense_buffer[0] = 0x0;
}

/*
 * Control/Bulk/Interrupt transport
 */

/* The interrupt handler for CBI devices */
void usb_stor_CBI_irq(struct urb *urb)
{
	struct us_data *us = (struct us_data *)urb->context;

	US_DEBUGP("USB IRQ received for device on host %d\n", us->host_no);
	US_DEBUGP("-- IRQ data length is %d\n", urb->actual_length);
	US_DEBUGP("-- IRQ state is %d\n", urb->status);
	US_DEBUGP("-- Interrupt Status (0x%x, 0x%x)\n",
			us->irqbuf[0], us->irqbuf[1]);

	/* reject improper IRQs */
	if (urb->actual_length != 2) {
		US_DEBUGP("-- IRQ too short\n");
		return;
	}

	/* is the device removed? */
	if (urb->status == -ENODEV) {
		US_DEBUGP("-- device has been removed\n");
		return;
	}

	/* was this a command-completion interrupt? */
	if (us->irqbuf[0] && (us->subclass != US_SC_UFI)) {
		US_DEBUGP("-- not a command-completion IRQ\n");
		return;
	}

	/* was this a wanted interrupt? */
	if (!atomic_read(us->ip_wanted)) {
		US_DEBUGP("ERROR: Unwanted interrupt received!\n");
		return;
	}

	/* adjust the flag */
	atomic_set(us->ip_wanted, 0);
		
	/* copy the valid data */
	us->irqdata[0] = us->irqbuf[0];
	us->irqdata[1] = us->irqbuf[1];

	/* wake up the command thread */
	US_DEBUGP("-- Current value of ip_waitq is: %d\n",
			atomic_read(&us->ip_waitq.count));
	up(&(us->ip_waitq));
}

int usb_stor_CBI_transport(Scsi_Cmnd *srb, struct us_data *us)
{
	int result;

	/* Set up for status notification */
	atomic_set(us->ip_wanted, 1);

	/* re-initialize the mutex so that we avoid any races with
	 * early/late IRQs from previous commands */
	init_MUTEX_LOCKED(&(us->ip_waitq));

	/* COMMAND STAGE */
	/* let's send the command via the control pipe */
	result = usb_stor_control_msg(us, usb_sndctrlpipe(us->pusb_dev,0),
				      US_CBI_ADSC, 
				      USB_TYPE_CLASS | USB_RECIP_INTERFACE, 0, 
				      us->ifnum, srb->cmnd, srb->cmd_len);

	/* check the return code for the command */
	US_DEBUGP("Call to usb_stor_control_msg() returned %d\n", result);
	if (result < 0) {
		/* Reset flag for status notification */
		atomic_set(us->ip_wanted, 0);
	}

	/* if the command was aborted, indicate that */
	if (result == -ECONNRESET)
		return USB_STOR_TRANSPORT_ABORTED;

	/* STALL must be cleared when it is detected */
	if (result == -EPIPE) {
		US_DEBUGP("-- Stall on control pipe. Clearing\n");
		result = usb_stor_clear_halt(us,	
			usb_sndctrlpipe(us->pusb_dev, 0));

		/* if the command was aborted, indicate that */
		if (result == -ECONNRESET)
			return USB_STOR_TRANSPORT_ABORTED;
		return USB_STOR_TRANSPORT_FAILED;
	}

	if (result < 0) {
		/* Uh oh... serious problem here */
		return USB_STOR_TRANSPORT_ERROR;
	}

	/* DATA STAGE */
	/* transfer the data payload for this command, if one exists*/
	if (usb_stor_transfer_length(srb)) {
		usb_stor_transfer(srb, us);
		result = srb->result;
		US_DEBUGP("CBI data stage result is 0x%x\n", result);

		/* report any errors */
		if (result == US_BULK_TRANSFER_ABORTED) {
			atomic_set(us->ip_wanted, 0);
			return USB_STOR_TRANSPORT_ABORTED;
		}
		if (result == US_BULK_TRANSFER_FAILED) {
			atomic_set(us->ip_wanted, 0);
			return USB_STOR_TRANSPORT_FAILED;
		}
	}

	/* STATUS STAGE */

	/* go to sleep until we get this interrupt */
	US_DEBUGP("Current value of ip_waitq is: %d\n", atomic_read(&us->ip_waitq.count));
	down(&(us->ip_waitq));

	/* if we were woken up by an abort instead of the actual interrupt */
	if (atomic_read(us->ip_wanted)) {
		US_DEBUGP("Did not get interrupt on CBI\n");
		atomic_set(us->ip_wanted, 0);
		return USB_STOR_TRANSPORT_ABORTED;
	}

	US_DEBUGP("Got interrupt data (0x%x, 0x%x)\n", 
			us->irqdata[0], us->irqdata[1]);

	/* UFI gives us ASC and ASCQ, like a request sense
	 *
	 * REQUEST_SENSE and INQUIRY don't affect the sense data on UFI
	 * devices, so we ignore the information for those commands.  Note
	 * that this means we could be ignoring a real error on these
	 * commands, but that can't be helped.
	 */
	if (us->subclass == US_SC_UFI) {
		if (srb->cmnd[0] == REQUEST_SENSE ||
		    srb->cmnd[0] == INQUIRY)
			return USB_STOR_TRANSPORT_GOOD;
		else
			if (((unsigned char*)us->irq_urb->transfer_buffer)[0])
				return USB_STOR_TRANSPORT_FAILED;
			else
				return USB_STOR_TRANSPORT_GOOD;
	}

	/* If not UFI, we interpret the data as a result code 
	 * The first byte should always be a 0x0
	 * The second byte & 0x0F should be 0x0 for good, otherwise error 
	 */
	if (us->irqdata[0]) {
		US_DEBUGP("CBI IRQ data showed reserved bType %d\n",
				us->irqdata[0]);
		return USB_STOR_TRANSPORT_ERROR;
	}

	switch (us->irqdata[1] & 0x0F) {
		case 0x00: 
			return USB_STOR_TRANSPORT_GOOD;
		case 0x01: 
			return USB_STOR_TRANSPORT_FAILED;
		default: 
			return USB_STOR_TRANSPORT_ERROR;
	}

	/* we should never get here, but if we do, we're in trouble */
	return USB_STOR_TRANSPORT_ERROR;
}

/*
 * Control/Bulk transport
 */
int usb_stor_CB_transport(Scsi_Cmnd *srb, struct us_data *us)
{
	int result;

	/* COMMAND STAGE */
	/* let's send the command via the control pipe */
	result = usb_stor_control_msg(us, usb_sndctrlpipe(us->pusb_dev,0),
				      US_CBI_ADSC, 
				      USB_TYPE_CLASS | USB_RECIP_INTERFACE, 0, 
				      us->ifnum, srb->cmnd, srb->cmd_len);

	/* check the return code for the command */
	US_DEBUGP("Call to usb_stor_control_msg() returned %d\n", result);
	if (result < 0) {
		/* if the command was aborted, indicate that */
		if (result == -ECONNRESET)
			return USB_STOR_TRANSPORT_ABORTED;

		/* a stall is a fatal condition from the device */
		if (result == -EPIPE) {
			US_DEBUGP("-- Stall on control pipe. Clearing\n");
			result = usb_stor_clear_halt(us,
				usb_sndctrlpipe(us->pusb_dev, 0));

			/* if the command was aborted, indicate that */
			if (result == -ECONNRESET)
				return USB_STOR_TRANSPORT_ABORTED;
			return USB_STOR_TRANSPORT_FAILED;
		}

		/* Uh oh... serious problem here */
		return USB_STOR_TRANSPORT_ERROR;
	}

	/* DATA STAGE */
	/* transfer the data payload for this command, if one exists*/
	if (usb_stor_transfer_length(srb)) {
		usb_stor_transfer(srb, us);
		result = srb->result;
		US_DEBUGP("CB data stage result is 0x%x\n", result);

		/* report any errors */
		if (result == US_BULK_TRANSFER_ABORTED) {
			return USB_STOR_TRANSPORT_ABORTED;
		}
		if (result == US_BULK_TRANSFER_FAILED) {
			return USB_STOR_TRANSPORT_FAILED;
		}
	}

	/* STATUS STAGE */
	/* NOTE: CB does not have a status stage.  Silly, I know.  So
	 * we have to catch this at a higher level.
	 */
	return USB_STOR_TRANSPORT_GOOD;
}

/*
 * Bulk only transport
 */

/* Determine what the maximum LUN supported is */
int usb_stor_Bulk_max_lun(struct us_data *us)
{
	unsigned char *data;
	int result;
	int pipe;

	data = kmalloc(sizeof *data, GFP_KERNEL);
	if (!data) {
		return 0;
	}

	/* issue the command -- use usb_control_msg() because
	 *  the state machine is not yet alive */
	pipe = usb_rcvctrlpipe(us->pusb_dev, 0);
	result = usb_control_msg(us->pusb_dev, pipe,
				 US_BULK_GET_MAX_LUN, 
				 USB_DIR_IN | USB_TYPE_CLASS | 
				 USB_RECIP_INTERFACE,
				 0, us->ifnum, data, sizeof(*data), HZ);

	US_DEBUGP("GetMaxLUN command result is %d, data is %d\n", 
		  result, *data);

	/* if we have a successful request, return the result */
	if (result == 1) {
		result = *data;
		kfree(data);
		return result;
	} else {
		kfree(data);
	}

	/* if we get a STALL, clear the stall */
	if (result == -EPIPE) {
		US_DEBUGP("clearing endpoint halt for pipe 0x%x\n", pipe);

		/* Use usb_clear_halt() because the state machine
		 *  is not yet alive */
		usb_clear_halt(us->pusb_dev, pipe);
	}

	/* return the default -- no LUNs */
	return 0;
}

int usb_stor_Bulk_reset(struct us_data *us);

int usb_stor_Bulk_transport(Scsi_Cmnd *srb, struct us_data *us)
{
	struct bulk_cb_wrap *bcb;
	struct bulk_cs_wrap *bcs;
	int result;
	int pipe;
	int partial;
	int ret = USB_STOR_TRANSPORT_ERROR;

	bcb = kmalloc(sizeof *bcb, in_interrupt() ? GFP_ATOMIC : GFP_NOIO);
	if (!bcb) {
		return USB_STOR_TRANSPORT_ERROR;
	}
	bcs = kmalloc(sizeof *bcs, in_interrupt() ? GFP_ATOMIC : GFP_NOIO);
	if (!bcs) {
		kfree(bcb);
		return USB_STOR_TRANSPORT_ERROR;
	}

	/* set up the command wrapper */
	bcb->Signature = cpu_to_le32(US_BULK_CB_SIGN);
	bcb->DataTransferLength = cpu_to_le32(usb_stor_transfer_length(srb));
	bcb->Flags = srb->sc_data_direction == SCSI_DATA_READ ? 1 << 7 : 0;
	bcb->Tag = ++(us->tag);
	bcb->Lun = srb->cmnd[1] >> 5;
	if (us->flags & US_FL_SCM_MULT_TARG)
		bcb->Lun |= srb->target << 4;
	bcb->Length = srb->cmd_len;

	/* construct the pipe handle */
	pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);

	/* copy the command payload */
	memset(bcb->CDB, 0, sizeof(bcb->CDB));
	memcpy(bcb->CDB, srb->cmnd, bcb->Length);

	/* send it to out endpoint */
	US_DEBUGP("Bulk command S 0x%x T 0x%x Trg %d LUN %d L %d F %d CL %d\n",
		  le32_to_cpu(bcb->Signature), bcb->Tag,
		  (bcb->Lun >> 4), (bcb->Lun & 0x0F), 
		  le32_to_cpu(bcb->DataTransferLength), bcb->Flags, bcb->Length);
	result = usb_stor_bulk_msg(us, bcb, pipe, US_BULK_CB_WRAP_LEN, 
				   &partial);
	US_DEBUGP("Bulk command transfer result=%d\n", result);

	/* if the command was aborted, indicate that */
	if (result == -ECONNRESET) {
		ret = USB_STOR_TRANSPORT_ABORTED;
		goto out;
	}

	/* if we stall, we need to clear it before we go on */
	if (result == -EPIPE) {
		US_DEBUGP("clearing endpoint halt for pipe 0x%x\n", pipe);
		result = usb_stor_clear_halt(us, pipe);

		/* if the command was aborted, indicate that */
		if (result == -ECONNRESET) {
			ret = USB_STOR_TRANSPORT_ABORTED;
			goto out;
		}
		result = -EPIPE;
	} else if (result) {
		/* unknown error -- we've got a problem */
		ret = USB_STOR_TRANSPORT_ERROR;
		goto out;
	}

	/* if the command transfered well, then we go to the data stage */
	if (result == 0) {

		/* Genesys Logic interface chips need a 100us delay between
		 * the command phase and the data phase.  Some systems need
		 * even more, probably because of clock rate inaccuracies. */
		if (us->pusb_dev->descriptor.idVendor == USB_VENDOR_ID_GENESYS)
			udelay(110);

		/* send/receive data payload, if there is any */
		if (bcb->DataTransferLength) {
			usb_stor_transfer(srb, us);
			result = srb->result;
			US_DEBUGP("Bulk data transfer result 0x%x\n", result);

			/* if it was aborted, we need to indicate that */
			if (result == US_BULK_TRANSFER_ABORTED) {
				ret = USB_STOR_TRANSPORT_ABORTED;
				goto out;
			}
		}
	}

	/* See flow chart on pg 15 of the Bulk Only Transport spec for
	 * an explanation of how this code works.
	 */

	/* construct the pipe handle */
	pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);

	/* get CSW for device status */
	US_DEBUGP("Attempting to get CSW...\n");
	result = usb_stor_bulk_msg(us, bcs, pipe, US_BULK_CS_WRAP_LEN, 
				   &partial);

	/* if the command was aborted, indicate that */
	if (result == -ECONNRESET) {
		ret = USB_STOR_TRANSPORT_ABORTED;
		goto out;
	}

	/* did the attempt to read the CSW fail? */
	if (result == -EPIPE) {
		US_DEBUGP("clearing endpoint halt for pipe 0x%x\n", pipe);
		result = usb_stor_clear_halt(us, pipe);

		/* if the command was aborted, indicate that */
		if (result == -ECONNRESET) {
			ret = USB_STOR_TRANSPORT_ABORTED;
			goto out;
		}

		/* get the status again */
		US_DEBUGP("Attempting to get CSW (2nd try)...\n");
		result = usb_stor_bulk_msg(us, bcs, pipe,
					   US_BULK_CS_WRAP_LEN, &partial);

		/* if the command was aborted, indicate that */
		if (result == -ECONNRESET) {
			ret = USB_STOR_TRANSPORT_ABORTED;
			goto out;
		}

		/* if it fails again, we need a reset and return an error*/
		if (result == -EPIPE) {
			US_DEBUGP("clearing halt for pipe 0x%x\n", pipe);
			result = usb_stor_clear_halt(us, pipe);

			/* if the command was aborted, indicate that */
			if (result == -ECONNRESET) {
				ret = USB_STOR_TRANSPORT_ABORTED;
			} else {
				ret = USB_STOR_TRANSPORT_ERROR;
			}
			goto out;
		}
	}

	/* if we still have a failure at this point, we're in trouble */
	US_DEBUGP("Bulk status result = %d\n", result);
	if (result) {
		ret = USB_STOR_TRANSPORT_ERROR;
		goto out;
	}

	/* check bulk status */
	US_DEBUGP("Bulk status Sig 0x%x T 0x%x R %d Stat 0x%x\n",
		  le32_to_cpu(bcs->Signature), bcs->Tag, 
		  bcs->Residue, bcs->Status);
	if ((bcs->Signature != cpu_to_le32(US_BULK_CS_SIGN) && bcs->Signature != cpu_to_le32(US_BULK_CS_OLYMPUS_SIGN)) ||
	    bcs->Tag != bcb->Tag || 
	    bcs->Status > US_BULK_STAT_PHASE || partial != 13) {
		US_DEBUGP("Bulk logical error\n");
		ret = USB_STOR_TRANSPORT_ERROR;
		goto out;
	}

	/* based on the status code, we report good or bad */
	switch (bcs->Status) {
		case US_BULK_STAT_OK:
			/* command good -- note that data could be short */
			ret = USB_STOR_TRANSPORT_GOOD;
			goto out;

		case US_BULK_STAT_FAIL:
			/* command failed */
			ret = USB_STOR_TRANSPORT_FAILED;
			goto out;

		case US_BULK_STAT_PHASE:
			/* phase error -- note that a transport reset will be
			 * invoked by the invoke_transport() function
			 */
			ret = USB_STOR_TRANSPORT_ERROR;
			goto out;
	}

	/* we should never get here, but if we do, we're in trouble */

 out:
	kfree(bcb);
	kfree(bcs);
	return ret;
}

/***********************************************************************
 * Reset routines
 ***********************************************************************/

/* This issues a CB[I] Reset to the device in question
 */
int usb_stor_CB_reset(struct us_data *us)
{
	unsigned char cmd[12];
	int result;

	US_DEBUGP("CB_reset() called\n");

	/* if the device was removed, then we're already reset */
	if (!us->pusb_dev)
		return SUCCESS;

	memset(cmd, 0xFF, sizeof(cmd));
	cmd[0] = SEND_DIAGNOSTIC;
	cmd[1] = 4;
	result = usb_control_msg(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev,0),
				 US_CBI_ADSC, 
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, cmd, sizeof(cmd), HZ*5);

	if (result < 0) {
		US_DEBUGP("CB[I] soft reset failed %d\n", result);
		return FAILED;
	}

	/* long wait for reset */
	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule_timeout(HZ*6);
	set_current_state(TASK_RUNNING);

	US_DEBUGP("CB_reset: clearing endpoint halt\n");
	usb_stor_clear_halt(us,
			usb_rcvbulkpipe(us->pusb_dev, us->ep_in));
	usb_stor_clear_halt(us,
			usb_sndbulkpipe(us->pusb_dev, us->ep_out));

	US_DEBUGP("CB_reset done\n");
	/* return a result code based on the result of the control message */
	return SUCCESS;
}

/* This issues a Bulk-only Reset to the device in question, including
 * clearing the subsequent endpoint halts that may occur.
 */
int usb_stor_Bulk_reset(struct us_data *us)
{
	int result;

	US_DEBUGP("Bulk reset requested\n");

	/* if the device was removed, then we're already reset */
	if (!us->pusb_dev)
		return SUCCESS;

	result = usb_control_msg(us->pusb_dev, 
				 usb_sndctrlpipe(us->pusb_dev,0), 
				 US_BULK_RESET_REQUEST, 
				 USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, NULL, 0, HZ*5);

	if (result < 0) {
		US_DEBUGP("Bulk soft reset failed %d\n", result);
		return FAILED;
	}

	/* long wait for reset */
	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule_timeout(HZ*6);
	set_current_state(TASK_RUNNING);

	usb_stor_clear_halt(us,
			usb_rcvbulkpipe(us->pusb_dev, us->ep_in));
	usb_stor_clear_halt(us,
			usb_sndbulkpipe(us->pusb_dev, us->ep_out));
	US_DEBUGP("Bulk soft reset completed\n");
	return SUCCESS;
}
