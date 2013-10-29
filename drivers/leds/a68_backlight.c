/*
 * leds-msm-pmic.c - MSM PMIC LEDs driver.
 *
 * Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/platform_device.h>
#include <mach/pmic.h>
#include <linux/pwm.h>
#include <linux/mfd/pm8xxx/pm8921.h>
#include <linux/time.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <mach/board.h>
#include <linux/leds.h>
#include <mach/gpio.h>
#include <mach/irqs.h>
#include <linux/microp_notify.h>
#include <linux/microp_notifier_controller.h>	//ASUS_BSP Lenter+
#include "../video/msm/mipi_novatek.h"
#include <linux/microp.h>
#include <linux/microp_api.h>
#include <linux/microp_pin_def.h>

#define A68_PWM_FREQ_HZ 22000L
#define A68_PWM_PERIOD_USEC (USEC_PER_SEC / A68_PWM_FREQ_HZ)
#define A68_PWM_LEVEL 255L
#define A68_PWM_DUTY_LEVEL \
    (A68_PWM_PERIOD_USEC * 2550L / A68_PWM_LEVEL)
#define A80_PWM_FREQ_HZ 3        //195 , 9375 , 12500 , 18750
#define PM8XXX_LPG_CTL_REGS		7
#define MAX_BACKLIGHT_BRIGHTNESS 255
#define MIN_BACKLIGHT_BRIGHTNESS 0
#define PM8921_GPIO_BASE        NR_GPIO_IRQS
#define PM8921_GPIO_PM_TO_SYS(pm_gpio)  (pm_gpio - 1 + PM8921_GPIO_BASE)

DECLARE_COMPLETION(brightness_comp);

static int a68_set_backlight(int);
int pad_set_backlight(int);
static int a80_mipi_set_backlight(int);
int a80_set_pmic_backlight(int);
static struct a68_backlight_data *backlight_pdata;
static struct pwm_device *bl_lpm;
static int backlight_value = 90;
static struct workqueue_struct *backlight_workqueue;
static struct delayed_work turn_off_panel_work;
static bool bl_busy = FALSE;
int backlight_mode_state = 0;
static int a80_pmic_set_pwm = 0;
int pwm_frequency[] = {195,9375,12500,18750};

extern int AX_MicroP_setPWMValue(uint8_t);

enum backlight_mode {
    phone = 0,
    pad,
};


//ASUS_BSP jacob kung: add for debug mask ++
#include <linux/module.h>
/* Debug levels */
#define NO_DEBUG       0
#define DEBUG_POWER     1
#define DEBUG_INFO  2
#define DEBUG_VERBOSE 5
#define DEBUG_RAW      8
#define DEBUG_TRACE   10

static int debug = DEBUG_INFO;

module_param(debug, int, 0644);

MODULE_PARM_DESC(debug, "Activate debugging output");

#define backlight_debug(level, ...) \
		if (debug >= (level)) \
			pr_info(__VA_ARGS__);
//ASUS_BSP jacob kung: add for debug mask --


struct  pm8xxx_pwm_period define_pwm_parameter[] = {
		{PM_PWM_SIZE_9BIT,PM_PWM_CLK_19P2MHZ,PM_PWM_PDIV_6,5},//195Hz
		{PM_PWM_SIZE_9BIT,PM_PWM_CLK_19P2MHZ,PM_PWM_PDIV_2,1},//9.375KHz
		{PM_PWM_SIZE_9BIT,PM_PWM_CLK_19P2MHZ,PM_PWM_PDIV_3,0},//12.5KHz
		{PM_PWM_SIZE_9BIT,PM_PWM_CLK_19P2MHZ,PM_PWM_PDIV_2,0},//18.75 KHz
	};

