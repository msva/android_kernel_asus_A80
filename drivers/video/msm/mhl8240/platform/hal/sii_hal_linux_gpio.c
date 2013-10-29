/*

SiI8240 Linux Driver

Copyright (C) 2011-2012 Silicon Image Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation version 2.

This program is distributed .as is. WITHOUT ANY WARRANTY of any
kind, whether express or implied; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the
GNU General Public License for more details.

*/


/**
 * @file sii_hal_linux_gpio.c
 *
 * @brief Linux implementation of GPIO pin support needed by Silicon Image
 *        MHL devices.
 *
 * $Author: Dave Canfield
 * $Rev: $
 * $Date: Feb. 9, 2011
 *
 *****************************************************************************/

#define SII_HAL_LINUX_GPIO_C
/***** #include statements ***************************************************/
#include "sii_hal.h"
#include "sii_hal_priv.h"
#include "si_c99support.h"
#include "si_osdebug.h"
//joe1_++
#ifdef CONFIG_EEPROM_NUVOTON
#include <linux/microp.h>
#include <linux/microp_api.h>
#include <linux/microp_pin_def.h>
#endif //#ifdef CONFIG_EEPROM_NUVOTON
//joe1_--

//#include <mach/msm_iomap.h>
//#include <linux/gpio.h>
//#include <linux/io.h>
/***** local macro definitions ***********************************************/


/***** local type definitions ************************************************/
#define PM8921_GPIO_BASE    152
#define PM8921_GPIO_PM_TO_SYS(pm_gpio)  (pm_gpio - 1 + PM8921_GPIO_BASE)

/***** local variable declarations *******************************************/


/***** local function prototypes *********************************************/


/***** global variable declarations *******************************************/


// Simulate the DIP switches
bool	pinDbgMsgs	= false;	// simulated pinDbgSw2 0=Print
bool	pinAllowD3	= true;	// false allows debugging
bool	pinOverrideTiming = true;	// simulated pinDbgSw2
bool	pinDataLaneL	= true;		// simulated pinDbgSw3
bool	pinDataLaneH	= true;		// simulated pinDbgSw4
//ASUS_BSP+++ larry lai : Non-transcode mode, do HDCP
#ifdef SII8240_NO_TRANSCODE_MODE
#ifdef SII8240_HDCP_ON
bool	pinDoHdcp	= true;		// simulated pinDbgSw5
#else
bool	pinDoHdcp	= false;		// simulated pinDbgSw5
#endif
#else
bool	pinDoHdcp	= false;		// simulated pinDbgSw5
#endif
//ASUS_BSP--- larry lai : Non-transcode mode, do HDCP
bool    pinWakePulseEn = true;   // wake pulses enabled by default
bool    pinForcePackedPixel=false;
//ASUS_BSP+++ larry lai : Non-transcode mode
#ifdef SII8240_NO_TRANSCODE_MODE
bool	pinTranscodeMode=false;
#else
bool	pinTranscodeMode=true;
#endif
//ASUS_BSP+++ larry lai : Non-transcode mode
//	 Simulate the GPIO pins
bool	 pinTxHwReset	= false;	// simulated reset pin %%%% TODO possible on Beagle?
bool	 pinM2uVbusCtrlM	= true;		// Active high, needs to be low in MHL connected state, high otherwise.
bool	 pinVbusEnM	= true;		// Active high input. If high sk is providing power, otherwise not.
bool	pinMhlVbusSense=false;

// Simulate the LEDs
bool	pinMhlConn	= true;		// MHL Connected LED 
bool	pinUsbConn	= false;	// USB connected LED
bool	pinSourceVbusOn	= true;		// Active low LED. On when pinMhlVbusSense and pinVbusEnM are active
bool	pinSinkVbusOn	= true;		// Active low LED. On when pinMhlVbusSense is active and pinVbusEnM is not active

/***** local functions *******************************************************/


/***** public functions ******************************************************/
extern bool asus_padstation_exist_realtime(void);

/*****************************************************************************/
/**
 * @brief Configure platform GPIOs needed by the MHL device.
 *
 *****************************************************************************/

