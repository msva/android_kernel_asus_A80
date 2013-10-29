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

#include <linux/delay.h>
#include <mach/gpio.h>
#include "SP_TX_DRV.h"
#include "SP_TX_Reg.h"
#include "SP_TX_CTRL.h"
#include "Colorado2.h"


//for EDID
BYTE checksum;
BYTE SP_TX_EDID_PREFERRED[18];//edid DTD array
BYTE sp_tx_ds_edid_hdmi; //Downstream is HDMI flag
BYTE sp_tx_ds_edid_3d_present;//downstream monitor support 3D
BYTE DTDbeginAddr;
BYTE EDID_Print_Enable;
static BYTE EDIDExtBlock[128];

unsigned long pclk; //input video pixel clock 
long int M_val,N_val;
SP_LINK_BW sp_tx_bw;  //linktraining banwidth
BYTE sp_tx_link_config_done;//link config done flag
BYTE sp_tx_lane_count; //link training lane count


struct AudiInfoframe SP_TX_AudioInfoFrmae;
struct Packet_AVI SP_TX_Packet_AVI;                     
struct Packet_MPEG SP_TX_Packet_MPEG;           
struct Packet_SPD SP_TX_Packet_SPD; 
SP_TX_System_State sp_tx_system_state;


//*****************************************************************
//GLOBAL VARIABLES DEFINITION FOR HDMI START
//*****************************************************************
BYTE g_HDMI_DVI_Status;
BYTE g_Cur_Pix_Clk;
BYTE g_Video_Stable_Cntr;
BYTE g_Audio_Stable_Cntr;
BYTE g_Sync_Expire_Cntr;
BYTE g_HDCP_Err_Cnt;

unsigned long g_Cur_H_Res;
unsigned long g_Cur_V_Res;
BYTE g_Video_Muted;
BYTE g_Audio_Muted;
BYTE g_CTS_Got;
BYTE g_Audio_Got;
BYTE g_VSI_Got;
BYTE g_No_VSI_Counter;

//BYTE set_color_mode_counter, deep_color_set_done,video_format_supported;
HDMI_RX_System_State  hdmi_system_state;

//*****************************************************************
//GLOBAL VARIABLES DEFINITION FOR HDMI END
//*****************************************************************




void SP_TX_Initialization(void)
{
    BYTE c;

	//software reset	 
	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_RST_CTRL_REG, &c);
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_RST_CTRL_REG, c | SP_TX_RST_SW_RST);
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_RST_CTRL_REG, c & ~SP_TX_RST_SW_RST);

	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_EXTRA_ADDR_REG, 0x50);//EDID address for AUX access
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_HDCP_CTRL, 0x02);	//disable HDCP polling mode.
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_LINK_DEBUG_REG, 0x30);//enable M value read out


	/*added for B0 to enable enable c-wire polling-ANX.Fei-20110831*/
	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_DEBUG_REG1, &c);
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DEBUG_REG1, (c|0x82));//disable polling HPD, force hotplug for HDCP, enable polling


	/*added for B0 to change the c-wire termination from 100ohm to 50 ohm for polling error issue-ANX.Fei-20110916*/
	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, PLL_FILTER_CTRL1, &c);
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, PLL_FILTER_CTRL1, (c|0x30));//change the c-wire termination from 100ohm to 50 ohm

	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_HDCP_CONTROL_0_REG, &c);
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_HDCP_CONTROL_0_REG, c | 0x03);//set KSV valid

	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, PLL_FILTER_CTRL6, 0x00);//open short portect and 5V detect
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, ANALOG_DEBUG_REG2, 0x06);//set power on time 1.5ms


	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_COMMON_INT_MASK1, 0x00);//mask all int
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_COMMON_INT_MASK2, 0x00);//mask all int
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_COMMON_INT_MASK3, 0x00);//mask all int
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_COMMON_INT_MASK4, 0x00);//mask all int
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_INT_MASK, 0xb4);//0xb0 unmask IRQ request Int & c-wire polling error int

	//PHY parameter for cts
	//Swing 200mv
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x30, 0x16);//0db
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x36, 0x1b);//3.5db
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x39, 0x22);//6db
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x3b, 0x23);//9db

	//Pre-emphasis
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x46, 0x09);//3.5db
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x49, 0x16);//6db
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x4b, 0x1F);//9db

	//Swing 400mv
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x31, 0x26);//0db
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x37, 0x28);//3.5db
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x3A, 0x2F);//6db

	//Pre-emphasis
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x47, 0x10);//3.5db
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x4A, 0x1F);//6db


	//Swing 600mv
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x34, 0x36);//0db
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x38, 0x3c);//3.5db
	//emp     
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x48, 0x10);//3.5db

	//Swing 800mv
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x35, 0x3F);//0db

	//M value select, select clock with downspreading
	SP_TX_API_M_GEN_CLK_Select(1);

	//SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DP_POLLING_PERIOD, 0x01);

	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, 0xE5, 0x02);//set low power

}


void SP_TX_Power_Down(SP_TX_POWER_BLOCK sp_tx_pd_block)
{
    BYTE c;
    
    SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_POWERD_CTRL_REG , &c);
	
	if(sp_tx_pd_block == SP_TX_PWR_REG)//power down register
		c |= SP_POWERD_REGISTER_REG;
	else if(sp_tx_pd_block == SP_TX_PWR_HDCP)//power down IO
		c |= SP_POWERD_HDCP_REG;
	else if(sp_tx_pd_block == SP_TX_PWR_AUDIO)//power down audio
		c |= SP_POWERD_AUDIO_REG;
	else if(sp_tx_pd_block == SP_TX_PWR_VIDEO)//power down video
		c |= SP_POWERD_VIDEO_REG;
	else if(sp_tx_pd_block == SP_TX_PWR_LINK)//power down link
		c |= SP_POWERD_LINK_REG;
	else if(sp_tx_pd_block == SP_TX_PWR_TOTAL)//power down total.
		c |= SP_POWERD_TOTAL_REG;
		
    SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_POWERD_CTRL_REG, c);
	
    //D("SP_TX_Power_Down");
}

void SP_TX_Power_On(SP_TX_POWER_BLOCK sp_tx_pd_block)
{
    BYTE c;
    
    	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_POWERD_CTRL_REG , &c);

	if(sp_tx_pd_block == SP_TX_PWR_REG)//power on register
		c &= ~SP_POWERD_REGISTER_REG;
	else if(sp_tx_pd_block == SP_TX_PWR_HDCP)//power on IO
		c &= ~SP_POWERD_HDCP_REG;
	else if(sp_tx_pd_block == SP_TX_PWR_AUDIO)//power on audio
		c &= ~SP_POWERD_AUDIO_REG;
	else if(sp_tx_pd_block == SP_TX_PWR_VIDEO)//power on video
		c &= ~SP_POWERD_VIDEO_REG;
	else if(sp_tx_pd_block == SP_TX_PWR_LINK)//power on link
		c &= ~SP_POWERD_LINK_REG;
	else if(sp_tx_pd_block == SP_TX_PWR_TOTAL)//power on total.
		c &= ~SP_POWERD_TOTAL_REG;


    	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_POWERD_CTRL_REG, c);
      //D("SP_TX_Power_On");
	
}

void SP_TX_Set_ColorSpace(void)
{
	BYTE c;
	BYTE Color_Space;
	
	SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_AVI_DATA00_REG, &Color_Space);  
	Color_Space &=0x60;
	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL2_REG, &c);
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL2_REG, ((c&0xfc)|Color_Space>>5));//set input video 12-bit
}

void SP_TX_Set_3D_Packets(void)
{
	BYTE c;
	BYTE hdmi_video_format,vsi_header,v3d_structure;
	
	// SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_STATUS7_REG, &c);
	 //SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_STATUS7_REG, c);
	if(g_VSI_Got)
	{
		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, 0xea, &c);
		if(!(c&0x01))
		{
			SP_TX_Read_Reg(HDMI_PORT1_ADDR, HDMI_RX_MPEG_TYPE_REG, &vsi_header);
			SP_TX_Read_Reg(HDMI_PORT1_ADDR, HDMI_RX_MPEG_DATA03_REG, &hdmi_video_format);
			if((vsi_header==0x81)&&((hdmi_video_format &0xe0)== 0x40))
			{
		D("3D VSI packet is detected. Config VSI and VSC packet");

				SP_TX_Read_Reg(HDMI_PORT1_ADDR, HDMI_RX_MPEG_DATA05_REG, &v3d_structure);
				switch(v3d_structure&0xf0)
				{
					case 0x00://frame packing
						v3d_structure = 0x02;
						break;
					case 0x20://Line alternative
						v3d_structure = 0x03;
						break;
					case 0x30://Side-by-side(full)
						v3d_structure = 0x04;
						break;
					default:
						D("3D structure is not supported");
						break;
				}

		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, 0xeb, v3d_structure);//set 3D format type to stacked frame
		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, 0xea, &c);
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, 0xea, c|0x01);//enable VSC

		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, 0x90, &c);
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, 0x90, (c&0xfe));//set spd_en=0 

		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, 0x90, &c);
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, 0x90, (c|0x10));//read spd_update=1

		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, 0x90, &c);
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, 0x90, c|0x01);//read spd_en=1

	}
		}
		g_No_VSI_Counter = 0;
		g_VSI_Got = 0;
	
	}else{

		g_No_VSI_Counter ++;
		if(g_No_VSI_Counter >5)
		{
			SP_TX_Read_Reg(SP_TX_PORT0_ADDR, 0xea, &c);
			if(c&0x01)
			{
				D("No new VSI is received, disable  VSC packet");
				SP_TX_Write_Reg(SP_TX_PORT0_ADDR, 0xea, 0x00);//disable VSC
			}
			g_No_VSI_Counter = 0;
		
		}


	
	}
}

void SP_TX_Config_Slimport_Output(void)
{
	BYTE c;


	SP_CTRL_Clean_HDCP();

   	SP_TX_Set_ColorSpace();

       SP_TX_AVI_Setup();//initial AVI infoframe packet
       SP_TX_Config_Packets(AVI_PACKETS);

	

	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_COMMON_INT_MASK1, 0xf5);//unmask video clock change&format change int
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_COMMON_INT_STATUS1, 0x0a);//Clear video format and clock change-20111206-ANX.Fei

	mdelay(50);
	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_SYS_CTRL3_REG, &c);
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_SYS_CTRL3_REG, c);
	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_SYS_CTRL3_REG, &c);
	if(!(c & SP_TX_SYS_CTRL3_STRM_VALID))
	{
		D("video stream not valid!");
		return;
	}else
		D("video stream valid!");

		 
        if(sp_tx_ds_edid_hdmi)
        {
      		 SP_CTRL_Set_System_State(SP_TX_HDCP_AUTHENTICATION);
        }
        else
        {
            SP_TX_Power_Down(SP_TX_PWR_AUDIO);//power down audio when DVI
            SP_CTRL_Set_System_State(SP_TX_HDCP_AUTHENTICATION);
        }

}


/*
void SP_TX_Disable_Video_Input(void)
{
    BYTE c;
    SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL1_REG, &c);
    SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL1_REG, c &~SP_TX_VID_CTRL1_VID_EN);
}

*/

void SP_TX_LVTTL_Bit_Mapping(void)//the default mode is 12bit ddr
{
    
	HDMI_color_depth hdmi_input_color_depth = Hdmi_legacy;
	BYTE c,c1;
	
		SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_DEEP_COLOR_CTRL_REG, &c);  
	c &= 0xF0;
	if(c == 0x00)
	{
		SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_VIDEO_STATUS_REG1, &c1);
		c1 &= 0xf0;
		if(c1 == 0x00)
			hdmi_input_color_depth = Hdmi_legacy;
		else if(c1 == 0x40)   
			hdmi_input_color_depth = Hdmi_24bit;
		else if(c1 == 0x50)   
			hdmi_input_color_depth = Hdmi_30bit;
		else if(c1 == 0x60)   
			hdmi_input_color_depth = Hdmi_36bit;
		else 
			D("HDMI input color depth is not supported .\n");
	}
	else if(c == 0x40)   
		hdmi_input_color_depth = Hdmi_24bit;
	else if(c == 0x50)   
		hdmi_input_color_depth = Hdmi_30bit;
	else if(c == 0x60)   
		hdmi_input_color_depth = Hdmi_36bit;
	else if(c == 0x70)   
		D("HDMI input color depth is not supported .\n");

	switch(hdmi_input_color_depth)
	{
		case Hdmi_legacy:
		case Hdmi_24bit:
			SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL1_REG, &c);
			SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL1_REG, c&~0x04);//set SDR mode
			
			SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL2_REG, &c);
			SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL2_REG, ((c&0x8f)|0x10));//set input video 8-bit
			for(c=0; c<24; c++)
			{	 
				SP_TX_Write_Reg(SP_TX_PORT2_ADDR, 0x40+c, 0x00+c);
			}
			break;
		case Hdmi_30bit:
			SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL1_REG, &c);
			SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL1_REG, c&~0x04);//set SDR mode
			
			SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL2_REG, &c);
			SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL2_REG, ((c&0x8f)|0x20));//set input video 10-bit
			for(c=0; c<10; c++)
			{	 
				SP_TX_Write_Reg(SP_TX_PORT2_ADDR, 0x40+c, 0x02+c);//bit0-9:2~11
			}
			for(c=0; c<10; c++)
			{	 
				SP_TX_Write_Reg(SP_TX_PORT2_ADDR, (c+0x4a), (c+0x0E));//bit10-19:14~23
			}
			for(c=0; c<10; c++)
			{	 
				SP_TX_Write_Reg(SP_TX_PORT2_ADDR, 0x54+c, 0x1A+c);//bit20-29:26~35
			}
			break;
		case Hdmi_36bit:
			SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL1_REG, &c);
			SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL1_REG, c&~0x04);//set SDR mode
			
			SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL2_REG, &c);
			SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL2_REG, ((c&0x8f)|0x30));//set input video 12-bit
			for(c=0; c<36; c++)
			{	 
				SP_TX_Write_Reg(SP_TX_PORT2_ADDR, 0x40+c, 0x00+c);
			}
			break;
		default:
			break;
	}
        
	

}