struct pwm_device {
	int			pwm_id;		/* = bank/channel id */
	int			in_use;
	const char		*label;
	struct pm8xxx_pwm_period	period;
	int			pwm_value;
	int			pwm_period;
	int			pwm_duty;
	u8			pwm_lpg_ctl[PM8XXX_LPG_CTL_REGS];
	u8			pwm_ctl1;
	u8			pwm_ctl2;
	int			irq;
	struct pm8xxx_pwm_chip	*chip;
	int			bypass_lut;
	int			dtest_mode_supported;
};
enum brightness_mode {
    NORMAL = 0,
    AUTO,
    OUTDOOR,
};

static int new_level=0, old_level=0;
void cabl_bl_scale(int cabl_scale, int bl_min_lvl, int mode)
{
    int index;

	if (backlight_value == 0 || backlight_value ==1000 || backlight_value == 2000) 
		return;

    if (backlight_value >= bl_min_lvl) 
    {
        switch (mode) {
            case NORMAL:
                index = 0;
                new_level = backlight_value * cabl_scale / 1024;
                if (new_level < index + bl_min_lvl) {
                    new_level = 30;
                }
            break;

            case OUTDOOR:
                index = 1000;
                new_level = (((backlight_value - index) * cabl_scale / 1024 )-64) * 4/3
                            + index;
                if (new_level < index + bl_min_lvl) {
                    new_level = 1030;
                }
            break;

            case AUTO:
                index = 2000;
                new_level = (backlight_value - index) * cabl_scale / 1024 + index;
                if (new_level < index + bl_min_lvl) {
                    new_level = 2030;
                }
            break;

            default:
                return;
        	}

        if (new_level == old_level || new_level == backlight_value) {
            return;
        }

        old_level = new_level;

        backlight_debug(DEBUG_VERBOSE,"[BL][cabl] mode(%d), scale(%d), new_bl(%d), orig_bl(%d)\n",mode ,cabl_scale , new_level, backlight_value);

        if (bl_busy) {
            wait_for_completion_timeout(&brightness_comp, msecs_to_jiffies(500));
        }

		if (g_A68_hwID <= A80_SR1)
			{	
        		a80_mipi_set_backlight(new_level);
			}
		else if ((A80_SR2 <= g_A68_hwID) && (g_A68_hwID <= A80_SR4) )
			{
				a80_set_pmic_backlight(new_level);
			}
		else 
			{
				a80_mipi_set_backlight(new_level);
			}
    	}
}
EXPORT_SYMBOL(cabl_bl_scale);

