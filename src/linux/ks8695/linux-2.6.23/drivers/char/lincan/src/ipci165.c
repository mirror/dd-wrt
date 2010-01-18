/* ipci165.c
 * Linux CAN-bus device driver for IXXAT iPC-I 165 (PCI) compatible HW.
 * Written for new CAN driver version by Radim Kalas
 * email:kalas@unicontrols.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/setup.h"
#include "../include/finish.h"
#include "../include/ipci165.h"
#include "../include/ipci165_fw.h"
#include "../include/kthread.h"

#include <ctype.h>

#ifndef IRQF_SHARED
#define IRQF_SHARED SA_SHIRQ
#endif  /*IRQF_SHARED*/

can_irqreturn_t ipci165_irq_handler(CAN_IRQ_HANDLER_ARGS(irq_number, dev_id));
int ipci165_baud_rate(struct canchip_t *chip, int rate, int clock, int sjw,
                      int sampl_pt, int flags);
int ipci165_set_btregs(struct canchip_t *chip, unsigned short btr0,
                       unsigned short btr1);
int ipci165_start_chip(struct canchip_t *chip);

#ifdef CAN_DEBUG
  void dump_mem(char *ptr, int size);
#else
#define dump_mem(a,b)
#endif

#define ipci165_load_btr(btr,btr0,btr1) {*btr = btr0; *(btr+1) = btr1;}

/**
 * ipci165_delay - Delay the execution
 * @msdelay: milliseconds to wait
 *
 * Return value: no return value
 * File: src/ipci165.c
 */
static void ipci165_delay(long msdelay)
{
#ifdef CAN_WITH_RTL
  if(!rtl_rt_system_is_idle())
  {
    rtl_delay(1000000l*msdelay);
  } else
#endif /*CAN_WITH_RTL*/
  {
    set_current_state(TASK_UNINTERRUPTIBLE);
    schedule_timeout((msdelay*HZ)/1000+1);
  }
}

/**
 * ipci165_generate_irq - Generate irq for HW
 * @candev: Pointer to hardware/board specific functions
 *
 * Return value: The function returns zero on success or non zero on failure
 * File: src/ipci165.c
 */
void ipci165_generate_irq(struct candevice_t *candev)
{
  can_ioptr_t crm_addr = candev->aux_base_addr;
  can_writeb(can_readb(crm_addr + CRM_UCR) & 0xFB, crm_addr + CRM_UCR);
  can_writeb(can_readb(crm_addr + CRM_UCR) | 0x04, crm_addr + CRM_UCR);
}

/**
 * bci_command - Send command to controller
 * @candev: Pointer to hardware/board specific functions
 * @cmd: Command to be performed
 * @size: Command data size
 * @data: Command data
 *
 * Return value: The function returns zero on success or non zero on failure
 * File: src/ipci165.c
 */
int bci_command(struct candevice_t *candev, char cmd, int size, char *data)
{
  can_ioptr_t dpram_addr = candev->dev_base_addr;

  DEBUGMSG ("ipci165_bci_command\n");

  if (size > BCI_CMD_MAX_LEN)
  {
    DEBUGMSG ("ipci165_bci_command: parameter error\n");
    return -EINVAL;
  }

  /* grant access to the command buffer */
  can_spin_lock(&candev->device_lock);

  // check command buffer status
  if (can_readb(dpram_addr + OF_BCI_SYNC) != 0)
  {
    /* something went wrong ... */
    can_spin_unlock(&candev->device_lock);
    DEBUGMSG ("ipci165_bci_command: command buffer is busy\n");
    return (-EBUSY);
  }

  // prepare command
  can_writeb(cmd, dpram_addr + OF_BCI_CMD);
  can_writeb(size + 1, dpram_addr + OF_BCI_NUM);
  memcpy_toio(dpram_addr + OF_BCI_DATA, data, size);

  // set flag for firmware
  can_writeb(1, dpram_addr + OF_BCI_SYNC);

  // generate interrupt to microcontroller
  ipci165_generate_irq (candev);

  return 0;
}

/**
 * bci_response - Get response from controller
 * @candev: Pointer to hardware/board specific functions
 * @cmd: Command to get response for
 * @size: Command data size
 * @data: Command data
 *
 * Return value: The function returns zero on success or non zero on failure
 * File: src/ipci165.c
 */
int bci_response(struct candevice_t *candev, char cmd, int *size, char *data)
{
  can_ioptr_t dpram_addr = candev->dev_base_addr;
  char tmp;
  int delay;

  DEBUGMSG ("ipci165_bci_response\n");

  delay = 1000;
  while (can_readb(dpram_addr + OF_BCI_SYNC) != 2)
  {
    /* wait 1 ms */
    /*    ipci165_delay(1); */
    udelay(100);
    if (--delay == 0)
    {
      /* timeout occured */
      /* release the lock */
      can_spin_unlock(&candev->device_lock);
      CANMSG ("BCI timeout!\n");
      return -EBUSY;
    }
  }

  /* we will not copy the command filed, so decrement the size by 1 */
  tmp = can_readb(dpram_addr + OF_BCI_NUM) - 1;
  if (*size > tmp) *size = tmp;

  if (can_readb(dpram_addr + OF_BCI_CMD) != cmd)
  {
    /* release the buffer */
    can_writeb(0, dpram_addr + OF_BCI_SYNC);
    /* unlock the access */
    can_spin_unlock(&candev->device_lock);

    DEBUGMSG ("ipci165_bci_command: invalid answer\n");
    return -EIO;
  }
  memcpy_fromio(data, dpram_addr + OF_BCI_DATA, *size);

  /* release the buffer */
  can_writeb(0, dpram_addr + OF_BCI_SYNC);
  /* unlock the access */
  can_spin_unlock(&candev->device_lock);
  return 0;
}

/**
 * ipci165_restart_can - Flush queues and sestart can controller
 * @candev: Pointer to hardware/board specific functions
 * @chip_idx: chip number
 *
 * Return value: The function returns zero on success or non zero on failure
 * File: src/ipci165.c
 */
int ipci165_restart_can(struct canchip_t *chip)
{
  char data[3];
  int size;
  int i;

  struct ipci165_chip_t *chip_data;
  unsigned long msg_ofs;

  /* reset CAN */
  data[0] = chip->chip_idx;
  size = 1;
  if (bci_command(chip->hostdevice, CMD_RESET_CAN, 1, data) ||
      bci_response(chip->hostdevice, CMD_RESET_CAN, &size, data) ||
      (data[0] == 0))
  {
    CANMSG ("CAN reset failed!\n");
    return -ENODEV;
  }

  /* flush TX/RX queues in DP-RAM */
  chip_data = (struct ipci165_chip_t *)chip->chip_data;
  msg_ofs = BCI_MSG_STATUS;

  for (i = 0; i< BCI_QUEUE_SIZE; i++)
  {
    can_writeb(BCI_MSG_STATUS_FREE, chip_data->rx_queue.addr + msg_ofs);
    can_writeb(BCI_MSG_STATUS_FREE, chip_data->tx_queue.addr + msg_ofs);
    msg_ofs += BCI_MSG_SIZE;
  }

  /* In- and output buffer re-initialization */
  canqueue_ends_flush_inlist(chip->msgobj[0]->qends);
  canqueue_ends_flush_outlist(chip->msgobj[0]->qends);

  /* start CAN */
  data[0] = chip->chip_idx;
  size = 1;
  if (bci_command(chip->hostdevice, CMD_START_CAN, 1, data) ||
      bci_response(chip->hostdevice, CMD_START_CAN, &size, data) ||
      (data[0] == 0))
  {
    CANMSG ("start chip failed!\n");
    return -ENODEV;
  }
  return 0;
}

