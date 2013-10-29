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

#ifndef _SP_TX_DRV_H
#define _SP_TX_DRV_H

typedef unsigned char BYTE;
typedef unsigned int  WORD;
typedef unsigned char *pByte;


#define D(fmt, arg...) printk("<1>```%s:%d: " fmt, __FUNCTION__, __LINE__, ##arg)

#define MAX_BUF_CNT 6

#define DVI_MODE 0x00
#define HDMI_MODE 0x01

#define Deep_Color_legacy 0x00
#define Deep_Color_24bit 0x40
#define Deep_Color_30bit 0x50
#define Deep_Color_36bit 0x60


// Threshhold Time //
#define VIDEO_STABLE_TH 3     
#define AUDIO_STABLE_TH 1
#define SCDT_EXPIRE_TH 10
typedef enum
{
    SP_TX_INITIAL = 1,
    SP_TX_WAIT_SLIMPORT_PLUGIN,
    SP_TX_PARSE_EDID,
    SP_TX_CONFIG_HDMI_INPUT,
    SP_TX_LINK_TRAINING,
    SP_TX_CONFIG_SLIMPORT_OUTPUT,
    SP_TX_HDCP_AUTHENTICATION,
    SP_TX_PLAY_BACK
} SP_TX_System_State;




typedef enum{
       Hdmi_legacy,
	Hdmi_24bit,
	Hdmi_30bit,
	Hdmi_36bit
} HDMI_color_depth;


#define SP_TX_HDCP_FAIL_THRESHOLD         10

#define EDID_Dev_ADDR 0xa0
#define EDID_SegmentPointer_ADDR 0x60

extern BYTE sp_tx_lane_count;

extern BYTE sp_tx_link_config_done;
extern BYTE  checksum;
extern BYTE sp_tx_edid_err_code;
extern BYTE sp_tx_hdcp_enable,sp_tx_ssc_enable;
extern BYTE edid_pclk_out_of_range;
extern BYTE SP_TX_EDID_PREFERRED[18];
extern BYTE bEDIDBreak;
extern BYTE sp_tx_ds_edid_hdmi;
extern BYTE sp_tx_ds_edid_3d_present;
extern unsigned long pclk;
extern BYTE ByteBuf[MAX_BUF_CNT];
extern BYTE EDID_Print_Enable;



typedef enum
{
	SP_TX_PWR_REG,
	SP_TX_PWR_HDCP,
	SP_TX_PWR_AUDIO,
	SP_TX_PWR_VIDEO,
	SP_TX_PWR_LINK,
	SP_TX_PWR_TOTAL
}SP_TX_POWER_BLOCK;



typedef enum
{
	AVI_PACKETS,
	SPD_PACKETS,
	MPEG_VS_PACKETS
}PACKETS_TYPE;

struct Packet_AVI{
     unsigned char AVI_data[13];
} ;


struct Packet_SPD{
     unsigned char SPD_data[25];    
};      


struct Packet_MPEG{
     unsigned char MPEG_data[13];   
} ;


struct AudiInfoframe
{
     unsigned char type;
     unsigned char version; 
     unsigned char length;
     unsigned char pb_byte[11]; //modify to 10 bytes from 28.2008/10/23   
};

typedef enum
{
	COMMON_INT_1 = 0,
	COMMON_INT_2 = 1,
	COMMON_INT_3 = 2,
	COMMON_INT_4 = 3,
	SP_INT_STATUS = 6
}INTStatus;

typedef enum
{
	BW_54G = 0x14,
	BW_27G = 0x0A,
	BW_162G = 0x06,
	BW_NULL = 0x00
}SP_LINK_BW;