/* GPIO parameters */
/* direction */
#define	PM_GPIO_DIR_OUT			0x01
#define	PM_GPIO_DIR_IN			0x02
#define	PM_GPIO_DIR_BOTH		(PM_GPIO_DIR_OUT | PM_GPIO_DIR_IN)

/* output_buffer */
#define	PM_GPIO_OUT_BUF_OPEN_DRAIN	1
#define	PM_GPIO_OUT_BUF_CMOS		0

/* pull */
#define	PM_GPIO_PULL_UP_30		0
#define	PM_GPIO_PULL_UP_1P5		1
#define	PM_GPIO_PULL_UP_31P5		2
#define	PM_GPIO_PULL_UP_1P5_30		3
#define	PM_GPIO_PULL_DN			4
#define	PM_GPIO_PULL_NO			5

/* vin_sel: Voltage Input Select */
#define	PM_GPIO_VIN_VPH			0 /* 3v ~ 4.4v */
#define	PM_GPIO_VIN_BB			1 /* ~3.3v */
#define	PM_GPIO_VIN_S4			2 /* 1.8v */
#define	PM_GPIO_VIN_L15			3
#define	PM_GPIO_VIN_L4			4
#define	PM_GPIO_VIN_L3			5
#define	PM_GPIO_VIN_L17			6

/* vin_sel: Voltage Input select on PM8058 */
#define PM8058_GPIO_VIN_VPH		0
#define PM8058_GPIO_VIN_BB		1
#define PM8058_GPIO_VIN_S3		2
#define PM8058_GPIO_VIN_L3		3
#define PM8058_GPIO_VIN_L7		4
#define PM8058_GPIO_VIN_L6		5
#define PM8058_GPIO_VIN_L5		6
#define PM8058_GPIO_VIN_L2		7

/* vin_sel: Voltage Input Select on PM8038*/
#define PM8038_GPIO_VIN_VPH		0
#define PM8038_GPIO_VIN_BB		1
#define PM8038_GPIO_VIN_L11		2
#define PM8038_GPIO_VIN_L15		3
#define PM8038_GPIO_VIN_L4		4
#define PM8038_GPIO_VIN_L3		5
#define PM8038_GPIO_VIN_L17		6

/* out_strength */
#define	PM_GPIO_STRENGTH_NO		0
#define	PM_GPIO_STRENGTH_HIGH		1
#define	PM_GPIO_STRENGTH_MED		2
#define	PM_GPIO_STRENGTH_LOW		3

/* function */
#define	PM_GPIO_FUNC_NORMAL		0
#define	PM_GPIO_FUNC_PAIRED		1
#define	PM_GPIO_FUNC_1			2
#define	PM_GPIO_FUNC_2			3
#define	PM_GPIO_DTEST1			4
#define	PM_GPIO_DTEST2			5
#define	PM_GPIO_DTEST3			6
#define	PM_GPIO_DTEST4			7

 struct pm_gpio {
	int		direction;
	int		output_buffer;
	int		output_value;
	int		pull;
	int		vin_sel;
	int		out_strength;
	int		function;
	int		inv_int_pol;
	int		disable_pin;
};
    struct pm_gpio gpio44_param = {
        .direction = PM_GPIO_DIR_OUT,
        .output_buffer = PM_GPIO_OUT_BUF_CMOS,
        .output_value = 0,
        .pull = PM_GPIO_PULL_NO,
        .vin_sel = 2,
        .out_strength = PM_GPIO_STRENGTH_HIGH,
        .function = PM_GPIO_FUNC_PAIRED,
        .inv_int_pol = 0,
        .disable_pin = 0,
    };

 int pm8xxx_gpio_config(int gpio, struct pm_gpio *param);