/* this is the thread function that we are executing */
/**
 * ipci165_kthread - Thread restarting can controller after bus-off.
 * @kthread: pointer to kernel thread descriptor
 * @chip_idx: chip number
 *
 * Return value: no return value
 * File: src/ipci165.c
 */
void ipci165_kthread(kthread_t *kthread)
{
  struct canchip_t *chip = (struct canchip_t *)kthread->arg;
  struct ipci165_chip_t *chip_data = (struct ipci165_chip_t *)chip->chip_data;

  /* setup the thread environment */
  init_kthread(kthread, "ipci165");

  /* this is normal work to do */
  CANMSG ("kernel thread started!\n");

  /* an endless loop in which we are doing our work */
  for(;;)
  {
    /* fall asleep */
    wait_event_interruptible(kthread->queue,test_bit(CHIP_FLAG_BUS_OFF,&chip_data->flags));

    /* We need to do a memory barrier here to be sure that
    the flags are visible on all CPUs. */
    mb();

    /* here we are back from sleep because we caught a signal. */
    if (kthread->terminate)
    {
      /* we received a request to terminate ourself */
      break;
    }

    {
      clear_bit(CHIP_FLAG_BUS_OFF,&chip_data->flags);
      set_bit(CHIP_FLAG_RESET,&chip_data->flags);
      /* this is normal work to do */
      ipci165_restart_can(chip);

      clear_bit(CHIP_FLAG_RESET,&chip_data->flags);

      /* wait at least 100ms for next reset */
      ipci165_delay(100);
    }
  }
  /* here we go only in case of termination of the thread */

  /* cleanup the thread, leave */
  CANMSG ("kernel thread terminated!\n");
  exit_kthread(kthread);

  /* returning from the thread here calls the exit functions */
}

/**
 * ipci165_qfull_latency - Compute delay to send out full tx queue
 * @candev: Pointer to candevice/board structure
 * @obj: pointer to message object state structure
 *
 * Return Value: The function returns computed delay in jiffies
 * File: src/ipci165.c
 */
long ipci165_qfull_latency(struct msgobj_t *obj)
{
  long latency;
  latency = obj->hostchip->baudrate;
  if(latency){
    latency=(long)HZ*(CAN_FRAME_MIN_BIT_LEN * BCI_QUEUE_SIZE)/latency + 1;
  }

  return latency;
}

/**
 * ipci165_connect_irq: Installs interrupt routine and enable irq on HW
 * @candev: Pointer to candevice/board structure
 *
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/ipci165.c
 */
int ipci165_connect_irq(struct candevice_t *candev)
{
  can_ioptr_t crm_addr = candev->aux_base_addr;
  unsigned char icr;
  DEBUGMSG ("ipci165_connect_irq\n");

  /* install interrupt routine */
  if (request_irq(candev->sysdevptr.pcidev->irq,
                  ipci165_irq_handler,
                  IRQF_SHARED,
                  DEVICE_NAME,
                  candev))
    return -ENODEV;

  // Enable interrupt to PC
  can_writeb(can_readb(crm_addr + CRM_ICR) | 0x40, crm_addr + CRM_ICR);
  udelay (100);
  icr = can_readb(crm_addr + CRM_ICR);
  return 0;
}

/**
 * ipci165_disconnect_irq - Disable irq on HW
 * @candev: Pointer to candevice/board structure
 *
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/ipci165.c
 */
void ipci165_disconnect_irq(struct candevice_t *candev)
{
  can_ioptr_t crm_addr = candev->aux_base_addr;
  unsigned char icr;
  DEBUGMSG ("ipci165_disconnect_irq\n");

  // Enable interrupt to PC
  can_writeb(can_readb(crm_addr + CRM_ICR) & ~0x40, crm_addr + CRM_ICR);
  udelay (100);
  icr = can_readb(crm_addr + CRM_ICR);
  /* deinstall interrupt routine */
  free_irq(candev->sysdevptr.pcidev->irq, candev);
}

/* * * CAN Functionality * * */

/**
 * ipci165_chip_config - Can chip configuration
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/ipci165.c
 */
int ipci165_chip_config(struct canchip_t *chip)
{
  struct ipci165_chip_t *chip_data = chip->chip_data;
  char data[3];
  int ret, size;

  DEBUGMSG ("ipci165_chip_config[%i]\n",chip->chip_idx);

  /* comupte the base address of tx and rx queue for the channel */
  chip_data->tx_queue.addr = chip->chip_base_addr + OF_CH1_TX_QUEUE +
      chip->chip_idx * (OF_CH2_TX_QUEUE-OF_CH1_TX_QUEUE);
  chip_data->rx_queue.addr = chip->chip_base_addr + OF_CH1_RX_QUEUE +
      chip->chip_idx * (OF_CH2_RX_QUEUE-OF_CH1_RX_QUEUE);

  /* reset CAN */
  data[0] = chip->chip_idx;

  size = 1;
  if (bci_command(chip->hostdevice, CMD_RESET_CAN, 1, data) ||
      bci_response(chip->hostdevice, CMD_RESET_CAN, &size, data) ||
      (data[0] == 0))
  {
    CANMSG ("CAN reset failed!\n");
    return -ENODEV;
  }

  /* configure rx queue */
  data[0] = chip->chip_idx;
  data[1] = BCI_LATENCY_MODE;
  data[2] = 0; /* dummy */

  size = 1;
  if (bci_command(chip->hostdevice, CMD_CONFIG_RX_QUEUE, 3, data) ||
      bci_response(chip->hostdevice, CMD_CONFIG_RX_QUEUE, &size, data) ||
      (data[0] == 0))
  {
    CANMSG ("config RX queue failed!\n");
    return -ENODEV;
  }
  /* setup baud rate */
  if (!chip->baudrate) chip->baudrate = 1000000;
  if ((ret = ipci165_baud_rate(chip, chip->baudrate, chip->clock, 0, 0, 0))) return ret;

  /* start can communication */
  if ((ret = ipci165_start_chip(chip))) return ret;

  return 0;
}

/**
 * ipci165_baud_rate - Set communication parameters
 * @chip: pointer to chip state structure
 * @rate: baud rate in Hz
 * @clock: not used
 * @sjw: not used
 * @sampl_pt: not used
 * @flags: not used
 *
 * Return Value: negative value reports error.
 * File: src/ipci165.c
 */
