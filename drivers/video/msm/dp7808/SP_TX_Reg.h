// ---------------------------------------------------------------------------
// Analogix Confidential Strictly Private
//
//
// ---------------------------------------------------------------------------
// >>>>>>>>>>>>>>>>>>>>>>>>> COPYRIGHT NOTICE <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// ---------------------------------------------------------------------------
// Copyright 2004-2012 (c) Analogix 
//
//Analogix owns the sole copyright to this software. Under international
// copyright laws you (1) may not make a copy of this software except for
// the purposes of maintaining a single archive copy, (2) may not derive
// works herefrom, (3) may not distribute this work to others. These rights
// are provided for information clarification, other restrictions of rights
// may apply as well.
//
// This is an unpublished work.
// ---------------------------------------------------------------------------
// >>>>>>>>>>>>>>>>>>>>>>>>>>>> WARRANTEE <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// ---------------------------------------------------------------------------
// Analogix  MAKES NO WARRANTY OF ANY KIND WITH REGARD TO THE USE OF
// THIS SOFTWARE, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
// PURPOSE.
// ---------------------------------------------------------------------------



#define SP_TX_PORT0_ADDR 0x70
#define SP_TX_PORT1_ADDR 0x7A
#define SP_TX_PORT2_ADDR 0x72

#define HDMI_PORT0_ADDR 0x7e
#define HDMI_PORT1_ADDR 0x80


#ifndef _SP_TX_Reg_def_H
#define _SP_TX_Reg_def_H
/***************************************************************/
//  DEV_ADDR = 0x7e , HDMI port register
#define HDMI_RX_PORT_SEL_REG 0x10		
#define HDMI_RX_DDC0_EN 0x10
#define HDMI_RX_TMDS0_EN 0x01

#define HDMI_RX_SRST_REG 0x11
#define HDMI_RX_VIDEO_RST 0x10		
#define HDMI_RX_HDCP_MAN_RST 0X04
#define HDMI_RX_TMDS_RST 0X02
#define HDMI_RX_SW_MAN_RST 0X01

#define HDMI_RX_SRST2_REG 0X12
#define HDMI_RX_VFIFO_RST 0X08
#define HDMI_RX_DDC_RST 	0X04
#define HDMI_RX_MISC_RST  0X02
#define HDMI_RX_HW_RST		0X01

#define HDMI_RX_SARST_REG 0X13
#define HDMI_RX_DDC_RST_CTRL 0X10  		
#define HDMI_RX_HDCP_RST_CTRL 0X02
#define HDMI_RX_SW_RST_CTRL 	0X01

#define HDMI_RX_SYS_STATUS_REG			0X14
#define HDMI_RX_P0_PWR5V	0X80
#define HDMI_RX_PWR5V			0X08
#define HDMI_RX_VSYNC			0X04
#define HDMI_RX_CLK_DET		0X02
#define HDMI_RX_HDMI_DET  0X01

#define HDMI_RX_HDMI_STATUS_REG	0X15
#define HDMI_RX_DEEP_COLOR_MODE 0X40
#define HDMI_RX_HDMI_AUD_LAYOUT 0X08
#define HDMI_RX_MUTE_STAT				0X04
#define HDMI_RX_HDMI_MODE				0X01

#define HDMI_RX_HDMI_MUTE_CTRL_REG 0X16
#define HDMI_RX_MUTE_POL	0X04
#define HDMI_RX_AUD_MUTE	0X02
#define HDMI_RX_VID_MUTE	0X01

#define HDMI_RX_SYS_CTRL1_REG 0X17


#define HDMI_RX_SYS_PWDN1_REG 0X18
#define HDMI_RX_PWDN_CTRL	0X01

#define HDMI_RX_SYS_PWDN2_REG 0X19
#define HDMI_RX_PD_PCLK 0X80

#define HDMI_RX_SYS_PWDN3_REG 0X1A
#define HDMI_RX_PD_ALL  0X01
#define HDMI_RX_PIO_CTRL 0X1B
#define HDMI_RX_PIO_TEST_CLOCK_EN  0X02

#define HDMI_RX_CLK_CTRL_REG 0X1C
#define HDMI_RX_CLK_AUTO_SWITCH 0X01

#define HDMI_RX_AEC_CTRL_REG 0X20	
#define HDMI_RX_AVC_OE 0x80
#define HDMI_RX_AAC_OE 0X40
#define HDMI_RX_AVC_EN 0X02
#define HDMI_RX_AAC_EN 0X01

#define HDMI_RX_AEC_EN0_REG 0X24 //0XB6
#define HDMI_RX_AEC_EN07 0X80
#define HDMI_RX_AEC_EN06 0X40
#define HDMI_RX_AEC_EN05 0X20
#define HDMI_RX_AEC_EN04 0X10
#define HDMI_RX_AEC_EN03 0X08
#define HDMI_RX_AEC_EN02 0X04
#define HDMI_RX_AEC_EN01 0X02
#define HDMI_RX_AEC_EN00 0X01

#define HDMI_RX_AEC_EN1_REG 0X25
#define HDMI_RX_AEC_EN15 0X80
#define HDMI_RX_AEC_EN14 0X40
#define HDMI_RX_AEC_EN13 0X20
#define HDMI_RX_AEC_EN12 0X10
#define HDMI_RX_AEC_EN11 0X08
#define HDMI_RX_AEC_EN10 0X04
#define HDMI_RX_AEC_EN09 0X02
#define HDMI_RX_AEC_EN08 0X01

#define HDMI_RX_AEC_EN2_REG 0X26
#define HDMI_RX_AEC_EN23 0X80
#define HDMI_RX_AEC_EN22 0X40
#define HDMI_RX_AEC_EN21 0X20
#define HDMI_RX_AEC_EN20 0X10
#define HDMI_RX_AEC_EN19 0X08
#define HDMI_RX_AEC_EN18 0X04
#define HDMI_RX_AEC_EN17 0X02
#define HDMI_RX_AEC_EN16 0X01

#define HDMI_RX_INT_STATE_REG 0X30
#define HDMI_RX_INTR 0X01

#define HDMI_RX_INT_STATUS1_REG 0X31
#define HDMI_RX_HDMI_DVI 0X80
#define HDMI_RX_CKDT_CHANGE 0X40
#define HDMI_RX_SCDT_CHANGE 0X20
#define HDMI_RX_PCLK_CHANGE 0X10
#define HDMI_RX_PLL_UNLOCK  0X08
#define HDMI_RX_CABLE_UNPLUG 0X04
#define HDMI_RX_SET_MUTE 0X02
#define HDMI_RX_SW_INTR 0X01

#define HDMI_RX_INT_STATUS2_REG 0X32
#define HDMI_RX_AUTH_START 0X80
#define HDMI_RX_AUTH_DONE 0X40
#define HDMI_RX_HDCP_ERR 0X20
#define HDMI_RX_ECC_ERR 0X10


#define HDMI_RX_INT_STATUS3_REG 0X33
#define HDMI_RX_AUD_MODE_CHANGE 0X01

#define HDMI_RX_INT_STATUS4_REG 0X34
#define HDMI_RX_VSYNC_DET 0X80
#define HDMI_RX_SYNC_POL_CHANGE 0X40
#define HDMI_RX_V_RES_CHANGE 0X20
#define HDMI_RX_H_RES_CHANGE 0X10
#define HDMI_RX_I_P_CHANGE 0X08
#define HDMI_RX_DP_CHANGE 0X04
#define HDMI_RX_COLOR_DEPTH_CHANGE 0X02
#define HDMI_RX_COLOR_MODE_CHANGE 0X01

#define HDMI_RX_INT_STATUS5_REG 0X35
#define HDMI_RX_VFIFO_OVERFLOW 0X80
#define HDMI_RX_VFIFO_UNDERFLOW 0X40
#define HDMI_RX_CTS_N_ERR 0X08
#define HDMI_RX_NO_AVI 0X02
#define HDMI_RX_AUDIO_RCV 0X01

#define HDMI_RX_INT_STATUS6_REG 0X36
#define HDMI_RX_CTS_RCV 0X80
#define HDMI_RX_NEW_UNR_PKT 0X40
#define HDMI_RX_NEW_MPEG 0X20
#define HDMI_RX_NEW_AUD 0X10
#define HDMI_RX_NEW_SPD 0X08
#define HDMI_RX_NEW_ACP 0X04
#define HDMI_RX_NEW_AVI 0X02
#define HDMI_RX_NEW_CP  0X01

#define HDMI_RX_INT_STATUS7_REG 0X37
#define HDMI_RX_NO_VSI 0X80
#define HDMI_RX_HSYNC_DET 0X20
#define HDMI_RX_NEW_VS 0X10
#define HDMI_RX_NO_ACP 0X08
#define HDMI_RX_REF_CLK_CHG 0X04
#define HDMI_RX_CEC_RX_READY 0X02
#define HDMI_RX_CEC_TX_DONE 0X01