halReturn_t HalGpioInit(void)
{
	int status;
//    	static int mhl_rst_gpio;	

	/* Configure GPIO used to perform a hard reset of the device. */
//    mhl_rst_gpio = PM8921_GPIO_PM_TO_SYS(W_RST_GPIO);

    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "@@@ HalInit mhl_rst_gpio =  %d ###\n", g_GPIO_MHL_RST_N);
    	
    status = gpio_request(g_GPIO_MHL_RST_N, "W_RST#");
	if (status < 0)
	{
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"### HalInit gpio_request for GPIO %d (H/W Reset) failed, status: %d\n",
    			g_GPIO_MHL_RST_N, status);
		return HAL_RET_FAILURE;
	}

   switch (g_A68_hwID)
   {
        case A68_EVB:
               printk("[MHL] a68 gpio config table = EVB\n");  

			status = pm8xxx_gpio_config(g_GPIO_MHL_RST_N, &gpio44_param);
			if (status) {
			    SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "pm8xxx_gpio_config 44 failed rc=%d\n", status);
				return HAL_RET_FAILURE;
			}
		       gpio_set_value(g_GPIO_MHL_RST_N, 1);	

               break;
         case A68_SR1_1:
                printk("[MHL] a68 gpio config table = SR1_1\n");  

	  	   gpio_direction_output(g_GPIO_MHL_RST_N, 1);

               break;
         case A68_SR2:
                printk("[MHL] a68 gpio config table = SR2\n");  

	  	   gpio_direction_output(g_GPIO_MHL_RST_N, 1);

               break;

	 default:
               printk("[MHL] a68 gpio config table default use = SR1_1\n");

	  	   gpio_direction_output(g_GPIO_MHL_RST_N, 1);

               break;
   }


	status = gpio_request(W_USB_SW_SEL_GPIO, "W_USB_SW_SEL");
	if (status < 0)
	{
		SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
				"HalInit gpio_request for GPIO %d failed, status: %d\n",
				W_USB_SW_SEL_GPIO, status);
		gpio_free(g_GPIO_MHL_RST_N);		
		return HAL_RET_FAILURE;
	}

//A80 SR1_1 still not use MHL_3V3_EN_GPIO
   switch (g_A68_hwID)
   {
	case A80_EVB:
	case A80_SR1:			
	case A80_SR2:
	case A80_SR3:
	case A80_SR4:
	case A80_SR5:
	case A80_ER:
	case A80_PR:
		status = gpio_request(MHL_3V3_EN_GPIO, "MHL_3V3_EN");
		if (status < 0)
		{
	    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
	    			"HalInit gpio_request for GPIO %d (VBUS) failed, status: %d\n",
	    			MHL_3V3_EN_GPIO, status);
			gpio_free(W_USB_SW_SEL_GPIO);
			gpio_free(g_GPIO_MHL_RST_N);
			return HAL_RET_FAILURE;
		}

		status = gpio_direction_output(MHL_3V3_EN_GPIO, 0);
		if (status < 0)
		{
	    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
	    			"HalInit gpio_direction_output for GPIO %d (MHL 3V3 enable) failed, status: %d\n",
	    			MHL_3V3_EN_GPIO, status);
			gpio_free(W_USB_SW_SEL_GPIO);
			gpio_free(g_GPIO_MHL_RST_N);				
			gpio_free(MHL_3V3_EN_GPIO);
			return HAL_RET_FAILURE;
		}

		gpio_set_value(MHL_3V3_EN_GPIO, 1);
          break;
	 default:
	 	// for A68, not use MHL_3V3_EN_GPIO
          break;
   }
	/*
	 * Configure the GPIO used as an interrupt input from the device
	 * NOTE: GPIO support should probably be initialized BEFORE enabling
	 * interrupt support
	 */
	status = gpio_request(g_GPIO_MHL_IRQ_N, "MHL_IRQ_N");
	if(status < 0)
	{
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"HalInitGpio gpio_request for GPIO %d (interrupt)failed, status: %d\n",
    			g_GPIO_MHL_IRQ_N, status);
		gpio_free(W_USB_SW_SEL_GPIO);
		gpio_free(g_GPIO_MHL_RST_N);
		gpio_free(MHL_3V3_EN_GPIO);				
		return HAL_RET_FAILURE;
	}

	status = gpio_direction_input(g_GPIO_MHL_IRQ_N);
	if(status < 0)
	{
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"HalInitGpio gpio_direction_input for GPIO %d (interrupt)failed, status: %d",
    			g_GPIO_MHL_IRQ_N, status);
		gpio_free(g_GPIO_MHL_IRQ_N);
		gpio_free(W_USB_SW_SEL_GPIO);
		gpio_free(g_GPIO_MHL_RST_N);
		gpio_free(MHL_3V3_EN_GPIO);		
		return HAL_RET_FAILURE;
	}
	