int a80_set_pmic_backlight(value)
{
	int ret = 0;
	int index =  0;
	struct pm8xxx_pwm_period *pwm_period;
	static bool bl_en = FALSE;
	bl_busy = TRUE;

	if (bl_lpm) 
		{
		if (!a80_pmic_set_pwm)
			{
			backlight_debug(DEBUG_INFO,"[BL]Set pmic PWM frequency+++\n");
			pwm_period = &bl_lpm->period;
			pwm_period->pwm_size = define_pwm_parameter[A80_PWM_FREQ_HZ].pwm_size;
			pwm_period->clk = define_pwm_parameter[A80_PWM_FREQ_HZ].clk;
			pwm_period->pre_div = define_pwm_parameter[A80_PWM_FREQ_HZ].pre_div;
			pwm_period->pre_div_exp = define_pwm_parameter[A80_PWM_FREQ_HZ].pre_div_exp;
			bl_lpm->pwm_period = pwm_frequency[A80_PWM_FREQ_HZ];
			
			ret = pm8xxx_pwm_config_period(bl_lpm,pwm_period);
			if (ret) 
				{
				pr_err("pwm_config on config_period failed %d\n", ret);
               			 return ret;
				}
			
			backlight_debug(DEBUG_INFO,"[BL]Set pmic PWM frequency---\n");
			a80_pmic_set_pwm = 1;
			}
//=========================================================================
		backlight_debug(DEBUG_VERBOSE,"[BL]Set pmic backlight value == %d\n",value);
		 if(value >= 2000 && value <= 2255) 
		 	{
       			value -= 2000;
   		 	}
#ifdef ASUS_FACTORY_BUILD  
		index = ((value - 20) * 533)/255 + 20;
            	//20~255 mapping 21~511
            	if (index >= 511) {
                	index = 511;
            	}
            	else if (index <= 0) {
                	index = 0;
            	}
#else

		if (value <= 255) {  //normal mode
			index = ((value - 20) * 319)/ 280 + 21;  //TEMP chang 21~340 to 21~288 fot HW by monitor 20mA
			backlight_debug(DEBUG_VERBOSE,"[BL]Set pmic backlight normal index == %d\n",index);
                	//20~255 mapping 21~340; max: ? nits, min: ? nits, default: ? nits( ? %)

			 if (index >= 340) {
                    	index = 340;
                	}
                	else if (index <= 0) {
                    	index = 0;
                	}	
		}
		else if (value > 1000 && value <= 1255) //outdoor mode
        	{
                	index = ((value - 1020) * 381)/ 298 + 130; //TEMP chang 130~511 to 130~430 fot HW by monitor 20mA
			//1020~1255 mapping 130~511; max: ? nits, min: ? nits( ? %)
					backlight_debug(DEBUG_VERBOSE,"[BL]Set pmic backlight outdoor index == %d\n",index);
                	if (index >= 511) {
                		index = 511;
                	}
                	else if (index <= 130) {
                		index = 130;
                	}
		}
#endif
//=========================================================================
			backlight_debug(DEBUG_VERBOSE,"[BL]Set pmic PWM value == %d \n",index);
		if (index == 0){
				backlight_debug(DEBUG_VERBOSE,"[BL]BL_en off\n");
				gpio_set_value(2, 0);
				bl_en = FALSE;
			}
		else{
			if (!bl_en){
					backlight_debug(DEBUG_VERBOSE,"[BL]BL_en on\n");
					gpio_set_value(2, 1);
					bl_en = TRUE;
				}
			}
		
	        ret = pm8xxx_pwm_config_pwm_value(bl_lpm,index);
		if (ret) 
			{
			pr_err("pwm_config on lpm failed %d\n", ret);
			return ret;
			}
		if (index) 
			{
			ret = pwm_enable(bl_lpm);
			if (ret)
				pr_err("pwm enable/disable on lpm failed"
	                        "for bl =(%d)\n",  value);
			} 
		else{
			pwm_disable(bl_lpm);
	        }
		}
	complete(&brightness_comp);
    bl_busy = FALSE;
    return 0;
}