#define HDMI_RX_ECC_CTRL_REG 0X3E

#define HDMI_RX_PKT_RX_INDU_INT_CTRL 0X3F
#define HDMI_RX_NEW_VS_CTRL 0X80
#define HDMI_RX_NEW_UNR 0X40
#define HDMI_RX_NEW_MPEG 0X20
#define HDMI_RX_NEW_AUD 0X10
#define HDMI_RX_NEW_SPD 0X08
#define HDMI_RX_NEW_ACP 0X04
#define HDMI_RX_NEW_AVI 0X02

#define HDMI_RX_INT_CTRL_REG 0X40
#define HDMI_RX_SOFT_INTR 0X04
#define HDMI_RX_INTR_TYPE 0X02
#define HDMI_RX_INTR_POL  0X01

#define HDMI_RX_INT_MASK1_REG 0X41
#define HDMI_RX_INT_MASK2_REG 0X42
#define HDMI_RX_INT_MASK3_REG 0X43
#define HDMI_RX_INT_MASK4_REG 0X44
#define HDMI_RX_INT_MASK5_REG 0X45
#define HDMI_RX_INT_MASK6_REG 0X46
#define HDMI_RX_INT_MASK7_REG 0X47

#define HDMI_RX_TMDS_CTRL_REG1 0X50
#define HDMI_RX_TMDS_CTRL_REG2 0X51
#define HDMI_RX_TMDS_CTRL_REG3 0X52
#define HDMI_RX_TMDS_CTRL_REG4 0X53
#define HDMI_RX_TMDS_CTRL_REG5 0X54
#define HDMI_RX_TMDS_CTRL_REG6 0X55


#define HDMI_RX_TMDS_CTRL_REG7 0X56
#define HDMI_RX_TMDS_CTRL_REG8 0X57
#define HDMI_RX_TMDS_CTRL_REG9 0X58
#define HDMI_RX_TMDS_CTRL_REG10 0X59
#define HDMI_RX_TMDS_CTRL_REG11 0X5a
#define HDMI_RX_TMDS_CTRL_REG12 0X5b
#define HDMI_RX_TMDS_CTRL_REG13 0X5c
#define HDMI_RX_TMDS_CTRL_REG14 0X5d
#define HDMI_RX_TMDS_CTRL_REG15 0X5e
#define HDMI_RX_TMDS_CTRL_REG16 0X5f
#define HDMI_RX_TMDS_CTRL_REG17 0X60
#define HDMI_RX_TMDS_CTRL_REG18 0X61
#define HDMI_RX_TMDS_CTRL_REG19 0X62
#define HDMI_RX_TMDS_CTRL_REG20 0X63
#define HDMI_RX_TMDS_CTRL_REG21 0X64
#define HDMI_RX_TMDS_CTRL_REG22 0X65
#define HDMI_RX_TMDS_CTRL_REG23 0X66
#define HDMI_RX_TMDS_CTRL_REG24 0X67


#define HDMI_RX_VIDEO_STATUS_REG1 0X70
#define HDMI_RX_DEFAULT_PHASE 0X08
#define HDMI_RX_VIDEO_TYPE 0X04
#define HDMI_RX_VSYNC_POL 0X02
#define HDMI_RX_HSYNC_POL 0X01

#define HDMI_RX_HTOTAL_LOW 0X71
#define HDMI_RX_HTOTAL_HIGH 0X72
#define HDMI_RX_VTOTAL_LOW 0X73
#define HDMI_RX_VTOTAL_HIGH 0X74

#define HDMI_RX_HACT_LOW 0X75
#define HDMI_RX_HACT_HIGH 0X76
#define HDMI_RX_VACT_LOW 0X77
#define HDMI_RX_VACT_HIGH 0X78

#define HDMI_RX_V_SYNC_WIDTH 0X79
#define HDMI_RX_V_BACK_PORCH 0X7A
#define HDMI_RX_H_FRONT_PORCH_LOW 0X7B
#define HDMI_RX_H_FRONT_PORCH_HIGH 0X7C

#define HDMI_RX_H_SYNC_WIDTH_LOW 0X7D
#define HDMI_RX_H_SYNC_WIDTH_HIGH 0X7E

#define HDMI_RX_VIDEO_STATUS_REG2 0X7F

#define HDMI_RX_VIDEO_MODE1_REG 0X80
#define HDMI_RX_CSC_BT709_EN 0X10
#define HDMI_RX_RANGE_Y2R 0X08
#define HDMI_RX_CSPACE_Y2R 0X04
#define HDMI_RX_RANGE_R2Y 0X02
#define HDMI_RX_CSPACE_R2Y 0X01

#define HDMI_RX_VIDEO_MODE2_REG 0X81
#define HDMI_RX_VID_DS_MODE 0X08
#define HDMI_RX_VID_US_MODE 0X04
#define HDMI_RX_UP_SAMPLE 0X02
#define HDMI_RX_DOWN_SAMPLE 0X01

#define HDMI_RX_VID_PROCESSING_CTRL_REG 0X82
#define HDMI_RX_BRU_EN 0X80
#define HDMI_RX_BRU_RND_DIR 0X10
#define HDMI_RX_BACK_PORCH_MODE 0X04
#define HDMI_RX_DE_DELAY 0X02
#define HDMI_RX_INV_FIELD 0X01

#define HDMI_RX_VID_DATA_RNG_CTRL_REG 0X83
#define HDMI_RX_YC_LIMT 0X10
#define HDMI_RX_OUTPUT_LIMIT_EN 0X08
#define HDMI_RX_OUTPUT_LIMIT_RANGE 0X04
#define HDMI_RX_R2Y_INPUT_LIMIT 0X02
#define HDMI_RX_XVYCC_LIMIT 0X01

#define HDMI_RX_VID_OUTPUT_CTRL1_REG 0X84
#define HDMI_RX_INV_VSYNC 0X80
#define HDMI_RX_INV_HSYNC 0X40
#define HDMI_RX_CLK48B_POL 0X20
#define HDMI_RX_EMBED_SYNC 0X02
#define HDMI_RX_MUX_YC 0X01

#define HDMI_RX_VID_OUTPUT_CTRL2_REG 0X85
#define HDMI_RX_BIT_SWAP 0X08
#define HDMI_RX_BUS_MODE 0X04
#define HDMI_RX_O_YC422 0X02
#define HDMI_RX_O_YCBCR 0X01

#define HDMI_RX_VID_OUTPUT_CTRL3_REG 0X86


#define HDMI_RX_CH1_VID_BLANK_REG 0X87
#define HDMI_RX_CH2_VID_BLANK_REG 0X88
#define HDMI_RX_CH3_VID_BLANK_REG 0X89

#define HDMI_RX_VID_CH_MAP_REG 0X8A

#define HDMI_RX_VID_PCLK_CNTR_REG 0X8B

#define HDMI_RX_TMDS_CH0_SYNC_STATUS 0X90
#define HDMI_RX_TMDS_CH1_SYNC_STATUS 0X91
#define HDMI_RX_TMDS_CH2_SYNC_STATUS 0X92
#define HDMI_RX_TMDS_CH_ALIGN_STATUS 0X93
#define HDMI_RX_TMDS_PLL_PCLK_STATUS 0X94
#define HDMI_RX_TMDS_CLK_DEBUG_REG 0X95


#define HDMI_RX_ACR_N_HARDWARE_VALUE1_REG 0XA6
#define HDMI_RX_ACR_N_HARDWARE_VALUE2_REG 0XA7
#define HDMI_RX_ACR_N_HARDWARE_VALUE3_REG 0XA8

#define HDMI_RX_ACR_CTS_HARDWARE_VALUE1_REG 0XAC
#define HDMI_RX_ACR_CTS_HARDWARE_VALUE2_REG 0XAD
#define HDMI_RX_ACR_CTS_HARDWARE_VALUE3_REG 0XAE

#define HDMI_RX_AUD_IN_SPDIF_CH_STATUS1_REG 0XC7
#define HDMI_RX_SW_CPRGT 0X04
#define HDMI_RX_NON_PCM 0X02
#define HDMI_RX_PROF_APP 0X01

#define HDMI_RX_AUD_IN_SPDIF_CH_STATUS2_REG 0XC8
#define HDMI_RX_AUD_IN_SPDIF_CH_STATUS3_REG 0XC9
#define HDMI_RX_AUD_IN_SPDIF_CH_STATUS4_REG 0XCA
#define HDMI_RX_AUD_IN_SPDIF_CH_STATUS5_REG 0XCB


#define HDMI_RX_CEC_CTRL_REG 0XD0
#define HDMI_RX_CEC_RX_EN 0X08
#define HDMI_RX_CEC_TX_ST 0X04
#define HDMI_RX_CEC_PIN_SEL 0X02
#define HDMI_RX_CEC_RST 0X01