int ipci165_baud_rate(struct canchip_t *chip, int rate, int clock, int sjw,
                      int sampl_pt, int flags)
{
  DEBUGMSG ("ipci165_baud_rate[%i]\n",chip->chip_idx);

  switch (rate) {
    case 10000:  return ipci165_set_btregs(chip, BCI_10KB);
    case 20000:  return ipci165_set_btregs(chip, BCI_20KB);
    case 50000:  return ipci165_set_btregs(chip, BCI_50KB);
    case 100000: return ipci165_set_btregs(chip, BCI_100KB);
    case 125000: return ipci165_set_btregs(chip, BCI_125KB);
    case 250000: return ipci165_set_btregs(chip, BCI_250KB);
    case 500000: return ipci165_set_btregs(chip, BCI_500KB);
    case 1000000:return ipci165_set_btregs(chip, BCI_1000KB);
    default: return -EINVAL;
  }

  return 0;
}

/**
 * ipci165_set_btregs - Configure bitrate registers
 * @chip: pointer to chip state structure
 * @btr0: bitrate register 0
 * @btr1: bitrate register 1
 *
 * Return Value: negative value reports error.
 * File: src/ipci165.c
 */
int ipci165_set_btregs(struct canchip_t *chip, unsigned short btr0,
                       unsigned short btr1)
{
  unsigned char data[3];
  int size;

  DEBUGMSG ("ipci165_set_btregs[%i]: btr0=%02x, btr1=%02x\n",chip->chip_idx,
            (unsigned)btr0,(unsigned)btr1);
  
  /* configure the chip */
  data[0] = chip->chip_idx;
  data[1] = btr0;
  data[2] = btr1;
  
  size = 1;
  if (bci_command(chip->hostdevice, CMD_INIT_CAN, 3, data) ||
      bci_response(chip->hostdevice, CMD_INIT_CAN, &size, data) ||
      (data[0] == 0))
  {
    CANMSG ("baud rate setup failed!\n");
    return -ENODEV;
  }
  return 0;
}

/**
 * ipci165_stop_chip - Start chip message processing
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/ipci165.c
 */
int ipci165_start_chip(struct canchip_t *chip)
{
  char data[1];
  int size;

  DEBUGMSG ("ipci165_start_chip[%i]\n",chip->chip_idx);
  
  /* start CAN */
  data[0] = chip->chip_idx;
  
  size = 1;
  if (bci_command(chip->hostdevice, CMD_START_CAN, 1, data) ||
      bci_response(chip->hostdevice, CMD_START_CAN, &size, data) ||
      (data[0] == 0))
  {
    CANMSG ("start chip failed!\n");
    return -ENODEV;
  }
  return 0;
}

/**
 * ipci165_stop_chip -  Stop chip message processing
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/ipci165.c
 */
int ipci165_stop_chip(struct canchip_t *chip)
{
  char data[1];
  int size;

  DEBUGMSG ("ipci165_stop_chip[%i]\n",chip->chip_idx);
  
  /* configure the chip */
  data[0] = chip->chip_idx;
  
  size = 1;
  if (bci_command(chip->hostdevice, CMD_STOP_CAN, 1, data) ||
      bci_response(chip->hostdevice, CMD_STOP_CAN, &size, data) ||
      (data[0] == 0))
  {
    CANMSG ("stop chip failed!\n");
    return -ENODEV;
  }
  return 0;
}

/**
 * ipci165_pre_read_config - Prepare message object for message reception
 * @chip: pointer to chip state structure
 * @obj: pointer to message object state structure
 *
 * Return Value: negative value reports error.
 *	Positive value indicates immediate reception of message.
 * File: src/ipci165.c
 */
int ipci165_pre_read_config(struct canchip_t *chip, struct msgobj_t *obj)
{
  return 0;
}

/**
 * ipci165_pre_write_config - Prepare message object for message transmission
 * @chip: pointer to chip state structure
 * @obj: pointer to message object state structure
 * @msg: pointer to CAN message
 *
 * Return Value: negative value reports error.
 * File: src/ipci165.c
 */
int ipci165_pre_write_config(struct canchip_t *chip, struct msgobj_t *obj,
                             struct canmsg_t *msg)
{
  return 0;
}

/**
 * ipci165_send_msg - Initiate message transmission
 * @chip: pointer to chip state structure
 * @obj: pointer to message object state structure
 * @msg: pointer to CAN message
 *
 * This function is called after ipci165_pre_write_config() function,
 * which prepares data in chip buffer.
 * Return Value: negative value reports error.
 * File: src/ipci165.c
 */
int ipci165_send_msg(struct canchip_t *chip, struct msgobj_t *obj,
                     struct canmsg_t *msg)
{
  return 0;
}

/**
 * ipci165_check_tx_stat - Checks state of transmission engine
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 *	Positive return value indicates transmission under way status.
 *	Zero value indicates finishing of all issued transmission requests.
 * File: src/ipci165.c
 */
int ipci165_check_tx_stat(struct canchip_t *chip)
{
  return 0;
}

/**
 * ipci165_irq_read_handler - ISR code responsible for receiving
 * @chip: pointer to chip state structure
 * @obj: pointer to attached queue description
 *
 * The main purpose of this function is to read message from CAN controller and
 * transfer them to attached queues
 * File: src/ipci165.c
 */
