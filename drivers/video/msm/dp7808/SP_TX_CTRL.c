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
#include "SP_TX_CTRL.h"
#include "SP_TX_DRV.h"
#include "SP_TX_Reg.h"
#include "Colorado2.h"




BYTE ByteBuf[MAX_BUF_CNT];

BYTE timer_slot;

//for EDID
BYTE edid_pclk_out_of_range;//the flag when clock out of 256Mhz
BYTE sp_tx_edid_err_code;//EDID read  error type flag
BYTE bEDIDBreak; //EDID access break


//for HDCP
BYTE sp_tx_hdcp_auth_pass; //HDCP auth pass flag
BYTE sp_tx_hdcp_auth_fail_counter;  //HDCP auth fail count flag
BYTE sp_tx_hdcp_enable;//enable HDCP process indicator
BYTE sp_tx_ssc_enable; //SSC control anbale indicator
BYTE sp_tx_hdcp_capable_chk;//DPCD HDCP capable check flag 
BYTE sp_tx_hw_hdcp_en;//enable hardware HDCP
BYTE sp_tx_hdcp_auth_done;
BYTE sp_tx_hw_lt_done;//hardware linktraining done indicator
BYTE sp_tx_hw_lt_enable;//hardware linktraining enable


BYTE sp_tx_ds_hdmi_plugged;//amazon downstream HDMI plug in flag
BYTE sp_tx_ds_hdmi_sensed; //amazon downstream HDMI is power on
BYTE sp_tx_ds_monitor_plugged;//amazon downstream HDMI plug in flag
BYTE sp_tx_pd_mode; //ANX7808 power state flag


//external interrupt flag
BYTE ext_int_index;

BYTE sp_tx_rx_anx7730;
BYTE CEC_abort_message_received = 0;
BYTE CEC_get_physical_adress_message_received = 0;
BYTE  CEC_logic_addr = 0x00;
int      CEC_loop_number = 0;
BYTE CEC_resent_flag = 0;





void SP_CTRL_Main_Procss(void)
{

    printk("SP_CTRL_Main_Procss ++++\n");

#ifndef EXT_INT
    SP_CTRL_Int_Irq_Handler();
#else

    if(ext_int_index)
     {
     	ext_int_index = 0;
	SP_CTRL_Int_Irq_Handler();
     }
#endif
    SP_CTRL_TimerProcess();

    printk("SP_CTRL_Main_Procss ---\n");
	
}

void SP_CTRL_TimerProcess (void) 
{

        if (timer_slot == 4) 
            timer_slot = 0;
        else 
            timer_slot ++;
        if (timer_slot == 0) 
        {
            SP_CTRL_Timer_Slot1(); 
        }
        else if (timer_slot == 1) 
        {
            SP_CTRL_Timer_Slot2(); 
        } 
        else if (timer_slot == 2) 
        {
            SP_CTRL_Timer_Slot3(); 
        } 
	 else if (timer_slot == 3) 
        {
            SP_CTRL_Timer_Slot4(); 
        } 
    
}


void SP_CTRL_Timer_Slot1(void) 
{
	if(sp_tx_system_state == SP_TX_WAIT_SLIMPORT_PLUGIN)
		SP_CTRL_Slimport_Plug_Process();
	
	if(sp_tx_system_state == SP_TX_PARSE_EDID)
		SP_CTRL_EDID_Process();
}

void SP_CTRL_Timer_Slot2(void)
{

	if(sp_tx_system_state == SP_TX_CONFIG_HDMI_INPUT)
		SP_TX_Config_HDMI_Input();
	
	if(sp_tx_system_state == SP_TX_LINK_TRAINING)
	{
#ifdef SSC_EN
		sp_tx_ssc_enable =1;
#else
		sp_tx_ssc_enable =0;
#endif
		if(SP_TX_LT_Pre_Config())//pre-config not done
		return;

		SP_TX_HW_Link_Training();
	}
}

void SP_CTRL_Timer_Slot3(void)
{

	if(sp_tx_system_state == SP_TX_CONFIG_SLIMPORT_OUTPUT)
		SP_TX_Config_Slimport_Output();
    
       if(sp_tx_system_state == SP_TX_HDCP_AUTHENTICATION)
	{
#ifdef HDCP_EN
	sp_tx_hdcp_enable = 1;
#else
	sp_tx_hdcp_enable = 0;
#endif
		
		
		if(sp_tx_hdcp_enable)
			SP_CTRL_HDCP_Process();
		else
		{
			SP_TX_Power_Down(SP_TX_PWR_HDCP);// Poer down HDCP link clock domain logic for B0 version-20110913-ANX.Fei
			SP_TX_Show_Infomation();
                     SP_TX_Video_Mute(0);
			SP_CTRL_Set_System_State(SP_TX_PLAY_BACK);
		}
	}
}
void SP_CTRL_Timer_Slot4(void)
{
	if(sp_tx_system_state == SP_TX_PLAY_BACK)
	SP_CTRL_PlayBack_Process();
}


BYTE SP_CTRL_Chip_Detect(void)
{
	return SP_TX_Chip_Located();
}


void SP_CTRL_Chip_Initial(void)
{
	printk("[MYDP] Chip_Initial+++\n");
	SP_TX_VBUS_PowerDown(); // Disable the power supply for ANX7730
	SP_TX_Hardware_PowerDown();//Power down ANX7805 totally
		printk("[MYDP] Chip_Initial+++\n");
	
	SP_CTRL_Variable_Init();
	SP_CTRL_Set_System_State(SP_TX_WAIT_SLIMPORT_PLUGIN);
	printk("[MYDP] Chip_Initial---\n");
}