#define HDMI_RX_CEC_RX_STATUS_REG 0XD1
#define HDMI_RX_CEC_RX_BUSY 0X80
#define HDMI_RX_CEC_RX_FULL 0X20
#define HDMI_RX_CEC_RX_EMP 0X10

#define HDMI_RX_CEC_TX_STATUS_REG 0XD2
#define HDMI_RX_CEC_TX_BUSY 0X80
#define HDMI_RX_CEC_TX_FAIL 0X40
#define HDMI_RX_CEC_TX_FULL 0X20
#define HDMI_RX_CEC_TX_EMP 0X10

#define HDMI_RX_CEC_FIFO_REG 0XD3

#define HDMI_RX_CEC_SPEED_CTRL_REG 0XD4

#define HDMI_RX_HDMI_BIST_CTRL 0XDE
#define HDMI_RX_VID_BIST_EN   0X80
#define HDMI_RX_BIST_ERR_ST   0X40
#define HDMI_RX_AUD_BIST_EN   0X20

#define HDMI_RX_TMDS_PRBS_TEST_REG 0XE0
#define HDMI_RX_PRBS_TEST_EN 0X80

#define HDMI_RX_HDMI_CRITERIA_REG 0XE1

#define HDMI_RX_HDCP_EN_CRITERIA_REG 0XE2
#define HDMI_RX_ENC_EN_MODE 0X20

#define HDMI_RX_CHIP_CTRL_REG 0XE3
#define HDMI_RX_MAN_HDMI5V_DET 0X08
#define HDMI_RX_PLLLOCK_CKDT_EN 0X04
#define HDMI_RX_ANALOG_CKDT_EN 0X02
#define HDMI_RX_DIGITAL_CKDT_EN 0X01

#define HDMI_RX_HARDWARE_REG 0XE4
#define HDMI_RX_MAN_OSCCLK_DET 0X80
#define HDMI_RX_CHIP_DEBUG_SEL 0X40
#define HDMI_RX_CHIP_DEBUG_EN 0X20

#define HDMI_RX_PACKET_DECODE_CTRL_REG 0XE8
#define HDMI_RX_AUTO_STOP_DEC_ON_HDCPERR 0X02
#define HDMI_RX_STOP_DEC 0X01

#define HDMI_RX_HDCP_KEY_STATUS_REG 0XE9
#define HDMI_RX_BIST_ERR 0X02

#define HDMI_RX_HDCP_KEY_COMMAND_REG 0XEA 

#define HDMI_RX_DEEP_COLOR_DEBUG_REG 0XEB
#define HDMI_RX_VFIFO_OVERFLOW 0X80
#define HDMI_RX_VFIFO_UNDERFLOW 0X40
#define HDMI_RX_VFIFO_TMDS_PHASE_MATCH 0X20
#define HDMI_RX_VFIFO_SYNC_ERR 0X10

#define HDMI_RX_DEEP_COLOR_CTRL_REG 0XEC
#define HDMI_RX_EN_MAN_DEEP_COLOR 0X08

#define HDMI_RX_VID_FIFO_CTRL 0XED
#define HDMI_RX_MAN_DEF_PHASE 0X08
#define HDMI_RX_MAN_DEF_PHASE_VAL 0X04

#define HDMI_RX_ECC_ERR_CTRL 0XF9

#define HDMI_RX_EEPROM_ACCESS_CTRL 0XFC



//End for DEV_addr 0x7E
/***************************************************************/
//  DEV_ADDR = 0x80 , HDMI PORT1  and HDCP registers

#define HDMI_RX_HDCP_BKSV1 0X00
#define HDMI_RX_HDCP_BKSV2 0X01
#define HDMI_RX_HDCP_BKSV3 0X02
#define HDMI_RX_HDCP_BKSV4 0X03
#define HDMI_RX_HDCP_BKSV5 0X04

#define HDMI_RX_HDCP_RI1 0X05
#define HDMI_RX_HDCP_RI2 0X06

#define HDMI_RX_HDCP_PJ 0X07
#define HDMI_RX_HDCP_AKSV1 0X08
#define HDMI_RX_HDCP_AKSV2 0X09
#define HDMI_RX_HDCP_AKSV3 0X0A
#define HDMI_RX_HDCP_AKSV4 0X0B
#define HDMI_RX_HDCP_AKSV5 0X0C

#define HDMI_RX_HDCP_AINFO 0X0D
#define HDMI_RX_11EN 0X02

#define HDMI_RX_HDCP_AN1 0X0E
#define HDMI_RX_HDCP_AN2 0X0F
#define HDMI_RX_HDCP_AN3 0X10
#define HDMI_RX_HDCP_AN4 0X11
#define HDMI_RX_HDCP_AN5 0X12
#define HDMI_RX_HDCP_AN6 0X13
#define HDMI_RX_HDCP_AN7 0X14
#define HDMI_RX_HDCP_AN8 0X15

#define HDMI_RX_DS_VH0_0_REG 0X16
#define HDMI_RX_DS_VH0_1_REG 0X17
#define HDMI_RX_DS_VH0_2_REG 0X18
#define HDMI_RX_DS_VH0_3_REG 0X19
#define HDMI_RX_DS_VH1_0_REG 0X1A
#define HDMI_RX_DS_VH1_1_REG 0X1B
#define HDMI_RX_DS_VH1_2_REG 0X1C
#define HDMI_RX_DS_VH1_3_REG 0X1D
#define HDMI_RX_DS_VH2_0_REG 0X1E
#define HDMI_RX_DS_VH2_1_REG 0X1F
#define HDMI_RX_DS_VH2_2_REG 0X20
#define HDMI_RX_DS_VH2_3_REG 0X21
#define HDMI_RX_DS_VH3_0_REG 0X22
#define HDMI_RX_DS_VH3_1_REG 0X23
#define HDMI_RX_DS_VH3_2_REG 0X24
#define HDMI_RX_DS_VH3_3_REG 0X25
#define HDMI_RX_DS_VH4_0_REG 0X26
#define HDMI_RX_DS_VH4_1_REG 0X27
#define HDMI_RX_DS_VH4_2_REG 0X28
#define HDMI_RX_DS_VH4_3_REG 0X29

#define HDMI_RX_HDCP_BCAPS 0X2A
#define HDMI_RX_HDMI_CAP 0X80
#define HDMI_RX_REPERTER 0X40
#define HDMI_RX_READY 0X20
#define HDMI_RX_FAST 0X10
#define HDMI_RX_11FEATURE 0X02
#define HDMI_RX_FAST_REAUTH 0X01

#define HDMI_RX_HDCP_BSTATUS1 0X2B
#define HDMI_RX_DEV_EXC 0X80

#define HDMI_RX_HDCP_BSTATUS2 0X2C
#define HDMI_RX_HDMI_MODE_IN_BSTATUS 0X10
#define HDMI_RX_CAS_EXC 0X08

#define HDMI_RX_HDCP_REPEATER_KSV_FIFO 0X2D

#define HDMI_RX_DEVICE_IDENTIFICATION 0X2E

#define HDMI_RX_HDCP_KSV_SHA_STARTL_REG 0X2F
#define HDMI_RX_HDCP_KSV_SHA_STARTH_REG 0X30

#define HDMI_RX_HDCP_SHA_LENGTHL_REG 0X31
#define HDMI_RX_HDCP_SHA_LENGTHH_REG 0X32

#define HDMI_RX_HDCP_SHA_CTRL_REG 0X33
#define HDMI_RX_SHA_MODE 0X02
#define HDMI_RX_SHA_GO 0X01

#define HDMI_RX_DS_BSTATUSL_REG 0X34
#define HDMI_RX_DS_BSTATUSH_REG 0X35

#define HDMI_RX_DS_M0_0_REG 0X36
#define HDMI_RX_DS_M0_1_REG 0X37
#define HDMI_RX_DS_M0_2_REG 0X38
#define HDMI_RX_DS_M0_3_REG 0X39
#define HDMI_RX_DS_M0_4_REG 0X3A
#define HDMI_RX_DS_M0_5_REG 0X3B
#define HDMI_RX_DS_M0_6_REG 0X3C
#define HDMI_RX_DS_M0_7_REG 0X3D

#define HDMI_RX_HDCP_DEBUG_REG 0X3E
#define HDMI_RX_STOPHDCP 0X80
#define HDMI_RX_BYPASSHDCP 0X40
#define HDMI_RX_DDC_DLY_EN 0X10

#define HDMI_RX_HDCP_STATUS_REG 0X3F
#define HDMI_RX_ADV_CIPHER 0X80
#define HDMI_RX_LOAD_KEY_DONE 0X40
#define HDMI_RX_DECRYPT 0X20
#define HDMI_RX_AUTHEN 0X10
#define HDMI_RX_BKSV_DISABLE 0X02
#define HDMI_RX_CLEAR_RI 0X01