void ipci165_irq_read_handler(struct canchip_t *chip, struct msgobj_t *obj)
{
  struct ipci165_chip_t *chip_data = (struct ipci165_chip_t *)chip->chip_data;
  struct bci_queue_t *queue = &(chip_data)->rx_queue;
  can_ioptr_t         queue_addr = queue->addr;
  can_ioptr_t         msg_addr   = queue_addr + queue->idx * BCI_MSG_SIZE;

  int len;
  unsigned char frame_info;
  unsigned status;
  unsigned short tmp16;
  unsigned long  tmp32;

  DEBUGMSG ("ipci165_irq_read_handler[%i]\n",chip->chip_idx);

  do {
    dump_mem(msg_addr, BCI_MSG_SIZE);
    if (can_readb(msg_addr + BCI_MSG_TYPE) == BCI_MSG_TYPE_CAN)
    {
#if 0
      printk("ST(0)=%x, ST(1)=%x\n",can_readw(chip->chip_base_addr+OF_CAN1_STATUS),
             can_readw(chip->chip_base_addr+OF_CAN2_STATUS));
      for (tmp16 = 0 ; tmp16 < BCI_QUEUE_SIZE ; tmp16 ++)
        printk ("MSG_ST(%i)=%x\n",tmp16,can_readb(chip->chip_base_addr + OF_CH2_TX_QUEUE + tmp16*BCI_MSG_SIZE + BCI_MSG_STATUS));
      /* this is a can message */
      DEBUGMSG ("ipci165_irq_read_handler[%i]: message in buffer\n",chip->chip_idx);
#endif

      frame_info = can_readb(msg_addr + BCI_MSG_FRAME);
      len =  frame_info & 0x0f;
      if(len > CAN_MSG_LENGTH) len = CAN_MSG_LENGTH;
      obj->rx_msg.length = len;
      obj->rx_msg.flags  = (frame_info & BCI_MSG_FRAME_RTR ? MSG_RTR : 0);
      obj->rx_msg.cob    = 0;
      obj->rx_msg.timestamp.tv_sec = 0;
      obj->rx_msg.timestamp.tv_usec = 
          BCI_TIMESTAMP_RES * can_readl(msg_addr + BCI_MSG_TIMESTAMP);
      /*  BCI_TIMESTAMP_RES * le32_to_cpu(can_readl(msg_addr + BCI_MSG_TIMESTAMP)); */

      /* fill CAN message timestamp */
      /* can_filltimestamp(&obj->rx_msg.timestamp); */

      if (frame_info & BCI_MSG_FRAME_EXT)
      {
        /* extended frame - 29 bit identifier */
        obj->rx_msg.flags |= MSG_EXT;
        /* the ID is stored in motorola format (big endian), left justified  */
        /* obj->rx_msg.id = be32_to_cpu(can_readl(msg_addr + BCI_MSG_ID) >> 3); */
        memcpy_fromio(&tmp32, msg_addr + BCI_MSG_ID, 4);
        obj->rx_msg.id = be32_to_cpu(tmp32 >> 3);
        if (len > 0)
          memcpy_fromio(obj->rx_msg.data, msg_addr + BCI_MSG_EXT_DATA, len);
      } else
      {
        /* standard frame - 11 bit identifier */
        /* the ID is stored in motorola format (big endian), left justified */
        /* obj->rx_msg.id = be16_to_cpu(can_readw(msg_addr + BCI_MSG_ID) >> 5); */
        memcpy_fromio(&tmp16, msg_addr + BCI_MSG_ID, 2);
        obj->rx_msg.id = be16_to_cpu(tmp16 >> 5);
        if (len > 0)
          memcpy_fromio(obj->rx_msg.data, msg_addr + BCI_MSG_STD_DATA, len);
      }
      canque_filter_msg2edges(obj->qends, &obj->rx_msg);
    }
    else
    {
      /* this is a status message */
      status = can_readw(msg_addr + BCI_MSG_CAN_STATUS);
      DEBUGMSG ("ipci165_irq_read_handler[%i]: CAN status=%04x\n",chip->chip_idx, status);

      /* wake up the reset thread if the CAN is in bus off */
      if (status & BCI_CAN_STATUS_BUS_OFF) 
      {
        CANMSG ("BUS-OFF detected! Restarting\n");
        set_bit(CHIP_FLAG_BUS_OFF,&chip_data->flags);
        wake_up(&chip_data->kthread.queue);
      }

      if(obj->tx_slot)
      {
        canque_notify_inends(obj->tx_qedge, CANQUEUE_NOTIFY_ERRTX_BUS);
      }

    }
    DEBUGMSG ("ipci165_irq_read_handler[%i]: device status\n", chip->chip_idx);
    dump_mem(chip->chip_base_addr + OF_STATUS_BUFFER, 12);

    /* update pointer */
    queue->idx = (queue->idx + 1) % BCI_QUEUE_SIZE;
    /* release the buffer */
    can_writeb(BCI_MSG_STATUS_FREE, msg_addr + BCI_MSG_STATUS);
    msg_addr = queue_addr + queue->idx * BCI_MSG_SIZE;

  } while (can_readb(msg_addr + BCI_MSG_STATUS) == BCI_MSG_STATUS_FULL);

}

/**
 * ipci165_irq_write_handler - ISR code responsible for transmitting
 * @chip: pointer to chip state structure
 * @obj: pointer to attached queue description
 *
 * The main purpose of this function is to read message from attached queues
 * and transfer message contents into CAN controller chip.
 * File: src/ipci165.c
 */
void ipci165_irq_write_handler(struct canchip_t *chip, struct msgobj_t *obj)
{
  struct ipci165_chip_t *chip_data = ((struct ipci165_chip_t *)chip->chip_data);
  struct bci_queue_t *queue      = &chip_data->tx_queue;
  can_ioptr_t         queue_addr = queue->addr;
  can_ioptr_t         msg_addr   = queue_addr + queue->idx * BCI_MSG_SIZE;
  struct canque_slot_t *tx_slot;

  int len;
  unsigned char frame_info, ext;
  unsigned short tmp16;
  unsigned long  tmp32;

  DEBUGMSG ("ipci165_irq_write_handler[%i]\n",chip->chip_idx);

  while ((canque_test_outslot(obj->qends, &obj->tx_qedge, &obj->tx_slot) >=0))
  {
    if (test_bit(CHIP_FLAG_RESET,&chip_data->flags) ||
        (can_readb(msg_addr + BCI_MSG_STATUS) == BCI_MSG_STATUS_FULL))
    {
      canque_again_outslot(obj->qends, obj->tx_qedge, obj->tx_slot);

      /* lost interrupt work around */
      ipci165_generate_irq(obj->hostchip->hostdevice);

      mod_timer(&obj->tx_timeout, jiffies + ipci165_qfull_latency(obj));
      DEBUGMSG("ipci165_irq_write_handler[%i]: scheduled retry\n", chip->chip_idx);

      return;
    }

    tx_slot = obj->tx_slot;
    DEBUGMSG ("msg[%i] : id=%lx dlc=%x flg=%02x\n",
              chip->chip_idx,
              (unsigned long)tx_slot->msg.id,
              (unsigned int)tx_slot->msg.length,
              (unsigned int)tx_slot->msg.flags);
    dump_mem(tx_slot->msg.data, tx_slot->msg.length);

    len = tx_slot->msg.length;
    if(len > CAN_MSG_LENGTH) len = CAN_MSG_LENGTH;

    ext = tx_slot->msg.flags;
    frame_info =
        len |
        ((tx_slot->msg.flags & MSG_RTR) ? BCI_MSG_FRAME_RTR : 0) |
        ((tx_slot->msg.flags & MSG_EXT) ? BCI_MSG_FRAME_EXT : 0);

    can_writeb(BCI_MSG_SIZE - 2, msg_addr + BCI_MSG_NUM);
    can_writeb(BCI_MSG_TYPE_CAN, msg_addr + BCI_MSG_TYPE);
    can_writeb(frame_info, msg_addr + BCI_MSG_FRAME);
    if (frame_info & BCI_MSG_FRAME_EXT)
    {
      /* extended frame - 29 bit identifier */
      /* the ID is stored in motorola format (big endian), left justified  */
      tmp32 = be32_to_cpu(tx_slot->msg.id) << 3;
      memcpy_toio(msg_addr + BCI_MSG_ID, &tmp32, 4);
      if (len > 0)
        memcpy_toio(msg_addr + BCI_MSG_EXT_DATA, tx_slot->msg.data, len);
    } else
    {
      /* standard frame - 11 bit identifier */
      /* the ID is stored in motorola format (big endian), left justified */
      tmp16 = be16_to_cpu(tx_slot->msg.id) << 5;
      memcpy_toio(msg_addr + BCI_MSG_ID, &tmp16, 2);
      if (len > 0)
        memcpy_toio(msg_addr + BCI_MSG_STD_DATA, tx_slot->msg.data, len);
    }

    dump_mem(msg_addr, BCI_MSG_SIZE);

    /* update pointer */
    queue->idx = (queue->idx + 1) % BCI_QUEUE_SIZE;
    /* mark the buffer as full */
    can_writeb(BCI_MSG_STATUS_FULL, msg_addr + BCI_MSG_STATUS);
    /* wake up the controller */
    ipci165_generate_irq(chip->hostdevice);

    /* next message address */
    msg_addr = queue_addr + queue->idx * BCI_MSG_SIZE;

    /* Do local transmitted message distribution if enabled. */
    /* This code should not be called directly there, because it breaks strict
    behavior of queues if O_SYNC is set. */
    if (processlocal){
      obj->tx_slot->msg.flags |= MSG_LOCAL;
      canque_filter_msg2edges(obj->qends, &obj->tx_slot->msg);
    }
    /* Free transmitted slot */
    canque_free_outslot(obj->qends, obj->tx_qedge, obj->tx_slot);
    obj->tx_slot = NULL;
  }
  return;
}