void SP_CTRL_Variable_Init(void)
{	
	sp_tx_edid_err_code = 0; 
	edid_pclk_out_of_range = 0; 
	sp_tx_link_config_done = 0; 

	sp_tx_ds_hdmi_plugged = 0;
	sp_tx_ds_hdmi_sensed =0;
	sp_tx_ds_monitor_plugged = 0;
	sp_tx_ds_edid_hdmi = 0;
	sp_tx_ds_edid_3d_present = 0;
	sp_tx_pd_mode = 1;//initial power state is power down.

	sp_tx_hdcp_auth_fail_counter = 0;
	sp_tx_hdcp_auth_pass = 0;
	sp_tx_hw_hdcp_en = 0;
	sp_tx_hdcp_capable_chk = 0;
	sp_tx_hdcp_enable = 0;
	sp_tx_ssc_enable = 0;
	sp_tx_hdcp_auth_done = 0;
       sp_tx_hw_lt_done = 0;
       sp_tx_hw_lt_enable = 0;

	bEDIDBreak = 0;
	EDID_Print_Enable = 0;
	
       sp_tx_rx_anx7730 = 1;//default the Rx is ANX7730

      //CEC support index initial
	CEC_abort_message_received = 0;
	CEC_get_physical_adress_message_received = 0;
	CEC_logic_addr = 0x00;
	CEC_loop_number = 0;
	CEC_resent_flag = 0;      
}



void SP_CTRL_Set_System_State(SP_TX_System_State ss) 
{

    D("SP_TX To System State: ");
    switch (ss) 
    {
        case SP_TX_INITIAL:
            sp_tx_system_state = SP_TX_INITIAL;
            D("SP_TX_INITIAL");
            break;
        case SP_TX_WAIT_SLIMPORT_PLUGIN: 
            sp_tx_system_state = SP_TX_WAIT_SLIMPORT_PLUGIN;
            D("SP_TX_WAIT_SLIMPORT_PLUGIN");
            break;
        case SP_TX_PARSE_EDID:
            sp_tx_system_state = SP_TX_PARSE_EDID;
            D("SP_TX_READ_PARSE_EDID");
            break;
        case SP_TX_CONFIG_HDMI_INPUT:
            sp_tx_system_state = SP_TX_CONFIG_HDMI_INPUT;
            D("SP_TX_CONFIG_HDMI_INPUT");
            break;
	 case SP_TX_CONFIG_SLIMPORT_OUTPUT:
            sp_tx_system_state = SP_TX_CONFIG_SLIMPORT_OUTPUT;
            D("SP_TX_CONFIG_SLIMPORT_OUTPUT");
            break;
        case SP_TX_LINK_TRAINING:
            sp_tx_system_state = SP_TX_LINK_TRAINING;
	       sp_tx_link_config_done = 0;
		sp_tx_hw_lt_enable = 0;
            D("SP_TX_LINK_TRAINING");
            break;
        case SP_TX_HDCP_AUTHENTICATION:
            sp_tx_system_state = SP_TX_HDCP_AUTHENTICATION;
            D("SP_TX_HDCP_AUTHENTICATION");
            break;
        case SP_TX_PLAY_BACK:
            sp_tx_system_state = SP_TX_PLAY_BACK;
            D("SP_TX_PLAY_BACK");
            break;
        default:
            break;
    }	
}

//check downstream cable stauts ok-20110906-ANX.Fei
BYTE SP_CTRL_Check_Cable_Status(void)
{
    BYTE c;
	SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x18,1,&c);
	if((c&0x28)==0x28)
		return 1; //RX OK

	return 0;

}

void SP_CTRL_Int_Irq_Handler(void)
{

	if(sp_tx_pd_mode )//When chip is power down, do not care the int.-ANX.Fei-20111020
	     return;
//ASUS BSP Wei_Lai+++
	if(sp_tx_rx_anx7730)
	{
	    if(!SP_CTRL_Check_Cable_Status())//wait for downstream cable stauts ok-20110906-ANX.Fei
	    {
	    	SP_CTRL_POLLING_ERR_Int_Handler();	   //RX not ready, check DPCD polling is still available 
	        return;	    	
	    }
	}
//----------

       SP_TX_Int_Irq_Handler();
	   
	if(sp_tx_system_state >=SP_TX_CONFIG_HDMI_INPUT)
		HDMI_RX_Int_Irq_Handler();
  
}