#define HDMI_RX_SPD_TYPE_REG 0X40
#define HDMI_RX_SPD_VER_REG 0X41
#define HDMI_RX_SPD_LEN_REG 0X42
#define HDMI_RX_SPD_CHKSUM_REG 0X43
#define HDMI_RX_SPD_DATA00_REG 0X44
#define HDMI_RX_SPD_DATA01_REG 0X45
#define HDMI_RX_SPD_DATA02_REG 0X46
#define HDMI_RX_SPD_DATA03_REG 0X47
#define HDMI_RX_SPD_DATA04_REG 0X48
#define HDMI_RX_SPD_DATA05_REG 0X49
#define HDMI_RX_SPD_DATA06_REG 0X4A
#define HDMI_RX_SPD_DATA07_REG 0X4B
#define HDMI_RX_SPD_DATA08_REG 0X4C
#define HDMI_RX_SPD_DATA09_REG 0X4D
#define HDMI_RX_SPD_DATA0A_REG 0X4E
#define HDMI_RX_SPD_DATA0B_REG 0X4F
#define HDMI_RX_SPD_DATA0C_REG 0X50
#define HDMI_RX_SPD_DATA0D_REG 0X51
#define HDMI_RX_SPD_DATA0E_REG 0X52
#define HDMI_RX_SPD_DATA0F_REG 0X53
#define HDMI_RX_SPD_DATA10_REG 0X54
#define HDMI_RX_SPD_DATA11_REG 0X55
#define HDMI_RX_SPD_DATA12_REG 0X56
#define HDMI_RX_SPD_DATA13_REG 0X57
#define HDMI_RX_SPD_DATA14_REG 0X58
#define HDMI_RX_SPD_DATA15_REG 0X59
#define HDMI_RX_SPD_DATA16_REG 0X5A
#define HDMI_RX_SPD_DATA17_REG 0X5B
#define HDMI_RX_SPD_DATA18_REG 0X5C
#define HDMI_RX_SPD_DATA19_REG 0X5D
#define HDMI_RX_SPD_DATA1A_REG 0X5E

#define HDMI_RX_ACP_HB0_REG 0X60
#define HDMI_RX_ACP_HB1_REG 0X61
#define HDMI_RX_ACP_HB2_REG 0X62
#define HDMI_RX_ACP_DATA00_REG 0X63
#define HDMI_RX_ACP_DATA01_REG 0X64
#define HDMI_RX_ACP_DATA02_REG 0X65
#define HDMI_RX_ACP_DATA03_REG 0X66
#define HDMI_RX_ACP_DATA04_REG 0X67
#define HDMI_RX_ACP_DATA05_REG 0X68
#define HDMI_RX_ACP_DATA06_REG 0X69
#define HDMI_RX_ACP_DATA07_REG 0X6A
#define HDMI_RX_ACP_DATA08_REG 0X6B
#define HDMI_RX_ACP_DATA09_REG 0X6C
#define HDMI_RX_ACP_DATA0A_REG 0X6D
#define HDMI_RX_ACP_DATA0B_REG 0X6E
#define HDMI_RX_ACP_DATA0C_REG 0X6F
#define HDMI_RX_ACP_DATA0D_REG 0X70
#define HDMI_RX_ACP_DATA0E_REG 0X71
#define HDMI_RX_ACP_DATA0F_REG 0X72
#define HDMI_RX_ACP_DATA10_REG 0X73
#define HDMI_RX_ACP_DATA11_REG 0X74
#define HDMI_RX_ACP_DATA12_REG 0X75
#define HDMI_RX_ACP_DATA13_REG 0X76
#define HDMI_RX_ACP_DATA14_REG 0X77
#define HDMI_RX_ACP_DATA15_REG 0X78
#define HDMI_RX_ACP_DATA16_REG 0X79
#define HDMI_RX_ACP_DATA17_REG 0X7A
#define HDMI_RX_ACP_DATA18_REG 0X7B
#define HDMI_RX_ACP_DATA19_REG 0X7C
#define HDMI_RX_ACP_DATA1A_REG 0X7D
#define HDMI_RX_ACP_DATA1B_REG 0X7E

#define HDMI_RX_UNRECOGNIZED_PACKET_REG 0X80

#define HDMI_RX_AVI_TYPE_REG 0XA0
#define HDMI_RX_AVI_VER_REG 0XA1
#define HDMI_RX_AVI_LEN_REG 0XA2
#define HDMI_RX_AVI_CHKSUM_REG 0XA3
#define HDMI_RX_AVI_DATA00_REG 0XA4
#define HDMI_RX_AVI_DATA01_REG 0XA5
#define HDMI_RX_AVI_DATA02_REG 0XA6
#define HDMI_RX_AVI_DATA03_REG 0XA7
#define HDMI_RX_AVI_DATA04_REG 0XA8
#define HDMI_RX_AVI_DATA05_REG 0XA9
#define HDMI_RX_AVI_DATA06_REG 0XAA
#define HDMI_RX_AVI_DATA07_REG 0XAB
#define HDMI_RX_AVI_DATA08_REG 0XAC
#define HDMI_RX_AVI_DATA09_REG 0XAD
#define HDMI_RX_AVI_DATA0A_REG 0XAE
#define HDMI_RX_AVI_DATA0B_REG 0XAF
#define HDMI_RX_AVI_DATA0C_REG 0XB0
#define HDMI_RX_AVI_DATA0D_REG 0XB1
#define HDMI_RX_AVI_DATA0E_REG 0XB2

#define HDMI_RX_AUDIO_TYPE_REG 0XC0
#define HDMI_RX_AUDIO_VER_REG 0XC1
#define HDMI_RX_AUDIO_LEN_REG 0XC2
#define HDMI_RX_AUDIO_CHKSUM_REG 0XC3
#define HDMI_RX_AUDIO_DATA00_REG 0XC4
#define HDMI_RX_AUDIO_DATA01_REG 0XC5
#define HDMI_RX_AUDIO_DATA02_REG 0XC6
#define HDMI_RX_AUDIO_DATA03_REG 0XC7
#define HDMI_RX_AUDIO_DATA04_REG 0XC8
#define HDMI_RX_AUDIO_DATA05_REG 0XC9
#define HDMI_RX_AUDIO_DATA06_REG 0XCA
#define HDMI_RX_AUDIO_DATA07_REG 0XCB
#define HDMI_RX_AUDIO_DATA08_REG 0XCC
#define HDMI_RX_AUDIO_DATA09_REG 0XCD
#define HDMI_RX_AUDIO_DATA0A_REG 0XCE
#define HDMI_RX_AUDIO_DATA0B_REG 0XCF
#define HDMI_RX_AUDIO_DATA0C_REG 0XD0
#define HDMI_RX_AUDIO_DATA0D_REG 0XD1
#define HDMI_RX_AUDIO_DATA0E_REG 0XD2
#define HDMI_RX_AUDIO_DATA0F_REG 0XD3
#define HDMI_RX_AUDIO_DATA10_REG 0XD4
#define HDMI_RX_AUDIO_DATA11_REG 0XD5
#define HDMI_RX_AUDIO_DATA12_REG 0XD6
#define HDMI_RX_AUDIO_DATA13_REG 0XD6
#define HDMI_RX_AUDIO_DATA14_REG 0XD7
#define HDMI_RX_AUDIO_DATA15_REG 0XD9
#define HDMI_RX_AUDIO_DATA16_REG 0XDA
#define HDMI_RX_AUDIO_DATA17_REG 0XDB
#define HDMI_RX_AUDIO_DATA18_REG 0XDC
#define HDMI_RX_AUDIO_DATA19_REG 0XDD

#define HDMI_RX_MPEG_TYPE_REG 0XE0
#define HDMI_RX_MPEG_VER_REG 0XE1
#define HDMI_RX_MPEG_LEN_REG 0XE2
#define HDMI_RX_MPEG_CHKSUM_REG 0XE3
#define HDMI_RX_MPEG_DATA00_REG 0XE4
#define HDMI_RX_MPEG_DATA01_REG 0XE5
#define HDMI_RX_MPEG_DATA02_REG 0XE6
#define HDMI_RX_MPEG_DATA03_REG 0XE7
#define HDMI_RX_MPEG_DATA04_REG 0XE8
#define HDMI_RX_MPEG_DATA05_REG 0XE9
#define HDMI_RX_MPEG_DATA06_REG 0XEA
#define HDMI_RX_MPEG_DATA07_REG 0XEB
#define HDMI_RX_MPEG_DATA08_REG 0XEC
#define HDMI_RX_MPEG_DATA09_REG 0XED
#define HDMI_RX_MPEG_DATA0A_REG 0XEF
#define HDMI_RX_MPEG_DATA0B_REG 0XF0
#define HDMI_RX_MPEG_DATA0C_REG 0XF1
#define HDMI_RX_MPEG_DATA0D_REG 0XF2
#define HDMI_RX_MPEG_DATA0E_REG 0XF3
#define HDMI_RX_MPEG_DATA0F_REG 0XF4
#define HDMI_RX_MPEG_DATA10_REG 0XF5
#define HDMI_RX_MPEG_DATA11_REG 0XF6
#define HDMI_RX_MPEG_DATA12_REG 0XF7
#define HDMI_RX_MPEG_DATA13_REG 0XF8
#define HDMI_RX_MPEG_DATA14_REG 0XF9
#define HDMI_RX_MPEG_DATA15_REG 0XFA
#define HDMI_RX_MPEG_DATA16_REG 0XFB
#define HDMI_RX_MPEG_DATA17_REG 0XFC
#define HDMI_RX_MPEG_DATA18_REG 0XFD
#define HDMI_RX_MPEG_DATA19_REG 0XFE

