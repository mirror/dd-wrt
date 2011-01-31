/* 
 *	oam.h - OAM cell processing
 *
 *	Written 2003 by Jorge Boncompte, DTI2
 *
*/

#ifndef OAM_H
#define OAM_H

#define OAM_UPRETRY	3	
#define OAM_DOWNRETRY	5
#define OAM_WAITRETRY	1

#define OAM_INIT	0
#define OAM_SENT	2
#define OAM_REPLY	3
#define OAM_WAIT10	4

#define VCC_UP		0
#define VCC_DOWN	1

#define OAM_INC_UPRETRY		fsm.upretry++; fsm.downretry=0
#define OAM_INC_DOWNRETRY	fsm.downretry++; fsm.upretry=0
#define OAM_WAIT_10_S		timer = 0; fsm.state = OAM_WAIT10

struct oamfsm {
	int state;
	short upretry;
	short downretry;
};

struct oamvcc {
	struct atmoam_ctrl ctrl;
	int state;
	unsigned long CTag;
	struct {
	    unsigned long InOAM;
	    unsigned long OutOAM;
	    unsigned long CrcErrors;
	    unsigned long CTagErrors;
	    unsigned long OutDrops;
	    struct {
		unsigned long InEndLoop;
		unsigned long InSegLoop;
//		unsigned long InAIS;
//		unsigned long InRDI;
		unsigned long OutEndLoop;
		unsigned long OutSegLoop;
//	        unsigned long OutRDI;
	    } F5;
	} stats;
};

void oam_process(struct atmoam_ctrl *ctrl);
void oam_fsm(void);
void oam_state_print(void);

#endif