void SP_CTRL_Slimport_Plug_Process(void)
{
	BYTE c;
	int i;
	BYTE AUX_STATUS=0;

printk("SP_CTRL_Slimport_Plug_Process+++\n");
	if (gpio_get_value(SLIMPORT_CABLE_DETECT) )
	{
printk("SP_CTRL_Slimport_Plug_Process++SLIMPORT_CABLE_DETECT=1++++\n");
	   mdelay(50);//dglitch the cable detection
	  if (gpio_get_value(SLIMPORT_CABLE_DETECT) )
	    {
printk("SP_CTRL_Slimport_Plug_Process+++SLIMPORT_CABLE_DETECT=1+++++++\n");
                if(sp_tx_pd_mode)
                {
printk("SP_CTRL_Slimport_Plug_Process+++sp_tx_pd_mode=1+++++++\n");
			SP_TX_Hardware_PowerOn();
			SP_TX_Power_On(SP_TX_PWR_REG);
			SP_TX_Power_On(SP_TX_PWR_TOTAL);
			SP_TX_Initialization();
			HDMI_RX_Initialize();
			sp_tx_pd_mode = 0; 
			SP_TX_VBUS_PowerOn();
			mdelay(500);
			// Startup downstream cable OCM 
		     	SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x25,0x5a);


			for(i=0;i<5;i++)//loop 5 times to check whether the Rx is ANX7730
		       {
		       	//check anx7805 rx anx7730 or not, if 0x00523 is not 0, it means the rx is anx7730. 
		       	SP_TX_AUX_DPCDRead_Bytes(0x00,0x00,0x00,1,&c);
				//D("0x00000 = %.2x\n",(long unsigned int)c);
				if(c==0x11)
				{
					AUX_STATUS = 1;
					break;	
				}			
				mdelay(100);
		       }

			if(!AUX_STATUS)
			{
				D("AUX ERR");
				SP_TX_Power_Down(SP_TX_PWR_TOTAL);
				SP_TX_Power_Down(SP_TX_PWR_REG);
				SP_TX_VBUS_PowerDown();
				SP_TX_Hardware_PowerDown();
				sp_tx_pd_mode = 1;
				sp_tx_ds_hdmi_sensed = 0;
				sp_tx_ds_hdmi_plugged =0;
				sp_tx_ds_monitor_plugged =0;
				sp_tx_link_config_done = 0;
				sp_tx_hw_lt_enable = 0;
				SP_CTRL_Set_System_State(SP_TX_WAIT_SLIMPORT_PLUGIN);
				return;

			}else{

				SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x23,1,&c);
				//D("0x00523 = %.2x\n",(long unsigned int)c);
				if((c !=0)&&(c!=0xff)&&((c&0x80)==0x80))
				{
					sp_tx_rx_anx7730 = 1;
					D("Downstream is HDMI dongle.");
				}else if(c ==0)
				{
					sp_tx_rx_anx7730 = 0;
					D("Downstream is VGA dongle.");	
				}
			}
			   
                }

				  
                if(sp_tx_rx_anx7730)
                {
                  // added for Y-cable & HDMI does not plug out-20111206-ANX.Fei
			SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x18,1,&c);
			if((c&0x1)==0x1)
				sp_tx_ds_hdmi_plugged =1;
			if((c&0x40)==0x40)
				sp_tx_ds_hdmi_sensed = 1;

			if(sp_tx_ds_hdmi_plugged&&sp_tx_ds_hdmi_sensed)//check both hotplug&Rx sense for ANX7730 B0, and only hotplug for ANX7730 A0  -20111011-ANX.Fei
				SP_CTRL_Set_System_State(SP_TX_PARSE_EDID);

                }else{
                     //check sink count for general monitor
                	SP_TX_AUX_DPCDRead_Bytes(0x00,0x02,0x00,1,&c);
			if(c&0x01)
				sp_tx_ds_monitor_plugged = 1;

			if(sp_tx_ds_monitor_plugged)
			{
			       //Indicate the Rx to clear the specific irq
				SP_TX_AUX_DPCDRead_Bytes(0x00,0x04,0x10,1,&c);
			      	SP_TX_AUX_DPCDWrite_Byte(0x00,0x04,0x10,(c|0x01));
				SP_CTRL_Set_System_State(SP_TX_PARSE_EDID);
			}
                }
                
	    }
		
	}
	else if(sp_tx_pd_mode==0)
	{
	printk("SP_CTRL_Slimport_Plug_Process--sp_tx_pd_mode=0-------------\n");
		SP_TX_Power_Down(SP_TX_PWR_TOTAL);
		SP_TX_Power_Down(SP_TX_PWR_REG);
		SP_TX_VBUS_PowerDown();
		SP_TX_Hardware_PowerDown();
		sp_tx_pd_mode = 1;
		sp_tx_ds_hdmi_sensed = 0;
		sp_tx_ds_hdmi_plugged =0;
		sp_tx_ds_monitor_plugged =0;
              sp_tx_link_config_done = 0;
		sp_tx_hw_lt_enable = 0;
		SP_CTRL_Set_System_State(SP_TX_WAIT_SLIMPORT_PLUGIN);
	}
	printk("SP_CTRL_Slimport_Plug_Process-----\n");
}
/*
void SP_CTRL_Video_Changed_Int_Handler (BYTE changed_source)
{
    //D("[INT]: SP_CTRL_Video_Changed_Int_Handler");
    if(sp_tx_system_state > SP_TX_CONFIG_SLIMPORT_OUTPUT)
    {
        switch(changed_source) 
        {
            case 0:
                D("Video:_______________Video clock changed!");
                SP_TX_Disable_Video_Input();
                SP_TX_Enable_Audio_Output(0);
                SP_CTRL_Clean_HDCP();
		  SP_CTRL_Set_System_State(SP_TX_CONFIG_SLIMPORT_OUTPUT);
                break;
            case 1:
               //D("Video:_______________Video format changed!");
                //SP_TX_Disable_Video_Input();
                // SP_TX_Enable_Audio_Output(0);
                //SP_CTRL_Set_System_State(SP_TX_CONFIG_VIDEO);
                break;
            default:
                break;
        } 
    }
}
*/
void SP_CTRL_PLL_Changed_Int_Handler(void)
{
   // D("[INT]: SP_CTRL_PLL_Changed_Int_Handler");
    if (sp_tx_system_state > SP_TX_PARSE_EDID)
    {
	  if(!SP_TX_Get_PLL_Lock_Status())
        {
            D("PLL:_______________PLL not lock!");
            SP_CTRL_Clean_HDCP();
            SP_CTRL_Set_System_State(SP_TX_LINK_TRAINING);
            sp_tx_link_config_done = 0;
        }
    }
}
void SP_CTRL_AudioClk_Change_Int_Handler(void)
{
    //D("[INT]: SP_CTRL_AudioClk_Change_Int_Handler");
    if (sp_tx_system_state > SP_TX_CONFIG_SLIMPORT_OUTPUT)
    {

            D("Audio:_______________Audio clock changed!");
	     SP_TX_Enable_Audio_Output(0);
            SP_CTRL_Clean_HDCP();
            SP_CTRL_Set_System_State(SP_TX_CONFIG_SLIMPORT_OUTPUT);
    }
}