#define HDMI_RX_SPD_INFO_CTRL 0X5F
#define HDMI_RX_ACP_INFO_CTRL 0X7F
#define HDMI_RX_GENERAL_CTRL 0X9F
#define HDMI_RX_MPEG_VS_CTRL 0XDF
#define HDMI_RX_MPEG_VS_INFO_CTRL 0XFF


/***************************************************************/
//  DEV_ADDR = 0x70 or 0x78 , Displayport mode and HDCP registers
#define SP_TX_HDCP_STATUS							  				0x00
#define SP_TX_HDCP_AUTH_PASS						  			0x02//bit position

#define SP_TX_HDCP_CONTROL_0_REG                  		0x01
#define SP_TX_HDCP_CONTROL_0_STORE_AN            0x80//bit position
#define SP_TX_HDCP_CONTROL_0_RX_REPEATER   	0x40//bit position
#define SP_TX_HDCP_CONTROL_0_RE_AUTH              0x20//bit position
#define SP_TX_HDCP_CONTROL_0_SW_AUTH_OK       0x10//bit position
#define SP_TX_HDCP_CONTROL_0_HARD_AUTH_EN   0x08//bit position
#define SP_TX_HDCP_CONTROL_0_HDCP_ENC_EN      0x04//bit position
#define SP_TX_HDCP_CONTROL_0_BKSV_SRM_PASS  0x02//bit position
#define SP_TX_HDCP_CONTROL_0_KSVLIST_VLD        0x01//bit position


#define SP_TX_HDCP_CONTROL_1_REG                  		0x02
#define SP_TX_HDCP_CONTROL_1_AINFO_EN   		0x04//bit position
#define SP_TX_HDCP_CONTROL_1_RCV_11_EN                  0x02//bit position
#define SP_TX_HDCP_CONTROL_1_HDCP_11_EN           		0x01//bit position

#define SP_TX_HDCP_LINK_CHK_FRAME_NUM				 	0x03
#define SP_TX_HDCP_CONTROL_2_REG						0x04

#define SP_TX_HDCP_AKSV0								0x05
#define SP_TX_HDCP_AKSV1								0x06
#define SP_TX_HDCP_AKSV2								0x07
#define SP_TX_HDCP_AKSV3								0x08
#define SP_TX_HDCP_AKSV4								0x09

//AKSV
#define SP_TX_HDCP_AN0									0x0A
#define SP_TX_HDCP_AN1									0x0B
#define SP_TX_HDCP_AN2									0x0C
#define SP_TX_HDCP_AN3									0x0D
#define SP_TX_HDCP_AN4									0x0E
#define SP_TX_HDCP_AN5									0x0F
#define SP_TX_HDCP_AN6									0x10
#define SP_TX_HDCP_AN7									0x11

//BKSV
#define SP_TX_HDCP_BKSV0								0x12
#define SP_TX_HDCP_BKSV1								0x13
#define SP_TX_HDCP_BKSV2								0x14
#define SP_TX_HDCP_BKSV3								0x15
#define SP_TX_HDCP_BKSV4								0x16

#define SP_TX_HDCP_R0_L									0x17
#define SP_TX_HDCP_R0_H									0x18

#define M_VID_0 0xC0
#define M_VID_1 0xC1
#define M_VID_2 0xC2
#define N_VID_0 0xC3
#define N_VID_1 0xC4
#define N_VID_2 0xC5

#define SP_TX_SYS_CTRL1_REG           					0x80
#define SP_TX_SYS_CTRL1_CHIP_AUTH_RESET        					0x80    // bit position
#define SP_TX_SYS_CTRL1_PD_BYPASS_CHIP_AUTH        					0x40    // bit position
#define SP_TX_SYS_CTRL1_DET_STA       					0x04    // bit position
#define SP_TX_SYS_CTRL1_FORCE_DET     					0x02    // bit position
#define SP_TX_SYS_CTRL1_DET_CTRL      					0x01    // bit position

#define SP_TX_SYS_CTRL2_REG           					0x81
#define SP_TX_SYS_CTRL2_CHA_STA       					0x04    // bit position
#define SP_TX_SYS_CTRL2_FORCE_CHA     					0x02    // bit position
#define SP_TX_SYS_CTRL2_CHA_CTRL      					0x01    // bit position

#define SP_TX_SYS_CTRL3_REG           					0x82
#define SP_TX_SYS_CTRL3_HPD_STATUS    					0x40    // bit position
#define SP_TX_SYS_CTRL3_F_HPD         					0x20    // bit position
#define SP_TX_SYS_CTRL3_HPD_CTRL      					0x10    // bit position
#define SP_TX_SYS_CTRL3_STRM_VALID    					0x04    // bit position
#define SP_TX_SYS_CTRL3_F_VALID       					0x02    // bit position
#define SP_TX_SYS_CTRL3_VALID_CTRL    					0x01    // bit position

#define SP_TX_SYS_CTRL4_REG			  					0x83
#define SP_TX_SYS_CTRL4_ENHANCED 	  					0x08//bit position

#define SP_TX_VID_CTRL				  					0x84

#define SP_TX_AUD_CTRL									0x87
#define SP_TX_AUD_CTRL_AUD_EN							0x01


#define SP_TX_PKT_EN_REG              					0x90
#define SP_TX_PKT_AUD_UP								0x80  // bit position
#define SP_TX_PKT_AVI_UD              					0x40  // bit position
#define SP_TX_PKT_MPEG_UD             					0x20  // bit position    
#define SP_TX_PKT_SPD_UD              					0x10  // bit position   
#define SP_TX_PKT_AUD_EN								0x08  // bit position=
#define SP_TX_PKT_AVI_EN              					0x04  // bit position          
#define SP_TX_PKT_MPEG_EN             					0x02  // bit position     
#define SP_TX_PKT_SPD_EN              					0x01  // bit position       


#define SP_TX_HDCP_CTRL 												0x92

#define SP_TX_LINK_BW_SET_REG         				 0xA0

#define SP_TX_TRAINING_PTN_SET_REG                   0xA2
#define SP_TX_SCRAMBLE_DISABLE						 0x20//bit 5

#define SP_TX_TRAINING_LANE0_SET_REG                 				0xA3
#define SP_TX_TRAINING_LANE0_SET_MAX_PRE_REACH        0x20        // bit position
#define SP_TX_TRAINING_LANE0_SET_MAX_DRIVE_REACH     0x04        // bit position

#define SSC_CTRL_REG1					 0xA7
#define SPREAD_AMP						 0x10//bit 4
#define MODULATION_FREQ					 0x01//bit 0


#define SP_TX_LINK_TRAINING_CTRL_REG                0xA8
#define SP_TX_LINK_TRAINING_CTRL_EN                 0x01        // bit position


#define SP_TX_DEBUG_REG1							0xB0
#define SP_TX_DEBUG_HPD_POLLING_DET						0x40//bit position
#define SP_TX_DEBUG_HPD_POLLING_EN						0x20//bit position
#define SP_TX_DEBUG_PLL_LOCK						0x10//bit position


#define SP_TX_DP_POLLING_PERIOD							0xB3


#define SP_TX_DP_POLLING_CTRL_REG							0xB4
#define SP_TX_AUTO_POLLING_DISABLE						0x01//bit position



#define SP_TX_LINK_DEBUG_REG                        0xB8
#define SP_TX_LINK_DEBUG_INSERT_ER                  0x02        // bit position
#define SP_TX_LINK_DEBUG_PRBS31_EN                  0x01        // bit position

#define SP_TX_SINK_COUNT_REG                0xB9

#define SP_TX_LINK_STATUS_REG1                               0xBB

#define SP_TX_SINK_STATUS_REG                                   0xBE
#define SP_TX_SINK_STATUS_SINK_STATUS_1          	0x02        // bit position
#define SP_TX_SINK_STATUS_SINK_STATUS_0          	0x01        // bit position


//#define SP_TX_LINK_TEST_COUNT                     0xC0


#define SP_TX_PLL_CTRL_REG											0xC7	
#define SP_TX_PLL_CTRL_PLL_PD           						0x80        // bit position
#define SP_TX_PLL_CTRL_PLL_RESET        					0x40        // bit position 
//#define SP_TX_PLL_CTRL_CPREG_BLEED      					0x08        // bit position 