static int a80_mipi_set_backlight(value)
{
    int index = 0;

    if (value == 0 || value == 1000 || value == 2000) {
        printk("[BL] turn off A80 backlight\n");
        renesas_set_brightness(0);
        return 0;
    }

		backlight_debug(DEBUG_VERBOSE,"[BL]driver IC backlight set value == %d\n",value);
    bl_busy = TRUE;
	if (value == 19 )
		{
			index = 16;
			backlight_debug(DEBUG_VERBOSE,"[BL]driver IC backlight set index == %d\n",index);
		}
	else
		{
    		#ifdef ASUS_FACTORY_BUILD  
			 if (value >= 2000 && value <= 2255) {
			 	 if (value == 2020) {
						index = 17;				
			 		}
				 else if (value == 2021) {
						index = 21;				
			 		}
				 else if (value == 2022) {
						index = 29;				
			 		}
				 else {
				   index = ((value - 2020) * 235)/ 365 + 29;  
				   backlight_debug(DEBUG_VERBOSE,"[BL]Set driver IC backlight auto index == %d\n",index);
                		//20~255 mapping 29~180; max: 300 nits, min: 50 nits, default: ? nits( ? %)

			 	   if (index >= 180) {
                    				index = 180;
			 	   	}
                   		else if (index <= 29) {
                    				index = 29;
						backlight_debug(DEBUG_VERBOSE,"[BL]driver IC backlight normal index too low so set index == %d\n",index);
                   				}
				 	}
			 	}
			else  if (value <= 255) {
					index = ((value - 20) * 235)/272 + 17;
            			//20~255 mapping 29~220 ; max: 360 nits, min: 50 nits, default: ? nits( ? %)
            				if (index >= 220) {
	                				index = 220;
            					}
            				else if (index <= 17) {
		                			index = 17;
							backlight_debug(DEBUG_VERBOSE,"[BL]driver IC backlight index too low so set index == %d\n",index);
            					}
				}
			else 
				index = 58;
			#else
			 if (value >= 2000 && value <= 2255) {
			 	 if (value == 2020) {
						index = 17;				
			 		}
				 else if (value == 2021) {
						index = 21;				
			 		}
				 else if (value == 2022) {
						index = 29;				
			 		}
				 else {
					index = ((value - 2020) * 235)/ 365 + 29;  
					backlight_debug(DEBUG_VERBOSE,"[BL]Set driver IC backlight auto index == %d\n",index);
                	//20~255 mapping 29~180; max: 300 nits, min: 50 nits, default: ? nits( ? %)

			 		if (index >= 180) {
                    					index = 180;
			 			}
               				 else if (index <= 29) {
                    					index = 29;
							backlight_debug(DEBUG_VERBOSE,"[BL]driver IC backlight normal index too low so set index == %d\n",index);
               				 	}
				 	}
			 	}

			else if (value <= 255) {  //normal mode
					index = ((value - 20) * 235)/ 412 + 17;  
					backlight_debug(DEBUG_VERBOSE,"[BL]Set driver IC backlight normal index == %d\n",index);
                		//20~255 mapping 29~151; max: 250 nits, min: 50 nits, default: ? nits( ? %)

			 		if (index >= 151) {
                    					index = 151;
			 			}
                			else if (index <= 17) {
			                    		index = 17;
							backlight_debug(DEBUG_VERBOSE,"[BL]driver IC backlight normal index too low so set index == %d\n",index);
                				}	
				}
			else if (value > 1000 && value <= 1255) {    //outdoor mode
                			index = ((value - 1020) * 235)/ 340 + 58;
					//1020~1255 mapping 61~220; max: 360 nits, min: 100 nits( ? %)
					backlight_debug(DEBUG_VERBOSE,"[BL]Set pmic backlight outdoor index == %d\n",index);

					if (index >= 220) {
                					index = 220;
						}
                			else if (index <= 58) {	
		                			index = 58;
                				}
				}
			else
				index = 58;
#endif
		}
	
    renesas_set_brightness(index);

    complete(&brightness_comp);
    bl_busy = FALSE;

    return 0;
}