void  SP_CTRL_Auth_Done_Int_Handler(void) 
{
	BYTE c;
       //D("[INT]: SP_CTRL_Auth_Done_Int_Handler");
	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_HDCP_STATUS, &c);
	if(c & SP_TX_HDCP_AUTH_PASS) 
	{
		SP_TX_AUX_DPCDRead_Bytes(0x06,0x80,0x2A,2,ByteBuf);
		c = ByteBuf[1];
		if(c&0x08)//Max cascade
		{
			//max cascade read, fail
			sp_tx_hdcp_auth_fail_counter = 0;
			SP_TX_Video_Mute(1);
			SP_TX_HDCP_Encryption_Disable();
			SP_TX_HDCP_ReAuth();
			D("Re-authentication!");			
		}
		else
		{
		D("Authentication pass in Auth_Done");
		sp_tx_hdcp_auth_pass = 1;
		sp_tx_hdcp_auth_fail_counter = 0;
	} 
	} 
	else 
	{
		D("Authentication failed in AUTH_done");
		sp_tx_hdcp_auth_pass = 0;
		sp_tx_hdcp_auth_fail_counter ++;
		if(sp_tx_hdcp_auth_fail_counter >= SP_TX_HDCP_FAIL_THRESHOLD) 
		{
		   ;//if fail times more than the threshold, stop the HDCP
		}
		else
		{
			sp_tx_hdcp_auth_fail_counter = 0;
			SP_TX_Video_Mute(1);
			SP_TX_HDCP_Encryption_Disable();
			SP_TX_HDCP_ReAuth();
                     //SP_TX_VBUS_PowerDown();//Let ANX7730 get power from HDMI link
			if(sp_tx_system_state > SP_TX_CONFIG_SLIMPORT_OUTPUT)
			{
				SP_CTRL_Set_System_State(SP_TX_HDCP_AUTHENTICATION);
				SP_CTRL_Clean_HDCP();
				return;
			}
		}
	}

	sp_tx_hdcp_auth_done = 1;
}

#if 0
void SP_CTRL_Auth_Change_Int_Handler(void) 
{
	BYTE c;
      // D("[INT]: SP_CTRL_Auth_Change_Int_Handler");
	SP_TX_Get_HDCP_status(&c);
	if(c & SP_TX_HDCP_AUTH_PASS) 
	{
		sp_tx_hdcp_auth_pass = 1;
		D("Authentication pass in Auth_Change");
	} 
	else 
	{
		D("Authentication failed in Auth_change");
		sp_tx_hdcp_auth_pass = 0;
		SP_TX_Video_Mute(1);
		SP_TX_HDCP_Encryption_Disable();
		if(sp_tx_system_state > SP_TX_CONFIG_VIDEO)
		{
			SP_CTRL_Set_System_State(SP_TX_HDCP_AUTHENTICATION);
			SP_CTRL_Clean_HDCP();
		}
	}
}
#endif

/*added for B0 version-ANX.Fei-20110901-Start*/

// hardware linktraining finish interrupt handle process
void SP_CTRL_LT_DONE_Int_Handler(void)
{
	BYTE c;
	//D("[INT]: SP_CTRL_LT_DONE_Int_Handler");

	if((sp_tx_hw_lt_done)||(sp_tx_system_state != SP_TX_LINK_TRAINING))
	return;

	SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_LINK_TRAINING_CTRL_REG, &c);
	if(c&0x80)
	{
		c = (c & 0x70) >> 4;
		D("HW LT failed in interrupt, ERR code = %.2x\n",c);

		 sp_tx_link_config_done = 0;
		 sp_tx_hw_lt_enable = 1;
		 SP_CTRL_Set_System_State(SP_TX_LINK_TRAINING);
		 
	}else{

	       sp_tx_hw_lt_done = 1;
		SP_TX_Read_Reg(SP_TX_PORT0_ADDR, SP_TX_TRAINING_LANE0_SET_REG, &c);
		D("HW LT succeed in interrupt ,LANE0_SET = %.2x\n",c);
	}

       // sp_tx_hw_lt_done = 0; 
}


 void SP_CTRL_LINK_CHANGE_Int_Handler(void)
 {

     BYTE lane0_1_status,sl_cr,al;
  
	
		if(sp_tx_system_state < SP_TX_CONFIG_SLIMPORT_OUTPUT)//(sp_tx_system_state < SP_TX_LINK_TRAINING )
			return;
      
           
            SP_TX_AUX_DPCDRead_Bytes(0x00,0x02,DPCD_LANE_ALIGN_STATUS_UPDATED,1,ByteBuf);
		al = ByteBuf[0];

		  
            SP_TX_AUX_DPCDRead_Bytes(0x00,0x02,DPCD_LANE0_1_STATUS,1,ByteBuf);
		lane0_1_status = ByteBuf[0];

            
           // D("al = %x, lane0_1 = %x\n",(long unsigned int)al,(long unsigned int)lane0_1_status);

            if(((lane0_1_status & 0x01) == 0) || ((lane0_1_status & 0x04) == 0))
            	  sl_cr = 0;
            else 
          	  sl_cr = 1;


            if(((al & 0x01) == 0) || (sl_cr == 0) )//align not done, CR not done
            {
                if((al & 0x01)==0)
                    D("Lane align not done\n");
                if(sl_cr == 0)
                    D("Lane clock recovery not done\n");

                //re-link training only happen when link traing have done		  
                if((sp_tx_system_state > SP_TX_LINK_TRAINING )&&sp_tx_link_config_done)
                {
                    sp_tx_link_config_done = 0;
                    SP_CTRL_Set_System_State(SP_TX_LINK_TRAINING);
                    D("IRQ:____________re-LT request!");
                }
            }
        

 }

// downstream DPCD IRQ request interrupt handle process
void SP_CTRL_SINK_IRQ_Int_Handler(void)
{
    //D("[INT]: SP_CTRL_SINK_IRQ_Int_Handler\n");
    SP_CTRL_IRQ_ISP();
}