#define SP_TX_ANALOG_POWER_DOWN_REG                   			0xC8
#define SP_TX_ANALOG_POWER_DOWN_MACRO_PD              	0x20        // bit position 
#define SP_TX_ANALOG_POWER_DOWN_AUX_PD                		0x10        // bit position 
#define SP_TX_ANALOG_POWER_DOWN_CH0_PD                		0x01        // bit position 

#define SP_TX_ANALOG_TEST_REG                         					0xC9

#define SP_TX_GNS_CTRL_REG                            							0xCD
#define SP_EQ_LOOP_CNT											0x40//bit position
#define SP_TX_VIDEO_MAP_CTRL                 			                            0x02       // bit position 
#define SP_TX_RS_CTRL                        					              	0x01       // bit position 

#define SP_TX_DOWN_SPREADING_CTRL1                                               0xD0   //guochuncheng
#define SP_TX_DOWN_SPREADING_CTRL2                                               0xD1
#define SP_TX_DOWN_SPREADING_CTRL3                                               0xD2
#define SP_TX_SSC_D_CTRL                                                             0x40       //bit position
#define SP_TX_FS_CTRL_TH_CTRL                                                   0x20       //bit position

#define SP_TX_M_CALCU_CTRL												0xD9
#define M_GEN_CLK_SEL													0x01//bit 0


#define SP_TX_EXTRA_ADDR_REG											0xCE
#define SP_TX_I2C_STRETCH_CTRL_REG                                                              0xDB
#define SP_TX_AUX_STATUS            										0xE0
#define SP_TX_DEFER_CTRL_REG            									0xE2
#define SP_TXL_DEFER_CTRL_EN  					                     		       0x80       // bit position 

#define SP_TX_BUF_DATA_COUNT_REG											0xE4
#define SP_TX_AUX_CTRL_REG              										0xE5
#define SP_TX_AUX_ADDR_7_0_REG          									0xE6
#define SP_TX_AUX_ADDR_15_8_REG         									0xE7
#define SP_TX_AUX_ADDR_19_16_REG        									0xE8

#define SP_TX_AUX_CTRL_REG2                                                 0xE9
#define SP_TX_ADDR_ONLY_BIT													0x02//bit 1


#define SP_TX_3D_VSC_CTRL                                                 0xEA
#define SP_TX_AUX_CTRL_RE2                                                 0xEB

#define SP_TX_BUF_DATA_0_REG                          0xf0
#define SP_TX_BUF_DATA_1_REG                          0xf1
#define SP_TX_BUF_DATA_2_REG                          0xf2
#define SP_TX_BUF_DATA_3_REG                          0xf3
#define SP_TX_BUF_DATA_4_REG                          0xf4
#define SP_TX_BUF_DATA_5_REG                          0xf5
#define SP_TX_BUF_DATA_6_REG                          0xf6
#define SP_TX_BUF_DATA_7_REG                          0xf7
#define SP_TX_BUF_DATA_8_REG                          0xf8
#define SP_TX_BUF_DATA_9_REG                          0xf9
#define SP_TX_BUF_DATA_10_REG                         0xfa
#define SP_TX_BUF_DATA_11_REG                         0xfb
#define SP_TX_BUF_DATA_12_REG                         0xfc
#define SP_TX_BUF_DATA_13_REG                         0xfd
#define SP_TX_BUF_DATA_14_REG                         0xfe
#define SP_TX_BUF_DATA_15_REG                         0xff

//End for Address 0x70 or 0x78

/***************************************************************/
//  DEV_ADDR = 0x72 or 0x76, System control registers
#define SP_TX_VND_IDL_REG             	0x00
#define SP_TX_VND_IDH_REG             	0x01
#define SP_TX_DEV_IDL_REG             	0x02
#define SP_TX_DEV_IDH_REG             	0x03
#define SP_TX_DEV_REV_REG             	0x04

#define SP_POWERD_CTRL_REG			  	0x05
#define SP_POWERD_REGISTER_REG			0x80// bit position
//#define SP_POWERD_MISC_REG			  	0x40// bit position
#define SP_POWERD_HDCP_REG			  	0x20// bit position
#define SP_POWERD_AUDIO_REG				0x10// bit position
#define SP_POWERD_VIDEO_REG			  	0x08// bit position
#define SP_POWERD_LINK_REG			  	0x04// bit position
#define SP_POWERD_TOTAL_REG			  	0x02// bit position
//#define SP_MODE_SEL_REG				  	0x01// bit position

#define SP_TX_RST_CTRL_REG            	0x06
#define SP_TX_RST_MISC_REG 			  	0x80	// bit position
#define SP_TX_RST_VIDCAP_REG		  	0x40	// bit position
#define SP_TX_RST_VIDFIF_REG          	0x20    // bit position
#define SP_TX_RST_AUDFIF_REG          	0x10    // bit position
#define SP_TX_RST_AUDCAP_REG         	0x08    // bit position
#define SP_TX_RST_HDCP_REG            	0x04    // bit position
#define SP_TX_RST_SW_RST             	0x02    // bit position
#define SP_TX_RST_HW_RST             	0x01    // bit position

#define SP_TX_RST_CTRL2_REG				0x07
#define SP_TX_RST_SSC					0x80//bit position
#define SP_TX_AC_MODE					0x40//bit position
//#define SP_TX_DDC_RST					0x10//bit position
//#define SP_TX_TMDS_BIST_RST				0x08//bit position
#define SP_TX_AUX_RST					0x04//bit position
#define SP_TX_SERDES_FIFO_RST			0x02//bit position
#define SP_TX_I2C_REG_RST				0x01//bit position


#define SP_TX_VID_CTRL1_REG           	0x08
#define SP_TX_VID_CTRL1_VID_EN       0x80    // bit position
#define SP_TX_VID_CTRL1_VID_MUTE   0x40    // bit position
#define SP_TX_VID_CTRL1_DE_GEN      0x20    // bit position
#define SP_TX_VID_CTRL1_DEMUX        0x10    // bit position
#define SP_TX_VID_CTRL1_IN_BIT		  	0x04    // bit position
#define SP_TX_VID_CTRL1_DDRCTRL		0x02    // bit position
#define SP_TX_VID_CTRL1_EDGE		  		0x01    // bit position

#define SP_TX_VID_CTRL2_REG           	0x09
#define SP_TX_VID_CTRL1_YCBIT_SEL  		0x04    // bit position

#define SP_TX_VID_CTRL3_REG           	0x0A
#define SP_TX_VID_CTRL3_HPD_OUT           	0x40

#define SP_TX_VID_CTRL4_REG           	0x0B
#define SP_TX_VID_CTRL4_E_SYNC_EN	  	0x80	  //bit position
#define SP_TX_VID_CTRL4_EX_E_SYNC    0x40    // bit position
#define SP_TX_VID_CTRL4_BIST          		0x08    // bit position
#define SP_TX_VID_CTRL4_BIST_WIDTH   0x04        // bit position

#define SP_TX_VID_CTRL5_REG           		0x0C

#define SP_TX_VID_CTRL6_REG           		0x0D
#define SP_TX_VID_UPSAMPLE					0x02//bit position

#define SP_TX_VID_CTRL7_REG           		0x0E
#define SP_TX_VID_CTRL8_REG           		0x0F
#define SP_TX_VID_CTRL9_REG           		0x10

#define SP_TX_VID_CTRL10_REG           	0x11
#define SP_TX_VID_CTRL10_INV_F         	0x08    // bit position
#define SP_TX_VID_CTRL10_I_SCAN        	0x04    // bit position
#define SP_TX_VID_CTRL10_VSYNC_POL   0x02    // bit position
#define SP_TX_VID_CTRL10_HSYNC_POL   0x01    // bit position

#define SP_TX_TOTAL_LINEL_REG         0x12
#define SP_TX_TOTAL_LINEH_REG         0x13
#define SP_TX_ACT_LINEL_REG           0x14
#define SP_TX_ACT_LINEH_REG           0x15
#define SP_TX_VF_PORCH_REG            0x16
#define SP_TX_VSYNC_CFG_REG           0x17
#define SP_TX_VB_PORCH_REG            0x18
#define SP_TX_TOTAL_PIXELL_REG        0x19
#define SP_TX_TOTAL_PIXELH_REG        0x1A
#define SP_TX_ACT_PIXELL_REG          0x1B
#define SP_TX_ACT_PIXELH_REG          0x1C
#define SP_TX_HF_PORCHL_REG           0x1D
#define SP_TX_HF_PORCHH_REG           0x1E
#define SP_TX_HSYNC_CFGL_REG          0x1F
#define SP_TX_HSYNC_CFGH_REG          0x20
#define SP_TX_HB_PORCHL_REG           0x21
#define SP_TX_HB_PORCHH_REG           0x22

#define SP_TX_VID_STATUS						0x23