static int a68_set_backlight(value)
{
    int ret, duty_us;
    int index = 0;
    static bool CABC_On = true;

    if(value >= 2000 && value <= 2255) {
        value -= 2000;
    }

    if (g_A68_hwID >= A68_SR2)  //driver ic support
    {
        if (value == 0 || value == 1000) {
            backlight_debug(DEBUG_VERBOSE,"[BL] turn off A68 backlight\n");
            sharp_set_brightness(0);
            return 0;
        }

#ifdef ASUS_FACTORY_BUILD
        if (g_A68_hwID == A68_SR2) 
        {
            index = ((value - 20) * ((153*10000)/ 235)) / 10000 + 8;
            //factory: 20~550nits

            if (index >= 160) {
                index = 160;
            }
            else if (index <= 0) {
                index = 0;
            }
        }
        else    //for ER
        {
            index = ((value - 20) * ((245*10000)/ 235)) / 10000 + 11;
            //20~255 mapping 11~255
            if (index >= 255) {
                index = 255;
            }
            else if (index <= 0) {
                index = 0;
            }
        }
#else
        if (value <= 255)   //normal mode
        {
            if (g_A68_hwID == A68_SR2) 
            {
                index = ((value - 20) * ((80*10000)/ 235)) / 10000 + 8;
                //20~255 mapping 8~87; max:300nits, min:20nits (3%), default:100nits (12%)

                if (index >= 87) {
                    index = 87;
                }
                else if (index <= 0) {
                    index = 0;
                }
            }
            else 
            {
                index = ((value - 20) * ((125*10000)/ 235)) / 10000 + 11;
                //20~255 mapping 11~135; max:300nits, min:20nits, default:100nits(17%)

                if (index >= 135) {
                    index = 135;
                }
                else if (index <= 0) {
                    index = 0;
                }
            }
        }
        else if (value > 1000 && value <= 1255) //outdoor mode
        {
            if (g_A68_hwID == A68_SR2) 
            {
                index = ((value - 1020) * ((120*10000)/ 235)) / 10000 + 41;
                //1020~1255 mapping 41~160; max:550nits, min:140nits(16%)

                if (index >= 160) {
                    index = 160;
                }
                else if (index <= 0) {
                    index = 0;
                }
            }
            else
            {
                index = ((value - 1020) * ((192*10000)/ 235)) / 10000 + 64;
                //1020~1255 mapping 64~255; max:550nits, min:140nits(25%)

                if (index >= 255) {
                    index = 255;
                }
                else if (index <= 0) {
                    index = 0;
                }
            }
        }
#endif
// ++ cabc on/off
        if (index <= 28 && CABC_On == true)
        {
            sharp_set_cabc(0);
            CABC_On = false;
        }
        else if (index > 28 && CABC_On == false)
        {
            sharp_set_cabc(3);
            CABC_On = true;
        }
// -- cabc on/off

        sharp_set_brightness(index);
    }

    else  //pmic support
    {
        if (bl_lpm) 
        {
            if(value == MAX_BACKLIGHT_BRIGHTNESS) {
                duty_us = A68_PWM_PERIOD_USEC;
                ret = pwm_config(bl_lpm, 
                        duty_us, A68_PWM_PERIOD_USEC);
            }
            else {
                ret = pwm_config(bl_lpm, 
                        A68_PWM_DUTY_LEVEL * (long) value / 2550L, A68_PWM_PERIOD_USEC);
            }

            if (ret) {
                pr_err("pwm_config on lpm failed %d\n", ret);
                return ret;
            }

            if (value) {
                ret = pwm_enable(bl_lpm);
                if (ret)
                    pr_err("pwm enable/disable on lpm failed"
                        "for bl =(%d)\n",  value);
            } else {
                pwm_disable(bl_lpm);
            }
        }
    }
    return 0;
}

int pad_set_backlight(int value)
{
    int ret, index = 0;
	static int previous_value;
    if (value == 0 || value == 1000 || value == 2000) {
        backlight_debug(DEBUG_VERBOSE,"[BL] turn off P03 backlight\n");
        AX_MicroP_setPWMValue(0);
        queue_delayed_work(backlight_workqueue, &turn_off_panel_work, msecs_to_jiffies(3000));
        previous_value = 0;
        return 0;
    }

   	 if (value > 2000 && value <= 2255) {		 //auto mode 30~300nits
		index = ((value - 2020) * 235)/ 575 + 9;
		//20~255 mapping 9~105; max: 300 nits, min: 30 nits, default: 100 nits( ? %)

        	if (index >= 105) {
           		 	index = 105;
        		}
        	else if (index <= 9) {
          			 index = 9;
   		     }
   	 }
    	else if (value <= 255) {				//normal mode
        	index = ((value - 20) * 235)/ 695 + 9;
        //20~255 mapping 9~89; max:250 nits, min:30 nits (7%), default:100nits (33%)

       		 if (index >= 88) {
           			 index = 88;
        		}
        	else if (index <= 9) {
           		 index = 9;
       			 }
   	 }
   	else if (value > 1000 && value <= 1255) {		//outdoor mode
        	index = ((value - 1020) * 235)/ 557 + 33;
        //1020~1255 mapping 12~150; max:406 nits, min:37 nits (46%)
		 if (index >= 132) {
            			index = 132;
		 	}
        	else if (index <= 33) {
            			index = 33;
        		}
   	}
	else
		index = 33;
	backlight_debug(DEBUG_VERBOSE,"(%s): P03 set backlight %d \n", __func__,index);
    ret = AX_MicroP_setPWMValue(index);

    if (ret < 0) {
        backlight_debug(DEBUG_INFO,"(%s): P03 set backlight fail\n", __func__);
    }

    if ((previous_value == 0) && (index > 0))
    {
        if (delayed_work_pending(&turn_off_panel_work))
        {
            cancel_delayed_work_sync(&turn_off_panel_work);
        }
        AX_MicroP_setGPIOOutputPin(OUT_uP_LCD_EN,1);
        backlight_debug(DEBUG_VERBOSE,"[BL] (%s): P03 turn on panel\n", __func__);
    }
    previous_value = index;

    return ret;
}