// c-wire polling error interrupt handle process
void SP_CTRL_POLLING_ERR_Int_Handler(void)
{
    BYTE c;
    int i;

    //D("[INT]: SP_CTRL_POLLING_ERR_Int_Handler\n");
    if((sp_tx_system_state < SP_TX_WAIT_SLIMPORT_PLUGIN)||sp_tx_pd_mode)
        return;

    for(i=0;i<5;i++)
    {
        SP_TX_AUX_DPCDRead_Bytes(0x00,0x00,0x00,1,&c);
	  if(c==0x11)
	      return;
        mdelay(10);
    }
    
     if(sp_tx_pd_mode ==0)
    {
        //D("read dpcd 0x00000=%.2x\n",(long unsigned int)c);	
        D("Cwire polling is corrupted,power down ANX7805.\n"); 
        SP_TX_Power_Down(SP_TX_PWR_TOTAL);
        SP_TX_Power_Down(SP_TX_PWR_REG);
        SP_TX_VBUS_PowerDown();
        SP_TX_Hardware_PowerDown();
        SP_CTRL_Set_System_State(SP_TX_WAIT_SLIMPORT_PLUGIN);
	SP_CTRL_Clean_HDCP();
	sp_tx_pd_mode = 1;
	sp_tx_ds_hdmi_plugged = 0; 
	sp_tx_ds_hdmi_sensed =0;
	sp_tx_ds_monitor_plugged =0;
	sp_tx_link_config_done =0;
	sp_tx_hw_lt_enable = 0;
    }
}