//==================================
	status = gpio_request(MHL_USB_ID_DETECT, "MHL_USB_ID_DETECT");
	if(status < 0)
	{
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"HalInitGpio gpio_request for GPIO %d (interrupt)failed, status: %d\n",
    			g_GPIO_MHL_IRQ_N, status);
		gpio_free(W_USB_SW_SEL_GPIO);
		gpio_free(g_GPIO_MHL_IRQ_N);		
		gpio_free(g_GPIO_MHL_RST_N);
		gpio_free(MHL_3V3_EN_GPIO);				
		return HAL_RET_FAILURE;
	}

	status = gpio_direction_input(MHL_USB_ID_DETECT);
//	status = gpio_direction_output(MHL_USB_ID_DETECT, 1);
	if(status < 0)
	{
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"HalInitGpio gpio_direction_input for GPIO %d (interrupt)failed, status: %d",
    			g_GPIO_MHL_IRQ_N, status);
		gpio_free(g_GPIO_MHL_IRQ_N);
		gpio_free(W_USB_SW_SEL_GPIO);
		gpio_free(g_GPIO_MHL_RST_N);	
		gpio_free(MHL_USB_ID_DETECT);		
		gpio_free(MHL_3V3_EN_GPIO);
		return HAL_RET_FAILURE;
	}
//===================================
   switch (g_A68_hwID)
   {
	case A68_EVB:
	case A68_SR1_1:
	case A68_SR1_2:
	case A68_SR2:
	case A68_ER1:
	case A68_ER2:
	case A68_ER3:
	case A68_PR:
	case A68_MP:   
//ASUS_BSP, A68 set MHL INT wakeup gpio +++
    	if (enable_irq_wake(gpio_to_irq(g_GPIO_MHL_IRQ_N)) !=0 )			
		printk("[MHL] Failed to mark GPIO %d as wake source\n", W_INT_GPIO);
//ASUS_BSP, A68 set MHL INT wakeup gpio ---		
		break;
	case A80_EVB:
	case A80_SR1:			
	case A80_SR2:
	case A80_SR3:
	case A80_SR4:
	case A80_SR5:
	case A80_ER:
	case A80_PR:
//ASUS_BSP, A80 set GPIO77 USB_ID wakeup gpio +++
    	if (enable_irq_wake(gpio_to_irq(MHL_USB_ID_DETECT)) !=0 )			
		printk("[MHL] Failed to mark GPIO %d as wake source\n", MHL_USB_ID_DETECT);
//ASUS_BSP, A80 set GPIO77 USB_ID wakeup gpio ---
		break;
	default:
//ASUS_BSP, A68 set MHL INT wakeup gpio +++
    	if (enable_irq_wake(gpio_to_irq(g_GPIO_MHL_IRQ_N)) !=0 )			
		printk("[MHL] Failed to mark GPIO %d as wake source\n", W_INT_GPIO);
//ASUS_BSP, A68 set MHL INT wakeup gpio ---		
          break;
   }

    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "---- HalGpioInit usb_sel = (%d) , rst =(%d) ---\n", gpio_get_value(W_USB_SW_SEL_GPIO), gpio_get_value(g_GPIO_MHL_RST_N) );

	return HAL_RET_SUCCESS;
}



/*****************************************************************************/
/**
 * @brief Release GPIO pins needed by the MHL device.
 *
 *****************************************************************************/
halReturn_t HalGpioTerm(void)
{
	halReturn_t 	halRet;
//    int mhl_rst_gpio;	


	halRet = HalInitCheck();
	if(halRet != HAL_RET_SUCCESS)
	{
		return halRet;
	}

	/* Configure GPIO used to perform a hard reset of the device. */
//    mhl_rst_gpio = PM8921_GPIO_PM_TO_SYS(W_RST_GPIO);

	gpio_free(g_GPIO_MHL_IRQ_N);
	gpio_free(g_GPIO_MHL_RST_N);
	gpio_free(MHL_USB_ID_DETECT);
	
	gpio_free(MHL_3V3_EN_GPIO);	

	return HAL_RET_SUCCESS;
}