void SP_TX_Int_Irq_Handler(void)
{
	BYTE c1,c2,c3,c4,c5;

	SP_TX_Get_Int_status(COMMON_INT_1,&c1);
	SP_TX_Get_Int_status(COMMON_INT_2,&c2);
	SP_TX_Get_Int_status(COMMON_INT_3,&c3);
	SP_TX_Get_Int_status(COMMON_INT_4,&c4);
	SP_TX_Get_Int_status(SP_INT_STATUS,&c5);

	//  if(c1 & SP_COMMON_INT1_VIDEO_CLOCK_CHG)//video clock change
	//      SP_CTRL_Video_Changed_Int_Handler(0);

	//  if(c1 & SP_COMMON_INT1_VIDEO_FORMAT_CHG)//video format change
	//   SP_CTRL_Video_Changed_Int_Handler(1);

	if(c1 & SP_COMMON_INT1_PLL_LOCK_CHG)//pll lock change
	SP_CTRL_PLL_Changed_Int_Handler();

	if(c1 & SP_COMMON_INT1_PLL_LOCK_CHG)//audio clock change
	SP_CTRL_AudioClk_Change_Int_Handler();

	// if(c2 & SP_COMMON_INT2_AUTHCHG)//auth change
	//SP_CTRL_Auth_Change_Int_Handler();

	if(c2 & SP_COMMON_INT2_AUTHDONE)//auth done
	SP_CTRL_Auth_Done_Int_Handler();

	/*added for B0 version-ANX.Fei-20110831-Begin*/
	if(c5 & SP_TX_INT_DPCD_IRQ_REQUEST)//IRQ int
	SP_CTRL_SINK_IRQ_Int_Handler();

	if(c5 & SP_TX_INT_STATUS1_POLLING_ERR)//c-wire polling error
	SP_CTRL_POLLING_ERR_Int_Handler();

	if(c5 & SP_TX_INT_STATUS1_TRAINING_Finish)//link training finish int
	SP_CTRL_LT_DONE_Int_Handler();

	if(c5 & SP_TX_INT_STATUS1_LINK_CHANGE)//link is lost  int
	SP_CTRL_LINK_CHANGE_Int_Handler();
	/*added for B0 version-ANX.Fei-20110831-End*/
}


 void SP_TX_Enable_Video_Input(BYTE Enable)
{
    BYTE c;

	//SP_TX_LVTTL_Bit_Mapping();
       if(Enable)
       {
		SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL1_REG, &c);
		c &= 0xf7;
		SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL1_REG, ( c | SP_TX_VID_CTRL1_VID_EN ));
		D("Slimport Video is Enabled!");
       
       }else{
		SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL1_REG, &c);
		SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL1_REG, ( c &~ SP_TX_VID_CTRL1_VID_EN ));
		D("Slimport Video is disabled!");
       
       }
}


void SP_TX_EnhaceMode_Set(void)
{
    BYTE c;    
	SP_TX_AUX_DPCDRead_Bytes(0x00,0x00,DPCD_MAX_LANE_COUNT,1,&c);
	if(c & 0x80)
	{

		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_SYS_CTRL4_REG, &c);
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_SYS_CTRL4_REG, c | SP_TX_SYS_CTRL4_ENHANCED);

		SP_TX_AUX_DPCDRead_Bytes(0x00,0x01,DPCD_LANE_COUNT_SET,1,&c);
		SP_TX_AUX_DPCDWrite_Byte(0x00,0x01,DPCD_LANE_COUNT_SET, c | 0x80);

		D("Enhance mode enabled");
	}
	else
	{

		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_SYS_CTRL4_REG, &c);
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_SYS_CTRL4_REG, c & (~SP_TX_SYS_CTRL4_ENHANCED));

		SP_TX_AUX_DPCDRead_Bytes(0x00,0x01,DPCD_LANE_COUNT_SET,1,&c);
		SP_TX_AUX_DPCDWrite_Byte(0x00,0x01,DPCD_LANE_COUNT_SET, c & (~0x80));

		D("Enhance mode disabled");
	}
}


void SP_TX_Clean_HDCP(void)
{
   // BYTE c;
    
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_HDCP_CONTROL_0_REG, 0x00);//disable HW HDCP

    //reset HDCP logic
    //SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_RST_CTRL_REG, &c);
    //SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_RST_CTRL_REG, c | SP_TX_RST_HDCP_REG);
    //SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_RST_CTRL_REG, c& ~SP_TX_RST_HDCP_REG);

    //set re-auth
    SP_TX_HDCP_ReAuth();

}

void SP_TX_HDCP_Encryption_Disable(void) 
{
    BYTE c;     
    SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_HDCP_CONTROL_0_REG, &c);
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_HDCP_CONTROL_0_REG, c & ~SP_TX_HDCP_CONTROL_0_HDCP_ENC_EN);
}

void SP_TX_HDCP_Encryption_Enable(void)
{
    BYTE c;
    SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_HDCP_CONTROL_0_REG, &c);
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_HDCP_CONTROL_0_REG, c |SP_TX_HDCP_CONTROL_0_HDCP_ENC_EN);
}

void SP_TX_HW_HDCP_Enable(void)
{
    BYTE c;
    SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_HDCP_CONTROL_0_REG, &c);
    //disable HDCP and encryption first
    c&=0xf3;
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_HDCP_CONTROL_0_REG, c );
    c|=0x0c;//enable HDCP and encryption
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_HDCP_CONTROL_0_REG, c );
    D("Hardware HDCP is enabled.");

}

void SP_TX_HW_HDCP_Disable(void)
{
    BYTE c;
    SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_HDCP_CONTROL_0_REG, &c);
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_HDCP_CONTROL_0_REG, c & ~SP_TX_HDCP_CONTROL_0_HARD_AUTH_EN);
}

void SP_TX_PCLK_Calc(SP_LINK_BW hbr_rbr)
{
    long int str_clk = 0;
    BYTE c;

    switch(hbr_rbr)
    {
      case BW_54G:
	  	str_clk = 540;
		break;
      case BW_27G:
	  	str_clk = 270;
		break;
      case BW_162G:
	  	str_clk = 162;
		break;
	default:
		break;
	  
    }


    SP_TX_Read_Reg(SP_TX_PORT0_ADDR,M_VID_2, &c);
    M_val = c * 0x10000;
    SP_TX_Read_Reg(SP_TX_PORT0_ADDR,M_VID_1, &c);
    M_val = M_val + c * 0x100;
    SP_TX_Read_Reg(SP_TX_PORT0_ADDR,M_VID_0, &c);
    M_val = M_val + c;

    SP_TX_Read_Reg(SP_TX_PORT0_ADDR,N_VID_2, &c);
    N_val = c * 0x10000;
    SP_TX_Read_Reg(SP_TX_PORT0_ADDR,N_VID_1, &c);
    N_val = N_val + c * 0x100;
    SP_TX_Read_Reg(SP_TX_PORT0_ADDR,N_VID_0, &c);
    N_val = N_val + c;

    str_clk = str_clk * M_val;
    pclk = str_clk ;
    pclk = pclk / N_val;
}


void SP_TX_Show_Infomation(void)
{
	BYTE c,c1;
	unsigned int h_res,h_act,v_res,v_act;
	unsigned int h_fp,h_sw,h_bp,v_fp,v_sw,v_bp;
	unsigned long fresh_rate;

	D("\n*************************SP Video Information*************************\n");


	SP_TX_Read_Reg(SP_TX_PORT0_ADDR,SP_TX_LINK_BW_SET_REG, &c);
	if(c==0x06)
	{
		D("   BW = 1.62G");
		SP_TX_PCLK_Calc(BW_162G);//str_clk = 162;
	}
	else if(c==0x0a)
	{
		D("   BW = 2.7G");
		SP_TX_PCLK_Calc(BW_27G);//str_clk = 270;
	}
	else if(c==0x14)
	{
		D("   BW = 5.4G");
		SP_TX_PCLK_Calc(BW_54G);//str_clk = 540;
	}
	
	

	if(sp_tx_ssc_enable)
		D("   SSC On");
	else
		D("   SSC Off");

	D("   M = %lu, N = %lu, PCLK = %ld MHz\n",M_val,N_val,pclk);

	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_TOTAL_LINE_STA_L,&c);
	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_TOTAL_LINE_STA_H,&c1);

	v_res = c1;
	v_res = v_res << 8;
	v_res = v_res + c;


	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_ACT_LINE_STA_L,&c);
	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_ACT_LINE_STA_H,&c1);

	v_act = c1;
	v_act = v_act << 8;
	v_act = v_act + c;


	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_TOTAL_PIXEL_STA_L,&c);
	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_TOTAL_PIXEL_STA_H,&c1);

	h_res = c1;
	h_res = h_res << 8;
	h_res = h_res + c;


	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_ACT_PIXEL_STA_L,&c);
	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_ACT_PIXEL_STA_H,&c1);

	h_act = c1;
	h_act = h_act << 8;
	h_act = h_act + c;

	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_H_F_PORCH_STA_L,&c);
	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_H_F_PORCH_STA_H,&c1);
	
	h_fp = c1;
	h_fp = h_fp << 8;
	h_fp = h_fp + c;

	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_H_SYNC_STA_L,&c);
	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_H_SYNC_STA_H,&c1);
	
	h_sw = c1;
	h_sw = h_sw << 8;
	h_sw = h_sw + c;

	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_H_B_PORCH_STA_L,&c);
	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_H_B_PORCH_STA_H,&c1);
	
	h_bp = c1;
	h_bp = h_bp << 8;
	h_bp = h_bp + c;

	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_V_F_PORCH_STA,&c);
	v_fp = c;

	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_V_SYNC_STA,&c);
	v_sw = c;

	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_V_B_PORCH_STA,&c);
	v_bp = c;
	
	D("   Total resolution is %d * %d \n", h_res, v_res);
	
	D("   HF=%d, HSW=%d, HBP=%d\n", h_fp, h_sw, h_bp);
	D("   VF=%d, VSW=%d, VBP=%d\n", v_fp, v_sw, v_bp);
	D("   Active resolution is %d * %d ", h_act, v_act);
	

	fresh_rate = pclk * 1000;
	fresh_rate = fresh_rate / h_res;
	fresh_rate = fresh_rate * 1000;
	fresh_rate = fresh_rate / v_res;
	D(" @ %ldHz\n", fresh_rate);
	

	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_VID_CTRL,&c);
	if((c & 0x06) == 0x00)
	D("   ColorSpace: RGB,");
	else if((c & 0x06) == 0x02)
	D("   ColorSpace: YCbCr422,");
	else if((c & 0x06) == 0x04)
	D("   ColorSpace: YCbCr444,");


	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_VID_CTRL,&c);
	if((c & 0xe0) == 0x00)
	D("  6 BPC");
	else if((c & 0xe0) == 0x20)
	D("  8 BPC");
	else if((c & 0xe0) == 0x40)
	D("  10 BPC");
	else if((c & 0xe0) == 0x60)
	D("  12 BPC");

#ifdef ANX7730
	SP_TX_AUX_DPCDRead_Bytes(0x00, 0x05, 0x23, 1, ByteBuf);
	D("   ANX7730 BB current FW Ver %.2x \n", (long unsigned int)(ByteBuf[0]&0x0f));

		
#endif

	D("\n********************************************************************\n");

}

   



