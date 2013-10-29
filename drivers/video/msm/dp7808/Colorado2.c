#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/spi/spi.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/ioport.h>
#include <linux/input-polldev.h>
#include <linux/i2c.h>
#include <linux/console.h>
#include <linux/fb.h>
#include <asm/types.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <sound/soc.h>
#include <mach/gpio.h>
#include "SP_TX_DRV.h"
#include "SP_TX_Reg.h"
#include "SP_TX_CTRL.h"
#include "Colorado2.h"


#undef NULL
#define NULL ((void *)0)


struct i2c_client *g_client;
int timer_en_cnt;
static struct workqueue_struct * Colorado2_workqueue;//define a workqueue
static struct delayed_work Colorado2_work;
//ASUS BSP Wei_Lai +++
extern enum DEVICE_HWID g_A68_hwID;
//ASUS BSP Wei_Lai ---

static const struct i2c_device_id Colorado2_register_id[] = {
     { "Colorado_i2c", 0 },
     { }
};

    struct pm_gpio gpio43_param = {
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

MODULE_DEVICE_TABLE(i2c, Colorado2_register_id);



static int Colorado2_init_gpio(void)
{
    int /*ret0,*/ ret1, ret2, ret3; //ASUS BSP Wei_Lai +++
      /* Default V12_CTRL low */
#if 0
	ret0 = gpio_request(SP_TX_PWR_V10_CTRL, "Colorado2_V10_Control"); 
	if(ret0)
	{
              printk("GPIO Colorado2_V10_Control request fail");
		goto err1; 
	}else
	{
		gpio_direction_output(SP_TX_PWR_V10_CTRL, 0);
	}
#endif
      /* Default SP_TX_HW_RESET low */
	ret1 = gpio_request(SP_TX_HW_RESET, "Colorado2_Reset_Control");
	if(ret1)
	{
	  	printk("GPIO Colorado2_Reset_Control request fail");
		goto err2;
	}else
	{
		gpio_direction_output(SP_TX_HW_RESET,0);
	}

	ret2 =  gpio_request(SLIMPORT_CABLE_DETECT,"Colorado2_Cable_Det");
	if(ret2)
	{
	      printk("GPIO Colorado2_Cable_Det request fail");
	       goto err3; 
	}else
	{
		gpio_direction_input(SLIMPORT_CABLE_DETECT); 
	}

       /* Default SP_TX_CHIP_PD_CTRL high */
	ret3 = gpio_request(SP_TX_CHIP_PD_CTRL, "Colorado2_CHIP_PD_Control");
         if (ret3) {
                 pr_err("%s: Failed to request gpio %d\n", __func__,
                         SP_TX_CHIP_PD_CTRL);
                 goto err4;
         }
	//ASUS BSP Wei_Lai +++
	ret3=pm8xxx_gpio_config(SP_TX_CHIP_PD_CTRL, &gpio43_param);
	if(ret3)
	{
	 printk("GPIO Colorado2_CHIP_PD_Control request fail");
		goto err4;
	}else
	{
		gpio_set_value(SP_TX_CHIP_PD_CTRL,1);
	}
	//ASUS BSP Wei_Lai ---
	return 0;
#if 0
	err1:
		gpio_free(SP_TX_PWR_V10_CTRL);
#endif
	err2:
		gpio_free(SP_TX_HW_RESET);
	err3:
		gpio_free(SLIMPORT_CABLE_DETECT);
	err4:
		gpio_free(SP_TX_CHIP_PD_CTRL);
		return -EINVAL;

}


static void Colorado2_free_gpio(void)
{

	gpio_free(SP_TX_PWR_V10_CTRL);
	gpio_free(SP_TX_HW_RESET);
	gpio_free(SLIMPORT_CABLE_DETECT);
	gpio_free(SP_TX_CHIP_PD_CTRL);
}



static int  Colorado2_System_Init (void)
{
    int ret;
return 0;						//ASUS BSP Wei_Lai for sr3 porting
    ret = SP_CTRL_Chip_Detect();
    if(ret<0)
    {
        printk("Chip detect error\n");
        return -ENODEV;
    }

    //Chip initialization
    SP_CTRL_Chip_Initial();
    return 0;

}

	
extern  struct regulator *devm_regulator_get(struct device *dev, const char *id);
extern int regulator_set_voltage(struct regulator *regulator, int min_uV, int max_uV);
extern int regulator_enable(struct regulator *regulator);


static int Colorado2_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int err = 0;
	//int ret=0;
	//struct regulator *colorado_v10;
	printk("##########Colorado_i2c_probe##############\n");

	memcpy(&g_client, &client, sizeof(client));	

      /*  colorado_v10=regulator_get(NULL,"8921_l12");
	 if (IS_ERR(colorado_v10)) {
		printk("unable to get colorado_v10\n");
		return PTR_ERR(colorado_v10);
	}
      ret = regulator_set_voltage(colorado_v10, 1200000, 1200000);
	if (ret) {
		printk("%s: unable to set the voltage for regulator "
			"colorado_v10\n", __func__);
		return ret;
	}
	ret = regulator_enable(colorado_v10);
	if (ret) {
		printk("%s: regulator_enable of colorado_v10 failed(%d)\n",
			__func__, ret);
		return ret;
	}	*/
	//msleep(1000);
	/*if (!i2c_check_functionality(client->adapter,
				     I2C_FUNC_SMBUS_I2C_BLOCK)) {
		dev_err(&client->dev, "i2c bus does not support the Colorado\n");
		err = -ENODEV;
		goto exit_kfree;
	}*/

	
	err = Colorado2_System_Init();
	if (err)
		goto exit_kfree;

exit_kfree:
	//kfree(data);
	return err;

}

