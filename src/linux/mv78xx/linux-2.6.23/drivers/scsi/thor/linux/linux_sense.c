#include "mv_include.h"
#include "mv_os.h"

#include "hba_header.h"
#include "linux_main.h"
#include "linux_sense.h"
#include "linux_helper.h"

void HBA_Translate_Req_Status_To_OS_Status(
	IN PHBA_Extension pHBA,
	IN struct scsi_cmnd *scmd,
	IN PMV_Request pReq
	)
{
	PSENSE_DATA  senseBuffer = (PSENSE_DATA) scmd->sense_buffer;
	int i;
	unsigned char *buf;
	struct scatterlist *sg;

	/* we really wanna do this? */
	if (scmd &&
	    (SCSI_CMD_MODE_SENSE_6 == scmd->cmnd[0]) &&
	    scmd->use_sg && 
	    !(MV_SCp(scmd)->map_atomic)) {
		/* 
		 * MODE_SENSE is an instant cmd for SATA devices, thus 
		 * map_atomic should be 1 before we call HBA_kunmap_sg.
		 * while for ATAPI it'll call HBA_kunmap_sg before handling
		 * the command, so its map_atomic should be 0, and we'll need
		 * to copy its buffer from sg list.
		 * - this is how we tell ATAPI from ATA/SATA mode sense -
		 */
		sg = (struct scatterlist *) pReq->Data_Buffer;
		buf =(unsigned char *) kmap_atomic(sg->page, KM_IRQ0)\
			+ sg->offset;
		/* 
		 * ATAPI's Mode parameter header is always 8 bytes 
		 * while MODE_SENSE_6's is 4 bytes.
		 */
		for (i=4;i<pReq->Data_Transfer_Length-4;i++){
			*(buf+i) = *(buf+i+4);
		}
			
		kunmap_atomic(buf, KM_IRQ0);
	}

	HBA_kunmap_sg(pReq);

	if (MV_SCp(scmd)->mapped) {
		if (scmd->use_sg) {
			MV_DBG(DMSG_FREQ, "__MV__ call pci_unmap_sg.\n");
			pci_unmap_sg(pHBA->pcidev,
				     (struct scatterlist *)scmd->request_buffer,
				     scmd->use_sg,
				     scsi_to_pci_dma_dir(scmd->sc_data_direction));
		} else {
			MV_DBG(DMSG_FREQ,"__MV__ call pci_unmap_single.\n");
			pci_unmap_single(pHBA->pcidev,
					 MV_SCp(scmd)->bus_address,
					 scmd->request_bufflen,
					 scsi_to_pci_dma_dir(scmd->sc_data_direction));
		}
	}

	MV_DBG(DMSG_SCSI_FREQ,
	       "HBA_Translate_Req_Status_To_OS_Status:"
	       " pReq->Scsi_Status = %x pcmd = %p.\n", 
	       pReq->Scsi_Status, scmd);
	
	switch(pReq->Scsi_Status) {
	case REQ_STATUS_SUCCESS:
		scmd->result = (DID_OK<<16);
		break;
	case REQ_STATUS_MEDIA_ERROR: //TBD
		scmd->result = (DID_BAD_TARGET<<16);
		break;
	case REQ_STATUS_BUSY:
		scmd->result = (DID_BUS_BUSY<<16);
		break;
	case REQ_STATUS_NO_DEVICE:
		scmd->result = (DID_NO_CONNECT<<16);
		break;
	case REQ_STATUS_HAS_SENSE:
		/* Sense buffer data is valid already. */
		scmd->result  = (DRIVER_SENSE << 24) | (DID_OK << 16);
		senseBuffer->Valid = 1;

		MV_DBG(DMSG_SCSI, "MV Sense: response %x SK %s length %x ASC %x "
		       "ASCQ %x.\n", ((MV_PU8)senseBuffer)[0],
		       MV_DumpSenseKey(((MV_PU8)senseBuffer)[2]),
		       ((MV_PU8)senseBuffer)[7],
		       ((MV_PU8)senseBuffer)[12],
		       ((MV_PU8)senseBuffer)[13]);
		break;
	default:
		scmd->result = (DRIVER_INVALID|SUGGEST_ABORT)<<24;
		scmd->result |= DID_ABORT<<16;
		break;
	}

	scmd->scsi_done(scmd);
}