void SP_TX_AUX_WR (BYTE offset)
{
	BYTE c,cnt;
	cnt = 0;
    //load offset to fifo
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_BUF_DATA_0_REG, offset);
    //set I2C write com 0x04 mot = 1
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG, 0x04);
	//enable aux
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, 0x01);
    SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);
    while(c&0x01)
    {
       mdelay(10);
        cnt ++;
        //D("cntwr = %.2x\n",(long unsigned int)cnt);
        if(cnt == 10)
        {
            D("write break");
            //SP_TX_RST_AUX();
            cnt = 0;
            bEDIDBreak=1;
            break;
        }
        SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);
    }

} 

void SP_TX_AUX_RD (BYTE len_cmd)
{
	BYTE c,cnt;
	cnt = 0;
	
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG, len_cmd);
	//enable aux
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, 0x01);
	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);
	while(c & 0x01)
	{
		mdelay(10);
		cnt ++;
        //D("cntrd = %.2x\n",(long unsigned int)cnt);
		if(cnt == 10)
		{
			D("read break");
			SP_TX_RST_AUX();
                     bEDIDBreak=1;
			break;
		}
		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);
	}

}



BYTE SP_TX_Chip_Located(void)
{
    BYTE c1=0,c2=0;
	int i=0;
    SP_TX_Hardware_PowerOn();
	for(;i<10;i++){
    SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_DEV_IDL_REG , &c1);
    SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_DEV_IDH_REG , &c2);
	msleep(20);
	printk("SP_TX_Chip_ID =  %x,%x=====================================\n",c1,c2);
	}

    if ((c1==0x08) && (c2==0x78))
        return 1;
    else  
        return 0;         
}
void SP_TX_Hardware_PowerOn(void) 
{
#if 1
        gpio_set_value(SP_TX_HW_RESET,0);//SP_TX_HW_RESET= 0;
        mdelay(20);
	printk("SP_TX_Hardware_PowerOn: SP_TX_HW_RESET low = %d\n", gpio_get_value(SP_TX_HW_RESET));		
#endif	
	gpio_set_value(SP_TX_CHIP_PD_CTRL,0);//SP_TX_CHIP_PD_CTRL= 0;	
	mdelay(20);
	
//	gpio_set_value(SP_TX_PWR_V10_CTRL,0);//SP_TX_PWR_V12_CTRL= 0;
//	mdelay(10);
//	gpio_set_value(SP_TX_PWR_V10_CTRL,1);//SP_TX_PWR_V12_CTRL= 1;
//	mdelay(500);


#if 1
       // gpio_set_value(SP_TX_HW_RESET,1);//SP_TX_HW_RESET= 0;
        mdelay(100);
	printk("SP_TX_Hardware_PowerOn: SP_TX_HW_RESET high= %d\n", gpio_get_value(SP_TX_HW_RESET));		

       // gpio_set_value(SP_TX_HW_RESET,0);//SP_TX_HW_RESET= 0;
        mdelay(500);
	printk("SP_TX_Hardware_PowerOn: SP_TX_HW_RESET high= %d\n", gpio_get_value(SP_TX_HW_RESET));		


        gpio_set_value(SP_TX_HW_RESET,1);//SP_TX_HW_RESET= 0;
        mdelay(20);
	printk("SP_TX_Hardware_PowerOn: SP_TX_HW_RESET high= %d\n", gpio_get_value(SP_TX_HW_RESET));		
#endif		
	printk("Chip is power on\n");

}

void SP_TX_Hardware_PowerDown(void) 
{
return ;
//	gpio_set_value(SP_TX_HW_RESET,0);// SP_TX_HW_RESET= 0;
	mdelay(10);
	//gpio_set_value(SP_TX_PWR_V10_CTRL,0);//SP_TX_PWR_V12_CTRL= 0;
	mdelay(10);
	gpio_set_value(SP_TX_CHIP_PD_CTRL,1);//SP_TX_CHIP_PD_CTRL = 1;
       mdelay(20);

	D("Chip is power down\n");
	
}

void SP_TX_VBUS_PowerOn(void) 
{
     BYTE c;

	 //power down macro
	 SP_TX_Read_Reg (SP_TX_PORT0_ADDR, SP_TX_ANALOG_POWER_DOWN_REG, &c);
	 c|= SP_TX_ANALOG_POWER_DOWN_MACRO_PD;
	 SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_ANALOG_POWER_DOWN_REG, c); 
    //Power up  5V detect and short portect circuit
	SP_TX_Read_Reg (SP_TX_PORT2_ADDR, PLL_FILTER_CTRL6, &c);
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, PLL_FILTER_CTRL6, c& ~0x30); 

    // Enable power 3.3v out
	SP_TX_Read_Reg (SP_TX_PORT2_ADDR, PLL_FILTER_CTRL1, &c);
	c&=0xFd;
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, PLL_FILTER_CTRL1, c);
	c|=0x02;
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, PLL_FILTER_CTRL1, c);
	
	mdelay(100);

	//power on macro
	SP_TX_Read_Reg (SP_TX_PORT0_ADDR, SP_TX_ANALOG_POWER_DOWN_REG, &c);
	c&= ~SP_TX_ANALOG_POWER_DOWN_MACRO_PD;
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_ANALOG_POWER_DOWN_REG, c); 
	
	D("3.3V output enabled");
	
}

void SP_TX_VBUS_PowerDown(void) 
{
	BYTE c;
	
    // Disableable power 3.3v out
	SP_TX_Read_Reg (SP_TX_PORT2_ADDR, PLL_FILTER_CTRL1, &c);
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, PLL_FILTER_CTRL1, (c& (~V33_SWITCH_ON))); 

    //Power down  5V detect and short portect circuit
	SP_TX_Read_Reg (SP_TX_PORT2_ADDR, PLL_FILTER_CTRL6, &c);
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, PLL_FILTER_CTRL6, c|0x30); 
	D("3.3V output disabled");
}



void SP_TX_CONFIG_SSC(SP_LINK_BW linkbw) 
{
	BYTE c;

	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SSC_CTRL_REG1, 0x00); 			// disable SSC first
	//SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_LINK_BW_SET_REG, 0x00);		//disable speed first

	
	SP_TX_AUX_DPCDRead_Bytes(0x00, 0x00,DPCD_MAX_DOWNSPREAD,1,&c);
          

#ifndef SSC_1
	//D("############### Config SSC 0.4% ####################");
	if(linkbw == BW_54G) 
	{
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL1, 0xc0);	              // set value according to mehran CTS report -ANX.Fei-20111009
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL2, 0x00);	              
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL3, 0x75);			// ctrl_th 
	
	}
	else if(linkbw == BW_27G) 
	{
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL1, 0x5f);			//  set value according to mehran CTS report -ANX.Fei-20111009
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL2, 0x00);	
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL3, 0x75);			// ctrl_th 
	}
	else 
	{
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL1, 0x9e);         	//  set value according to mehran CTS report -ANX.Fei-20111009
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL2, 0x00);	
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL3, 0x6d);			// ctrl_th 
	}
#else
      //D("############### Config SSC 1% ####################");
	if(linkbw == BW_54G) 
	{
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL1, 0xdd);	              // ssc d  1%, f0/8 mode
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL2, 0x01);	              
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL3, 0x76);			// ctrl_th 
	}
	else if(linkbw == BW_27G) 
	{
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL1, 0xef);			// ssc d  1%, f0/4 mode
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL2, 0x00);	
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL3, 0x76);			// ctrl_th 
	}
	else 
	{
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL1, 0x8e);			// ssc d 0.4%, f0/4 mode
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL2, 0x01);	
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL3, 0x6d);			// ctrl_th 
	}
#endif

	//Enable SSC
	SP_TX_SPREAD_Enable(1);


}


void SP_TX_Enable_Audio_Output(BYTE bEnable)
{
	BYTE c;

	SP_TX_Read_Reg (SP_TX_PORT0_ADDR, SP_TX_AUD_CTRL, &c);
	if(bEnable)
	{
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUD_CTRL, ( c| SP_TX_AUD_CTRL_AUD_EN ) ); // enable SP audio
		
		// write audio info-frame
		SP_TX_AudioInfoFrameSetup();
		SP_TX_InfoFrameUpdate(&SP_TX_AudioInfoFrmae);
	}
	else
	{
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUD_CTRL, (c &(~SP_TX_AUD_CTRL_AUD_EN))); // Disable SP audio

		
		SP_TX_Read_Reg (SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, &c);
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, ( c&(~SP_TX_PKT_AUD_EN )) ); // Disable the audio info-frame
	}
    

}


void SP_TX_AudioInfoFrameSetup(void)
{
	int i;
	BYTE c;
	
	SP_TX_AudioInfoFrmae.type = 0x84;
	SP_TX_AudioInfoFrmae.version = 0x01;
	SP_TX_AudioInfoFrmae.length = 0x0A;



	for(i=0;i<11;i++)
	{
		SP_TX_Read_Reg(HDMI_PORT1_ADDR, (0xc3+i), &c);
		SP_TX_AudioInfoFrmae.pb_byte[i]=c;
	}
/*
	SP_TX_AudioInfoFrmae.pb_byte[0]=0x00;//coding type ,refer to stream header, audio channel count,two channel
	SP_TX_AudioInfoFrmae.pb_byte[1]=0x00;//refer to stream header
	SP_TX_AudioInfoFrmae.pb_byte[2]=0x00;
	SP_TX_AudioInfoFrmae.pb_byte[3]=0x00;//for multi channel LPCM
	SP_TX_AudioInfoFrmae.pb_byte[4]=0x00;//for multi channel LPCM
	SP_TX_AudioInfoFrmae.pb_byte[5]=0x00;//reserved to 0
	SP_TX_AudioInfoFrmae.pb_byte[6]=0x00;//reserved to 0
	SP_TX_AudioInfoFrmae.pb_byte[7]=0x00;//reserved to 0
	SP_TX_AudioInfoFrmae.pb_byte[8]=0x00;//reserved to 0
	SP_TX_AudioInfoFrmae.pb_byte[9]=0x00;//reserved to 0
*/
}

void SP_TX_InfoFrameUpdate(struct AudiInfoframe* pAudioInfoFrame)
{
	BYTE c;

	c = pAudioInfoFrame->type;
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AUD_TYPE, c); // Audio infoframe

	
	c = pAudioInfoFrame->version;
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AUD_VER,	c);

	c = pAudioInfoFrame->length;
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AUD_LEN,	c);

	c = pAudioInfoFrame->pb_byte[0];
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AUD_DB0,c);
	
	c = pAudioInfoFrame->pb_byte[1];
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AUD_DB1,c);

	c = pAudioInfoFrame->pb_byte[2];
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AUD_DB2,c);

	c = pAudioInfoFrame->pb_byte[3];
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AUD_DB3,c);

	c = pAudioInfoFrame->pb_byte[4];
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AUD_DB4,c);

	c = pAudioInfoFrame->pb_byte[5];
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AUD_DB5,c);

	c = pAudioInfoFrame->pb_byte[6];
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AUD_DB6,c);

	c = pAudioInfoFrame->pb_byte[7];
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AUD_DB7,c);

	c = pAudioInfoFrame->pb_byte[8];
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AUD_DB8,c);

	c = pAudioInfoFrame->pb_byte[9];
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AUD_DB9,c);

	c = pAudioInfoFrame->pb_byte[10];
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AUD_DB10,c);
	

	SP_TX_Read_Reg (SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, &c);
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, ( c | SP_TX_PKT_AUD_UP ) ); // update the audio info-frame

		
	SP_TX_Read_Reg (SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, &c);
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, ( c | SP_TX_PKT_AUD_EN ) ); // enable the audio info-frame
}
	

void SP_TX_Config_Audio(void) 
{
	BYTE c ,current_bw;
	int i;
	unsigned long  M_AUD,LS_Clk= 0;
	unsigned long AUD_Freq = 0;
	D("############## Config audio #####################");

       SP_TX_Power_On(SP_TX_PWR_AUDIO);
	SP_TX_Read_Reg(HDMI_PORT0_ADDR, 0xCA, &c);
	switch(c&0x0f)
	{
	   case 0x00: AUD_Freq = 44.1;break;
	   case 0x02: AUD_Freq = 48;break;
	   case 0x03: AUD_Freq = 32;break;
	   case 0x08: AUD_Freq = 88.2;break;
	   case 0x0a: AUD_Freq = 96;break;
	   case 0x0c: AUD_Freq = 176.4;break;
	   case 0x0e: AUD_Freq = 192;break;
	   default: break;
	}

       SP_TX_Get_Link_BW(&current_bw);
	switch(current_bw)
	{
	   case BW_162G: LS_Clk = 162000;break;
	   case BW_27G: LS_Clk = 270000;break;
	   case BW_54G: LS_Clk = 540000;break;
	   default: break;
	}

	//D("AUD_Freq = %ld , LS_CLK = %ld\n",AUD_Freq,LS_Clk);
	
	M_AUD= ((512*AUD_Freq)/LS_Clk)*32768;
	//D("M_AUD = %ld\n",M_AUD);
	M_AUD = M_AUD +0x05;
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x67, (M_AUD&0xff));
	M_AUD = M_AUD>>8;
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x68, (M_AUD&0xff));
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x69, 0x00);


	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x5f, 0x00);//let hdmi audio sample into DP
	  
	
	//Start the M_AUD caculation
	//SP_TX_Read_Reg(SP_TX_PORT1_ADDR, 0x60, &c);
	SP_TX_Write_Reg(SP_TX_PORT1_ADDR, 0x60, 0x04);//enable audio auto adjust

       //configure layout and channel number
	SP_TX_Read_Reg(HDMI_PORT0_ADDR, 0x15, &c);
	if(c&0x08)
	{
		SP_TX_Read_Reg(SP_TX_PORT2_ADDR, 0xd5, &c);
		SP_TX_Write_Reg(SP_TX_PORT2_ADDR, 0xd5, c|0xe1);
	}

       //transfer audio chaneel status from HDMI Rx to Slinmport Tx
	for(i=0;i<5;i++)
	{
		SP_TX_Read_Reg(HDMI_PORT0_ADDR, (0xc7+i), &c);
		SP_TX_Write_Reg(SP_TX_PORT2_ADDR, (0xD0+i), c);
	}
	
	   
	SP_TX_Enable_Audio_Output(1);//enable audio
       SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_COMMON_INT_MASK1, &c);
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_COMMON_INT_MASK1, c|0x04);//Unmask audio clock change int

}