#define SP_TX_TOTAL_LINE_STA_L        0x24
#define SP_TX_TOTAL_LINE_STA_H        0x25
#define SP_TX_ACT_LINE_STA_L          0x26
#define SP_TX_ACT_LINE_STA_H          0x27
#define SP_TX_V_F_PORCH_STA           0x28
#define SP_TX_V_SYNC_STA              0x29
#define SP_TX_V_B_PORCH_STA           0x2A
#define SP_TX_TOTAL_PIXEL_STA_L       0x2B
#define SP_TX_TOTAL_PIXEL_STA_H       0x2C
#define SP_TX_ACT_PIXEL_STA_L         0x2D
#define SP_TX_ACT_PIXEL_STA_H         0x2E
#define SP_TX_H_F_PORCH_STA_L         0x2F
#define SP_TX_H_F_PORCH_STA_H         0x30
#define SP_TX_H_SYNC_STA_L            0x31
#define SP_TX_H_SYNC_STA_H            0x32
#define SP_TX_H_B_PORCH_STA_L         0x33
#define SP_TX_H_B_PORCH_STA_H         0x34

#define SP_TX_Video_Interface_BIST    0x35

#define SPDIF_AUDIO_CTRL0			0x36
#define SPDIF_AUDIO_CTRL0_SPDIF_IN  0x80 // bit position

#define SPDIF_AUDIO_STATUS0			0x38
#define SPDIF_AUDIO_STATUS0_CLK_DET 0x80
#define SPDIF_AUDIO_STATUS0_AUD_DET 0x01

#define SPDIF_AUDIO_STATUS1 0x39

#define AUDIO_BIST_CTRL 0x3c
#define AUDIO_BIST_EN 0x01

//#define AUDIO_BIST_CHANNEL_STATUS1 0xd0
//#define AUDIO_BIST_CHANNEL_STATUS2 0xd1
//#define AUDIO_BIST_CHANNEL_STATUS3 0xd2
//#define AUDIO_BIST_CHANNEL_STATUS4 0xd3
//#define AUDIO_BIST_CHANNEL_STATUS5 0xd4

#define SP_TX_VIDEO_BIT_CTRL_0_REG                    0x40
#define SP_TX_VIDEO_BIT_CTRL_1_REG                    0x41
#define SP_TX_VIDEO_BIT_CTRL_2_REG                    0x42
#define SP_TX_VIDEO_BIT_CTRL_3_REG                    0x43
#define SP_TX_VIDEO_BIT_CTRL_4_REG                    0x44
#define SP_TX_VIDEO_BIT_CTRL_5_REG                    0x45
#define SP_TX_VIDEO_BIT_CTRL_6_REG                    0x46
#define SP_TX_VIDEO_BIT_CTRL_7_REG                    0x47
#define SP_TX_VIDEO_BIT_CTRL_8_REG                    0x48
#define SP_TX_VIDEO_BIT_CTRL_9_REG                    0x49
#define SP_TX_VIDEO_BIT_CTRL_10_REG                   0x4a
#define SP_TX_VIDEO_BIT_CTRL_11_REG                   0x4b
#define SP_TX_VIDEO_BIT_CTRL_12_REG                   0x4c
#define SP_TX_VIDEO_BIT_CTRL_13_REG                   0x4d
#define SP_TX_VIDEO_BIT_CTRL_14_REG                   0x4e
#define SP_TX_VIDEO_BIT_CTRL_15_REG                   0x4f
#define SP_TX_VIDEO_BIT_CTRL_16_REG                   0x50
#define SP_TX_VIDEO_BIT_CTRL_17_REG                   0x51
#define SP_TX_VIDEO_BIT_CTRL_18_REG                   0x52
#define SP_TX_VIDEO_BIT_CTRL_19_REG                   0x53
#define SP_TX_VIDEO_BIT_CTRL_20_REG                   0x54
#define SP_TX_VIDEO_BIT_CTRL_21_REG                   0x55
#define SP_TX_VIDEO_BIT_CTRL_22_REG                   0x56
#define SP_TX_VIDEO_BIT_CTRL_23_REG                   0x57
#define SP_TX_VIDEO_BIT_CTRL_24_REG                   0x58
#define SP_TX_VIDEO_BIT_CTRL_25_REG                   0x59
#define SP_TX_VIDEO_BIT_CTRL_26_REG                   0x5a
#define SP_TX_VIDEO_BIT_CTRL_27_REG                   0x5b
#define SP_TX_VIDEO_BIT_CTRL_28_REG                   0x5c
#define SP_TX_VIDEO_BIT_CTRL_29_REG                   0x5d
#define SP_TX_VIDEO_BIT_CTRL_30_REG                   0x5e
#define SP_TX_VIDEO_BIT_CTRL_31_REG                   0x5f
#define SP_TX_VIDEO_BIT_CTRL_32_REG                   0x60
#define SP_TX_VIDEO_BIT_CTRL_33_REG                   0x61
#define SP_TX_VIDEO_BIT_CTRL_34_REG                   0x62
#define SP_TX_VIDEO_BIT_CTRL_35_REG                   0x63
#define SP_TX_VIDEO_BIT_CTRL_36_REG                   0x64
#define SP_TX_VIDEO_BIT_CTRL_37_REG                   0x65
#define SP_TX_VIDEO_BIT_CTRL_38_REG                   0x66
#define SP_TX_VIDEO_BIT_CTRL_39_REG                   0x67
#define SP_TX_VIDEO_BIT_CTRL_40_REG                   0x68
#define SP_TX_VIDEO_BIT_CTRL_41_REG                   0x69
#define SP_TX_VIDEO_BIT_CTRL_42_REG                   0x6a
#define SP_TX_VIDEO_BIT_CTRL_43_REG                   0x6b
#define SP_TX_VIDEO_BIT_CTRL_44_REG                   0x6c
#define SP_TX_VIDEO_BIT_CTRL_45_REG                   0x6d
#define SP_TX_VIDEO_BIT_CTRL_46_REG                   0x6e
#define SP_TX_VIDEO_BIT_CTRL_47_REG                   0x6f

//AVI info frame
#define SP_TX_AVI_TYPE              0x70
#define SP_TX_AVI_VER               0x71
#define SP_TX_AVI_LEN               0x72
#define SP_TX_AVI_DB0		     0x73
#define SP_TX_AVI_DB1               0x74
#define SP_TX_AVI_DB2               0x75
#define SP_TX_AVI_DB3               0x76
#define SP_TX_AVI_DB4               0x77
#define SP_TX_AVI_DB5               0x78
#define SP_TX_AVI_DB6               0x79
#define SP_TX_AVI_DB7               0x7A
#define SP_TX_AVI_DB8               0x7B
#define SP_TX_AVI_DB9               0x7C
#define SP_TX_AVI_DB10              0x7D
#define SP_TX_AVI_DB11              0x7E
#define SP_TX_AVI_DB12              0x7F
#define SP_TX_AVI_DB13              0x80
#define SP_TX_AVI_DB14              0x81
#define SP_TX_AVI_DB15              0x82

//Audio info frame
#define SP_TX_AUD_TYPE			 0x83
#define SP_TX_AUD_VER			 0x84
#define SP_TX_AUD_LEN			 0x85
#define SP_TX_AUD_DB0			 0x86
#define SP_TX_AUD_DB1			 0x87
#define SP_TX_AUD_DB2			 0x88
#define SP_TX_AUD_DB3			 0x89
#define SP_TX_AUD_DB4			 0x8A
#define SP_TX_AUD_DB5			 0x8B
#define SP_TX_AUD_DB6			 0x8C
#define SP_TX_AUD_DB7			 0x8D
#define SP_TX_AUD_DB8			 0x8E
#define SP_TX_AUD_DB9			 0x8F
#define SP_TX_AUD_DB10			 0x90

//SPD info frame
#define SP_TX_SPD_TYPE                0x91
#define SP_TX_SPD_VER                 0x92
#define SP_TX_SPD_LEN                 0x93
#define SP_TX_SPD_DATA0		0x94
#define SP_TX_SPD_DATA1               0x95
#define SP_TX_SPD_DATA2               0x96
#define SP_TX_SPD_DATA3               0x97
#define SP_TX_SPD_DATA4               0x98
#define SP_TX_SPD_DATA5               0x99
#define SP_TX_SPD_DATA6               0x9A
#define SP_TX_SPD_DATA7               0x9B
#define SP_TX_SPD_DATA8               0x9C
#define SP_TX_SPD_DATA9               0x9D
#define SP_TX_SPD_DATA10              0x9E
#define SP_TX_SPD_DATA11              0x9F
#define SP_TX_SPD_DATA12              0xA0
#define SP_TX_SPD_DATA13              0xA1
#define SP_TX_SPD_DATA14              0xA2
#define SP_TX_SPD_DATA15              0xA3
#define SP_TX_SPD_DATA16              0xA4
#define SP_TX_SPD_DATA17              0xA5
#define SP_TX_SPD_DATA18              0xA6
#define SP_TX_SPD_DATA19              0xA7
#define SP_TX_SPD_DATA20              0xA8
#define SP_TX_SPD_DATA21              0xA9
#define SP_TX_SPD_DATA22              0xAA
#define SP_TX_SPD_DATA23              0xAB
#define SP_TX_SPD_DATA24              0xAC
#define SP_TX_SPD_DATA25              0xAD
#define SP_TX_SPD_DATA26              0xAE
#define SP_TX_SPD_DATA27              0xAF