/**
 * ipci165_irq_sync_activities - Synchronized access to write handler
 * @chip: pointer to chip state structure
 * @obj: pointer to attached queue description
 *
 * Return Value: The function always returns zero
 * File: src/ipci165.c
 */
void ipci165_irq_sync_activities(struct canchip_t *chip, struct msgobj_t *obj)
{
  while(!can_msgobj_test_and_set_fl(obj,TX_LOCK)) 
  {
    if(can_msgobj_test_and_clear_fl(obj,TX_REQUEST)) 
    {
      ipci165_irq_write_handler(chip, obj);
    }

    can_msgobj_clear_fl(obj,TX_LOCK);
    if(can_msgobj_test_fl(obj,TX_REQUEST))
      continue;
/*    if(can_msgobj_test_fl(obj,FILTCH_REQUEST) && !obj->tx_slot)
    continue; */
    break;
  }
}

/**
 * ipci165_irq_chip_handler - ISR for dedicated chip
 * @chip: pointer to chip state structure
 *
 * The main purpose of this function is to perform all necessary channel
 * operations as a reaction on signalled interrupt. 
 * File: src/ipci165.c
 */
void ipci165_irq_chip_handler(struct canchip_t *chip)
{
  struct msgobj_t       *obj = chip->msgobj[0];
  struct ipci165_chip_t *chip_data = chip->chip_data;
  struct bci_queue_t    *queue; 

  DEBUGMSG ("ipci165_irq_chip_handler[%i]\n",chip->chip_idx);

  /* check receive queue for messages */
  queue = &chip_data->rx_queue;
  if (can_readb(queue->addr + queue->idx * BCI_MSG_SIZE + BCI_MSG_STATUS)
      == BCI_MSG_STATUS_FULL)
    ipci165_irq_read_handler(chip, obj);

  queue = &chip_data->tx_queue;
/*  if (can_readb(queue->addr + queue->idx * BCI_MSG_SIZE + BCI_MSG_STATUS)
  == BCI_MSG_STATUS_FREE) */
  {
    can_msgobj_set_fl(obj,TX_REQUEST);

    /* calls unican_irq_write_handler synchronized with other invocations */
    ipci165_irq_sync_activities(chip, obj);
  }

}

#define MAX_RETR 10

/**
 * ipci165_irq_handler - Interrupt service routine
 * @irq: interrupt vector number, this value is system specific
 * @dev_id: driver private pointer registered at time of request_irq() call.
 *	The CAN driver uses this pointer to store relationship of interrupt
 *	to chip state structure - @struct canchip_t
 * @regs: system dependent value pointing to registers stored in exception frame
 * 
 * The interrupt handler is activated when the ipci165 controller generates
 * an interrupt as a reaction an internal state change. The interrupt is
 * acknowledged and ipci165_irq_chip_handler is called for every channel.
 * File: src/ipci165.c
 */
can_irqreturn_t ipci165_irq_handler(CAN_IRQ_HANDLER_ARGS(irq_number, dev_id))
{
  int retval;
  struct candevice_t *candev = (struct candevice_t *)dev_id;

  can_ioptr_t crm_addr   = candev->aux_base_addr;
  can_ioptr_t ucr1_addr  = crm_addr + CRM_UCR + 1;
  struct canchip_t *chip;
  unsigned char icr;
  int i;

  /* DEBUGMSG ("ipci165_irq_handler\n"); */

  /* read interrupt control register (byte 0) */
  icr = can_readb(crm_addr + CRM_ICR);

  if ((icr & 0x44) == 0x44)
  {
    DEBUGMSG ("ipci165_irq_handler: pending interrupt\n");

    /* confirm pending interrupt */
    can_writeb(can_readb(ucr1_addr) | 0x01,  ucr1_addr);
    can_writeb(can_readb(ucr1_addr) & ~0x01, ucr1_addr);

    /* call interrupt handler for every channel */
    for (i=0 ; i < candev->nr_all_chips ; i++)
    {
      chip = candev->chip[i];
      if (chip->flags & CHIP_CONFIGURED)
        ipci165_irq_chip_handler(candev->chip[i]);
    }
    DEBUGMSG ("ipci165_irq_handler: interrupt handled\n");

    retval = CANCHIP_IRQ_HANDLED;
  } else {
    DEBUGMSG ("ipci165_irq_handler: not our interrupt\n");
    retval = CANCHIP_IRQ_NONE;
  }

  return CAN_IRQ_RETVAL(retval);
}

/**
 * ipci165_wakeup_tx - Wakeup TX processing
 * @chip: pointer to chip state structure
 * @obj: pointer to message object structure
 *
 * Function is responsible for initiating message transmition.
 * It is responsible for clearing of object TX_REQUEST flag
 *
 * Return Value: negative value reports error.
 * File: src/ipci165.c
 */
int ipci165_wakeup_tx(struct canchip_t *chip, struct msgobj_t *obj)
{
  DEBUGMSG ("ipci165_wakeup_tx\n");
  can_preempt_disable();

  can_msgobj_set_fl(obj,TX_REQUEST);

  /* calls ipci165_irq_write_handler synchronized with other invocations
  from kernel and IRQ context */
  ipci165_irq_sync_activities(chip, obj);

  can_preempt_enable();
  DEBUGMSG ("ipci165_wakeup_tx: finished\n");

  return 0;
}