void SP_TX_RST_AUX(void)
{
	BYTE c,c1;

	//D("reset aux");

	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_DEBUG_REG1, &c1);
	c = c1;
	c1&=0xdd;//clear HPD polling and Transmitter polling
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DEBUG_REG1, c1); //disable  polling  before reset AUX-ANX.Fei-2011.9.19 
	
	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_RST_CTRL2_REG, &c1);
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_RST_CTRL2_REG, c1|SP_TX_AUX_RST);
	mdelay(1);
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_RST_CTRL2_REG, c1& (~SP_TX_AUX_RST));

	//set original polling enable
	//SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_DEBUG_REG1, &c1);
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DEBUG_REG1, c); //enable  polling  after reset AUX-ANX.Fei-2011.9.19 
}


BYTE SP_TX_AUX_DPCDRead_Bytes(BYTE addrh, BYTE addrm, BYTE addrl,BYTE cCount,pByte pBuf)
{
	BYTE c,i;
	BYTE bOK;

	//clr buffer
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_BUF_DATA_COUNT_REG, 0x80);

	//set read cmd and count
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG, ((cCount-1) <<4)|0x09);


	//set aux address15:0
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_7_0_REG, addrl);
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_15_8_REG, addrm);

	//set address19:16 and enable aux
	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_19_16_REG, &c);
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_19_16_REG, (c & 0xf0) | addrh);


	//Enable Aux
	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, c | 0x01);


	mdelay(2);

	bOK = SP_TX_Wait_AUX_Finished();

	if(!bOK)
    	{
        	//D("aux read failed");
		SP_TX_RST_AUX();
        	return AUX_ERR;
    	}

		
	//SP_TX_Wait_AUX_Finished();
	for(i =0;i<cCount;i++)
	{
		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_BUF_DATA_0_REG+i, &c);

		*(pBuf+i) = c;

		if(i >= MAX_BUF_CNT)
			break;
	}

	return AUX_OK;
	
}


BYTE SP_TX_AUX_DPCDWrite_Bytes(BYTE addrh, BYTE addrm, BYTE addrl,BYTE cCount,pByte pBuf)
{
	BYTE c,i;
	BYTE bOK;
	

	//clr buffer
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_BUF_DATA_COUNT_REG, 0x80);

	//set write cmd and count;
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG, ((cCount-1) <<4)|0x08);

	//set aux address15:0
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_7_0_REG, addrl);
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_15_8_REG, addrm);

	//set address19:16
	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_19_16_REG, &c);
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_19_16_REG, (c & 0xf0) | addrh);


	//write data to buffer
	for(i =0;i<cCount;i++)
	{
		c = *pBuf;
		pBuf++;
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_BUF_DATA_0_REG+i, c);

		if(i >= MAX_BUF_CNT)
			break;
	}

	//Enable Aux
	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, c | 0x01);

	bOK = SP_TX_Wait_AUX_Finished();

	if(bOK)
		return AUX_OK;
	else
	{
        	//D("aux write failed");
		//SP_TX_RST_AUX();
		return AUX_ERR;
	}

}


void SP_TX_AUX_DPCDWrite_Byte(BYTE addrh, BYTE addrm, BYTE addrl, BYTE data1)
{

    BYTE c;


    //clr buffer
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_BUF_DATA_COUNT_REG, 0x80);

    //set write cmd and count;
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG, (0 <<4)|0x08);

    //set aux address15:0
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_7_0_REG, addrl);
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_15_8_REG, addrm);

    //set address19:16
    SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_19_16_REG, &c);
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_19_16_REG, (c & 0xf0) | addrh);


    //write data to buffer
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_BUF_DATA_0_REG, data1);

    //Enable Aux
    SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, c | 0x01);

    SP_TX_Wait_AUX_Finished();
	
    return ;
 }



BYTE SP_TX_Wait_AUX_Finished(void)
{
	BYTE c;
	BYTE cCnt;
	cCnt = 0;
	
	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_STATUS, &c);
	while(c & 0x10)
	{
		cCnt++;
               
		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_STATUS, &c);

		if(cCnt>100)
                 {
                   //D("AUX Operaton does not finished, and tome out.");
                   break; 
                 }	
	}

    if(c&0x0F)
    {
        //D("aux operation failed %.2x\n",(long unsigned int)c);
        return 0;
    }
    else
        return 1; //succeed
	
}



void SP_TX_SPREAD_Enable(BYTE bEnable)
{
	BYTE c;

	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SSC_CTRL_REG1, &c);
	
	if(bEnable)
	{
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SSC_CTRL_REG1, c | SPREAD_AMP);// enable SSC
		//SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL2, (c &(~0x04)));// powerdown SSC

		//reset SSC
		SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_RST_CTRL2_REG, &c);
		SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_RST_CTRL2_REG, c | SP_TX_RST_SSC);
		SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_RST_CTRL2_REG, c & (~SP_TX_RST_SSC));
                    
		//enable the DPCD SSC
		SP_TX_AUX_DPCDRead_Bytes(0x00, 0x01, DPCD_DOWNSPREAD_CTRL,1,&c);
              SP_TX_AUX_DPCDWrite_Byte(0x00, 0x01, DPCD_DOWNSPREAD_CTRL, (c | 0x10));

	}
	else
	{
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SSC_CTRL_REG1, (c & (~SPREAD_AMP)));// disable SSC
              //SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_DOWN_SPREADING_CTRL2, c|0x04);// powerdown SSC
		//disable the DPCD SSC
		SP_TX_AUX_DPCDRead_Bytes(0x00, 0x01, DPCD_DOWNSPREAD_CTRL,1,&c);
		SP_TX_AUX_DPCDWrite_Byte(0x00, 0x01, DPCD_DOWNSPREAD_CTRL, (c & 0xef));
	}
		
}



void SP_TX_Get_Int_status(INTStatus IntIndex, BYTE *cStatus)
{
	BYTE c;

	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_COMMON_INT_STATUS1 + IntIndex, &c);
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_COMMON_INT_STATUS1 + IntIndex, c);

	*cStatus = c;
}


BYTE SP_TX_Get_PLL_Lock_Status(void)
{
	BYTE c;
	
	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_DEBUG_REG1, &c);
	if(c & SP_TX_DEBUG_PLL_LOCK)
	{
		return 1;
	}
	else
		return 0;

}


void SP_TX_Get_HDCP_status( BYTE *cStatus)
{
	BYTE c;
	
	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_HDCP_STATUS, &c);

	*cStatus = c;
}


void SP_TX_HDCP_ReAuth(void)
{	
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_HDCP_CONTROL_0_REG, 0x23);
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_HDCP_CONTROL_0_REG, 0x03);
}



void SP_TX_Get_Rx_LaneCount(BYTE bMax,BYTE *cLaneCnt)
{
	if(bMax)
	    SP_TX_AUX_DPCDRead_Bytes(0x00, 0x00,DPCD_MAX_LANE_COUNT,1,cLaneCnt);
	else
	    SP_TX_AUX_DPCDRead_Bytes(0x00, 0x01,DPCD_LANE_COUNT_SET,1,cLaneCnt);

}


void SP_TX_Get_Rx_BW(BYTE bMax,BYTE *cBw)
{
	if(bMax)
	   SP_TX_AUX_DPCDRead_Bytes(0x00, 0x00,DPCD_MAX_LINK_RATE,1,cBw);
	else
	    SP_TX_AUX_DPCDRead_Bytes(0x00, 0x01,DPCD_LINK_BW_SET,1,cBw);
 
}


void SP_TX_Get_Link_BW(BYTE *bwtype)
{
	BYTE c;

	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_LINK_BW_SET_REG, &c);

	*bwtype = c;
}


void SP_TX_EDID_Read_Initial(void)
{
	BYTE c;

	//Set I2C address	
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_7_0_REG, 0x50);
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_15_8_REG, 0);
    SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_19_16_REG, &c);
    SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_19_16_REG, c & 0xf0);
}