static int Colorado2_i2c_remove(struct i2c_client *client)
{
	return 0;
}


static struct i2c_driver Colorado2_driver = {
     .driver = {
	  .name   = "Colorado_i2c",
	  .owner  = THIS_MODULE,
     },
     .probe            = Colorado2_i2c_probe,
     .remove         = Colorado2_i2c_remove,
     .id_table         = Colorado2_register_id,
};


void Colorado2_work_func(struct work_struct * work)
{

    printk("Colorado2_work_func ++++\n");
    if(timer_en_cnt > 0)
    {
        SP_CTRL_Main_Procss();
        timer_en_cnt--;
    }

    timer_en_cnt = 1;

	
    queue_delayed_work(Colorado2_workqueue, &Colorado2_work, msecs_to_jiffies(500));
    printk("Colorado2_work_func ---\n");
}


static int __init Colorado2_init(void)
{
	int ret0 =0,ret1=0;//,ret2=0,ret3=0; ASUS BSP Wei_Lai +++
//ASUS BSP Wei_Lai +++
	if(g_A68_hwID<A80_SR3)
		goto err0;
	printk("++++++++++++++++++++++++Colorado2_init  ++++++++++++++++\n");
//ASUS BSP Wei_Lai ---
        timer_en_cnt = 0;

	
	/* step 1 Request control GPIOs */
	ret0 = Colorado2_init_gpio();
	if(ret0){
		D("Colorado2_init_gpio fail");
		goto err0;
	}
	/* step 2 i2c_add_driver */
	ret1= i2c_add_driver(&Colorado2_driver);
	if (ret1){
		D("i2c_add_driver err!\n");
		goto err1;
	}

#if 0 //ASUS BSP Wei-Lai +++
	//Initial  the  work_func with the delayed work
	INIT_DELAYED_WORK(&Colorado2_work, Colorado2_work_func);

	/* Create a singlethread workqueue */
	Colorado2_workqueue = create_singlethread_workqueue("ANX7805_WORKQUEUE");

	if (Colorado2_workqueue == NULL)
	{
		printk("Colorado: Failed to creat work queue.\n");
		ret2 = -ENOMEM;
		goto err2;
	}

	ret3=queue_delayed_work(Colorado2_workqueue, &Colorado2_work, 0);
	if (ret3 == 0)
	{
		printk("Colorado_work was already on a queue.\n");
	}
#endif
	//ASUS BSP Wei_Lai +++
	printk("------------SP_TX init -----------------------------\n");
	//ASUS BSP Wei_Lai ---
	return 0;
#if 0 //ASUS BSP Wei-Lai +++
err2:
	destroy_workqueue(Colorado2_workqueue);
	return ret2;
#endif

err1:
	i2c_del_driver(&Colorado2_driver);
	return ret1;

err0:
	return ret0;
	

}



static void __exit Colorado2_exit(void)
{

	Colorado2_free_gpio();
	destroy_workqueue(Colorado2_workqueue);
	i2c_del_driver(&Colorado2_driver);
}




int Colorado2_send_msg(unsigned char addr, unsigned char *buf, unsigned short len, unsigned short rw_flag)
{
	int rc;
	g_client-> addr=addr;
	if(rw_flag)
	{

		rc=i2c_smbus_read_byte_data(g_client, *buf);
		
		*buf = rc;

	} else
	{
		rc=i2c_smbus_write_byte_data(g_client, buf[0], buf[1]);
	}
		
	return 0;
}

unsigned char SP_TX_Read_Reg(unsigned char dev,unsigned char offset, unsigned char *d)
{
	unsigned char c;
	int ret;
	c = offset;

	ret = Colorado2_send_msg(dev >> 1, &c, 1, 1);
	if(ret < 0){
		printk("Colorado_send_msg err!\n");
		return 1;
	}

	*d = c;
	return 0;
}


unsigned char SP_TX_Write_Reg(unsigned char dev,unsigned char offset, unsigned char d)
{
	unsigned char buf[2] = {offset, d};

	return Colorado2_send_msg(dev >> 1, buf, 2, 0);
}




module_init(Colorado2_init);
module_exit(Colorado2_exit);

MODULE_DESCRIPTION ("Slimport  transmitter ANX7808 driver");
MODULE_AUTHOR("FeiWang<fwang@analogixsemi.com>");
MODULE_LICENSE("GPL");