/*added for B0 version-ANX.Fei-20110901-End*/
void SP_CTRL_IRQ_ISP(void)
{
    BYTE c,c1,lane0_1_status,sl_cr,al;
    BYTE IRQ_Vector,Int_vector1,Int_vector2;


	SP_TX_AUX_DPCDRead_Bytes(0x00,0x02,DPCD_DEVICE_SERVICE_IRQ_VECTOR,1,ByteBuf);
	IRQ_Vector = ByteBuf[0];
	SP_TX_AUX_DPCDWrite_Bytes(0x00, 0x02, DPCD_DEVICE_SERVICE_IRQ_VECTOR,1, ByteBuf);//write clear IRQ


    if(IRQ_Vector & 0x04)//HDCP IRQ
    {
	if(sp_tx_hdcp_auth_pass)
	{
		SP_TX_AUX_DPCDRead_Bytes(0x06,0x80,0x29,1,&c1);
		//D("Bstatus = %.2x\n", (long unsigned int)c1);	
		if(c1 & 0x04)
		{
			if(sp_tx_system_state > SP_TX_HDCP_AUTHENTICATION) 
			{
				SP_CTRL_Set_System_State(SP_TX_HDCP_AUTHENTICATION);
				sp_tx_hw_hdcp_en = 0;
				SP_TX_HW_HDCP_Disable();
				D("IRQ:____________HDCP Sync lost!");
			}
		}
	}    
    }


    if((IRQ_Vector & 0x40)&&(sp_tx_rx_anx7730))//specific int
    {

       // D("Rx specific interrupt IRQ!\n");

	SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,DPCD_SPECIFIC_INTERRUPT_1,1,&Int_vector1);
	SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,DPCD_SPECIFIC_INTERRUPT_1,Int_vector1);


	SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,DPCD_SPECIFIC_INTERRUPT_2,1,&Int_vector2);
	SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,DPCD_SPECIFIC_INTERRUPT_2,Int_vector2);

  
	if((Int_vector1&0x01)==0x01)//check downstream HDMI hpd
	{
		//check downstream HDMI hotplug status plugin
		SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x18,1,&c);
		//D("Rx 00518= 0x%.2x!\n",(long unsigned int)c);
		if(c&0x01)
		{
			sp_tx_ds_hdmi_plugged = 1;
			D("Downstream HDMI is pluged!\n");
		}
	}
	
	if((Int_vector1&0x02)==0x02)
	{
		//check downstream HDMI hotplug status unplug
			SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x18,1,&c);
            		if((c&0x01)!=0x01)
            		{
				D("Downstream HDMI is unpluged!\n");
				sp_tx_ds_hdmi_plugged = 0;
				if((sp_tx_system_state>SP_TX_WAIT_SLIMPORT_PLUGIN)&&(!sp_tx_pd_mode))
				{
					SP_TX_Power_Down(SP_TX_PWR_TOTAL);
					SP_TX_Power_Down(SP_TX_PWR_REG);
					SP_TX_Hardware_PowerDown();
					SP_CTRL_Clean_HDCP();
					SP_CTRL_Set_System_State(SP_TX_WAIT_SLIMPORT_PLUGIN);
					sp_tx_ds_hdmi_sensed = 0;
					sp_tx_pd_mode = 1;
					sp_tx_link_config_done =0;
				}
			}
	}
	

        if(((Int_vector1&0x04)==0x04)&&(sp_tx_system_state > SP_TX_CONFIG_SLIMPORT_OUTPUT))
        {

		D("Rx specific  IRQ: Link is down!\n");

		SP_TX_AUX_DPCDRead_Bytes(0x00,0x02,DPCD_LANE_ALIGN_STATUS_UPDATED,1,ByteBuf);
		al = ByteBuf[0];

		SP_TX_AUX_DPCDRead_Bytes(0x00,0x02,DPCD_LANE0_1_STATUS,1,ByteBuf);
		lane0_1_status = ByteBuf[0];


            if(((lane0_1_status & 0x01) == 0) || ((lane0_1_status & 0x04) == 0))
            	 sl_cr = 0;
            else 
           	 sl_cr = 1;


            if(((al & 0x01) == 0) || (sl_cr == 0) )//align not done, CR not done
            {
                if((al & 0x01)==0)
                    D("Lane align not done\n");
                if(sl_cr == 0)
                    D("Lane clock recovery not done\n");

                //re-link training only happen when link traing have done		  
                if((sp_tx_system_state > SP_TX_LINK_TRAINING )&&sp_tx_link_config_done)
                {
                    sp_tx_link_config_done = 0;
                    SP_CTRL_Set_System_State(SP_TX_LINK_TRAINING);
                    D("IRQ:____________re-LT request!");
                }
            }
        

        }

        if((Int_vector1&0x08)==0x08)
        {
            D("Downstream HDCP is done!\n");
	     if((Int_vector1&0x10) !=0x10)
	     		D("Downstream HDCP is passed!\n");
	     else
	     {
		      if(sp_tx_system_state > SP_TX_CONFIG_SLIMPORT_OUTPUT )
		        {
		            SP_TX_Video_Mute(1);
		            SP_TX_HDCP_Encryption_Disable();
		            SP_CTRL_Clean_HDCP();
		            SP_CTRL_Set_System_State(SP_TX_HDCP_AUTHENTICATION);
		            D("Re-authentication due to downstream HDCP failure!");
		        }
	     }
	   

        }
        if((Int_vector1&0x20)==0x20)
        {
              D(" Downstream HDCP link integrity check fail!");
		//add for hdcp fail
		if(sp_tx_system_state > SP_TX_HDCP_AUTHENTICATION) 
		{
			SP_CTRL_Set_System_State(SP_TX_HDCP_AUTHENTICATION);
			sp_tx_hw_hdcp_en = 0;
			SP_TX_HW_HDCP_Disable();
			D("IRQ:____________HDCP Sync lost!");
		}
        }

        if((Int_vector1&0x40)==0x40)
        {
            D("Receive CEC command from upstream done!");
	     CEC_Rx_Process();
        }

        if((Int_vector1&0x80)==0x80)
        {
            D("CEC command transfer to downstream done!");
        }


	/*check downstream HDMI Rx sense status -20110906-ANX.Fei*/
	if((Int_vector2&0x04)==0x04)
	{
		SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x18,1,&c);
		if((c&0x40)==0x40)
		{
			//sp_tx_ds_hdmi_sensed= 1;//remove because of late  Rx sense
			D("Downstream HDMI termination is detected!\n");
		}
	}

	/*check downstream V21 power ready status -20110908-ANX.Fei*/
	if((sp_tx_ds_hdmi_plugged)&&(sp_tx_ds_hdmi_sensed == 0))//check V21 ready
	{

		SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x19,1,&c);
		if((c&0x01)==0x01)//DPCD V21 ready
		{
			sp_tx_ds_hdmi_sensed = 1; //Rx sense is ready when V21 power is ok.
			D("Downstream V21 power is ready!\n");
		}
	}


    }
    else  if((IRQ_Vector & 0x40)&&(!sp_tx_rx_anx7730))//specific int 	
    {
               //check sink count for general monitor
            	SP_TX_AUX_DPCDRead_Bytes(0x00,0x02,0x00,1,&c);
		if(c&0x01)
			sp_tx_ds_monitor_plugged = 1;
		else{
			sp_tx_ds_monitor_plugged = 0;
			if((sp_tx_system_state > SP_TX_WAIT_SLIMPORT_PLUGIN )&&(sp_tx_pd_mode ==0))
			{
				SP_CTRL_Set_System_State(SP_TX_WAIT_SLIMPORT_PLUGIN);
				SP_CTRL_Clean_HDCP();
				sp_tx_pd_mode = 0;
				sp_tx_ds_monitor_plugged =0;
				sp_tx_link_config_done =0;
				sp_tx_hw_lt_enable = 0;
			}
		}

    		//Indicate the Rx to clear the specific irq
		SP_TX_AUX_DPCDRead_Bytes(0x00,0x04,0x10,1,&c);
	      	SP_TX_AUX_DPCDWrite_Byte(0x00,0x04,0x10,(c|0x01));

			
            SP_TX_AUX_DPCDRead_Bytes(0x00,0x02,DPCD_LANE_ALIGN_STATUS_UPDATED,1,ByteBuf);
             	al = ByteBuf[0];

	     SP_TX_AUX_DPCDRead_Bytes(0x00,0x02,DPCD_LANE0_1_STATUS,1,ByteBuf);
	     	lane0_1_status = ByteBuf[0];
	  
            if(((lane0_1_status & 0x01) == 0) || ((lane0_1_status & 0x04) == 0))
          	  sl_cr = 0;
            else 
           	 sl_cr = 1;

            if(((al & 0x01) == 0) || (sl_cr == 0) )//align not done, CR not done
            {
                if((al & 0x01)==0)
                    D("Lane align not done\n");
                if(sl_cr == 0)
                    D("Lane clock recovery not done\n");

                //re-link training only happen when link traing have done		  
                if((sp_tx_system_state > SP_TX_LINK_TRAINING )&&sp_tx_link_config_done)
                {
                    sp_tx_link_config_done = 0;

                    SP_CTRL_Set_System_State(SP_TX_LINK_TRAINING);
                    D("IRQ:____________re-LT request!");
                }
            }
        

       	
    }



}

void SP_CTRL_Clean_HDCP(void)
{
      // D("HDCP Clean!");
	sp_tx_hdcp_auth_fail_counter = 0;
	sp_tx_hdcp_auth_pass = 0;
	sp_tx_hw_hdcp_en = 0;
	sp_tx_hdcp_capable_chk = 0;
	sp_tx_hdcp_auth_done = 0;

	SP_TX_Clean_HDCP();

}

