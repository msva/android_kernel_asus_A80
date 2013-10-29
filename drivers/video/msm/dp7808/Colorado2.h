#ifndef _COLORADO_H
#define _COLORADO_H
#include <linux/gpio.h>

/*
 * Below five GPIOs are example  for AP to control the Slimport chip ANX7805. 
 * Different AP needs to configure these control pins to corresponding GPIOs of AP.
 */

/*******************Slimport Control************************/
//ASUS BSP Wei_Lai +++
#define GPIO_CONFIG_DBG(gpio)         (MSM_TLMM_BASE + 0x1000 + (0x10 * (gpio)))
#define GPIO_IN_OUT_DBG(gpio)         (MSM_TLMM_BASE + 0x1004 + (0x10 * (gpio)))

#define PM8921_GPIO_BASE        NR_GPIO_IRQS
#define PM8921_GPIO_PM_TO_SYS(pm_gpio)  (pm_gpio - 1 + PM8921_GPIO_BASE)
//ASUS BSP Wei_Lai ---


#define SP_TX_PWR_V10_CTRL              (6)//AP IO Control - Power+V12
#define SP_TX_HW_RESET                      (43)//AP IO Control - Reset 
#define SLIMPORT_CABLE_DETECT         (55)//AP IO Input - Cable detect 
#define SP_TX_CHIP_PD_CTRL                PM8921_GPIO_PM_TO_SYS(13)//AP IO Control - CHIP_PW_HV 



#define SSC_EN
//#define SSC_1    
//#define HDCP_EN



#define AUX_ERR  1
#define AUX_OK   0



void Colorado2_work_func(struct work_struct * work);

unsigned char SP_TX_Write_Reg(unsigned char dev,unsigned char offset, unsigned char d);
unsigned char SP_TX_Read_Reg(unsigned char dev,unsigned char offset, unsigned char *d);

//ASUS BSP Wei_Lai +++
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


extern int pm8xxx_gpio_config(int gpio, struct pm_gpio *param);
//ASUS BSP Wei_Lai ---


#endif