BYTE SP_TX_AUX_EDIDRead_Byte(BYTE offset)
{
    BYTE c,i,edid[16],data_cnt,cnt,vsdbdata[4],VSDBaddr;
    BYTE bReturn=0;


    cnt = 0;
	   
    SP_TX_AUX_WR(offset);//offset 
    
    if((offset == 0x00) || (offset == 0x80))
	checksum = 0;
	
       SP_TX_AUX_RD(0xf5);//set I2C read com 0x05 mot = 1 and read 16 bytes
       
	data_cnt = 0;
	while(data_cnt < 16)
	{
		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_BUF_DATA_COUNT_REG, &c);
		c = c & 0x1f;
		if(c != 0)
		{
		    for( i = 0; i < c; i ++)
		    {
		        SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_BUF_DATA_0_REG + i, &edid[i + data_cnt]);
		        checksum = checksum + edid[i + data_cnt];
		    }
		}
		else
		{
			SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG, 0x01);
			//enable aux
			SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, 0x03);//set address only
			SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);
			while(c & 0x01)
        	      {
	        		mdelay(2);
	        		cnt ++;
	        		if(cnt == 10)
	        		{
	        			D("read break");
	        			SP_TX_RST_AUX();
	                           bEDIDBreak=1;
	        			    bReturn = 0x01;
	        		}
	                    SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);
        	      }
	             sp_tx_edid_err_code = 0xff;
	             bReturn = 0x02;// for fixing bug leading to dead lock in loop "while(data_cnt < 16)"
	             return bReturn;
		}
		data_cnt = data_cnt + c;
		if(data_cnt < 16)// 080610. solution for handle case ACK + M byte
		{
			
			SP_TX_RST_AUX();
                     mdelay(10);

			c = 0x05 | ((0x0f - data_cnt) << 4);//Read MOT = 1
			SP_TX_AUX_RD(c);
		}
	}

       SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG, 0x01);
	//enable aux
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, 0x03);//set address only to stop EDID reading
	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);
	while(c & 0x01)
		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);



	if(EDID_Print_Enable)
	{
		for(i=0;i<16;i++) 
		{
			if((i&0x0f)==0)
				D("\n edid: [%.2x]  %.2x  ", (unsigned int)offset, (unsigned int)edid[i]);
			else 
				D("%.2x  ", (unsigned int)edid[i]);

			if((i&0x0f)==0x0f)
				D("\n");
		}

	}


    if(offset == 0x00)
    {
        if((edid[0] == 0) && (edid[7] == 0) && (edid[1] == 0xff) && (edid[2] == 0xff) && (edid[3] == 0xff)
            && (edid[4] == 0xff) && (edid[5] == 0xff) && (edid[6] == 0xff))
            D("Good EDID header!");
        else
        {
            D("Bad EDID header!");
            sp_tx_edid_err_code = 0x01;
        }
            
    }

    else if(offset == 0x30)
    {
        for(i = 0; i < 10; i ++ )
            SP_TX_EDID_PREFERRED[i] = edid[i + 6];//edid[0x36]~edid[0x3f]
    }

    else if(offset == 0x40)
    {
        for(i = 0; i < 8; i ++ )
            SP_TX_EDID_PREFERRED[10 + i] = edid[i];//edid[0x40]~edid[0x47]
    }

    else if(offset == 0x70)
    {
       checksum = checksum&0xff;
        checksum = checksum - edid[15];
        checksum = ~checksum + 1;
        if(checksum != edid[15])
        {
            D("Bad EDID check sum1!");
            sp_tx_edid_err_code = 0x02;
            checksum = edid[15];
        }
	 else
	 	D("Good EDID check sum1!");
    }
    else if( (offset >= 0x80)&&(sp_tx_ds_edid_hdmi==0))
    {
       if(offset ==0x80)
       {
	       if(edid[0] !=0x02)
		   return 0x03;
       }
        for(i=0;i<16;i++)//record all 128 data in extsion block.
            EDIDExtBlock[offset-0x80+i]=edid[i];
/*
		for(i=0;i<16;i++) 
		{
			if((i&0x0f)==0)
				D("\n edid: [%.2x]  %.2x  ", (unsigned int)offset, (unsigned int)EDIDExtBlock[offset-0x80+i]);
			else 
				D("%.2x  ", (unsigned int)edid[i]);

			if((i&0x0f)==0x0f)
				D("\n");
		}*/
        
        if(offset ==0x80)
           DTDbeginAddr = edid[2];

        if(offset == 0xf0)
         {
		checksum = checksum - edid[15];
		checksum = ~checksum + 1;
		if(checksum != edid[15])
		{
			D("Bad EDID check sum2!");
			sp_tx_edid_err_code = 0x02;
		}
		else
			D("Good EDID check sum2!");
	 
            for(VSDBaddr = 0x04;VSDBaddr <DTDbeginAddr;)
            {

                vsdbdata[0] = EDIDExtBlock[VSDBaddr];
                vsdbdata[1] = EDIDExtBlock[VSDBaddr+1];
                vsdbdata[2] = EDIDExtBlock[VSDBaddr+2];
                vsdbdata[3] = EDIDExtBlock[VSDBaddr+3];

                if((vsdbdata[0]&0xe0)==0x60)
                {


			if((vsdbdata[0]&0x1f) > 0x08)
			{
				if((EDIDExtBlock[VSDBaddr+8]&0xc0)	== 0x80)
				{
					if((EDIDExtBlock[VSDBaddr+11]&0x80)	== 0x80)
					sp_tx_ds_edid_3d_present = 1;
					D("Downstream monitor supports 3D");
						
				}
				else if((EDIDExtBlock[VSDBaddr+8]&0xc0)	== 0x40)
				{
					if((EDIDExtBlock[VSDBaddr+11]&0x80)	== 0x80)
					sp_tx_ds_edid_3d_present = 1;
					D("Downstream monitor supports 3D");
						
				}
				else if((EDIDExtBlock[VSDBaddr+8]&0xc0)	== 0xc0)
				{
					if((EDIDExtBlock[VSDBaddr+13]&0x80)	== 0x80)
					sp_tx_ds_edid_3d_present = 1;
					D("Downstream monitor supports 3D");
						
				}
    				else if((EDIDExtBlock[VSDBaddr+8]&0xc0)	== 0x00)
				{     
					if((EDIDExtBlock[VSDBaddr+9]&0x80)	== 0x80)
					sp_tx_ds_edid_3d_present = 1;
					D("Downstream monitor supports 3D");
						
				}
				else
				{
					sp_tx_ds_edid_3d_present = 0;
					D("Downstream monitor does not support 3D");
				}
			
			
			}
							
                    if((vsdbdata[1]==0x03)&&(vsdbdata[2]==0x0c)&&(vsdbdata[3]==0x00))
                    {
                        sp_tx_ds_edid_hdmi = 1;
                        return 0;
                    }
                    else
                    {
                        sp_tx_ds_edid_hdmi = 0;
                        return 0x03;
                    }

                }	
                else
                {
                    sp_tx_ds_edid_hdmi = 0;
                    VSDBaddr = VSDBaddr+(vsdbdata[0]&0x1f);
                    VSDBaddr = VSDBaddr + 0x01;
                }
        
                if(VSDBaddr > DTDbeginAddr)
                    return 0x03;

            } 
         }
          
    }      

return bReturn;
}

void SP_TX_Parse_Segments_EDID(BYTE segment, BYTE offset)
{
	BYTE c,cnt;
	int i;

	//set I2C write com 0x04 mot = 1
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG, 0x04);

	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_7_0_REG, 0x30);	

	// adress_only
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, 0x03);//set address only

	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);
	
	//while(c & 0x01)
	//	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);
	SP_TX_Wait_AUX_Finished();
	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG, &c);

	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_BUF_DATA_0_REG, segment);

	//set I2C write com 0x04 mot = 1
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG, 0x04);
	//enable aux
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, 0x01);
	cnt = 0;
	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);

	while(c&0x01)
	{
		mdelay(10);
		cnt ++;
		//D("cntwr = %.2x\n",(long unsigned int)cnt);
		if(cnt == 10)
		{
			D("write break");
			SP_TX_RST_AUX();
			cnt = 0;
			bEDIDBreak=1;
		      	return;// bReturn;
		}
		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);
	
	}

	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_ADDR_7_0_REG, 0x50);//set EDID addr 0xa0	
	// adress_only
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, 0x03);//set address only

	SP_TX_AUX_WR(offset);//offset   
	//adress_only
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, 0x03);//set address only

	SP_TX_AUX_RD(0xf5);//set I2C read com 0x05 mot = 1 and read 16 bytes    
       cnt = 0;
	for(i=0; i<16; i++)
	{
		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_BUF_DATA_COUNT_REG, &c);
		while((c & 0x1f) == 0)
		{
			mdelay(2);
			cnt ++;
			SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_BUF_DATA_COUNT_REG, &c);
			if(cnt == 10)
	        	{
				D("read break");
				SP_TX_RST_AUX();
				bEDIDBreak=1;
				return;
	        	}
		}


		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_BUF_DATA_0_REG+i, &c);
		//D("edid[0x%.2x] = 0x%.2x\n",(long unsigned int)(offset+i),(long unsigned int)c);
	} 

	///*
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG, 0x01);
	//enable aux
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, 0x03);//set address only to stop EDID reading
	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);
	while(c & 0x01)
		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);

}





BYTE SP_TX_Get_EDID_Block(void)
{
	BYTE c;
       SP_TX_AUX_WR(0x00);
       SP_TX_AUX_RD(0x01);
       SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_BUF_DATA_0_REG, &c);
       //D("[a0:00] = %.2x\n", (long unsigned int)c);

       SP_TX_AUX_WR(0x7e);
       SP_TX_AUX_RD(0x01);
       SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_BUF_DATA_0_REG, &c);

	D("EDID Block = %d\n",(int)(c+1));
	return c;
    }




void SP_TX_AddrOnly_Set(BYTE bSet)
{
	BYTE c;

	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, &c);
	if(bSet)
	{
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, (c|SP_TX_ADDR_ONLY_BIT));
	}
	else
	{
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, (c&~SP_TX_ADDR_ONLY_BIT));
	}	
}

/*
void SP_TX_Scramble_Enable(BYTE bEnabled)
{
	BYTE c;

	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_TRAINING_PTN_SET_REG, &c);
	if(bEnabled)//enable scramble
	{
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_TRAINING_PTN_SET_REG, (c&~SP_TX_SCRAMBLE_DISABLE));
	}
	else
	{
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_TRAINING_PTN_SET_REG, (c|SP_TX_SCRAMBLE_DISABLE));
	}

}
*/



void SP_TX_API_M_GEN_CLK_Select(BYTE bSpreading)
{
    BYTE c;

    SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_M_CALCU_CTRL, &c);    
    if(bSpreading)
    {
            //M value select, select clock with downspreading
            SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_M_CALCU_CTRL, (c | M_GEN_CLK_SEL));    		      
    }
    else
    {
            //M value select, initialed as clock without downspreading
            SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_M_CALCU_CTRL, c&(~M_GEN_CLK_SEL));
    }
}


void SP_TX_Config_Packets(PACKETS_TYPE bType)
{
	BYTE c;

	switch(bType)
	{
		case AVI_PACKETS:
			//clear packet enable
			SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, &c);
			SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, c&(~SP_TX_PKT_AVI_EN));
 
			//get input color space
			//SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_VID_CTRL, &c);

			//SP_TX_Packet_AVI.AVI_data[0] = SP_TX_Packet_AVI.AVI_data[0] & 0x9f;
			//SP_TX_Packet_AVI.AVI_data[0] = SP_TX_Packet_AVI.AVI_data[0] | ((c&0x06) <<4);

			SP_TX_Load_Packet(AVI_PACKETS);

			//send packet update
			SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, &c);
			SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, c | SP_TX_PKT_AVI_UD);

			//enable packet
			SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, &c);
			SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, c | SP_TX_PKT_AVI_EN);			
			
			break;
			
		case SPD_PACKETS:
			//clear packet enable
			SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, &c);
			SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, c&(~SP_TX_PKT_SPD_EN));

			SP_TX_Load_Packet(SPD_PACKETS);

			//send packet update
			SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, &c);
			SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, c | SP_TX_PKT_SPD_UD);

			//enable packet
			SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, &c);
			SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, c | SP_TX_PKT_SPD_EN);	

			break;

		case MPEG_VS_PACKETS:
			//clear packet enable
			SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, &c);
			SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, c&(~SP_TX_PKT_MPEG_EN));

			SP_TX_Load_Packet(MPEG_VS_PACKETS);

			//send packet update
			SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, &c);
			SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, c | SP_TX_PKT_MPEG_UD);

			//enable packet
			SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, &c);
			SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_PKT_EN_REG, c | SP_TX_PKT_MPEG_EN);	

			break;

		default:
			break;
	}
	
}

void SP_TX_Load_Packet (PACKETS_TYPE type)
{
    BYTE i;
    
    switch(type)
    {
        case AVI_PACKETS:
            SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AVI_TYPE , 0x82);
            SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AVI_VER , 0x02);
            SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AVI_LEN , 0x0d);
			
            for(i=0;i<13;i++)
            {
                SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_AVI_DB0 + i, SP_TX_Packet_AVI.AVI_data[i]);                
            }
            break;

        case SPD_PACKETS:
            SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_SPD_TYPE , 0x83);
            SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_SPD_VER , 0x01);
            SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_SPD_LEN , 0x19);
            for(i=0;i<25;i++)
            {
                SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_SPD_DATA1 + i, SP_TX_Packet_SPD.SPD_data[i]);
            }
            break;

        case MPEG_VS_PACKETS:
            SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_MPEG_TYPE , 0x81);//mpeg 0x85,vsi 0x81
            SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_MPEG_VER , 0x01);
            SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_MPEG_LEN , 0x0d);
            for(i=0;i<13;i++)
            {
                SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_MPEG_DATA1 + i, SP_TX_Packet_MPEG.MPEG_data[i]);
            }
            break;
			
	default:
	    break;
    }
}
void SP_TX_AVI_Setup(void)
{
	BYTE c;
	int i;
	//SP_TX_Packet_AVI.AVI_data[0]=0x10;// force output video color space to RGB
	for(i=0;i<13;i++)
	{
		SP_TX_Read_Reg(HDMI_PORT1_ADDR,(HDMI_RX_AVI_DATA00_REG+i), &c);  
		SP_TX_Packet_AVI.AVI_data[i] = c;
	}
}
/*
void SP_TX_MPEG_VS_Setup(void)
{
	BYTE c;
	int i;
	for(i=0;i<13;i++)
	{
		SP_TX_Read_Reg(HDMI_PORT1_ADDR,(HDMI_RX_MPEG_DATA00_REG+i), &c);  
		SP_TX_Packet_MPEG.MPEG_data[i]= c;
	}
}
*/

BYTE SP_TX_BW_LC_Sel(void)
{
	BYTE over_bw=0;
	unsigned int pixel_clk = 0;
	HDMI_color_depth hdmi_input_color_depth = Hdmi_legacy;
	BYTE c,c1;
    
	D("input pclk = %d\n",(unsigned int)pclk);

	SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_DEEP_COLOR_CTRL_REG, &c);  
	c &= 0xF0;
	if(c == 0x00)
	{
		SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_VIDEO_STATUS_REG1, &c1);
		c1 &= 0xf0;
		if(c1 == 0x00)
			hdmi_input_color_depth = Hdmi_legacy;
		else if(c1 == 0x40)   
			hdmi_input_color_depth = Hdmi_24bit;
		else if(c1 == 0x50)   
			hdmi_input_color_depth = Hdmi_30bit;
		else if(c1 == 0x60)   
			hdmi_input_color_depth = Hdmi_36bit;
		else 
			D("HDMI input color depth is not supported .\n");
	}
	else if(c == 0x40)   
		hdmi_input_color_depth = Hdmi_24bit;
	else if(c == 0x50)   
		hdmi_input_color_depth = Hdmi_30bit;
	else if(c == 0x60)   
		hdmi_input_color_depth = Hdmi_36bit;
	else if(c == 0x70)   
		D("HDMI input color depth is not supported .\n");

	switch(hdmi_input_color_depth)
	{
		case Hdmi_legacy:
		case Hdmi_24bit:	
			pixel_clk = pclk;
			break;
		case Hdmi_30bit:	
			pixel_clk = pclk*5/4;
			break;
		case Hdmi_36bit:	
			pixel_clk = pclk*3/2;
			break;
		default:
			break;
	}
	
	if(pixel_clk <= 54)
	{
		sp_tx_bw = BW_162G;
	}
	else if((54 < pixel_clk) && (pixel_clk <= 90))
	{
		if(sp_tx_bw >= BW_27G)
			sp_tx_bw = BW_27G;
		else
			over_bw = 1;
	}
	else if((90 < pixel_clk) && (pixel_clk <= 180))
	{
		if(sp_tx_bw >= BW_54G)
			sp_tx_bw = BW_54G;
		else 
			over_bw = 1;
	}
	else
		over_bw = 1;

	
    if(over_bw)
        D("over bw!\n");
     else
	D("The optimized BW =%.2x\n",sp_tx_bw);

     return over_bw;

}