void SP_CTRL_EDID_Process(void)
{
	BYTE i;
	//read DPCD 00000-0000b
        for(i = 0; i <= 0x0b; i ++)
        SP_TX_AUX_DPCDRead_Bytes(0x00,0x00,i,1,ByteBuf);
           
	SP_CTRL_EDID_Read();
	SP_TX_RST_AUX();
	if(bEDIDBreak)
	D("ERR:EDID corruption!\n");
	HDMI_RX_Set_HPD();
       HDMI_RX_TMDS_En();
	SP_CTRL_Set_System_State(SP_TX_CONFIG_HDMI_INPUT);
}


void SP_CTRL_EDID_Read(void)//add adress only cmd before every I2C over AUX access.-fei
{
	BYTE i,j,test_vector,edid_block = 0,segment = 0,offset = 0;

	SP_TX_EDID_Read_Initial();

	checksum = 0;
	sp_tx_ds_edid_hdmi =0;
	bEDIDBreak = 0;
	//Set the address only bit
	SP_TX_AddrOnly_Set(1);
	//set I2C write com 0x04 mot = 1
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG, 0x04);
	//enable aux
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, SP_TX_AUX_CTRL_REG2, 0x01);
	SP_TX_Wait_AUX_Finished();

	edid_block = SP_TX_Get_EDID_Block();
	if(edid_block < 2)
	{

		edid_block = 8 * (edid_block + 1);
		for(i = 0; i < edid_block; i ++)
		{
			if(!bEDIDBreak)
			SP_TX_AUX_EDIDRead_Byte(i * 16);
		}

		//clear the address only bit
		SP_TX_AddrOnly_Set(0);

	}else{

		for(i = 0; i < 16; i ++)
		{
			if(!bEDIDBreak)
			SP_TX_AUX_EDIDRead_Byte(i * 16);
		}

		//clear the address only bit
		SP_TX_AddrOnly_Set(0);
		if(!bEDIDBreak)
		{
			edid_block = (edid_block + 1);
			for(i=0; i<((edid_block-1)/2); i++)//for the extern 256bytes EDID block
			{
				D("EXT 256 EDID block");		
				segment = i + 1;		
				for(j = 0; j<16; j++)
				{
					SP_TX_Parse_Segments_EDID(segment, offset);
					offset = offset + 0x10;
				}
			}

			if(edid_block%2)//if block is odd, for the left 128BYTEs EDID block
			{
				D("Last block");
				segment = segment + 1;
				for(j = 0; j<8; j++)
				{
					SP_TX_Parse_Segments_EDID(segment, offset);
					offset = offset + 0x10;
				}
			}	

		}


	}


	SP_TX_AddrOnly_Set(0);   //clear the address only bit

	SP_TX_RST_AUX();

	if(sp_tx_ds_edid_hdmi)
		SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x26, 0x01);//inform ANX7730 the downstream is HDMI
	else
		SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x26, 0x00);//inform ANX7730 the downstream is not HDMI


	SP_TX_AUX_DPCDRead_Bytes(0x00,0x02,0x18,1,ByteBuf);


	test_vector = ByteBuf[0];

	if(test_vector & 0x04)//test edid
	{
		// D("check sum = %.2x\n", (long unsigned int)checksum);
		SP_TX_AUX_DPCDWrite_Byte(0x00,0x02,0x61,checksum);   
		SP_TX_AUX_DPCDWrite_Byte(0x00,0x02,0x60,0x04);
		D("Test read EDID finished");
	}
}


void SP_CTRL_HDCP_Process(void)
{
    BYTE c;
    
  
    if(!sp_tx_hdcp_capable_chk)
    {            
        sp_tx_hdcp_capable_chk = 1; 

	SP_TX_AUX_DPCDRead_Bytes(0x06, 0x80, 0x28,1,&c);
	if(!(c & 0x01))
        {  
            D("Sink is not capable HDCP");
            SP_TX_Video_Mute(1);//when Rx does not support HDCP, force to send blue screen
            SP_CTRL_Set_System_State(SP_TX_PLAY_BACK);
            return;
        }
    }

    if(!sp_tx_hw_hdcp_en)
    {      
        SP_TX_Power_On(SP_TX_PWR_HDCP);// Poer on HDCP link clock domain logic for B0 version-20110913-ANX.Fei
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, 0x40, 0xb0);
	SP_TX_Write_Reg(SP_TX_PORT0_ADDR, 0x42, 0xc8);//increase KSV list wait timer for HDCP CTS 
        SP_TX_HW_HDCP_Enable();
		
	SP_TX_Write_Reg(SP_TX_PORT2_ADDR, SP_COMMON_INT_MASK2, 0xfc);//unmask auth change&done int
        sp_tx_hw_hdcp_en = 1;
    }

    if(sp_tx_hdcp_auth_done)//this flag will be only set in auth done interrupt
    {
         sp_tx_hdcp_auth_done = 0;   
            
        if(sp_tx_hdcp_auth_pass)
        {            
        
            //SP_TX_HDCP_Encryption_Enable();
            SP_TX_Video_Mute(0);
            D("@@@@@@@@@@@@@@@@@@@@@@@hdcp_auth_pass@@@@@@@@@@@@@@@@@@@@\n");
            
        }
        else
        {            
            //SP_TX_HDCP_Encryption_Disable();
            SP_TX_Video_Mute(1);
            D("***************************hdcp_auth_failed*********************************\n");
            return;
        }
        
 
        SP_CTRL_Set_System_State(SP_TX_PLAY_BACK);
        //SP_TX_RST_AUX();
        SP_TX_Show_Infomation();
    }

}


void SP_CTRL_PlayBack_Process(void)
{	

       SP_TX_Set_3D_Packets();  
}