void ipci165_do_tx_timeout(unsigned long data)
{
  struct msgobj_t *obj=(struct msgobj_t *)data;

  DEBUGMSG ("ipci165_do_tx_timeout\n");

  can_preempt_disable();

  can_msgobj_set_fl(obj,TX_REQUEST);

  /* calls ipci165_irq_write_handler synchronized with other invocations
  from kernel and IRQ context */
  ipci165_irq_sync_activities(obj->hostchip, obj);

  can_preempt_enable();
  DEBUGMSG ("ipci165_do_tx_timeout: finished\n");
}

/**
 * ipci165_attach_to_chip: - attaches to the chip, setups registers and state
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/ipci165.c
 */
int ipci165_attach_to_chip(struct canchip_t *chip)
{
  return 0;
}

/**
 * ipci165_release_chip: - called before chip structure removal if %CHIP_ATTACHED is set
 * @chip: pointer to chip state structure
 *
 * Return Value: negative value reports error.
 * File: src/ipci165.c
 */
int ipci165_release_chip(struct canchip_t *chip)
{
  ipci165_stop_chip(chip);
  /* disable interrupts in the hardware, etc. */
  return 0;
}

/* * * iPC-I 165/PCI Board Functionality * * */

/**
 * ipci165_request_io - Reserve io or memory range for can board
 * @candev: pointer to candevice/board which asks for io. Field @io_addr
 *	of @candev is used in most cases to define start of the range
 *
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/ipci165.c
 */
int ipci165_request_io(struct candevice_t *candev)
{
  unsigned long dpram_addr; /* physical address before remap for this function */
  unsigned long crm_addr;   /* physical address before remap for this function */
  unsigned long fix_addr;   /* physical address before remap for this function */
  int i,j;

  DEBUGMSG ("ipci165_request_io\n");

  crm_addr   = pci_resource_start(candev->sysdevptr.pcidev,0);
  dpram_addr = pci_resource_start(candev->sysdevptr.pcidev,2);

  DEBUGMSG ("ipci165_request_io: crm = 0x%lx, dpram = 0x%lx\n",crm_addr, dpram_addr);

  /* verify, if our HW is buggy, and try to fix it */
#if 0
  if (test_bit (7, &crm_addr))
  {
    CANMSG ("Wrong PCI base address [0x%lx](PLX PCI9050 bug)!\n", dpram_addr);

    fix_addr = pci_resource_start(candev->sysdevptr.pcidev,3);

    if (fix_addr == 0)
    {
      CANMSG ("This card was not fixed!\n");

      if (candev->aux_base_addr == NULL)
      {
        CANMSG ("You have to specify IO address parameter!\n");
        return -EINVAL;
      }
      CANMSG ("Using specified IO address value for the memory [0x%lx]\n",
              can_ioptr2ulong(candev->aux_base_addr));
    }
    else
    {
      CANMSG ("Fixed card. Using of 3 region [0x%lx]\n", fix_addr);
      candev->aux_base_addr = fix_addr;
    }

    pci_write_config_dword (candev->sysdevptr.pcidev,
                            PCI_BASE_ADDRESS_0, fix_addr);
  }
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
  if(pci_request_region(candev->sysdevptr.pcidev, 2, "kv_ipci165_dpram") == 0)
  {
    if(pci_request_region(candev->sysdevptr.pcidev, 0, "kv_ipci165_reg") == 0)
    {
#else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
  if(pci_request_regions(candev->sysdevptr.pcidev, "kv_ipci165") == 0)
  {
#endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/

      if ((candev->dev_base_addr = ioremap(dpram_addr, 
           pci_resource_len(candev->sysdevptr.pcidev,2))))
      {
        DEBUGMSG ("ipci165_request_io: dpram remapped to 0x%lx\n", candev->dev_base_addr);

        if ((candev->aux_base_addr = ioremap(crm_addr, 
             pci_resource_len(candev->sysdevptr.pcidev,0))))
        {
          DEBUGMSG ("ipci165_request_io: crm remapped to 0x%lx\n", can_ioptr2ulong(candev->aux_base_addr));
          /* all resources has been allocated */

          /* Because of my mapping, I cannot use the
             can_base_addr_fixup(candev, remap_addr) to remap the addresses */
          for(i=0;i<candev->nr_all_chips;i++)
          {
            candev->chip[i]->chip_base_addr = candev->dev_base_addr;
            for(j=0;j<candev->chip[i]->max_objects;j++)
              candev->chip[i]->msgobj[j]->obj_base_addr = candev->dev_base_addr;
          }

          return 0;

        } else CANMSG("Unable to remap memory at: 0x%lx\n", crm_addr);
        iounmap(candev->aux_base_addr);

      } else CANMSG("Unable to remap memory at: 0x%lx\n", dpram_addr);
      iounmap(candev->dev_base_addr);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
      pci_release_region(candev->sysdevptr.pcidev, 0);
    } else CANMSG("Request of kv_ipci165_reg range failed\n");

    pci_release_region(candev->sysdevptr.pcidev, 2);
  } else CANMSG("Request of kv_ipci165_dpram range failed\n");

#else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
    pci_release_regions(candev->sysdevptr.pcidev);
  } else CANMSG("Request of kv_ipci165 regions failed\n");
#endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/

  return -ENODEV;
}

/**
 * ipci165_release_io - Free reserved io memory range
 * @candev: pointer to candevice/board which releases io
 *
 * Return Value: The function always returns zero
 * File: src/ipci165.c
 */
int ipci165_release_io(struct candevice_t *candev)
{
  struct ipci165_chip_t *chip_data;
  int i;
  
  /* disable irq on HW */
  ipci165_disconnect_irq(candev);

#if 0
  /* terminate the kernel threads */
  for (i = 0 ; i < candev->nr_all_chips ; i++)
  {
    chip_data = (struct ipci165_chip_t *)candev->chip[i]->chip_data;
    stop_kthread(&chip_data->restart_thread);
  }
#endif

  iounmap(candev->aux_base_addr);
  iounmap(candev->dev_base_addr);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))
  pci_release_region(candev->sysdevptr.pcidev, 2);
  pci_release_region(candev->sysdevptr.pcidev, 0);
#else /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/
  pci_release_regions(candev->sysdevptr.pcidev);
#endif /*(LINUX_VERSION_CODE > KERNEL_VERSION(2,4,21))*/

  return 0;
}

/**
 * ipci165_download_fw - Download FW into CAN hardware
 * @candev: Pointer to candevice/board structure
 *
 * Return Value: returns zero on success
 * File: src/ipci165.c
 */