BYTE SP_TX_HW_Link_Training (void)
{
    
    BYTE c;

    if(!sp_tx_hw_lt_enable)
    {

        D("Hardware link training");
        if(sp_tx_ssc_enable)
            SP_TX_CONFIG_SSC(sp_tx_bw);
        else
            SP_TX_SPREAD_Enable(0);

        //set bandwidth
        SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_LINK_BW_SET_REG, sp_tx_bw);

        SP_TX_EnhaceMode_Set();

        ByteBuf[0] = 0x01;
	SP_TX_AUX_DPCDWrite_Bytes(0x00,0x06,0x00, 0x01, ByteBuf); //Set sink to D0 mode

	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_TRAINING_LANE0_SET_REG, 0x09);//link training from 400mv3.5db for ANX7730 B0-ANX.Fei-20111011

        SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_LINK_TRAINING_CTRL_REG, SP_TX_LINK_TRAINING_CTRL_EN);

        sp_tx_hw_lt_enable = 1;
        return 1;

    }

   mdelay(10);
  
      if(!sp_tx_hw_lt_done)
      	{
		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_LINK_TRAINING_CTRL_REG, &c);
		if(c&0x01)
		{
			return 1; //link training is not finished.
		}else{

			if(c&0x80)
			{
				c = (c & 0x70) >> 4;
				D("HW LT failed in process, ERR code = %.2x\n",c);
				sp_tx_link_config_done = 0;
				sp_tx_hw_lt_enable = 0;
				SP_CTRL_Set_System_State(SP_TX_LINK_TRAINING);
				return 1;

			}else{
				sp_tx_hw_lt_done = 1;
				SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_TRAINING_LANE0_SET_REG, &c);
				D("HW LT succeed in process,LANE0_SET = %.2x\n",c);
			}
		}
	}else {
		SP_TX_AUX_DPCDRead_Bytes(0x00,0x02,0x02, 1, ByteBuf);
		if(ByteBuf[0] != 0x07)
		{
			sp_tx_hw_lt_enable = 0;
			sp_tx_hw_lt_done = 0;
			return 1;
		}else{
			sp_tx_hw_lt_done = 1; 
			SP_CTRL_Set_System_State(SP_TX_CONFIG_SLIMPORT_OUTPUT);
			return 0;
		}
      }
	return 1;
}



BYTE SP_TX_LT_Pre_Config(void)
{
    BYTE legel_bw,legel_lc,c; 
    BYTE link_bw;

	legel_bw = legel_lc = 1;

	if(!SP_TX_Get_PLL_Lock_Status())
	{
		D("PLL not lock!");
		return 1;
	}

        if(!sp_tx_link_config_done)
	{    	 
		SP_TX_Get_Rx_LaneCount(1,&c);
		sp_tx_lane_count = c & 0x0f;
		SP_TX_Get_Rx_BW(1,&c);
		sp_tx_bw = c;


		if((sp_tx_bw != BW_27G) && (sp_tx_bw != BW_162G)&& (sp_tx_bw != BW_54G))
			legel_bw = 0;
		if((sp_tx_lane_count != 0x01) && (sp_tx_lane_count != 0x02) && (sp_tx_lane_count != 0x04))
			legel_lc = 0;
		if((legel_bw == 0) || (legel_lc == 0))
			return 1;

		SP_TX_Power_On(SP_TX_PWR_VIDEO);
		SP_TX_Video_Mute(1);
		//enable video input
		SP_TX_Enable_Video_Input(1);
		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_SYS_CTRL2_REG, &c);
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_SYS_CTRL2_REG, c);
		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_SYS_CTRL2_REG, &c);
		if(c & SP_TX_SYS_CTRL2_CHA_STA)
		{
			D("Stream clock not stable!");
			return 1;
		}

		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_SYS_CTRL3_REG, &c);
		SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_SYS_CTRL3_REG, c);
		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_SYS_CTRL3_REG, &c);
		if(!(c & SP_TX_SYS_CTRL3_STRM_VALID))
		{
			D("video stream not valid!");
			return 1;
		}

	SP_TX_Get_Link_BW(&link_bw);	//Get  transmit lane count&link bw
	SP_TX_PCLK_Calc(link_bw);//Calculate the pixel clock

	//Optimize the LT to get minimum power consumption
	if(SP_TX_BW_LC_Sel())
	{
		D("****Over bandwidth****");
		return 1;
		}
		sp_tx_lane_count = 1;//ANX7808 supports 1 lanes max.
		sp_tx_link_config_done = 1;
	}

        //SP_TX_Enable_Audio_Output(0);	//Disable audio  output

    return 0;// pre-config done
}



void SP_TX_Video_Mute(BYTE enable)
{
        BYTE c;
	if(enable)
	{
		SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL1_REG, &c);
		c |=SP_TX_VID_CTRL1_VID_MUTE;
		SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL1_REG , c);
	}
	else
	{
		SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL1_REG, &c);
		c &=~SP_TX_VID_CTRL1_VID_MUTE;
		SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL1_REG , c);
	}
		

}



//*****************************************************************
//Functions defination for HDMI Input
//*****************************************************************

void HDMI_RX_Set_HPD(void)
{
	BYTE c;

	//set HPD low
	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL3_REG, &c);
	c &=~SP_TX_VID_CTRL3_HPD_OUT;
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL3_REG , c);
	mdelay(100);
	//set HPD 
	SP_TX_Read_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL3_REG, &c);
	c |=SP_TX_VID_CTRL3_HPD_OUT;
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_TX_VID_CTRL3_REG , c);
	mdelay(500);
	D("HPD high is issued\n");
}

void HDMI_RX_TMDS_En(void)
{
       SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_PORT_SEL_REG, 0x11); // Enable DDC and TMDS after HPD
	D("TMDS and DDC are enabled\n");
}


void HDMI_RX_Set_Sys_State(BYTE ss)
{
    if(hdmi_system_state!= ss)
    {
        D("");
        hdmi_system_state = ss;
        switch (ss)
        {
            case HDMI_CLOCK_DET:
                D("HDMI_RX:  HDMI_CLOCK_DET");
                break;
            case HDMI_SYNC_DET:
                D("HDMI_RX:  HDMI_SYNC_DET");
                break;
            case HDMI_VIDEO_CONFIG:
                D("HDMI_RX:  HDMI_VIDEO_CONFIG");
                break;
            case HDMI_AUDIO_CONFIG:
                D("HDMI_RX:  HDMI_AUDIO_CONFIG");
                HDMI_RX_Restart_Audio_Chk();
                break;
            case HDMI_PLAYBACK:
                D("HDMI_RX:  HDMI_PLAYBACK");
                break;
            default:
                break;
        }
    }
}

/*


BYTE HDMI_RX_Get_Input_Color_Depth(void)
{
    BYTE input_color_depth;
    SP_TX_Read_Reg(HDMI_PORT0_ADDR, HDMI_RX_VIDEO_STATUS_REG1, &input_color_depth);
    input_color_depth = input_color_depth & 0xf0;
    if (input_color_depth == Deep_Color_legacy)
        return Hdmi_legacy;
    else if (input_color_depth == Deep_Color_24bit)
        return Hdmi_24bit;
    else if (input_color_depth == Deep_Color_30bit)
        return Hdmi_30bit;
    else if (input_color_depth == Deep_Color_36bit)
        return Hdmi_36bit;
}

void HDMI_RX_Set_Color_Depth(BYTE ColorDepth)
{
    BYTE c;
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_DEEP_COLOR_CTRL_REG , &c);
       SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_DEEP_COLOR_CTRL_REG , 
        c & 0x0f | HDMI_RX_EN_MAN_DEEP_COLOR | ColorDepth);
}

void HDMI_RX_Set_Color_Mode(void)
{

 D("HDMI_RX_Set_Color_Mode!");
    if(!video_format_supported )
    { 
        D("video format not supported!");
         if (!g_Video_Muted)
        {  
            HDMI_RX_Mute_Video();
        }
        if (!g_Audio_Muted)
        {
            HDMI_RX_Mute_Audio();
        }
        deep_color_set_done = 0;
        if(set_color_mode_counter > 3)
            set_color_mode_counter = 0;
    }
    else 
        set_color_mode_counter = 0;

    if(!deep_color_set_done && (hdmi_system_state >= HDMI_VIDEO_CONFIG))
    {
        if((HDMI_RX_Get_Input_Color_Depth() == Hdmi_24bit) ||
            (HDMI_RX_Get_Input_Color_Depth() == Hdmi_30bit) || 
            (HDMI_RX_Get_Input_Color_Depth() == Hdmi_36bit))
        {
               SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_DEEP_COLOR_CTRL_REG , 0x00);
            mdelay(32);
            D("RX gets 3 GCP and works in deep color mode!");
        }
        else
        {
            D("RX doesn't get 3 GCP and trys to judge which color mode it should work in !");
            D("set_color_mode_counter = %.2x\n", (long unsigned int)set_color_mode_counter);
            switch(set_color_mode_counter)
            {
                case 0:
                    HDMI_RX_Set_Color_Depth(Deep_Color_legacy);
                    mdelay(32);
                    break;
                case 1:
                    HDMI_RX_Set_Color_Depth(Deep_Color_36bit);
                    mdelay(32);
                    break;
                case 2:
                    HDMI_RX_Set_Color_Depth(Deep_Color_30bit);
                    mdelay(32);
                    break;
                case 3:
                    HDMI_RX_Set_Color_Depth(Deep_Color_24bit);
                    mdelay(32);
                    break;
                default:
                    break;
            }
            set_color_mode_counter++;
        }
        deep_color_set_done = 1;
        video_format_supported = 1;
        D("set_color_mode_counter = %.2x  format_supported = %.2x \n", 
			(long unsigned int)set_color_mode_counter, (long unsigned int)video_format_supported );

    }

}

*/
void HDMI_RX_Mute_Video(void)
{
    BYTE c;

    D("Mute Video.");
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_HDMI_MUTE_CTRL_REG, &c);
    SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_HDMI_MUTE_CTRL_REG, c | HDMI_RX_VID_MUTE);
    g_Video_Muted = 1;
}

void HDMI_RX_Unmute_Video(void)
{
    BYTE c;

    D("Unmute Video.");
    SP_TX_Read_Reg(HDMI_PORT0_ADDR, HDMI_RX_HDMI_MUTE_CTRL_REG, &c);
    SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_HDMI_MUTE_CTRL_REG, c & (~HDMI_RX_VID_MUTE));	
    g_Video_Muted =0;
}

void HDMI_RX_Mute_Audio(void)
{
    BYTE c;

	D("Mute Audio.");
	SP_TX_Read_Reg(HDMI_PORT0_ADDR, HDMI_RX_HDMI_MUTE_CTRL_REG, &c);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_HDMI_MUTE_CTRL_REG, c | HDMI_RX_AUD_MUTE);
	g_Audio_Muted = 1;
}

void HDMI_RX_Unmute_Audio(void)
{
    BYTE c;

    D("Unmute Audio.");
    SP_TX_Read_Reg(HDMI_PORT0_ADDR, HDMI_RX_HDMI_MUTE_CTRL_REG, &c);
       SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_HDMI_MUTE_CTRL_REG, c & (~HDMI_RX_AUD_MUTE));
    g_Audio_Muted = 0;
}


char HDMI_RX_Is_Video_Change(void)
{
    BYTE ch, cl;
    unsigned long n; 
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_HTOTAL_LOW, &cl);
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_HTOTAL_HIGH, &ch);
    n = ch;
    n = (n << 8) + cl;
    if ((g_Cur_H_Res < (n-10)) || (g_Cur_H_Res > (n+10))) 
    {
        D("H_Res changed.");
        D("Current H_Res = %ld\n", n);
        return 1;
    }
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_VTOTAL_LOW, &cl);
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_VTOTAL_HIGH, &ch);
    n = ch;
    n = (n << 8) + cl;
    if ((g_Cur_V_Res < (n-10)) || (g_Cur_V_Res > (n+10)))
    {
        D("V_Res changed.\n");
        D("Current V_Res = %ld\n", n);
        return 1;
    }
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_HDMI_STATUS_REG, &cl);

    cl &= HDMI_RX_HDMI_MODE;
    if(g_HDMI_DVI_Status != cl)
    {
        D("DVI to HDMI or HDMI to DVI Change.");
        return 1;
    }

    return 0;
}