void CEC_Abort_Message_Process()
{
	unsigned char c, i;
	CEC_abort_message_received = 0;
	SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x70,0x01);//reset CEC

	mdelay(10);
	//D("indicator_addr = %.2x\n",(long unsigned int)indicator_addr);
	SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x80,CEC_logic_addr);
    	SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x80,0x00);//Feature Abort Message
	//SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x80,0x00);//Feature Abort Message

	SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x70,1,&c);
    	SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x70,(c|0x04));//send CEC massage
	
    	SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x72,1,&c);

	CEC_loop_number = 0;
	while(c&0x80)//tx busy
	{
		CEC_loop_number ++; 				
    	       SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x72,1,&c);
		
		if(CEC_loop_number > 50)
		{
			mdelay(10);
			CEC_resent_flag = 1;
	              SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x70,0x01);//reset CEC
	              SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x70,0x48);//Enable CEC receiver and Set CEC logic address

			D("loop number > 20000, break from sent");
			break;						
		}
	}
    
    	SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x72,1,&c);
	//D("Recieved Abort Message and Send Feature Abort Message Finished, TX STATUS = %.2x\n",(long unsigned int)c);
        D("R&s Finished, STATUS = %.2x\n",c);
	mdelay(220);
	
	for(i = 0;i<5;i++)
	{
		mdelay(10);
            	SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x11,1,&c);
              SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x11,c);
              D("IRQ 2 = 0x%.2x!\n",c);

		if((c&0x02)||CEC_resent_flag)// if sent failed, resend 1 time, add for 9.3-1
		{		
			CEC_resent_flag = 0;
			//D("Resend Feature Abort Message.....................................................\n");
	              SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x70,0x01);//reset CEC
			mdelay(10);
                	SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x80,CEC_logic_addr);
                    	SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x80,0x00);//Feature Abort Message

                     SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x70,1,&c);
   	              SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x70,(c|0x04));//send CEC massage
			
    	              SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x72,1,&c);
			CEC_loop_number = 0;
			while(c&0x80)
			{
				CEC_loop_number ++; 						
    	                     SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x72,1,&c);
				
				if(CEC_loop_number > 20) 
				{								
					CEC_resent_flag = 1;
					mdelay(10);
	                            SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x70,0x01);//reset CEC
	                            SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x70,0x48);//Enable CEC receiver and Set CEC logic address
			
					D("loop number > 20000, break from sent");
					break;
				}
			}	
    	              SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x72,1,&c);
                    D("R&s Finished, STATUS = %.2x\n",c);
                    // D("Recieved Abort Message and Send Feature Abort Message Finished, TX STATUS = %.2x\n",(long unsigned int)c); 
		}
	
	}				
	
       SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x70,0x01);//reset CEC
	SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x70,0x48);//Enable CEC receiver and Set CEC logic address
}


void CEC_Get_Physical_Address_Message_Process()
{
	unsigned char c, i;
	CEC_get_physical_adress_message_received = 0;
	SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x70,  0x01);//reset CEC
	SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x80,  0x4f);//destination address is 0x0F;
	SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x80,  0x84);//Report Physical Address
	SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x80,  0x00);
	SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x80,  0x00);
	SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x80,  0x00);
	
	//I2C_Master_Read_Reg(DP_TX_PORT3_ADDR,0x80,&c);  
	SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x70,0x04);//send CEC massage.
	
	SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x72,1,&c);  			
	while(c&0x80)				
		SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x72,1,&c);  	
	
	SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x72,1,&c);  	
	D("Received Get Physical Address Message and Send Broadcast Finished, TX STATUS = %.2x\n",c);

	for(i = 0;i<5;i++)
	{
		mdelay(100);
		SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x11,1,&c); 
              SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x11,c);
              D("IRQ 2 = 0x%.2x!\n",c);

		if(c&0x02)// if sent failed, resend 1 time, add for 9.3-1
		{					
			D("Resend Feature Abort Message.....................................................\n");
			SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x70,0x01);//reset CEC
			SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x80,  0x4f);//destination address is 0x0F;
			SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x80,  0x84);//Report Physical Address
			SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x80,  0x00);
			SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x80,  0x00);
			SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x80,  0x00);				
			//I2C_Master_Read_Reg(DP_TX_PORT3_ADDR,0x80,&c);  
			SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x70, 0x04);//send CEC massage.
			
			SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x72,1,&c);  			
			while(c&0x80)  //tx busy
				SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x72,1,&c);  	
			SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x72,1,&c);  
			D("Recieved Abort Message and Send Feature Abort Message Finished, TX STATUS = %.2x\n",c); 
		}				
	}

	
        SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x70,0x01);//reset CEC
	 SP_TX_AUX_DPCDWrite_Byte(0x00,0x05,0x70,0x48);//Enable CEC receiver and Set CEC logic address
}


void CEC_Rx_Process(void)
{	
	//unsigned char p;
    //unsigned char cec[16];
	unsigned char i,c,c0;
 
              D("Receive CEC command from upstream done!");

		SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x71,1,&c);//Check RX FIFO Length

		//D("The CEC Rx status = %.2x\n",(long unsigned int)c);

		if (c & 0x20)
			c =16;
		else
			c = c&0x0f;
		//D("c = %.2x\n",(long unsigned int)c);

		for(i=0;i<16;i++)
		{
		       SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x80,1,&c0);
			//D("RX indicator is: %.2x, ", (long unsigned int)c);
		       SP_TX_AUX_DPCDRead_Bytes(0x00,0x05,0x80,1,&c0);
			D("FIFO[%.2x] = %.2x\n",i,c0);
	
			if(i==0) 
			{
				CEC_logic_addr = (c0 & 0x0f)<<4;
				CEC_logic_addr=CEC_logic_addr|((c0 & 0xf0)>>4);
			}
			if((i == 1)&&(c0 == 0xFF)) CEC_abort_message_received = 1;
			if((i == 1)&&(c0 == 0x83)) CEC_get_physical_adress_message_received = 1;

		}

		if(CEC_abort_message_received)
			CEC_Abort_Message_Process();	
		else if(CEC_get_physical_adress_message_received)
			CEC_Get_Physical_Address_Message_Process(); 


}