int ipci165_download_fw(struct candevice_t *candev)
{
  can_ioptr_t dpram_addr = candev->dev_base_addr;
  char board_name[BOARD_NAME_LEN+1];
  char hw_version[HW_VERSION_LEN+1];
  char mode[MODE_LEN+1];

  struct ipci165_fw_t *fwArray = ipci165_fw;
  int attempt;

  DEBUGMSG ("ipci165_download_fw\n");

  /* read name and version */  
  memcpy_fromio (board_name, dpram_addr + BOARD_NAME_OFS, BOARD_NAME_LEN);
  board_name[BOARD_NAME_LEN] = 0;

  memcpy_fromio (hw_version, dpram_addr + HW_VERSION_OFS, HW_VERSION_LEN);
  hw_version[HW_VERSION_LEN] = 0;

  CANMSG ("Board Name: %s\n", board_name);
  CANMSG ("HW Version: %s\n", hw_version);

/*
  if ((hw_version[0] != 'V') && (hw_version[0] != 'v'))
{
  CANMSG ("This board is too old and not supported by the BCI !\n");
  return -ENODEV;
}
*/

  /* detect & test mode */
  memcpy_fromio (mode, dpram_addr + MODE_OFS, MODE_LEN);
  mode[MODE_LEN] = 0;

  if (strncmp (mode, "PC-Loader V", 11))
  {
    CANMSG ("Unknown mode [%s], can't download firmware!\n",mode);
    return -ENODEV;
  }

  while (fwArray->len)
  {
    /* fill buffer */
    can_writeb(LD_CMD_DOWNLOAD, dpram_addr + OF_LD_CMD);
    can_writeb(fwArray->len, dpram_addr + OF_LD_NUM);
    can_writeb(0, dpram_addr + OF_LD_NUM + 1);

    can_writel(fwArray->addr, dpram_addr + OF_LD_ADDRESS);
    /*    can_writel already performes the cpu_to_le32 conversion by itself   */
    /*    can_writel(cpu_to_le32(fwArray->addr), dpram_addr + OF_LD_ADDRESS); */

    memcpy_toio(dpram_addr + OF_LD_DATA, fwArray->a_data, fwArray->len);

#if 0
    dump_mem((void *)(dpram_addr + OF_LD_SYNC), fwArray->len + 8);
#endif
    /* buffer is prepared, set flag for loader */
    can_writeb(1, dpram_addr + OF_LD_SYNC);

    /* update pointer */
    fwArray++;

    /* wait for the loader */
    attempt = 1000;
    while (can_readb(dpram_addr + OF_LD_SYNC) != 0)
    {
      udelay(100);
      if (--attempt == 0)
      {
        /* timeout occured */
        CANMSG ("Firmware download failed!\n");
        return -ENODEV;
      }
    }
  }
  CANMSG ("Firmware downladed successfully\n");

  /* start the FW */
  can_writeb(LD_CMD_START_FW, dpram_addr + OF_LD_CMD);
  can_writeb(1, dpram_addr + OF_LD_SYNC);
  ipci165_delay (500);

  return 0;
}

/**
 * ipci165_reset - Hardware reset routine
 * @candev: Pointer to candevice/board structure
 *
 * Return Value: The function returns zero on success or %-ENODEV on failure
 * File: src/ipci165.c
 */
int ipci165_reset(struct candevice_t *candev)
{
  can_ioptr_t crm_addr = candev->aux_base_addr;
  unsigned long test_data;
  char buffer[BCI_CMD_MAX_LEN];
  int i, size, chips;
  unsigned char ucr;
  struct canchip_t *chip;
  struct ipci165_chip_t *chip_data;

  DEBUGMSG ("ipci165_reset: hardware reset\n");

  /* reset the HW */
  ucr = can_readb(crm_addr + CRM_UCR + 3);
  can_writeb(ucr | 0x40, crm_addr + CRM_UCR + 3);
  udelay(100);
  can_writeb(ucr & ~0x40, crm_addr + CRM_UCR + 3);

  /* wait a little bit */
  ipci165_delay(200);

  /* download FW */
  if (ipci165_download_fw(candev)) return -ENODEV;

  /* enable irq on HW */
  if (ipci165_connect_irq(candev))
    {
    CANMSG ("Interrupt routine installation for IRQ %i failed!\n",
            candev->sysdevptr.pcidev->irq);
    return -ENODEV;
    }

  /* test BCI interface */
  test_data = 0x12345678;
  size = sizeof(test_data);
  if (bci_command(candev, CMD_TEST, size, (char *)&test_data) ||
      bci_response(candev, CMD_TEST, &size, (char *)&test_data) ||
      (test_data != ~0x12345678))
  {
    CANMSG ("BCI test failed! Test pattern is %lx\n", test_data);
    return -ENODEV;
  }

  /* get Firmware identification */
  /* send command, fw requests 1 dummy byte */
  size = BCI_CMD_MAX_LEN;
  if (bci_command(candev, CMD_ID, 1, (char *)&test_data) ||
      bci_response(candev, CMD_ID, &size, buffer))
  {
    CANMSG ("Firmware Identification reading failed!\n");
    return -ENODEV;
  }
  CANMSG ("Firmware: %s\n",buffer);

  /* get Firmware version */
  /* send command, fw requests 1 dummy byte */
  size = BCI_CMD_MAX_LEN;
  if (bci_command(candev, CMD_VERSION, 1, (char *)&test_data) ||
      bci_response(candev, CMD_VERSION, &size, buffer))
  {
    CANMSG ("Firmware Version reading failed!\n");
    return -ENODEV;
  }
  CANMSG ("Version: %s\n",buffer);

  /* get Board Info */
  /* send command, fw requests 1 dummy byte */
  size = BOARD_INFO_SIZE;
  if (bci_command(candev, CMD_GET_BOARD_INFO, 1, (char *)&test_data) ||
      bci_response(candev, CMD_GET_BOARD_INFO, &size, (char *) buffer))
  {
    CANMSG ("Get Board Info failed!\n");
    return -ENODEV;
  }

  chips = le16_to_cpu(*(unsigned short*)(buffer+OF_BOARD_INFO_CHIPS));
  /* shouldn't be, but who knows ... */
  if (chips > 2) chips = 2;

  CANMSG ("Chips: %i\n",chips);
  CANMSG ("Chip 1 Type: %s\n",buffer+OF_BOARD_INFO_CHIP1_TYPE);

  /* update board info */
  if (chips == 1)
  {
    /* we have to correct the number in candev and release allocated
       structures */
    candev->nr_all_chips = chips;
    canchip_done(candev->chip[1]);

  } else CANMSG ("Chip 2 Type: %s\n",buffer+OF_BOARD_INFO_CHIP2_TYPE); 

  /* start kernel threads */
  for (i = 0 ; i < chips ; i++)
  {
    chip = candev->chip[i];
    chip_data = (struct ipci165_chip_t *)chip->chip_data;
    chip_data->kthread.arg = chip;
    start_kthread(ipci165_kthread, &chip_data->kthread);
  }

  CANMSG ("HW is up and working.\n");
  return 0;
}

/**
 * ipci165_init_hw_data - Initialize hardware cards
 * @candev: Pointer to candevice/board structure
 *
 * Return Value: The function always returns zero
 * File: src/ipci165.c
 */