static int lcd_backlight_registered;
void asus_set_bl_brightness(int value)
{
    static bool bFirst = true;
    int rc;
    struct pm_gpio gpio26_param = {
        .direction = PM_GPIO_DIR_OUT,
        .output_buffer = PM_GPIO_OUT_BUF_CMOS,
        .output_value = 0,
        .pull = PM_GPIO_PULL_NO,
        .vin_sel = 2,
        .out_strength = PM_GPIO_STRENGTH_HIGH,
        .function = PM_GPIO_FUNC_2,
        .inv_int_pol = 0,
        .disable_pin = 0,
    };

	backlight_debug(DEBUG_VERBOSE,"[BL] (%s):\n", __func__);

    if (value == 0 || value == 1000 || value == 2000) {
        old_level = 0;
    }

    if (g_A68_hwID < A68_SR2)
    {
        if(bFirst) 
        {
            rc = pm8xxx_gpio_config(PM8921_GPIO_PM_TO_SYS(26), &gpio26_param);
            if (rc) {
                pr_err("gpio_config 26 failed (2), rc=%d\n", rc);
                return;
            }
            bFirst = false;
        }
    }

    backlight_value = value;

    if (g_A68_hwID < A80_EVB) 
    {
        if (backlight_mode_state == phone) {
            a68_set_backlight(backlight_value);
        }
        else if (backlight_mode_state == pad) {
            pad_set_backlight(backlight_value);
        }
    }
    else if (g_A68_hwID >= A80_EVB && g_A68_hwID <= A80_SR1) 
    {
        if (backlight_mode_state == phone) {
            a80_mipi_set_backlight(backlight_value);
        }
        else if (backlight_mode_state == pad) {
			pad_set_backlight(backlight_value);
        }
    }
	else if (g_A68_hwID >= A80_SR5) 
    {
        if (backlight_mode_state == phone) {
            a80_mipi_set_backlight(backlight_value);
        }
        else if (backlight_mode_state == pad) {
			pad_set_backlight(backlight_value);
        }
    }
	else 
    {
    	if(bFirst) 
        {
            rc = pm8xxx_gpio_config(PM8921_GPIO_PM_TO_SYS(26), &gpio26_param);
            if (rc) {
                pr_err("gpio_config 26 failed (2), rc=%d\n", rc);
                return;
            }
            bFirst = false;
        }
        if (backlight_mode_state == phone) {
            a80_set_pmic_backlight(backlight_value);
        }
        else if (backlight_mode_state == pad) {
			pad_set_backlight(backlight_value);
        }
    }
}

static void set_p03_panel_off_func(struct work_struct *work)
{
    backlight_debug(DEBUG_VERBOSE,"[BL] (%s): P03 turn off panel\n", __func__);
    AX_MicroP_setGPIOOutputPin(OUT_uP_LCD_EN,0);
    AX_MicroP_setGPIOOutputPin(OUT_uP_PAD_LOW_BAT,0);
}