//Mpeg source info frame
#define SP_TX_MPEG_TYPE               0xB0
#define SP_TX_MPEG_VER                0xB1
#define SP_TX_MPEG_LEN                0xB2
#define SP_TX_MPEG_DATA0              0xB3
#define SP_TX_MPEG_DATA1              0xB4
#define SP_TX_MPEG_DATA2              0xB5
#define SP_TX_MPEG_DATA3              0xB6
#define SP_TX_MPEG_DATA4              0xB7
#define SP_TX_MPEG_DATA5              0xB8
#define SP_TX_MPEG_DATA6              0xB9
#define SP_TX_MPEG_DATA7              0xBA
#define SP_TX_MPEG_DATA8              0xBB
#define SP_TX_MPEG_DATA9              0xBC
#define SP_TX_MPEG_DATA10             0xBD
#define SP_TX_MPEG_DATA11            0xBE
#define SP_TX_MPEG_DATA12            0xBF
#define SP_TX_MPEG_DATA13            0xC0
#define SP_TX_MPEG_DATA14            0xC1
#define SP_TX_MPEG_DATA15            0xC2
#define SP_TX_MPEG_DATA16            0xC3
#define SP_TX_MPEG_DATA17            0xC4
#define SP_TX_MPEG_DATA18            0xC5
#define SP_TX_MPEG_DATA19            0xC6
#define SP_TX_MPEG_DATA20            0xC7
#define SP_TX_MPEG_DATA21            0xC8
#define SP_TX_MPEG_DATA22            0xC9
#define SP_TX_MPEG_DATA23            0xCA
#define SP_TX_MPEG_DATA24            0xCB
#define SP_TX_MPEG_DATA25            0xCC
#define SP_TX_MPEG_DATA26            0xCD
#define SP_TX_MPEG_DATA27            0xCE

//#define GNSS_CTRL_REG				0xCD
//#define ENABLE_SSC_FILTER			0x80//bit 

//#define SSC_D_VALUE					 0xD0
//#define SSC_CTRL_REG2					 0xD1

#define ANALOG_DEBUG_REG1			0xDC
#define ANALOG_SEL_BG				0x40//bit 4
#define ANALOG_SWING_A_30PER		0x08//bit 3

#define ANALOG_DEBUG_REG2			0xDD
#define ANALOG_24M_SEL				0x08//bit 3
//#define ANALOG_FILTER_ENABLED		0x10//bit 4


#define ANALOG_DEBUG_REG3			0xDE

#define PLL_FILTER_CTRL1			0xDF
#define PD_RING_OSC					0x40//bit 6
#define V33_SWITCH_ON                        0x02 //bit 1

#define PLL_FILTER_CTRL2			0xE0
#define PLL_FILTER_CTRL3			0xE1
#define PLL_FILTER_CTRL4			0xE2
#define PLL_FILTER_CTRL5			0xE3
#define PLL_FILTER_CTRL6			0xE4

#define SP_TX_I2S_CTRL			0xE6
//#define SP_TX_I2S_SWAP_WORD_LENGTH			0xE7
#define SP_TX_I2S_FMT			0xD5
#define SP_TX_I2S_CH_Status1			0xD0
#define SP_TX_I2S_CH_Status2			0xD1
#define SP_TX_I2S_CH_Status3			0xD2
#define SP_TX_I2S_CH_Status4			0xD3
#define SP_TX_I2S_CH_Status5			0xD4

//interrupt
#define SP_COMMON_INT_STATUS1     0xF1
#define SP_COMMON_INT1_PLL_LOCK_CHG 	0x40//bit position
#define SP_COMMON_INT1_VIDEO_FORMAT_CHG 0x08//bit position
#define SP_COMMON_INT1_AUDIO_CLK_CHG	0x04//bit position
#define SP_COMMON_INT1_VIDEO_CLOCK_CHG  0x02//bit position


#define SP_COMMON_INT_STATUS2	  0xF2
#define SP_COMMON_INT2_AUTHCHG	  0x02 //bit position
#define SP_COMMON_INT2_AUTHDONE	  0x01 //bit position

#define SP_COMMON_INT_STATUS3	  0xF3
#define SP_COMMON_INT3_AFIFO_UNDER	0x80//bit position
#define SP_COMMON_INT3_AFIFO_OVER	0x40//bit position

#define SP_COMMON_INT_STATUS4	    0xF4
#define SP_COMMON_INT4_PLUG                0x01   // bit position
#define SP_COMMON_INT4_ESYNC_ERR          0x10   // bit position
#define SP_COMMON_INT4_HPDLOST		0x02   //bit position
#define SP_COMMON_INT4_HPD_CHANGE   0x04   //bit position


#define SP_TX_INT_STATUS1		  0xF7
#define SP_TX_INT_DPCD_IRQ_REQUEST	  0x80 //bit position
#define SP_TX_INT_STATUS1_HPD	  0x40 //bit position
#define SP_TX_INT_STATUS1_TRAINING_Finish       0x20   // bit position
#define SP_TX_INT_STATUS1_POLLING_ERR        0x10   // bit position
#define SP_TX_INT_STATUS1_LINK_CHANGE       0x04   // bit position

#define SP_TX_INT_SINK_CHG		  0x08//bit position

//interrupt mask
#define SP_COMMON_INT_MASK1			  0xF8
#define SP_COMMON_INT_MASK2			  0xF9
#define SP_COMMON_INT_MASK3			  0xFA
#define SP_COMMON_INT_MASK4			  0xFB
#define SP_INT_MASK					  					0xFE
#define SP_TX_INT_CTRL_REG            		0xFF
//End for dev_addr 0x72 or 0x76

/***************************************************************/
/***************************************************************/

//DPCD regs
#define DPCD_DPCD_REV                                                0x00
#define DPCD_MAX_LINK_RATE                                      0x01
#define DPCD_MAX_LANE_COUNT                                   0x02
#define DPCD_MAX_DOWNSPREAD                                  0x03
#define DPCD_NORP                                                        0x04
#define DPCD_DOWNSTREAMPORT_PRESENT                  0x05

#define DPCD_RECEIVE_PORT0_CAP_0                        0x08
#define DPCD_RECEIVE_PORT0_CAP_1                        0x09
#define DPCD_RECEIVE_PORT0_CAP_2                        0x0a
#define DPCD_RECEIVE_PORT0_CAP_3                        0x0b

#define DPCD_LINK_BW_SET                                      0x00
#define DPCD_LANE_COUNT_SET                                0x01
#define DPCD_TRAINING_PATTERN_SET                     0x02
#define DPCD_TRAINNIG_LANE0_SET                         0x03
#define DPCD_TRAINNIG_LANE1_SET                         0x04
#define DPCD_TRAINNIG_LANE2_SET                         0x05
#define DPCD_TRAINNIG_LANE3_SET                         0x06
#define DPCD_DOWNSPREAD_CTRL                            0x07

#define DPCD_SINK_COUNT                                          0x00
#define DPCD_DEVICE_SERVICE_IRQ_VECTOR              0x01
#define DPCD_LANE0_1_STATUS                                   0x02
#define DPCD_LANE2_3_STATUS                                   0x03
#define DPCD_LANE_ALIGN_STATUS_UPDATED            0x04
#define DPCD_SINK_STATUS                                          0x05
#define DPCD_ADJUST_REQUEST_LANE0_1                     0x06
#define DPCD_ADJUST_REQUEST_LANE2_3                     0x07
#define DPCD_TRAINING_SCORE_LANE0                       0x08
#define DPCD_TRAINING_SCORE_LANE1                       0x09
#define DPCD_TRAINING_SCORE_LANE2                       0x0a
#define DPCD_TRAINING_SCORE_LANE3                       0x0b

#define DPCD_TEST_REQUEST                               0x18
#define DPCD_TEST_LINK_RATE                             0x19

#define DPCD_TEST_LANE_COUNT                            0x20

#define DPCD_TEST_Response                              0x60
#define TEST_ACK                                                  0x01
#define DPCD_TEST_EDID_Checksum_Write                   0x04//bit position

#define DPCD_TEST_EDID_Checksum                         0x61


#define DPCD_SPECIFIC_INTERRUPT_1                               0x10
#define DPCD_USER_COMM1                                             0x22//define for downstream HDMI Rx sense detection

#define DPCD_SPECIFIC_INTERRUPT_2                               0x11

#endif