int ipci165_init_hw_data(struct candevice_t *candev)
{
  struct pci_dev *pcidev = NULL;
  unsigned short SubsystemID;

  DEBUGMSG ("ipci165_init_hw_data\n");

  /* find iPC-I 165 on PCI bus */
  do
  {
    pcidev = pci_find_device(IPCI165_VENDOR_ID, IPCI165_DEVICE_ID, pcidev);
    if(pcidev == NULL) return -ENODEV;

    /* check subvendor ID */
    pci_read_config_word (pcidev, PCI_SUBSYSTEM_ID, &SubsystemID);
    if ((SubsystemID != IPCI165_SUBSYSTEM_ID) &&
        (SubsystemID != CP350_SUBSYSTEM_ID))
      break;
  }
  while(can_check_dev_taken(pcidev));

  /* enable it */
  if (pci_enable_device (pcidev))
  {
    CANMSG ("Cannot enable PCI device\n");
    return -EIO;
  }

  candev->sysdevptr.pcidev = pcidev;
  candev->res_addr=0;
  candev->nr_82527_chips=0;
  candev->nr_sja1000_chips=0;
  /* we do not know yet, whether our HW has one or two chan chips. Let's
     prepare configuration for maximal configuration = 2. This will be
     corrected later on */
  candev->nr_all_chips=2; 
  candev->flags |= CANDEV_PROGRAMMABLE_IRQ*0;
  /* initialize device spinlock */
  can_spin_lock_init(&candev->device_lock);

  return 0;
}

#define CHIP_TYPE "ipci165"

/**
 * ipci165_init_chip_data - Initialize chips
 * @candev: Pointer to candevice/board structure)
 * @chipnr: Number of the CAN chip on the hardware card
 *
 * Return Value: The function always returns zero
 * File: src/ipci165.c
 */
int ipci165_init_chip_data(struct candevice_t *candev, int chipnr)
{
  struct canchip_t      *chip = candev->chip[chipnr];
  struct ipci165_chip_t *chip_data;

  DEBUGMSG ("ipci165_init_chip_data\n");

  chip->chip_type = CHIP_TYPE;
  chip->chip_base_addr = 0; /* mapping not known yet */
  chip->clock = 10000000;
  chip->int_clk_reg = 0x0;
  chip->int_bus_reg = 0x0;
  chip->max_objects = 1;

#if 0
  /* initialize interrupt handling only for channel 0. The interrupt
     is shared between the channels so we have to work it out in one
     interrupt routine. */
  if (chipnr == 0)
  {
    chip->chipspecops->irq_handler=ipci165_irq_handler;
    chip->chip_irq=candev->sysdevptr.pcidev->irq;
    chip->flags |= CHIP_IRQ_PCI;
  } else
  {
    chip->chipspecops->irq_handler=NULL;
  }
#else
  chip->chipspecops->irq_handler = NULL;
  chip->chip_irq = 0;
  chip->flags |= CHIP_IRQ_CUSTOM;
#endif

  chip_data = can_checked_malloc(sizeof(struct ipci165_chip_t));
  if(!chip_data) return -ENOMEM;
  chip_data->rx_queue.idx = 0;
  chip_data->rx_queue.addr = 0;
  chip_data->tx_queue.idx = 0;
  chip_data->tx_queue.addr = 0;
  chip->chip_data = chip_data;

  CANMSG("initializing ipci165 chip operations\n");
  chip->chipspecops->attach_to_chip=ipci165_attach_to_chip;
  chip->chipspecops->release_chip=ipci165_release_chip;
  chip->chipspecops->chip_config=ipci165_chip_config;
  chip->chipspecops->baud_rate=ipci165_baud_rate;
  chip->chipspecops->set_btregs=ipci165_set_btregs;
  chip->chipspecops->start_chip=ipci165_start_chip;
  chip->chipspecops->stop_chip=ipci165_stop_chip;
  chip->chipspecops->pre_read_config=ipci165_pre_read_config;
  chip->chipspecops->wakeup_tx=ipci165_wakeup_tx;
  chip->chipspecops->filtch_rq=NULL;
  chip->chipspecops->irq_accept=NULL;

  chip->chipspecops->standard_mask=NULL;
  chip->chipspecops->extended_mask=NULL;
  chip->chipspecops->message15_mask=NULL;
  chip->chipspecops->clear_objects=NULL;
  chip->chipspecops->config_irqs=NULL;
  chip->chipspecops->pre_write_config=NULL;
  chip->chipspecops->send_msg=NULL;
  chip->chipspecops->check_tx_stat=NULL;
  chip->chipspecops->remote_request=NULL;
  chip->chipspecops->enable_configuration=NULL;
  chip->chipspecops->disable_configuration=NULL;

  return 0;
}

/**
 * ipci165_init_obj_data - Initialize message buffers
 * @chip: Pointer to chip specific structure
 * @objnr: Number of the message buffer
 *
 * Return Value: The function always returns zero
 * File: src/ipci165.c
 */
int ipci165_init_obj_data(struct canchip_t *chip, int objnr)
{
  struct msgobj_t *obj=chip->msgobj[objnr];
  
  DEBUGMSG ("ipci165_init_obj_data\n");
  
  obj->obj_base_addr = 0; /* not known yet */
  obj->tx_timeout.function = ipci165_do_tx_timeout;
  obj->tx_timeout.data = (unsigned long)obj;
  return 0;
}

/**
 * ipci165_program_irq - Program interrupts
 * @candev: Pointer to candevice/board structure
 *
 * Return value: The function returns zero on success or %-ENODEV on failure
 * File: src/ipci165.c
 */
int ipci165_program_irq(struct candevice_t *candev)
{
  return 0;
}

/**
 * ipci165_register - Register Board Support Functions
 * @candev: Pointer to hardware/board specific functions
 *
 * Return value: The function returns zero on success or %-ENODEV on failure
 * File: src/ipci165.c
 */
int ipci165_register(struct hwspecops_t *hwspecops)
{
  hwspecops->request_io = ipci165_request_io;
  hwspecops->release_io = ipci165_release_io;
  hwspecops->reset = ipci165_reset;
  hwspecops->init_hw_data = ipci165_init_hw_data;
  hwspecops->init_chip_data = ipci165_init_chip_data;
  hwspecops->init_obj_data = ipci165_init_obj_data;
  hwspecops->write_register = NULL;
  hwspecops->read_register = NULL;
  hwspecops->program_irq = ipci165_program_irq;
  return 0;
}

#ifdef CAN_DEBUG
void dump_mem(char *ptr, int size)
{
  int to, j;
  unsigned char str[80], buf[16];
  char *strp;

  for (; size > 0; size -= 16)
  {
    to = size > 16 ? 16 : size;
    memcpy (buf,ptr, to);
    strp = str;
    for (j = 0; j < to ; j++)
      strp += sprintf(strp, "%02x ",buf[j]);
    for (; j < 16 ; j++) 
      strp += sprintf(strp, "   ");
    for (j = 0; j < to ; j++)
      *strp++= isprint(buf[j]) ? buf[j] : '.';

    DEBUGMSG ("[%lx] %s\n", (long unsigned)ptr, str);
    ptr += to;
  }
}
#endif