/*****************************************************************************/
/**
 * @brief Platform specific function to control the reset pin of the MHL
 * 		  transmitter device.
 *
 *****************************************************************************/
halReturn_t HalGpioSetTxResetPin(bool value)
{
	halReturn_t 	halRet;
//    	static int mhl_rst_gpio;	

	halRet = HalInitCheck();
	if(halRet != HAL_RET_SUCCESS)
	{
		return halRet;
	}

	//gpio_set_value(W_RST_GPIO, value);
//       mhl_rst_gpio = PM8921_GPIO_PM_TO_SYS(W_RST_GPIO);
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "HalGpioSetTxResetPin mhl_rst_gpio =  %d \n", g_GPIO_MHL_RST_N);

    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "+++ HalGpioSetTxResetPin rst =(%d) ---\n", gpio_get_value(g_GPIO_MHL_RST_N) );

    	
       gpio_set_value(g_GPIO_MHL_RST_N, value);

    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "--- HalGpioSetTxResetPin rst =(%d) ---\n", gpio_get_value(g_GPIO_MHL_RST_N) );

	return HAL_RET_SUCCESS;
}


halReturn_t HalGpioSetMHLSwitchPin(bool value)
{
	halReturn_t 	halRet;
	int status;

//joe1_++
#ifdef CONFIG_EEPROM_NUVOTON
	if (AX_MicroP_IsP01Connected() && asus_padstation_exist_realtime()) {
		return HAL_RET_SUCCESS;
	}
#endif //#ifdef CONFIG_EEPROM_NUVOTON
//joe1_--
	
	halRet = HalInitCheck();
	if(halRet != HAL_RET_SUCCESS)
	{
		return halRet;
	}

	status = gpio_direction_output(W_USB_SW_SEL_GPIO, value);
	if (status < 0)
	{
    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
    			"HalInit gpio_direction_output for GPIO %d failed, status: %d\n",
    			W_USB_SW_SEL_GPIO, status);
		return HAL_RET_FAILURE;
	}
//    	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE, "--- HalGpioSetUSBSwitchPin =(%d) ---\n", gpio_get_value(W_USB_SW_SEL_GPIO) );
	printk("[MHL] --- HalGpioSetUSBSwitchPin =(%d) ---\n", gpio_get_value(W_USB_SW_SEL_GPIO));
	
	return HAL_RET_SUCCESS;
}

extern bool asus_padstation_exist(void);

halReturn_t HalGpioGetP03PlugPin(void)
{
	halReturn_t 	halRet;
	
	halRet = HalInitCheck();
	if(halRet != HAL_RET_SUCCESS)
	{
		return halRet;
	}

	if (asus_padstation_exist())
        {
		return HAL_RET_SUCCESS;
        }

	return HAL_RET_FAILURE;
}


/*****************************************************************************/
/**
 * @brief Platform specific function to control power on the USB port.
 *
 *****************************************************************************/
halReturn_t HalGpioSetUsbVbusPowerPin(bool value)
{
	halReturn_t 	halRet;

	halRet = HalInitCheck();
	if(halRet != HAL_RET_SUCCESS)
	{
		return halRet;
	}

	gpio_set_value(M2U_VBUS_CTRL_M, value);
	return HAL_RET_SUCCESS;
}



/*****************************************************************************/
/**
 * @brief Platform specific function to control Vbus power on the MHL port.
 *
 *****************************************************************************/
halReturn_t HalGpioSetVbusPowerPin(bool powerOn)
{
	halReturn_t 	halRet;

	halRet = HalInitCheck();
	if(halRet != HAL_RET_SUCCESS)
	{
		return halRet;
	}

	SII_DEBUG_PRINT(SII_OSAL_DEBUG_TRACE,
			"HalGpioSetVbusPowerPin called but this function is not implemented yet!\n");

	return HAL_RET_SUCCESS;
}