static struct led_classdev a68_backlight_led = {
    .name       = "lcd-backlight",
    .brightness = MAX_BACKLIGHT_BRIGHTNESS,
    //.brightness_set = set_bl_brightness,
};

static int __devinit a68_backlight_probe(struct platform_device *pdev)
{
    backlight_pdata = pdev->dev.platform_data;

    if (backlight_pdata == NULL) {
        pr_err("%s.invalid platform data.\n", __func__);
        return -ENODEV;
    }

    if (backlight_pdata != NULL) {
        bl_lpm = pwm_request(backlight_pdata->gpio[0],
            "backlight");
    }

    if (bl_lpm == NULL || IS_ERR(bl_lpm)) {
        pr_err("%s pwm_request() failed\n", __func__);
        bl_lpm = NULL;
    }
    pr_debug("bl_lpm = %p lpm = %d\n", bl_lpm,
        backlight_pdata->gpio[0]);

    if (0/*led_classdev_register(&pdev->dev, &a68_backlight_led)*/)
    {
        backlight_debug(DEBUG_VERBOSE, "led_classdev_register failed\n");            
    }
    else
    {
        lcd_backlight_registered = 1;
    }

	if ((g_A68_hwID >= A80_EVB && g_A68_hwID <= A80_SR1) || g_A68_hwID >= A80_SR5){
			backlight_debug(DEBUG_INFO,"[BL](%s): Driver IC device, set pmic off\n", __func__);
            gpio_set_value(PM8921_GPIO_PM_TO_SYS(26),0);
		}
    backlight_workqueue  = create_singlethread_workqueue("P03BACKLIGHTWORKQUEUE");
    INIT_DELAYED_WORK(&turn_off_panel_work, set_p03_panel_off_func);

    return 0;
}

static int a68_backlight_remove(struct platform_device *pdev)
{
    if (lcd_backlight_registered) {
        lcd_backlight_registered = 0;
        led_classdev_unregister(&a68_backlight_led);
    }
    return 0;
}
#if 1
static int change_backlight_mode(struct notifier_block *this, unsigned long event, void *ptr)
{
        switch (event) {
        case P01_ADD:
                backlight_mode_state = pad;
				backlight_debug(DEBUG_VERBOSE,"[BL][mod] %s change to Pad\n",__func__);
                //p03_set_backlight(backlight_value);   //plug-in/out set bl from led-class
                return NOTIFY_DONE;

        case P01_REMOVE:
                backlight_mode_state = phone;
				backlight_debug(DEBUG_VERBOSE,"[BL][mod] %s change to Phone\n",__func__);
                //a68_set_backlight(backlight_value);   //plug-in/out set bl from led-class
                return NOTIFY_DONE;

        default:
                return NOTIFY_DONE;
        }
}
#endif
#if 1
static struct notifier_block my_hs_notifier = {
        .notifier_call = change_backlight_mode,
        .priority = VIBRATOR_MP_NOTIFY,
};
#endif
static struct platform_driver this_driver = {
    .probe  = a68_backlight_probe,
    .remove = a68_backlight_remove,
    .driver = {
        .name   = "a68_backlight",
    },
};

static int __init msm_pmic_led_init(void)
{
    register_microp_notifier(&my_hs_notifier);
    notify_register_microp_notifier(&my_hs_notifier, "a68_backlight"); //ASUS_BSP Lenter+
    return platform_driver_register(&this_driver);
}
module_init(msm_pmic_led_init);

static void __exit msm_pmic_led_exit(void)
{
    platform_driver_unregister(&this_driver);
}
module_exit(msm_pmic_led_exit);

MODULE_DESCRIPTION("MSM PMIC8921 A68 backlight driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:board-8064");