void HDMI_RX_Get_Video_Info(void)
{
    BYTE ch, cl;
    unsigned int n;

    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_HTOTAL_LOW, &cl);
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_HTOTAL_HIGH, &ch);
    n = ch;
    n = (n << 8) + cl;
    g_Cur_H_Res = n;
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_VTOTAL_LOW, &cl);
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_VTOTAL_HIGH, &ch);
    n = ch;
    n = (n << 8) + cl;
    g_Cur_V_Res = n;

    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_VID_PCLK_CNTR_REG, &cl);
    g_Cur_Pix_Clk = cl;
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_HDMI_STATUS_REG, &cl);
    g_HDMI_DVI_Status = ((cl & HDMI_RX_HDMI_MODE) == HDMI_RX_HDMI_MODE);
}

void HDMI_RX_Show_Video_Info(void)
{
    BYTE c,c1;
    BYTE cl,ch;
    unsigned long n;
    unsigned long h_res,v_res;
    
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_HACT_LOW, &cl);
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_HACT_HIGH, &ch);
    n = ch;
    n = (n << 8) + cl;
    h_res = n;
    
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_VACT_LOW, &cl);
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_VACT_HIGH, &ch);	
    n = ch;
    n = (n << 8) + cl;
    v_res = n;
    
    D("");
    D("******************************HDMI_RX Info*******************************");
    D("HDMI_RX Is Normally Play Back.\n");
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_HDMI_STATUS_REG, &c); 
    if(c & HDMI_RX_HDMI_MODE)
        D("HDMI_RX Mode = HDMI Mode.\n");
    else
        D("HDMI_RX Mode = DVI Mode.\n");

    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_VIDEO_STATUS_REG1, &c); 
    if(c & HDMI_RX_VIDEO_TYPE)
    {
        v_res += v_res;
    }       
    D("HDMI_RX Video Resolution = %ld * %ld ",h_res,v_res);
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_VIDEO_STATUS_REG1, &c); 
    if(c & HDMI_RX_VIDEO_TYPE)
        D("    Interlace Video.");
    else
        D("    Progressive Video.");

    
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_SYS_CTRL1_REG, &c);  
    if((c & 0x30) == 0x00)
        D("Input Pixel Clock = Not Repeated.\n");
    else if((c & 0x30) == 0x10)
        D("Input Pixel Clock = 2x Video Clock. Repeated.\n");
    else if((c & 0x30) == 0x30)
        D("Input Pixel Clock = 4x Vvideo Clock. Repeated.\n");

    if((c & 0xc0) == 0x00)
        D("Output Video Clock = Not Divided.\n");
    else if((c & 0xc0) == 0x40)
        D("Output Video Clock = Divided By 2.\n");
    else if((c & 0xc0) == 0xc0)
        D("Output Video Clock = Divided By 4.\n");

    if(c & 0x02)
        D("Output Video Using Rising Edge To Latch Data.\n");
    else
        D("Output Video Using Falling Edge To Latch Data.\n");


    D("Input Video Color Depth = "); 
        SP_TX_Read_Reg(HDMI_PORT0_ADDR,0x70, &c1);
	 c1 &= 0xf0;
	 if(c1 == 0x00)
            D("Legacy Mode.\n");
	 else if(c1 == 0x40)   
            D("24 Bit Mode.\n");
        else if(c1 == 0x50)   
            D("30 Bit Mode.\n");
        else if(c1 == 0x60)   
            D("36 Bit Mode.\n");
        else if(c1 == 0x70)   
            D("48 Bit Mode.\n");

    D("Input Video Color Space = ");
    SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_AVI_DATA00_REG, &c);  
    c &= 0x60;
    if(c == 0x20)        
        D("YCbCr4:2:2 .\n");
    else if(c == 0x40)   
        D("YCbCr4:4:4 .\n");
    else if(c == 0x00)   
        D("RGB.\n");
    else                 
        D("Unknow 0x44 = 0x%.2x\n",(int)c);
    
    


    SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_HDCP_STATUS_REG, &c); 
    if(c & HDMI_RX_AUTHEN)
        D("Authentication is attempted.");
    else
        D("Authentication is not attempted.");

    for(cl=0;cl<20;cl++)
    {
        SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_HDCP_STATUS_REG, &c); 
        if(c & HDMI_RX_DECRYPT)
            break;
        else
            mdelay(10);
    }
    if(cl < 20)
        D("Decryption is active.");
    else
        D("Decryption is not active.");
             
    D("***********************************************************************************");
    D("");
}



void HDMI_RX_Show_Audio_Info(void)
{
    BYTE c;

    
    D("Audio Fs = ");
    SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_AUD_IN_SPDIF_CH_STATUS4_REG, &c); 
    c &= 0x0f;
    switch(c)
    {
        case 0x00: 
            D("44.1 KHz.");
            break;
        case 0x02: 
            D("48 KHz.");
            break;
        case 0x03: 
            D("32 KHz.");
            break;
        case 0x08: 
            D("88.2 KHz.");
            break;        
        case 0x0a: 
            D("96 KHz.");
            break;
        case 0x0e: 
            D("192 KHz.");
            break;
        default :         
            break;
    }
             
    D("");
}

   

void  HDMI_RX_Restart_Audio_Chk(void)
{
    if (hdmi_system_state == HDMI_AUDIO_CONFIG)  
    {
        D("WAIT_AUDIO: HDMI_RX_Restart_Audio_Chk.");
        g_CTS_Got = 0;
        g_Audio_Got = 0;
    }
}


void HDMI_RX_Init_var(void)
{
    HDMI_RX_Set_Sys_State(HDMI_CLOCK_DET);
    g_Cur_H_Res = 0;
    g_Cur_V_Res = 0;
    g_Cur_Pix_Clk = 0;
    g_Video_Muted = 1;
    g_Audio_Muted = 1;
    g_Audio_Stable_Cntr = 0;
    g_Sync_Expire_Cntr = 0;
    g_HDCP_Err_Cnt = 0;
    g_HDMI_DVI_Status = DVI_MODE;

}


void HDMI_RX_Initialize(void)
{
    BYTE c;

	HDMI_RX_Init_var();

	// Mute audio and video output
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_HDMI_MUTE_CTRL_REG, 0x03);
	SP_TX_Write_Reg(HDMI_PORT1_ADDR,HDMI_RX_HDCP_BCAPS, 0x81);

	// Set clock detect source
	// Enable pll_lock, digital clock detect, disable analog clock detect
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_CHIP_CTRL_REG, 0xe5);

	//select TMDS recovery clock
	//SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_TMDS_CTRL9, &c);
	//SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_TMDS_CTRL9, c | 0x40);
	//enable HW AVMUTE control
	SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_AEC_CTRL_REG, &c);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_AEC_CTRL_REG, c | HDMI_RX_AVC_OE);

	//change HDCP enable criteria value to meet HDCP CTS
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_HDCP_EN_CRITERIA_REG, 0x08);

	//  reset HDCP
	SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_SRST_REG, &c);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_SRST_REG , c | HDMI_RX_HDCP_MAN_RST);
	mdelay(10) ;
	SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_SRST_REG, &c);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_SRST_REG , c & ~HDMI_RX_HDCP_MAN_RST);

	SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_SRST_REG, &c);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_SRST_REG , c | HDMI_RX_SW_MAN_RST);
	mdelay(10);
	SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_SRST_REG, &c);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_SRST_REG , c & ~HDMI_RX_SW_MAN_RST);
	//reset TMDS
	SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_SRST_REG, &c);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_SRST_REG , c | HDMI_RX_TMDS_RST);

	// Init Interrupt
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_MASK1_REG, 0xff);    
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_MASK2_REG, 0xf3);    
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_MASK3_REG, 0x3f);    
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_MASK4_REG, 0x17);    
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_MASK5_REG, 0xff);    
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_MASK6_REG, 0xff);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_MASK7_REG, 0x07);

	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_AEC_EN0_REG, 0xe7);   
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_AEC_EN1_REG, 0x17);    
	// SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_AEC_EN2_REG, 0xf4);    
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_AEC_EN2_REG, 0xf0);


	//Set output video bus width to 18bits DDR
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_VID_OUTPUT_CTRL3_REG, 0X00);//((c & 0xf3) | 0x08));


	//# Enable AVC and AAC
	SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_AEC_CTRL_REG , &c);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_AEC_CTRL_REG , c | HDMI_RX_AVC_EN );

	SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_AEC_CTRL_REG, &c);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_AEC_CTRL_REG, c | HDMI_RX_AAC_EN);

	SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_SYS_PWDN1_REG, &c);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_SYS_PWDN1_REG, c &(~HDMI_RX_PWDN_CTRL)); // power up all

	// Set EQ Value
	//SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_TMDS_CTRL_REG1, 0xec);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_TMDS_CTRL_REG2, 0x00);
	//SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_TMDS_CTRL_REG3, 0x55);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_TMDS_CTRL_REG4, 0X28);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_TMDS_CTRL_REG5, 0xe3);
	//SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_TMDS_CTRL_REG6, 0X28);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_TMDS_CTRL_REG7, 0X50);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_TMDS_CTRL_REG21, 0x04);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_TMDS_CTRL_REG22, 0x38);

	//Range limitation for RGB input
	SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_VID_DATA_RNG_CTRL_REG, &c);
	SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_VID_DATA_RNG_CTRL_REG, (c | HDMI_RX_R2Y_INPUT_LIMIT)); 

	SP_TX_Write_Reg(HDMI_PORT0_ADDR, HDMI_RX_PIO_CTRL, 0x02);//set GPIO control by HPD

	SP_TX_Write_Reg(HDMI_PORT0_ADDR, HDMI_RX_PKT_RX_INDU_INT_CTRL, 0x80);//generate interrupt on any received HDMI Vendor Specific packet; 

	SP_TX_Write_Reg(HDMI_PORT0_ADDR, HDMI_RX_CEC_CTRL_REG, 0x01);//Reset CEC 
	SP_TX_Write_Reg(HDMI_PORT0_ADDR, HDMI_RX_CEC_SPEED_CTRL_REG, 0x40);//CEC SPPED to 27MHz
	SP_TX_Write_Reg(HDMI_PORT0_ADDR, HDMI_RX_CEC_CTRL_REG, 0x08);//Set  CEC Logic address to 15,unregistered, and enable CEC receiver
	D("HDMI Rx is initialized..."); 
}


void HDMI_RX_Clk_Detected_Int(void)
{
    BYTE c;

    D("*HDMI_RX Interrupt: Pixel Clock Change.\n");
     SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_SYS_STATUS_REG, &c);
    if (c & HDMI_RX_CLK_DET)
    {
        D("   Pixel clock existed. \n");
	if (hdmi_system_state == HDMI_CLOCK_DET) 
	HDMI_RX_Set_Sys_State(HDMI_SYNC_DET);
    } 
    else
    {
	HDMI_RX_Mute_Audio();
	HDMI_RX_Mute_Video();
	SP_TX_Video_Mute(1);
	SP_TX_Enable_Video_Input(0);
	SP_TX_Enable_Audio_Output(0);
	 if(sp_tx_system_state > SP_TX_CONFIG_HDMI_INPUT)
	 SP_CTRL_Set_System_State(SP_TX_CONFIG_HDMI_INPUT);
	  if(hdmi_system_state > HDMI_CLOCK_DET)
	 {
	 D("   Pixel clock lost. \n");
	 HDMI_RX_Set_Sys_State(HDMI_CLOCK_DET);
	 }
        g_Sync_Expire_Cntr = 0;
    }	
}

void HDMI_RX_Sync_Det_Int(void)
{
    BYTE c;
    
    D("*HDMI_RX Interrupt: Sync Detect."); 
     SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_SYS_STATUS_REG, &c);
    
    if (c & HDMI_RX_HDMI_DET) // sync existed
    {
            D("Sync found.");
            HDMI_RX_Mute_Audio();
            HDMI_RX_Mute_Video();
	     if (hdmi_system_state == HDMI_SYNC_DET) 
            HDMI_RX_Set_Sys_State(HDMI_VIDEO_CONFIG);
            g_Video_Stable_Cntr = 0;
            HDMI_RX_Get_Video_Info();// save first video info  
    } 
    else    // sync lost
    {
        D("Sync lost.");   
	 SP_TX_Video_Mute(1);
	 SP_TX_Enable_Video_Input(0);
	 SP_TX_Enable_Audio_Output(0);
        HDMI_RX_Mute_Audio();
        HDMI_RX_Mute_Video();
	if(sp_tx_system_state > SP_TX_CONFIG_HDMI_INPUT)
	 SP_CTRL_Set_System_State(SP_TX_CONFIG_HDMI_INPUT);
        if((c & 0x02) && (hdmi_system_state > HDMI_SYNC_DET))//check if the clock lost
        {
            HDMI_RX_Set_Sys_State(HDMI_SYNC_DET); 
        }
        else 
        { 
            HDMI_RX_Set_Sys_State(HDMI_CLOCK_DET); 
        }
    }
}