void SP_TX_Initialization(void);
void SP_TX_Show_Infomation(void);
//void SP_TX_Disable_Video_Input(void);
void SP_TX_Power_Down(SP_TX_POWER_BLOCK sp_tx_pd_block);
void SP_TX_Power_On(SP_TX_POWER_BLOCK sp_tx_pd_block);
void SP_TX_Enable_Video_Input(BYTE Enable);
void SP_TX_AVI_Setup(void);
//void SP_TX_MPEG_VS_Setup(void);
void SP_TX_AUX_DPCDWrite_Byte(BYTE addrh, BYTE addrm, BYTE addrl, BYTE data1);
void SP_TX_HDCP_Encryption_Disable(void) ;
void SP_TX_HDCP_Encryption_Enable(void) ;
void SP_TX_HW_HDCP_Enable(void);
void SP_TX_HW_HDCP_Disable(void);
void SP_TX_Clean_HDCP(void);
void SP_TX_EnhaceMode_Set(void);
void SP_TX_CONFIG_SSC(SP_LINK_BW linkbw);
void SP_TX_Config_Audio(void);
BYTE SP_TX_Chip_Located(void);
void SP_TX_Hardware_PowerOn(void);
void SP_TX_Hardware_PowerDown(void);
void SP_TX_VBUS_PowerOn(void) ;
void SP_TX_VBUS_PowerDown(void) ;
void SP_TX_RST_AUX(void);
BYTE SP_TX_Wait_AUX_Finished(void);
BYTE SP_TX_AUX_DPCDRead_Bytes(BYTE addrh, BYTE addrm, BYTE addrl,BYTE cCount,pByte pBuf);
BYTE SP_TX_AUX_DPCDWrite_Bytes(BYTE addrh, BYTE addrm, BYTE addrl,BYTE cCount,pByte pBuf);
void SP_TX_SPREAD_Enable(BYTE bEnable);
void SP_TX_Enable_Audio_Output(BYTE bEnable);
void SP_TX_AudioInfoFrameSetup(void);
void SP_TX_InfoFrameUpdate(struct AudiInfoframe* pAudioInfoFrame);
void SP_TX_Get_Int_status(INTStatus IntIndex, BYTE *cStatus);
BYTE SP_TX_Get_PLL_Lock_Status(void);
void SP_TX_Get_HDCP_status( BYTE *cStatus);
void SP_TX_HDCP_ReAuth(void);
void SP_TX_Get_Rx_LaneCount(BYTE bMax,BYTE *cLaneCnt);
void SP_TX_Get_Rx_BW(BYTE bMax,BYTE *cBw);
void SP_TX_Get_Link_BW(BYTE *bwtype);
void SP_TX_Get_Link_Status(BYTE *cStatus);
void SP_TX_Get_lane_Setting(BYTE cLane, BYTE *cSetting);
void SP_TX_EDID_Read_Initial(void);
BYTE SP_TX_AUX_EDIDRead_Byte(BYTE offset);
void SP_TX_Parse_Segments_EDID(BYTE segment, BYTE offset);
BYTE SP_TX_Get_EDID_Block(void);
void SP_TX_AddrOnly_Set(BYTE bSet);
void SP_TX_Scramble_Enable(BYTE bEnabled);
void SP_TX_API_M_GEN_CLK_Select(BYTE bSpreading);
void SP_TX_Config_Packets(PACKETS_TYPE bType);
void SP_TX_Load_Packet (PACKETS_TYPE type);
BYTE SP_TX_BW_LC_Sel(void);
void SP_TX_PCLK_Calc(SP_LINK_BW hbr_rbr);
BYTE SP_TX_HW_Link_Training (void);
BYTE SP_TX_LT_Pre_Config(void);
void SP_TX_Video_Mute(BYTE enable);
void SP_TX_AUX_WR (BYTE offset);
void SP_TX_AUX_RD (BYTE len_cmd);
void SP_TX_Config_Slimport_Output(void);
void SP_TX_LVTTL_Bit_Mapping(void);//the default mode is 12bit ddr
void SP_TX_Set_ColorSpace(void);
void SP_TX_Set_3D_Packets(void);
void SP_TX_Int_Irq_Handler(void);


//*****************************************************************
//Functions protoype for HDMI Input
//*****************************************************************


typedef enum
{
    HDMI_CLOCK_DET = 1,
    HDMI_SYNC_DET,
    HDMI_VIDEO_CONFIG,
    HDMI_AUDIO_CONFIG,
    HDMI_PLAYBACK
} HDMI_RX_System_State;

void SP_TX_Config_HDMI_Input (void);
void HDMI_RX_Set_HPD(void);
void HDMI_RX_TMDS_En(void);
void HDMI_RX_Set_Sys_State(BYTE ss);
void HDMI_RX_Set_Color_Mode(void);
void HDMI_RX_Mute_Video(void);
void HDMI_RX_Unmute_Video(void);
void HDMI_RX_Mute_Audio(void);
void HDMI_RX_Unmute_Audio(void);
BYTE HDMI_RX_Get_Input_Color_Depth(void);
void HDMI_RX_Set_Color_Depth(BYTE ColorDepth);
char HDMI_RX_Is_Video_Change(void);
void HDMI_RX_Get_Video_Info(void);
void HDMI_RX_Show_Video_Info(void);
void HDMI_RX_Show_Audio_Info(void);
void  HDMI_RX_Restart_Audio_Chk(void);
void HDMI_RX_Init_var(void);
void HDMI_RX_Initialize(void);
void HDMI_RX_Clk_Detected_Int(void);
void HDMI_RX_Sync_Det_Int(void);
void HDMI_RX_HDMI_DVI_Int(void);
void HDMI_RX_AV_MUTE_Int(void);
void HDMI_RX_Cts_Rcv_Int(void);
void HDMI_RX_Audio_Rcv_Int(void);
void HDMI_RX_HDCP_Error_Int(void);
void HDMI_RX_New_AVI_Int(void);
void HDMI_RX_No_VSI_Int(void);
void HDMI_RX_New_VSI_Int(void);
void HDMI_RX_Int_Irq_Handler(void);







#endif