void HDMI_RX_HDMI_DVI_Int(void)
{
    BYTE c;

    D("*HDMI_RX Interrupt: HDMI-DVI Mode Change."); 
     SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_HDMI_STATUS_REG, &c);
    HDMI_RX_Get_Video_Info();
    if ((c & HDMI_RX_HDMI_MODE) == HDMI_RX_HDMI_MODE) 
    {
        D("HDMI_RX_HDMI_DVI_Int: HDMI MODE."); 
        if ( hdmi_system_state==HDMI_PLAYBACK)
        {
            HDMI_RX_Set_Sys_State(HDMI_AUDIO_CONFIG); 
        }
    } 
    else 
    {
        HDMI_RX_Unmute_Audio();
    }
}

void HDMI_RX_AV_MUTE_Int(void)
{
	BYTE AVMUTE_STATUS;
	//General Control Packet AV mute handler
	SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_HDMI_STATUS_REG, &AVMUTE_STATUS);
	if(AVMUTE_STATUS& HDMI_RX_MUTE_STAT) 
	{
        D("HDMI_RX AV mute packet received."); 
		if (!g_Video_Muted) 
			HDMI_RX_Mute_Video();
		if (!g_Audio_Muted)
			HDMI_RX_Mute_Audio();
		//In HDMI_RX, AV mute status bit is write clear.
		SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_HDMI_STATUS_REG, AVMUTE_STATUS & (~HDMI_RX_MUTE_STAT));
	}

}

void HDMI_RX_Cts_Rcv_Int(void)
{
	BYTE c;
   	 g_CTS_Got = 1;
	SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_SYS_STATUS_REG, &c);
	if((hdmi_system_state == HDMI_AUDIO_CONFIG)&&(c & HDMI_RX_HDMI_DET ))
	{
		if (g_CTS_Got && g_Audio_Got) 
		{
			if (g_Audio_Stable_Cntr >= AUDIO_STABLE_TH) 
			{
				HDMI_RX_Unmute_Audio();
				HDMI_RX_Unmute_Video();
				g_Audio_Stable_Cntr = 0;
				HDMI_RX_Show_Audio_Info();
				HDMI_RX_Set_Sys_State(HDMI_PLAYBACK);
				SP_TX_Config_Audio();
			} 
			else g_Audio_Stable_Cntr++;
		} 
		else g_Audio_Stable_Cntr = 0;
	}
}

void HDMI_RX_Audio_Rcv_Int(void)
{
	BYTE c;
	g_Audio_Got = 1;
	
	SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_SYS_STATUS_REG, &c);
	if((hdmi_system_state == HDMI_AUDIO_CONFIG)&&(c & HDMI_RX_HDMI_DET ))
	{
		if (g_CTS_Got && g_Audio_Got) 
		{
			if (g_Audio_Stable_Cntr >= AUDIO_STABLE_TH) 
			{
				HDMI_RX_Unmute_Audio();
				HDMI_RX_Unmute_Video();
				g_Audio_Stable_Cntr = 0;
				HDMI_RX_Show_Audio_Info();
				HDMI_RX_Set_Sys_State(HDMI_PLAYBACK);
				SP_TX_Config_Audio();
			} 
			else g_Audio_Stable_Cntr++;
		} 
		else g_Audio_Stable_Cntr = 0;
	}
}

void HDMI_RX_HDCP_Error_Int(void)
{
    g_Audio_Got = 0;
    g_CTS_Got = 0;

    if(g_HDCP_Err_Cnt >= 40 )
    {
        g_HDCP_Err_Cnt = 0;
        D("Lots of hdcp error occured ..."); 
        HDMI_RX_Mute_Audio();
        HDMI_RX_Mute_Video();

	//issue hotplug
	HDMI_RX_Set_HPD();

    }
    else if((hdmi_system_state==HDMI_CLOCK_DET) || (hdmi_system_state == HDMI_SYNC_DET))
    {
        g_HDCP_Err_Cnt = 0;
    }
    else
    {
        g_HDCP_Err_Cnt ++;
    }

}

void HDMI_RX_New_AVI_Int(void)
{
    BYTE c;
    
    D("*HDMI_RX Interrupt: New AVI Packet."); 

     SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_AVI_TYPE_REG, &c);
    //D("AVI Infoframe:\n");
    //D("0x%.2x ",c);
     SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_AVI_VER_REG, &c);
    //D("0x%.2x ",c);
     SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_AVI_LEN_REG, &c);
    //D("0x%.2x ",c);
     SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_AVI_CHKSUM_REG, &c);
    //D("    Check Sum = 0x%.2x \n",c);
     SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_AVI_DATA00_REG, &c);
   // D("0x%.2x ",c);
     SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_AVI_DATA01_REG, &c);
   // D("0x%.2x ",c);
     SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_AVI_DATA02_REG, &c);
   // D("0x%.2x ",c);
     SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_AVI_DATA03_REG, &c);
   // D("0x%.2x ",c);
     SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_AVI_DATA04_REG, &c);
   // D("0x%.2x ",c);
     SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_AVI_DATA05_REG, &c);
   // D("0x%.2x ",c);
     SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_AVI_DATA06_REG, &c);
   // D("0x%.2x ",c);
     SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_AVI_DATA07_REG, &c);
    //D("0x%.2x ",c);
     SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_AVI_DATA08_REG, &c);
    //D("0x%.2x ",c);
     SP_TX_Read_Reg(HDMI_PORT1_ADDR,HDMI_RX_AVI_DATA09_REG, &c);
    //D("0x%.2x \n",c);
	SP_TX_AVI_Setup();//initial AVI infoframe packet
	SP_TX_Config_Packets(AVI_PACKETS);
}


void HDMI_RX_New_VSI_Int(void)
{
       //if(V3D_EN)
       //SP_TX_Set_3D_Packets();
       g_VSI_Got = 1;
	//debug_puts("New VSI packet is received.");
}

void HDMI_RX_No_VSI_Int(void)//bug, int can not be cleared-fei 2012.5.10
{
	//BYTE c;

	//SP_TX_Write_Reg(SP_TX_PORT0_ADDR, 0xea, 0x00);//enable VSC

}
void SP_TX_Config_HDMI_Input (void)
{
    BYTE c, c1;
    BYTE AVMUTE_STATUS,SYS_STATUS;


	SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_SYS_STATUS_REG, &SYS_STATUS);

	if(!(SYS_STATUS& HDMI_RX_PWR5V))
	{
		SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_SYS_PWDN3_REG , &c1);
		if(!(c1 & HDMI_RX_PWDN_CTRL))
		{
			SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_SYS_PWDN3_REG , c1 | HDMI_RX_PD_ALL); // power down all	
			D("HDMI_RX power down");
		}
	}
	else
	{
		if(hdmi_system_state== HDMI_CLOCK_DET) 
		{
			SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_SYS_PWDN3_REG , &c1);
			if(c1 & HDMI_RX_PWDN_CTRL)
			{
				SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_SYS_PWDN3_REG , c1 & (~HDMI_RX_PD_ALL)); // power up all	
				D("HDMI_RX power on");
			}
		}
	}

	if((SYS_STATUS & HDMI_RX_CLK_DET)&&(hdmi_system_state == HDMI_CLOCK_DET) )
	HDMI_RX_Set_Sys_State(HDMI_SYNC_DET);
	
    
	if(hdmi_system_state== HDMI_SYNC_DET) 
	{
		SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_SYS_STATUS_REG, &c);
		if (!(c & HDMI_RX_HDMI_DET)) 
		{
			if (g_Sync_Expire_Cntr >= SCDT_EXPIRE_TH) 
			{
				D("No sync for long time."); 
				//misc reset
				SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_TMDS_CTRL_REG18, &c);
				SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_TMDS_CTRL_REG18 , c | 0x10);
				mdelay(10);
				SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_TMDS_CTRL_REG18, &c);
				SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_TMDS_CTRL_REG18 , c & ~0x10);
				HDMI_RX_Set_Sys_State(HDMI_CLOCK_DET);
				g_Sync_Expire_Cntr = 0;
			}
			else
				g_Sync_Expire_Cntr++;
			return;
		}
		else{
			g_Sync_Expire_Cntr = 0;// clear sync expire counter
			HDMI_RX_Set_Sys_State(HDMI_VIDEO_CONFIG);
		}

	}


	//HDMI_RX_Set_Color_Mode();

	if ( hdmi_system_state < HDMI_VIDEO_CONFIG)  
	return;

	if (HDMI_RX_Is_Video_Change()) 
	{
		D("Video Changed , mute video and mute audio");
		g_Video_Stable_Cntr = 0;

		if (!g_Video_Muted)
			HDMI_RX_Mute_Video();
		if (!g_Audio_Muted)
			HDMI_RX_Mute_Audio();

	} else if (g_Video_Stable_Cntr < VIDEO_STABLE_TH) 
	{
		g_Video_Stable_Cntr++;
		D("WAIT_VIDEO: Wait for video stable cntr.");
	} else if (hdmi_system_state == HDMI_VIDEO_CONFIG) 
	{
		SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_HDMI_STATUS_REG, &AVMUTE_STATUS);
		if(!(AVMUTE_STATUS& HDMI_RX_MUTE_STAT) )
		{
			HDMI_RX_Get_Video_Info();
			HDMI_RX_Unmute_Video();	
			SP_TX_LVTTL_Bit_Mapping();
			SP_CTRL_Set_System_State(SP_TX_LINK_TRAINING);
			HDMI_RX_Show_Video_Info();
			if (g_HDMI_DVI_Status) 
			{
				D("HDMI mode: Video is stable."); 
				HDMI_RX_Set_Sys_State(HDMI_AUDIO_CONFIG);
			} else {
				D("DVI mode: Video is stable.");
				HDMI_RX_Unmute_Audio();
				HDMI_RX_Set_Sys_State(HDMI_PLAYBACK);
			}
		}
	}
	HDMI_RX_Get_Video_Info();

}

void HDMI_RX_Int_Irq_Handler(void)
{
BYTE c1,c2,c3,c4,c5,c6,c7;

	 if(hdmi_system_state < HDMI_CLOCK_DET)
	   return;
	
	 SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_STATUS1_REG , &c1);
	 SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_STATUS1_REG , c1);

	 SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_STATUS2_REG, &c2);
	 SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_STATUS2_REG, c2);

	 SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_STATUS3_REG, &c3);
	 SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_STATUS3_REG, c3);


	 SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_STATUS4_REG, &c4);
	 SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_STATUS4_REG, c4);

	 SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_STATUS5_REG, &c5);
	 SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_STATUS5_REG, c5);

	 SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_STATUS6_REG, &c6);
	 SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_STATUS6_REG, c6);

	 SP_TX_Read_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_STATUS7_REG, &c7);
	 SP_TX_Write_Reg(HDMI_PORT0_ADDR,HDMI_RX_INT_STATUS7_REG, c7);

	if (c1 & HDMI_RX_CKDT_CHANGE)//clk detect change interrupt
	    HDMI_RX_Clk_Detected_Int();

	if (c1 & HDMI_RX_SCDT_CHANGE)  // SYNC detect interrupt
	    HDMI_RX_Sync_Det_Int();

	if (c1 & HDMI_RX_HDMI_DVI) // HDMI_DVI detect interrupt               
	    HDMI_RX_HDMI_DVI_Int();
	if (c1 & HDMI_RX_SET_MUTE) // HDMI_DVI detect interrupt               
	    HDMI_RX_AV_MUTE_Int();
	

	if (c6 & HDMI_RX_NEW_AVI) 
	    HDMI_RX_New_AVI_Int();
	if (c7 & HDMI_RX_NEW_VS) 
	    HDMI_RX_New_VSI_Int();

	if (c7 & HDMI_RX_NO_VSI) 
	    HDMI_RX_No_VSI_Int();
	
	/*
	if ((c6 & HDMI_RX_CTS_ACR_CHANGE) || (c3 & HDMI_RX_ACR_PLL_UNLOCK))  
	{ 
	    HDMI_RX_Restart_Audio_Chk();
	}
	*/

	if (c6 & HDMI_RX_CTS_RCV)  
	    HDMI_RX_Cts_Rcv_Int();
	

	if (c5 & HDMI_RX_AUDIO_RCV)  
	    HDMI_RX_Audio_Rcv_Int();


	if ( c2 & HDMI_RX_HDCP_ERR)   // HDCP error
	    HDMI_RX_HDCP_Error_Int();
	else
	    g_HDCP_Err_Cnt = 0;
	

}








