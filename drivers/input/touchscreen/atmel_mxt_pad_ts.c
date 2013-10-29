/*
 * Atmel maXTouch Touchscreen driver
 *
 * Copyright (C) 2010 Samsung Electronics Co.Ltd
 * Copyright (C) 2011 Atmel Corporation
 * Author: Joonyoung Shim <jy0922.shim@samsung.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/i2c.h>
#include <linux/i2c/atmel_mxt_pad_ts.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

//Desmond++
#ifdef CONFIG_EEPROM_NUVOTON 
#include <linux/microp_api.h> 
#include <linux/microp_pin_def.h>
#include <linux/microp_notify.h>
#include <linux/microp_notifier_controller.h>	//ASUS_BSP Lenter+
#endif //CONFIG_EEPROM_NUVOTON
#include <linux/gpio.h>
//Desmond--

/* Version */
#define MXT_VER_20		20
#define MXT_VER_21		21
#define MXT_VER_22		22

/* Firmware files */
#define MXT_FW_NAME		"maxtouch.fw"
#define MXT_CFG_NAME		"maxtouch.cfg"
#define MXT_CFG_MAGIC		"OBP_RAW V1"

/* Registers */
#define MXT_FAMILY_ID		0x00
#define MXT_VARIANT_ID		0x01
#define MXT_VERSION		0x02
#define MXT_BUILD		0x03
#define MXT_MATRIX_X_SIZE	0x04
#define MXT_MATRIX_Y_SIZE	0x05
#define MXT_OBJECT_NUM		0x06
#define MXT_OBJECT_START	0x07

#define MXT_OBJECT_SIZE		6

#define MXT_MAX_BLOCK_WRITE	256

/* Object types */
#define MXT_DEBUG_DIAGNOSTIC_T37	37
#define MXT_GEN_MESSAGE_T5		5
#define MXT_GEN_COMMAND_T6		6
#define MXT_GEN_POWER_T7		7
#define MXT_GEN_ACQUIRE_T8		8
#define MXT_GEN_DATASOURCE_T53		53
#define MXT_TOUCH_MULTI_T9		9
#define MXT_TOUCH_KEYARRAY_T15		15
#define MXT_TOUCH_PROXIMITY_T23		23
#define MXT_TOUCH_PROXKEY_T52		52
#define MXT_PROCI_GRIPFACE_T20		20
#define MXT_PROCG_NOISE_T22		22
#define MXT_PROCI_ACTIVE_STYLUS_T63	63
#define MXT_PROCI_ONETOUCH_T24		24
#define MXT_PROCI_TWOTOUCH_T27		27
#define MXT_PROCI_GRIP_T40		40
#define MXT_PROCI_PALM_T41		41
#define MXT_PROCI_TOUCHSUPPRESSION_T42	42
#define MXT_PROCI_STYLUS_T47		47
#define MXT_PROCG_NOISESUPPRESSION_T48	48
#define MXT_SPT_COMMSCONFIG_T18		18
#define MXT_SPT_GPIOPWM_T19		19
#define MXT_SPT_SELFTEST_T25		25
#define MXT_SPT_CTECONFIG_T28		28
#define MXT_SPT_USERDATA_T38		38
#define MXT_SPT_DIGITIZER_T43		43
#define MXT_SPT_MESSAGECOUNT_T44	44
#define MXT_SPT_CTECONFIG_T46		46
#define MXT_SPT_NOISESUPPRESSION_T48    48
#define MXT_PROCI_ADAPTIVETHRESHOLD_T55	55
#define MXT_PROCI_SHIELDLESS_T56	56
#define MXT_PROCI_EXTRATOUCHDATA_T57	57
/* MXT_GEN_MESSAGE_T5 object */
#define MXT_RPTID_NOMSG		0xff

/* MXT_GEN_COMMAND_T6 field */
#define MXT_COMMAND_RESET	0
#define MXT_COMMAND_BACKUPNV	1
#define MXT_COMMAND_CALIBRATE	2
#define MXT_COMMAND_REPORTALL	3
#define MXT_COMMAND_DIAGNOSTIC	5

/* MXT_GEN_POWER_T7 field */
#define MXT_POWER_IDLEACQINT	0
#define MXT_POWER_ACTVACQINT	1
#define MXT_POWER_ACTV2IDLETO	2

#define MXT_POWER_CFG_RUN		0
#define MXT_POWER_CFG_DEEPSLEEP		1

/* MXT_GEN_ACQUIRE_T8 field */
#define MXT_ACQUIRE_CHRGTIME	0
#define MXT_ACQUIRE_TCHDRIFT	2
#define MXT_ACQUIRE_DRIFTST	3
#define MXT_ACQUIRE_TCHAUTOCAL	4
#define MXT_ACQUIRE_SYNC	5
#define MXT_ACQUIRE_ATCHCALST	6
#define MXT_ACQUIRE_ATCHCALSTHR	7

/* MXT_TOUCH_MULTI_T9 field */
#define MXT_TOUCH_CTRL		0
#define MXT_TOUCH_XORIGIN	1
#define MXT_TOUCH_YORIGIN	2
#define MXT_TOUCH_XSIZE		3
#define MXT_TOUCH_YSIZE		4
#define MXT_TOUCH_BLEN		6
#define MXT_TOUCH_TCHTHR	7
#define MXT_TOUCH_TCHDI		8
#define MXT_TOUCH_ORIENT	9
#define MXT_TOUCH_MOVHYSTI	11
#define MXT_TOUCH_MOVHYSTN	12
#define MXT_TOUCH_NUMTOUCH	14
#define MXT_TOUCH_MRGHYST	15
#define MXT_TOUCH_MRGTHR	16
#define MXT_TOUCH_AMPHYST	17
#define MXT_TOUCH_XRANGE_LSB	18
#define MXT_TOUCH_XRANGE_MSB	19
#define MXT_TOUCH_YRANGE_LSB	20
#define MXT_TOUCH_YRANGE_MSB	21
#define MXT_TOUCH_XLOCLIP	22
#define MXT_TOUCH_XHICLIP	23
#define MXT_TOUCH_YLOCLIP	24
#define MXT_TOUCH_YHICLIP	25
#define MXT_TOUCH_XEDGECTRL	26
#define MXT_TOUCH_XEDGEDIST	27
#define MXT_TOUCH_YEDGECTRL	28
#define MXT_TOUCH_YEDGEDIST	29
#define MXT_TOUCH_JUMPLIMIT	30

/* MXT_PROCI_GRIPFACE_T20 field */
#define MXT_GRIPFACE_CTRL	0
#define MXT_GRIPFACE_XLOGRIP	1
#define MXT_GRIPFACE_XHIGRIP	2
#define MXT_GRIPFACE_YLOGRIP	3
#define MXT_GRIPFACE_YHIGRIP	4
#define MXT_GRIPFACE_MAXTCHS	5
#define MXT_GRIPFACE_SZTHR1	7
#define MXT_GRIPFACE_SZTHR2	8
#define MXT_GRIPFACE_SHPTHR1	9
#define MXT_GRIPFACE_SHPTHR2	10
#define MXT_GRIPFACE_SUPEXTTO	11

/* MXT_PROCI_NOISE field */
#define MXT_NOISE_CTRL		0
#define MXT_NOISE_OUTFLEN	1
#define MXT_NOISE_GCAFUL_LSB	3
#define MXT_NOISE_GCAFUL_MSB	4
#define MXT_NOISE_GCAFLL_LSB	5
#define MXT_NOISE_GCAFLL_MSB	6
#define MXT_NOISE_ACTVGCAFVALID	7
#define MXT_NOISE_NOISETHR	8
#define MXT_NOISE_FREQHOPSCALE	10
#define MXT_NOISE_FREQ0		11
#define MXT_NOISE_FREQ1		12
#define MXT_NOISE_FREQ2		13
#define MXT_NOISE_FREQ3		14
#define MXT_NOISE_FREQ4		15
#define MXT_NOISE_IDLEGCAFVALID	16

/* MXT_SPT_COMMSCONFIG_T18 */
#define MXT_COMMS_CTRL		0
#define MXT_COMMS_CMD		1

/* MXT_SPT_CTECONFIG_T28 field */
#define MXT_CTE_CTRL		0
#define MXT_CTE_CMD		1
#define MXT_CTE_MODE		2
#define MXT_CTE_IDLEGCAFDEPTH	3
#define MXT_CTE_ACTVGCAFDEPTH	4
#define MXT_CTE_VOLTAGE		5

#define MXT_VOLTAGE_DEFAULT	2700000
#define MXT_VOLTAGE_STEP	10000

/* Define for MXT_GEN_COMMAND_T6 */
#define MXT_BOOT_VALUE		0xa5
#define MXT_RESET_VALUE		0x01
#define MXT_BACKUP_VALUE	0x55

/* Define for MXT_PROCG_NOISESUPPRESSION_T42 */
#define MXT_T42_MSG_TCHSUP	(1 << 0)

/* Delay times */
#define MXT_BACKUP_TIME		50	/* msec */  //Desmond++ 25 -> 50
#define MXT_RESET_TIME		200	/* msec */
#define MXT_RESET_NOCHGREAD	400	/* msec */
#define MXT_FWRESET_TIME	1000	/* msec */
#define MXT_WAKEUP_TIME		25	/* msec */

/* Command to unlock bootloader */
#define MXT_UNLOCK_CMD_MSB	0xaa
#define MXT_UNLOCK_CMD_LSB	0xdc

/* Bootloader mode status */
#define MXT_WAITING_BOOTLOAD_CMD	0xc0	/* valid 7 6 bit only */
#define MXT_WAITING_FRAME_DATA	0x80	/* valid 7 6 bit only */
#define MXT_FRAME_CRC_CHECK	0x02
#define MXT_FRAME_CRC_FAIL	0x03
#define MXT_FRAME_CRC_PASS	0x04
#define MXT_APP_CRC_FAIL	0x40	/* valid 7 8 bit only */
#define MXT_BOOT_STATUS_MASK	0x3f
#define MXT_BOOT_EXTENDED_ID	(1 << 5)
#define MXT_BOOT_ID_MASK	0x1f

/* Define for T6 status byte */
#define MXT_STATUS_RESET	(1 << 7)
#define MXT_STATUS_OFL		(1 << 6)
#define MXT_STATUS_SIGERR	(1 << 5)
#define MXT_STATUS_CAL		(1 << 4)
#define MXT_STATUS_CFGERR	(1 << 3)
#define MXT_STATUS_COMSERR	(1 << 2)

/* T9 Touch status */
#define MXT_T9_UNGRIP		(1 << 0)
#define MXT_T9_SUPPRESS		(1 << 1)
#define MXT_T9_AMP		(1 << 2)
#define MXT_T9_VECTOR		(1 << 3)
#define MXT_T9_MOVE		(1 << 4)
#define MXT_T9_RELEASE		(1 << 5)
#define MXT_T9_PRESS		(1 << 6)
#define MXT_T9_DETECT		(1 << 7)

/* Touch orient bits */
#define MXT_XY_SWITCH		(1 << 0)
#define MXT_X_INVERT		(1 << 1)
#define MXT_Y_INVERT		(1 << 2)

/* T63 Stylus */
#define MXT_STYLUS_PRESS	(1 << 0)
#define MXT_STYLUS_RELEASE	(1 << 1)
#define MXT_STYLUS_MOVE		(1 << 2)
#define MXT_STYLUS_SUPPRESS	(1 << 3)

#define MXT_STYLUS_DETECT	(1 << 4)
#define MXT_STYLUS_TIP		(1 << 5)
#define MXT_STYLUS_ERASER	(1 << 6)
#define MXT_STYLUS_BARREL	(1 << 7)

#define MXT_STYLUS_PRESSURE_MASK	0x3F

//Desmond ++ debug macro
/* Debug levels*/
#define ATMEL_PAD_DEBUG_ERR      0
#define ATMEL_PAD_DEBUG_POWER    1
#define ATMEL_PAD_DEBUG_INFO     2
#define ATMEL_PAD_DEBUG_INTR     3
#define ATMEL_PAD_DEBUG_VERBOSE  5
#define ATMEL_PAD_DEBUG_MESSAGES 6
#define ATMEL_PAD_DEBUG_RAW      8
#define ATMEL_PAD_DEBUG_TRACE   10

#define mxt_debug(level, ...) \
	do { \
		if (debug >= (level)) \
			pr_info(__VA_ARGS__); \
	} while (0) 
//Desmond --

//Desmond ++
int g_isPadMode = 0;
int g_isIrqRequest = 0;
int g_isPadSleep = 0;
struct mxt_data *data_bak = 0;
static struct workqueue_struct *g_sis_wq_attach_detach;
#ifdef CONFIG_EEPROM_NUVOTON
static struct work_struct g_mp_attach_work;
static struct work_struct g_mp_detach_work;
static void attach_padstation_work(struct work_struct *work);
static void detach_padstation_work(struct work_struct *work);
#endif //CONFIG_EEPROM_NUVOTON
//Desmond --

//Desmond ++ Add Module Parameter
static int debug = ATMEL_PAD_DEBUG_POWER;
//static int debug = ATMEL_PAD_DEBUG_VERBOSE;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Activate debugging output");

static int gap_time_1 = 200;
module_param(gap_time_1, int, 0644);
MODULE_PARM_DESC(gap_time_1, "tune timing");
static int gap_time_2 = 200;
module_param(gap_time_2, int, 0644);
MODULE_PARM_DESC(gap_time_2, "tune timing2");
//Desmond --

/* T15 Key array */
int mxt_t15_keys[] = { };

static unsigned long mxt_t15_keystatus;

/* Touchscreen absolute values */
#define MXT_MAX_AREA		0xff

struct mxt_info {
	u8 family_id;
	u8 variant_id;
	u8 version;
	u8 build;
	u8 matrix_xsize;
	u8 matrix_ysize;
	u8 object_num;
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mxt_early_suspend(struct early_suspend *es);
static void mxt_late_resume(struct early_suspend *es);
#endif

struct mxt_object {
	u8 type;
	u16 start_address;
	u16 size;
	u16 instances;
	u8 num_report_ids;

	/* to map object and message */
	u8 min_reportid;
	u8 max_reportid;
};

enum mxt_device_state { INIT, APPMODE, BOOTLOADER, FAILED, SHUTDOWN };

/* Each client has this additional data */
struct mxt_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	const struct mxt_platform_data *pdata;
	enum mxt_device_state state;
	struct mxt_object *object_table;
	u16 mem_size;
	struct mxt_info info;
	unsigned int irq;
	unsigned int max_x;
	unsigned int max_y;
	struct bin_attribute mem_access_attr;
	bool debug_enabled;
	bool driver_paused;
	u8 bootloader_addr;
	u8 actv_cycle_time;
	u8 idle_cycle_time;
	u8 is_stopped;
	u8 max_reportid;
	u32 config_crc;
	u32 info_block_crc;
	u8 num_touchids;
	u8 num_stylusids;
	u8 *msg_buf;
	u8 last_message_count;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif

	/* Cached parameters from object table */
	u16 T5_address;
	u8 T5_msg_size;
	u8 T6_reportid;
	u16 T7_address;
	u8 T9_reportid_min;
	u8 T9_reportid_max;
	u8 T15_reportid_min;
	u8 T15_reportid_max;
	u8 T42_reportid_min;
	u8 T42_reportid_max;
	u16 T44_address;
	u8 T48_reportid;
	u8 T63_reportid_min;
	u8 T63_reportid_max;
};

/* I2C slave address pairs */
struct mxt_i2c_address_pair {
	u8 bootloader;
	u8 application;
};

static const struct mxt_i2c_address_pair mxt_i2c_addresses[] = {
//#ifdef BOOTLOADER_1664_1188
	{ 0x26, 0x4a },
	{ 0x27, 0x4b },
/*#else desmond++
	{ 0x24, 0x4a },
	{ 0x25, 0x4b },
	{ 0x26, 0x4c },
	{ 0x27, 0x4d },
	{ 0x34, 0x5a },
	{ 0x35, 0x5b },
#endif*/
};

static int mxt_bootloader_read(struct mxt_data *data, u8 *val, unsigned int count)
{
	int ret;
	struct i2c_msg msg;

	msg.addr = data->bootloader_addr;
	msg.flags = data->client->flags & I2C_M_TEN;
	msg.flags |= I2C_M_RD;
	msg.len = count;
	msg.buf = val;

	ret = i2c_transfer(data->client->adapter, &msg, 1);

	return (ret == 1) ? 0 : ret;
}

static int mxt_bootloader_write(struct mxt_data *data, const u8 * const val,
	unsigned int count)
{
	int ret;
	struct i2c_msg msg;

	msg.addr = data->bootloader_addr;
	msg.flags = data->client->flags & I2C_M_TEN;
	msg.len = count;
	msg.buf = (u8 *)val;

	ret = i2c_transfer(data->client->adapter, &msg, 1);

	return (ret == 1) ? 0 : ret;
}

static int mxt_get_bootloader_address(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	int i;

	for (i = 0; i < ARRAY_SIZE(mxt_i2c_addresses); i++) {
		if (mxt_i2c_addresses[i].application == client->addr) {
			data->bootloader_addr = mxt_i2c_addresses[i].bootloader;

			dev_info(&client->dev, "Bootloader i2c addr: 0x%02x\n",
				data->bootloader_addr);

			return 0;
		}
	}

	dev_err(&client->dev, "Address 0x%02x not found in address table\n",
		client->addr);
	return -EINVAL;
}

static int mxt_probe_bootloader(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
	int ret;
	u8 val;
	bool crc_failure;

	ret = mxt_get_bootloader_address(data);
	if (ret)
		return ret;

	ret = mxt_bootloader_read(data, &val, 1);
	if (ret) {
		dev_err(dev, "%s: i2c recv failed\n", __func__);
		return -EIO;
	}

	/* Check app crc fail mode */
	crc_failure = (val & ~MXT_BOOT_STATUS_MASK) == MXT_APP_CRC_FAIL;

	dev_err(dev, "Detected bootloader, status:%02X%s\n",
		val, crc_failure ? ", APP_CRC_FAIL" : "");

	return 0;
}

static int mxt_wait_for_chg(struct mxt_data *data)
{
	int timeout_counter = 0;
	int count = 1E6;

	if (data->pdata->read_chg == NULL) {
		msleep(10);
		return 0;
	}

	while ((timeout_counter++ <= count) && data->pdata->read_chg())
		udelay(20);

	if (timeout_counter > count) {
		dev_err(&data->client->dev, "mxt_wait_for_chg() timeout!\n");
		return -EIO;
	}

	return 0;
}

static u8 mxt_get_bootloader_version(struct mxt_data *data, u8 val)
{
	struct device *dev = &data->client->dev;
	u8 buf[3];

	if (val & MXT_BOOT_EXTENDED_ID) {
		if (mxt_bootloader_read(data, &buf[0], 3) != 0) {
			dev_err(dev, "%s: i2c failure\n", __func__);
			return -EIO;
		}

		dev_info(dev, "Bootloader ID:%d Version:%d\n", buf[1], buf[2]);

		return buf[0];
	} else {
		dev_info(dev, "Bootloader ID:%d\n", val & MXT_BOOT_ID_MASK);

		return val;
	}
}

static int mxt_check_bootloader(struct mxt_data *data,
				unsigned int state)
{
	struct device *dev = &data->client->dev;
	int ret;
	u8 val;

recheck:
	ret = mxt_bootloader_read(data, &val, 1);
	if (ret) {
		dev_err(dev, "%s: i2c recv failed, ret=%d\n",
			__func__, ret);
		return ret;
	}

	if (state == MXT_WAITING_BOOTLOAD_CMD) {
		val = mxt_get_bootloader_version(data, val);
	}

	switch (state) {
	case MXT_WAITING_BOOTLOAD_CMD:
		val &= ~MXT_BOOT_STATUS_MASK;
		break;
	case MXT_WAITING_FRAME_DATA:
	case MXT_APP_CRC_FAIL:
		val &= ~MXT_BOOT_STATUS_MASK;
		break;
	case MXT_FRAME_CRC_PASS:
		if (val == MXT_FRAME_CRC_CHECK) {
			mxt_wait_for_chg(data);
			goto recheck;
		} else if (val == MXT_FRAME_CRC_FAIL) {
			dev_err(dev, "Bootloader CRC fail\n");
			return -EINVAL;
		}
		break;
	default:
		return -EINVAL;
	}

	if (val != state) {
		dev_err(dev, "Invalid bootloader mode state 0x%02X\n", val);
		return -EINVAL;
	}

	return 0;
}

static int mxt_send_bootloader_cmd(struct mxt_data *data, bool unlock)
{
	int ret;
	u8 buf[2];

	if (unlock) {
		buf[0] = MXT_UNLOCK_CMD_LSB;
		buf[1] = MXT_UNLOCK_CMD_MSB;
	} else {
		buf[0] = 0x01;
		buf[1] = 0x01;
	}

	ret = mxt_bootloader_write(data, buf, 2);
	if (ret) {
		dev_err(&data->client->dev, "%s: i2c send failed, ret=%d\n",
				__func__, ret);
		return ret;
	}

	return 0;
}

static int mxt_read_reg(struct i2c_client *client,
			u16 reg, u16 len, void *val)
{
	struct device *dev = &client->dev;
	struct i2c_msg xfer[2];
	u8 buf[2];
	int ret;
	u8 retry = 0;

	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8) & 0xff;

	/* Write register */
	xfer[0].addr = client->addr;
	xfer[0].flags = 0;
	xfer[0].len = 2;
	xfer[0].buf = buf;

	/* Read data */
	xfer[1].addr = client->addr;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = len;
	xfer[1].buf = val;

retry_read:
	ret = i2c_transfer(client->adapter, xfer, ARRAY_SIZE(xfer));
	if (ret != ARRAY_SIZE(xfer)) {
		if (!retry) {
			dev_dbg(dev, "%s: i2c retry\n", __func__);
			msleep(MXT_WAKEUP_TIME);
			retry = 1;
			goto retry_read;
		} else {
			dev_err(dev, "%s: i2c transfer failed (%d)\n",
				__func__, ret);
			return -EIO;
		}
	}

	return 0;
}

static int mxt_write_reg(struct i2c_client *client, u16 reg, u8 val)
{
	struct device *dev = &client->dev;
	u8 retry = 0;
	u8 buf[3];

	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8) & 0xff;
	buf[2] = val;

retry_write:
	if (i2c_master_send(client, buf, 3) != 3) {
		if (!retry) {
			dev_dbg(dev, "%s: i2c retry\n", __func__);
			msleep(MXT_WAKEUP_TIME);
			retry = 1;
			goto retry_write;
		} else {
			dev_err(dev, "%s: i2c send failed\n", __func__);
			return -EIO;
		}
	}

	return 0;
}

int mxt_write_block(struct i2c_client *client, u16 addr, u16 length, u8 *value)
{
	int i;
	struct {
		__le16 le_addr;
		u8  data[MXT_MAX_BLOCK_WRITE];
	} i2c_block_transfer;

	if (length > MXT_MAX_BLOCK_WRITE)
		return -EINVAL;

	memcpy(i2c_block_transfer.data, value, length);

	i2c_block_transfer.le_addr = cpu_to_le16(addr);

	i = i2c_master_send(client, (u8 *) &i2c_block_transfer, length + 2);

	if (i == (length + 2))
		return 0;
	else
		return -EIO;
}

//Desmond ++ add for I2C_STRESS_TEST
#ifdef CONFIG_I2C_STRESS_TEST
#include <linux/i2c_testcase.h>
#define I2C_TEST_TOUCH_FAIL (-1)
static int TestMxtTouch(struct i2c_client *client)
{
	int lnResult = I2C_TEST_PASS;
	int error;
	struct mxt_info info;

	i2c_log_in_test_case("TestMxtTouch++\n");

	if(g_isPadMode) {
		error = mxt_read_reg(client, 0, sizeof(info), &info);
		if (error < 0)
			lnResult = I2C_TEST_TOUCH_FAIL;
		else{
			i2c_log_in_test_case("Family ID: %d\n",info.family_id);
			lnResult=I2C_TEST_PASS;
		}
	}else {
		i2c_log_in_test_case("Not in Pad mode, cannot test mxtTouch\n");
		lnResult = I2C_TEST_TOUCH_FAIL;
	}

	i2c_log_in_test_case("TestMxtTouch--\n");
	return lnResult;
};

static struct i2c_test_case_info gTouchTestCaseInfo[] =
{
	__I2C_STRESS_TEST_CASE_ATTR(TestMxtTouch),
};
#endif
//Desmond ++ add for I2C_STRESS_TEST

static struct mxt_object *mxt_get_object(struct mxt_data *data, u8 type)
{
	struct mxt_object *object;
	int i;
	for (i = 0; i < data->info.object_num; i++) {
		object = data->object_table + i;
		if (object->type == type)
			return object;
	}

	dev_err(&data->client->dev, "Invalid object type T%u\n", type);
	return NULL;
}

static int mxt_read_object(struct mxt_data *data,
				u8 type, u8 offset, u8 *val)
{
	struct mxt_object *object;
	u16 reg;

	object = mxt_get_object(data, type);
	if (!object)
		return -EINVAL;

	reg = object->start_address;
	return mxt_read_reg(data->client, reg + offset, 1, val);
}

static int mxt_write_object(struct mxt_data *data,
				 u8 type, u8 offset, u8 val)
{
	struct mxt_object *object;
	u16 reg;

	object = mxt_get_object(data, type);
	if (!object || offset >= object->size)
		return -EINVAL;

	if (offset >= object->size * object->instances) {
		dev_err(&data->client->dev, "Tried to write outside object T%d"
			" offset:%d, size:%d\n", type, offset, object->size);
		return -EINVAL;
	}

	reg = object->start_address;
	return mxt_write_reg(data->client, reg + offset, val);
}

//Desmond ++ Initial config
static void mxt_init_config(struct mxt_data *data)
{
	mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] %s ++\n", __func__);
/*
	int i;
	for(i=0; i<130; i++)
		mxt_write_object(data, MXT_DEBUG_DIAGNOSTIC_T37, i, 0);

	mxt_write_object(data, MXT_GEN_MESSAGE_T5, 0, 255);
	mxt_write_object(data, MXT_GEN_MESSAGE_T5, 1, 0);
	mxt_write_object(data, MXT_GEN_MESSAGE_T5, 2, 183);
	mxt_write_object(data, MXT_GEN_MESSAGE_T5, 3, 48);
	mxt_write_object(data, MXT_GEN_MESSAGE_T5, 4, 109);
	mxt_write_object(data, MXT_GEN_MESSAGE_T5, 5, 0);
	mxt_write_object(data, MXT_GEN_MESSAGE_T5, 6, 0);
	mxt_write_object(data, MXT_GEN_MESSAGE_T5, 7, 0);
	mxt_write_object(data, MXT_GEN_MESSAGE_T5, 8, 0);

	mxt_write_object(data, MXT_GEN_COMMAND_T6, 0, 0);
	mxt_write_object(data, MXT_GEN_COMMAND_T6, 1, 0);
	mxt_write_object(data, MXT_GEN_COMMAND_T6, 2, 0);
	mxt_write_object(data, MXT_GEN_COMMAND_T6, 3, 0);
	mxt_write_object(data, MXT_GEN_COMMAND_T6, 4, 0);
	mxt_write_object(data, MXT_GEN_COMMAND_T6, 5, 0);
*/
	mxt_write_object(data, MXT_GEN_POWER_T7, 0, 20);
	mxt_write_object(data, MXT_GEN_POWER_T7, 1, 255);
	mxt_write_object(data, MXT_GEN_POWER_T7, 2, 20);
	mxt_write_object(data, MXT_GEN_POWER_T7, 3, 67);

	mxt_write_object(data, MXT_GEN_ACQUIRE_T8, 0, 127);
	mxt_write_object(data, MXT_GEN_ACQUIRE_T8, 1, 0);
	mxt_write_object(data, MXT_GEN_ACQUIRE_T8, 2, 5);
	mxt_write_object(data, MXT_GEN_ACQUIRE_T8, 3, 5);
	mxt_write_object(data, MXT_GEN_ACQUIRE_T8, 4, 0);
	mxt_write_object(data, MXT_GEN_ACQUIRE_T8, 5, 0);
	mxt_write_object(data, MXT_GEN_ACQUIRE_T8, 6, 255);
	mxt_write_object(data, MXT_GEN_ACQUIRE_T8, 7, 4);
	mxt_write_object(data, MXT_GEN_ACQUIRE_T8, 8, 0);
	mxt_write_object(data, MXT_GEN_ACQUIRE_T8, 9, 0);

	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 0, 143);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 1, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 2, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 3, 32);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 4, 52);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 5, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 6, 100);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 7, 50);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 8, 2);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 9, 1);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 10, 10);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 11, 30);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 12, 5);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 13, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 14, 10);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 15, 20);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 16, 20);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 17, 20);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 18, 55);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 19, 4);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 20, 127);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 21, 7);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 22, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 23, 60);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 24, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 25, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 26, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 27, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 28, 64);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 29, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 30, 28);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 31, 10);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 32, 41);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 33, 44);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 34, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 35, 0);
/*
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 0, 129);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 1, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 2, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 3, 32);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 4, 52);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 5, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 6, 90);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 7, 55);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 8, 2);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 9, 4);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 10, 10);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 11, 30);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 12, 5);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 13, 32);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 14, 5);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 15, 20);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 16, 20);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 17, 20);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 18, 255);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 19, 11);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 20, 255);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 21, 11);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 22, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 23, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 24, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 25, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 26, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 27, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 28, 64);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 29, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 30, 28);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 31, 12);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 32, 41);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 33, 44);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 34, 0);
	mxt_write_object(data, MXT_TOUCH_MULTI_T9, 35, 0);

	mxt_write_object(data, MXT_PROCI_GRIP_T40, 0, 0);
	mxt_write_object(data, MXT_PROCI_GRIP_T40, 1, 20);
	mxt_write_object(data, MXT_PROCI_GRIP_T40, 2, 20);
	mxt_write_object(data, MXT_PROCI_GRIP_T40, 3, 20);
	mxt_write_object(data, MXT_PROCI_GRIP_T40, 4, 20);

	mxt_write_object(data, MXT_PROCI_TOUCHSUPPRESSION_T42, 0, 0);
	mxt_write_object(data, MXT_PROCI_TOUCHSUPPRESSION_T42, 1, 42);
	mxt_write_object(data, MXT_PROCI_TOUCHSUPPRESSION_T42, 2, 50);
	mxt_write_object(data, MXT_PROCI_TOUCHSUPPRESSION_T42, 3, 50);
	mxt_write_object(data, MXT_PROCI_TOUCHSUPPRESSION_T42, 4, 127);
	mxt_write_object(data, MXT_PROCI_TOUCHSUPPRESSION_T42, 5, 0);
	mxt_write_object(data, MXT_PROCI_TOUCHSUPPRESSION_T42, 6, 0);
	mxt_write_object(data, MXT_PROCI_TOUCHSUPPRESSION_T42, 7, 0);
	mxt_write_object(data, MXT_PROCI_TOUCHSUPPRESSION_T42, 8, 5);
	mxt_write_object(data, MXT_PROCI_TOUCHSUPPRESSION_T42, 9, 5);

	mxt_write_object(data, MXT_PROCI_STYLUS_T47, 0, 0);
	mxt_write_object(data, MXT_PROCI_STYLUS_T47, 1, 0);
	mxt_write_object(data, MXT_PROCI_STYLUS_T47, 2, 0);
	mxt_write_object(data, MXT_PROCI_STYLUS_T47, 3, 0);
	mxt_write_object(data, MXT_PROCI_STYLUS_T47, 4, 0);
	mxt_write_object(data, MXT_PROCI_STYLUS_T47, 5, 0);
	mxt_write_object(data, MXT_PROCI_STYLUS_T47, 6, 0);
	mxt_write_object(data, MXT_PROCI_STYLUS_T47, 7, 0);
	mxt_write_object(data, MXT_PROCI_STYLUS_T47, 8, 1);
	mxt_write_object(data, MXT_PROCI_STYLUS_T47, 9, 0);
	mxt_write_object(data, MXT_PROCI_STYLUS_T47, 10, 0);
	mxt_write_object(data, MXT_PROCI_STYLUS_T47, 11, 0);
	mxt_write_object(data, MXT_PROCI_STYLUS_T47, 12, 0);

	mxt_write_object(data, MXT_PROCI_ADAPTIVETHRESHOLD_T55, 0, 0);
	mxt_write_object(data, MXT_PROCI_ADAPTIVETHRESHOLD_T55, 1, 24);
	mxt_write_object(data, MXT_PROCI_ADAPTIVETHRESHOLD_T55, 2, 40);
	mxt_write_object(data, MXT_PROCI_ADAPTIVETHRESHOLD_T55, 3, 1);
	mxt_write_object(data, MXT_PROCI_ADAPTIVETHRESHOLD_T55, 4, 2);
	mxt_write_object(data, MXT_PROCI_ADAPTIVETHRESHOLD_T55, 5, 0);
	mxt_write_object(data, MXT_PROCI_ADAPTIVETHRESHOLD_T55, 6, 0);

	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 0, 1);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 1, 0);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 2, 0);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 3, 43);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 4, 22);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 5, 22);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 6, 22);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 7, 23);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 8, 23);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 9, 23);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 10, 23);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 11, 24);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 12, 24);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 13, 24);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 14, 24);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 15, 25);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 16, 25);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 17, 25);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 18, 25);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 19, 25);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 20, 26);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 21, 26);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 22, 26);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 23, 26);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 24, 26);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 25, 26);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 26, 26);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 27, 26);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 28, 26);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 29, 26);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 30, 26);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 31, 26);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 32, 26);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 33, 26);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 34, 26);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 35, 26);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 36, 0);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 37, 16);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 38, 1);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 39, 2);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 40, 0);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 41, 5);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 42, 0);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 43, 0);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 44, 0);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 45, 0);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 46, 0);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 47, 0);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 48, 0);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 49, 0);
	mxt_write_object(data, MXT_PROCI_SHIELDLESS_T56, 50, 0);

	mxt_write_object(data, MXT_SPT_SELFTEST_T25, 0, 0);
	mxt_write_object(data, MXT_SPT_SELFTEST_T25, 1, 0);
	mxt_write_object(data, MXT_SPT_SELFTEST_T25, 2, 0);
	mxt_write_object(data, MXT_SPT_SELFTEST_T25, 3, 0);
	mxt_write_object(data, MXT_SPT_SELFTEST_T25, 4, 0);
	mxt_write_object(data, MXT_SPT_SELFTEST_T25, 5, 0);
	mxt_write_object(data, MXT_SPT_SELFTEST_T25, 6, 0);
	mxt_write_object(data, MXT_SPT_SELFTEST_T25, 7, 0);
	mxt_write_object(data, MXT_SPT_SELFTEST_T25, 8, 0);
	mxt_write_object(data, MXT_SPT_SELFTEST_T25, 9, 0);
	mxt_write_object(data, MXT_SPT_SELFTEST_T25, 10, 0);
	mxt_write_object(data, MXT_SPT_SELFTEST_T25, 11, 0);
	mxt_write_object(data, MXT_SPT_SELFTEST_T25, 12, 0);
	mxt_write_object(data, MXT_SPT_SELFTEST_T25, 13, 0);
	mxt_write_object(data, MXT_SPT_SELFTEST_T25, 14, 200);

	mxt_write_object(data, MXT_SPT_DIGITIZER_T43, 0, 168);
	mxt_write_object(data, MXT_SPT_DIGITIZER_T43, 1, 0);
	mxt_write_object(data, MXT_SPT_DIGITIZER_T43, 2, 144);
	mxt_write_object(data, MXT_SPT_DIGITIZER_T43, 3, 0);
	mxt_write_object(data, MXT_SPT_DIGITIZER_T43, 4, 1);
	mxt_write_object(data, MXT_SPT_DIGITIZER_T43, 5, 1);
	mxt_write_object(data, MXT_SPT_DIGITIZER_T43, 6, 0);
	mxt_write_object(data, MXT_SPT_DIGITIZER_T43, 7, 0);
	mxt_write_object(data, MXT_SPT_DIGITIZER_T43, 8, 0);
	mxt_write_object(data, MXT_SPT_DIGITIZER_T43, 9, 0);
	mxt_write_object(data, MXT_SPT_DIGITIZER_T43, 10, 0);
*/
	mxt_write_object(data, MXT_SPT_CTECONFIG_T46, 0, 4);
	mxt_write_object(data, MXT_SPT_CTECONFIG_T46, 1, 0);
	mxt_write_object(data, MXT_SPT_CTECONFIG_T46, 2, 12);
	mxt_write_object(data, MXT_SPT_CTECONFIG_T46, 3, 16);
	mxt_write_object(data, MXT_SPT_CTECONFIG_T46, 4, 0);
	mxt_write_object(data, MXT_SPT_CTECONFIG_T46, 5, 0);
	mxt_write_object(data, MXT_SPT_CTECONFIG_T46, 6, 1);
	mxt_write_object(data, MXT_SPT_CTECONFIG_T46, 7, 0);
	mxt_write_object(data, MXT_SPT_CTECONFIG_T46, 8, 0);
	mxt_write_object(data, MXT_SPT_CTECONFIG_T46, 9, 15);
	mxt_write_object(data, MXT_SPT_CTECONFIG_T46, 10, 16);

/*	mxt_write_object(data, 62, 0, 1);
	mxt_write_object(data, 62, 1, 26);
	mxt_write_object(data, 62, 2, 0);
	mxt_write_object(data, 62, 3, 0);
	mxt_write_object(data, 62, 4, 0);
	mxt_write_object(data, 62, 5, 0);
	mxt_write_object(data, 62, 6, 0);
	mxt_write_object(data, 62, 7, 0);
	mxt_write_object(data, 62, 8, 0);
	mxt_write_object(data, 62, 9, 16);
	mxt_write_object(data, 62, 10, 20);
	mxt_write_object(data, 62, 11, 10);
	mxt_write_object(data, 62, 12, 25);
	mxt_write_object(data, 62, 13, 12);
	mxt_write_object(data, 62, 14, 5);
	mxt_write_object(data, 62, 15, 0);
	mxt_write_object(data, 62, 16, 10);
	mxt_write_object(data, 62, 17, 5);
	mxt_write_object(data, 62, 18, 5);
	mxt_write_object(data, 62, 19, 90);
	mxt_write_object(data, 62, 20, 16);
	mxt_write_object(data, 62, 21, 20);
	mxt_write_object(data, 62, 22, 48);
	mxt_write_object(data, 62, 23, 15);
	mxt_write_object(data, 62, 24, 63);
	mxt_write_object(data, 62, 25, 6);
	mxt_write_object(data, 62, 26, 6);
	mxt_write_object(data, 62, 27, 4);
	mxt_write_object(data, 62, 28, 64);
	mxt_write_object(data, 62, 29, 0);
	mxt_write_object(data, 62, 30, 0);
	mxt_write_object(data, 62, 31, 0);
	mxt_write_object(data, 62, 32, 0);
	mxt_write_object(data, 62, 33, 0);
	mxt_write_object(data, 62, 34, 116);
	mxt_write_object(data, 62, 35, 55);
	mxt_write_object(data, 62, 36, 2);
	mxt_write_object(data, 62, 37, 20);
	mxt_write_object(data, 62, 38, 1);
	mxt_write_object(data, 62, 39, 2);
	mxt_write_object(data, 62, 40, 10);
	mxt_write_object(data, 62, 41, 50);
	mxt_write_object(data, 62, 42, 50);
	mxt_write_object(data, 62, 43, 0);
	mxt_write_object(data, 62, 44, 0);
	mxt_write_object(data, 62, 45, 0);
	mxt_write_object(data, 62, 46, 0);
	mxt_write_object(data, 62, 47, 0);
	mxt_write_object(data, 62, 48, 0);
	mxt_write_object(data, 62, 49, 0);
	mxt_write_object(data, 62, 50, 0);
	mxt_write_object(data, 62, 51, 20);
	mxt_write_object(data, 62, 52, 15);
	mxt_write_object(data, 62, 53, 0);
	mxt_write_object(data, 62, 54, 0);
	mxt_write_object(data, 62, 55, 0);
	mxt_write_object(data, 62, 56, 0);
	mxt_write_object(data, 62, 57, 0);
	mxt_write_object(data, 62, 58, 0);
	mxt_write_object(data, 62, 59, 0);
	mxt_write_object(data, 62, 60, 0);
	mxt_write_object(data, 62, 61, 0);
	mxt_write_object(data, 62, 62, 0);
	mxt_write_object(data, 62, 63, 0);
	mxt_write_object(data, 62, 64, 0);
	mxt_write_object(data, 62, 65, 0);
	mxt_write_object(data, 62, 66, 0);
	mxt_write_object(data, 62, 67, 0);
	mxt_write_object(data, 62, 68, 0);
	mxt_write_object(data, 62, 69, 0);
	mxt_write_object(data, 62, 70, 0);
	mxt_write_object(data, 62, 71, 0);
	mxt_write_object(data, 62, 72, 0);
	mxt_write_object(data, 62, 73, 0);*/

	mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] %s --\n", __func__);
}
//Desmond --

static int mxt_soft_reset(struct mxt_data *data, u8 value)
{
	int timeout_counter = 0;
	struct device *dev = &data->client->dev;

	dev_info(dev, "Resetting chip\n");

	mxt_write_object(data, MXT_GEN_COMMAND_T6,
			MXT_COMMAND_RESET, value);

	if (data->pdata->read_chg == NULL) {
		msleep(MXT_RESET_NOCHGREAD);
	} else {
		msleep(MXT_RESET_TIME);

		timeout_counter = 0;
		while ((timeout_counter++ <= 100) && data->pdata->read_chg())
			msleep(20);
		if (timeout_counter > 100) {
			dev_err(dev, "No response after reset!\n");
			return -EIO;
		}
	}

	return 0;
}

static void mxt_proc_t6_messages(struct mxt_data *data, u8 *msg)
{
	//struct device *dev = &data->client->dev;
	u32 crc;
	u8 status = msg[1];

	crc = msg[2] | (msg[3] << 8) | (msg[4] << 16);

	if (crc != data->config_crc) {
		data->config_crc = crc;
		mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] T6 cfg crc 0x%06X\n", crc);
	}

	if (status)
		mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] T6 status %s%s%s%s%s%s\n",
			(status & MXT_STATUS_RESET) ? "RESET " : "",
			(status & MXT_STATUS_OFL) ? "OFL " : "",
			(status & MXT_STATUS_SIGERR) ? "SIGERR " : "",
			(status & MXT_STATUS_CAL) ? "CAL " : "",
			(status & MXT_STATUS_CFGERR) ? "CFGERR " : "",
			(status & MXT_STATUS_COMSERR) ? "COMSERR " : "");
}

static void mxt_input_sync(struct mxt_data *data)
{
	input_mt_report_pointer_emulation(data->input_dev, false);
	input_sync(data->input_dev);
}

//Desmond++
//#define CAL_REPORT_RATE
static void mxt_proc_t9_messages(struct mxt_data *data, u8 *message)
{
	struct device *dev = &data->client->dev;
	struct input_dev *input_dev = data->input_dev;
	u8 status;
	int x;
	int y;
	int area;
	int amplitude;
	u8 vector;
	int id;
#ifdef CAL_REPORT_RATE
	static u32 u32TouchCount = 0;
	static u64 u64timestamp_start = 0;
	u64 u64timestamp_end = 0;
#endif
	if (!input_dev || data->driver_paused)
		return;

	id = message[0] - data->T9_reportid_min;

	if (id < 0 || id > data->num_touchids) {
		dev_err(dev, "invalid touch id %d, total num touch is %d\n",
			id, data->num_touchids);
		return;
	}

	status = message[1];

	x = (message[2] << 4) | ((message[4] >> 4) & 0xf);
	y = (message[3] << 4) | ((message[4] & 0xf));
	if (data->max_x < 1024)
		x >>= 2;
	if (data->max_y < 1024)
		y >>= 2;
	area = message[5];
	amplitude = message[6];
	vector = message[7];

	mxt_debug(ATMEL_PAD_DEBUG_VERBOSE, "[Touch][mxt1664s][%d] %c%c%c%c%c%c%c%c x: %d y: %d area: %d amp: %d vector: %02X\n",
		id,
		(status & MXT_T9_DETECT) ? 'D' : '.',
		(status & MXT_T9_PRESS) ? 'P' : '.',
		(status & MXT_T9_RELEASE) ? 'R' : '.',
		(status & MXT_T9_MOVE) ? 'M' : '.',
		(status & MXT_T9_VECTOR) ? 'V' : '.',
		(status & MXT_T9_AMP) ? 'A' : '.',
		(status & MXT_T9_SUPPRESS) ? 'S' : '.',
		(status & MXT_T9_UNGRIP) ? 'U' : '.',
		x, y, area, amplitude, vector);

	input_mt_slot(input_dev, id);

	if ((status & MXT_T9_DETECT) && (status & MXT_T9_RELEASE)) {
		/* Touch in detect, just after being released, so
		 * get new touch tracking ID */
		input_mt_report_slot_state(input_dev, MT_TOOL_FINGER, 0);
		mxt_input_sync(data);
	}

	if (status & MXT_T9_DETECT) {
		/* Touch in detect, report X/Y position */
		input_mt_report_slot_state(input_dev, MT_TOOL_FINGER, 1);

		input_report_abs(input_dev, ABS_MT_POSITION_X, x);
		input_report_abs(input_dev, ABS_MT_POSITION_Y, y);
		input_report_abs(input_dev, ABS_MT_PRESSURE, amplitude);
		input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR, area);
		//input_report_abs(input_dev, ABS_MT_ORIENTATION, vector); //Desmond ++
	} else {
		/* Touch no longer in detect, so close out slot */
#ifdef CAL_REPORT_RATE
				u64timestamp_start = 0;
				u32TouchCount = 0;
#endif
		mxt_input_sync(data);
		input_mt_report_slot_state(input_dev, MT_TOOL_FINGER, 0);
	}
#ifdef CAL_REPORT_RATE
		if ( u64timestamp_start == 0 )
		{
			u64timestamp_start = get_jiffies_64();
		}
		else
		{
			u64timestamp_end = get_jiffies_64();

			u32TouchCount++;

			if ( ((u64timestamp_end -u64timestamp_start)/HZ) >= 5 ) //5 sec.
			{
				mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] report rate= %d\n", u32TouchCount/5);

				u64timestamp_start = 0;
				u32TouchCount = 0;
			}
		}
#endif
}

static void mxt_proc_t15_messages(struct mxt_data *data, u8 *msg)
{
	struct input_dev *input_dev = data->input_dev;
	//struct device *dev = &data->client->dev;
	u8 key;
	bool curr_state, new_state;
	bool sync = false;
	unsigned long keystates = le32_to_cpu(msg[2]);

	for (key = 0; key < ARRAY_SIZE(mxt_t15_keys); key++) {
		curr_state = test_bit(key, &mxt_t15_keystatus);
		new_state = test_bit(key, &keystates);

		if (!curr_state && new_state) {
			mxt_debug(ATMEL_PAD_DEBUG_VERBOSE, "[Touch][mxt1664s] T15 key press: %u\n", key);
			__set_bit(key, &mxt_t15_keystatus);
			input_event(input_dev, EV_KEY, mxt_t15_keys[key], 1);
			sync = true;
		} else if (curr_state && !new_state) {
			mxt_debug(ATMEL_PAD_DEBUG_VERBOSE, "[Touch][mxt1664s] T15 key release: %u\n", key);
			__clear_bit(key, &mxt_t15_keystatus);
			input_event(input_dev, EV_KEY, mxt_t15_keys[key], 0);
			sync = true;
		}
	}

	if (sync)
		input_sync(input_dev);
}

static void mxt_proc_t42_messages(struct mxt_data *data, u8 *msg)
{
	//struct device *dev = &data->client->dev;
	u8 status = msg[1];

	if (status & MXT_T42_MSG_TCHSUP)
		mxt_debug(ATMEL_PAD_DEBUG_VERBOSE, "[Touch][mxt1664s] T42 suppress\n");
	else
		mxt_debug(ATMEL_PAD_DEBUG_VERBOSE, "[Touch][mxt1664s] T42 normal\n");
}

static int mxt_proc_t48_messages(struct mxt_data *data, u8 *msg)
{
	//struct device *dev = &data->client->dev;
	u8 status, state;

	status = msg[1];
	state  = msg[4];

	mxt_debug(ATMEL_PAD_DEBUG_VERBOSE, "[Touch][mxt1664s] T48 state %d status %02X %s%s%s%s%s\n",
			state,
			status,
			(status & 0x01) ? "FREQCHG " : "",
			(status & 0x02) ? "APXCHG " : "",
			(status & 0x04) ? "ALGOERR " : "",
			(status & 0x10) ? "STATCHG " : "",
			(status & 0x20) ? "NLVLCHG " : "");

	return 0;
}

static void mxt_proc_t63_messages(struct mxt_data *data, u8 *msg)
{
	struct device *dev = &data->client->dev;
	struct input_dev *input_dev = data->input_dev;
	u8 id;
	u16 x, y;
	u8 pressure;

	if (!input_dev)
		return;

	/* stylus slots come after touch slots */
	id = data->num_touchids + (msg[0] - data->T63_reportid_min);

	if (id < 0 || id > (data->num_touchids + data->num_stylusids)) {
		dev_err(dev, "invalid stylus id %d, max slot is %d\n",
			id, data->num_stylusids);
		return;
	}

	x = msg[3] | (msg[4] << 8);
	y = msg[5] | (msg[6] << 8);
	pressure = msg[7] & MXT_STYLUS_PRESSURE_MASK;

	mxt_debug(ATMEL_PAD_DEBUG_VERBOSE, "[Touch][mxt1664s][%d] %c%c%c%c x: %d y: %d pressure: %d stylus:%c%c%c%c\n",
		id,
		(msg[1] & MXT_STYLUS_SUPPRESS) ? 'S' : '.',
		(msg[1] & MXT_STYLUS_MOVE)     ? 'M' : '.',
		(msg[1] & MXT_STYLUS_RELEASE)  ? 'R' : '.',
		(msg[1] & MXT_STYLUS_PRESS)    ? 'P' : '.',
		x, y, pressure,
		(msg[2] & MXT_STYLUS_BARREL) ? 'B' : '.',
		(msg[2] & MXT_STYLUS_ERASER) ? 'E' : '.',
		(msg[2] & MXT_STYLUS_TIP)    ? 'T' : '.',
		(msg[2] & MXT_STYLUS_DETECT) ? 'D' : '.');

	input_mt_slot(input_dev, id);

	if (msg[2] & MXT_STYLUS_DETECT) {
		input_mt_report_slot_state(input_dev, MT_TOOL_PEN, 1);
		input_report_abs(input_dev, ABS_MT_POSITION_X, x);
		input_report_abs(input_dev, ABS_MT_POSITION_Y, y);
		input_report_abs(input_dev, ABS_MT_PRESSURE, pressure);
	} else {
		input_mt_report_slot_state(input_dev, MT_TOOL_PEN, 0);
	}

	input_report_key(input_dev, BTN_STYLUS, (msg[2] & MXT_STYLUS_ERASER));
	input_report_key(input_dev, BTN_STYLUS2, (msg[2] & MXT_STYLUS_BARREL));

	mxt_input_sync(data);
}

static int mxt_proc_message(struct mxt_data *data, u8 *msg)
{
	u8 report_id = msg[0];

	if (report_id == MXT_RPTID_NOMSG)
		return -1;

	if (data->debug_enabled)
		print_hex_dump(KERN_DEBUG, "MXT MSG:", DUMP_PREFIX_NONE, 16, 1,
			       msg, data->T5_msg_size, false);

	if (report_id >= data->T9_reportid_min
	    && report_id <= data->T9_reportid_max) {
		mxt_proc_t9_messages(data, msg);
	} else if (report_id >= data->T63_reportid_min
		   && report_id <= data->T63_reportid_max) {
		mxt_proc_t63_messages(data, msg);
	} else if (report_id >= data->T15_reportid_min
		   && report_id <= data->T15_reportid_max) {
		mxt_proc_t15_messages(data, msg);
	} else if (report_id == data->T6_reportid) {
		mxt_proc_t6_messages(data, msg);
	} else if (report_id == data->T48_reportid) {
		mxt_proc_t48_messages(data, msg);
	} else if (report_id >= data->T42_reportid_min
		   && report_id <= data->T42_reportid_max) {
		mxt_proc_t42_messages(data, msg);
	}

	return 0;
}

static int mxt_read_count_messages(struct mxt_data *data, u8 count)
{
	struct device *dev = &data->client->dev;
	int ret;
	int i;
	u8 num_valid = 0;

	/* Safety check for msg_buf */
	if (count > data->max_reportid)
		return -EINVAL;

	/* Process remaining messages if necessary */
	ret = mxt_read_reg(data->client, data->T5_address,
				data->T5_msg_size * count, data->msg_buf);
	if (ret) {
		dev_err(dev, "Failed to read %u messages (%d)\n", count, ret);
		return ret;
	}

	for (i = 0;  i < count; i++) {
		ret = mxt_proc_message(data,
			data->msg_buf + data->T5_msg_size * i);

		if (ret == 0)
			num_valid++;
	}

	/* return number of messages read */
	return num_valid;
}

static irqreturn_t mxt_read_messages_t44(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
	int ret;
	u8 count, num_left;

	/* Read T44 and T5 together */
	ret = mxt_read_reg(data->client, data->T44_address,
		data->T5_msg_size + 1, data->msg_buf);
	if (ret) {
		dev_err(dev, "Failed to read T44 and T5 (%d)\n", ret);
		return IRQ_NONE;
	}

	count = data->msg_buf[0];

	if (count == 0) {
		mxt_debug(ATMEL_PAD_DEBUG_VERBOSE, "[Touch][mxt1664s] Interrupt triggered but zero messages\n");
		return IRQ_NONE;
	} else if (count > data->max_reportid) {
		mxt_debug(ATMEL_PAD_DEBUG_ERR, "[Touch][mxt1664s] T44 count exceeded max report id\n");
		count = data->max_reportid;
	}

	/* Process first message */
	ret = mxt_proc_message(data, data->msg_buf + 1);
	if (ret < 0) {
		mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] Unexpected invalid message\n");
		return IRQ_NONE;
	}

	num_left = count - 1;

	/* Process remaining messages if necessary */
	if (num_left) {
		ret = mxt_read_count_messages(data, num_left);
		if (ret < 0) {
			mxt_input_sync(data);
			return IRQ_NONE;
		} else if (ret != num_left) {
			mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] Unexpected invalid message\n");
		}
	}

	mxt_input_sync(data);

	return IRQ_HANDLED;
}

static int mxt_read_t9_messages_until_invalid(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
	int count, read;
	u8 tries = 2;

	count = data->max_reportid;

	/* Read messages until we force an invalid */
	do {
		read = mxt_read_count_messages(data, count);
		if (read < count)
			return 0;
	} while (--tries);

	dev_err(dev, "CHG pin isn't cleared\n");
	return -EBUSY;
}

static irqreturn_t mxt_read_t9_messages(struct mxt_data *data)
{
	int total_handled, num_handled;
	u8 count = data->last_message_count;

	if (count < 1 || count > data->max_reportid)
		count = 1;

	/* include final invalid message */
	total_handled = mxt_read_count_messages(data, count + 1);
	if (total_handled < 0)
		return IRQ_NONE;
	/* if there were invalid messages, then we are done */
	else if (total_handled <= count)
		goto update_count;

	/* read two at a time until an invalid message or else we reach
	 * reportid limit */
	do {
		num_handled = mxt_read_count_messages(data, 2);
		if (num_handled < 0)
			return IRQ_NONE;

		total_handled += num_handled;

		if (num_handled < 2)
			break;
	} while (total_handled < data->num_touchids);

update_count:
	data->last_message_count = total_handled;
	mxt_input_sync(data);

	return IRQ_HANDLED;
}

static irqreturn_t mxt_interrupt(int irq, void *dev_id)
{
	struct mxt_data *data = dev_id;
	int ret = 0;
	int cnt = 0;	//Desmond++

	//Desmond ++ Keep polling if CHG stay low
	while(gpio_get_value(MXT_PAD_TS_GPIO_IRQ) == 0 && !g_isPadSleep /*&& cnt < 5*/) {

		mxt_debug(ATMEL_PAD_DEBUG_INTR, "[Touch][mxt1664s] %s: chg stay high, cnt=%d", __func__, cnt);

		if (data->T44_address)
			ret = mxt_read_messages_t44(data);
		else
			ret = mxt_read_t9_messages(data);

		cnt++;
		msleep(10);
	}

	return ret;
}

static void mxt_read_current_crc(struct mxt_data *data)
{
	/* CRC has already been read */
	if (data->config_crc > 0)
		return;

	mxt_write_object(data, MXT_GEN_COMMAND_T6,
		MXT_COMMAND_REPORTALL, 1);

	msleep(30);

	/* Read all messages until invalid, this will update the
	   config crc stored in mxt_data */
	mxt_read_t9_messages_until_invalid(data);

	/* on failure, CRC is set to 0 and config will always be downloaded */
}

static u32 mxt_calculate_crc32(u32 crc, u8 firstbyte, u8 secondbyte)
{
	static const unsigned int crcpoly = 0x80001B;
	u32 result;
	u16 data_word;

	data_word = (u16)((u16)(secondbyte << 8) | firstbyte);
	result = ((crc << 1) ^ (u32)data_word);

	/* if bit 25 is set, XOR with crcpoly */
	if (result & 0x1000000) {
		result ^= crcpoly;
	}

	return result;
}

static u32 mxt_calculate_config_crc(u8 *base, off_t start_off, off_t end_off)
{
	u8 *i;
	u32 crc = 0;
	u8 *last_val = base + end_off - 1;

	if (end_off < start_off)
		return -EINVAL;

	for (i = base + start_off; i < last_val; i += 2) {
		crc = mxt_calculate_crc32(crc, *i, *(i + 1));
	}

	/* if len is odd, fill the last byte with 0 */
	if (i == last_val)
		crc = mxt_calculate_crc32(crc, *i, 0);

	/* Mask to 24-bit */
	return (crc & 0x00FFFFFF);
}

int mxt_download_config(struct mxt_data *data, const char *fn)
{
	struct device *dev = &data->client->dev;
	struct mxt_info cfg_info;
	struct mxt_object *object;
	const struct firmware *cfg = NULL;
	int ret;
	int offset;
	int data_pos;
	int byte_offset;
	int i;
	int config_start_offset;
	u32 info_crc, config_crc, calculated_crc;
	u8 *config_mem;
	size_t config_mem_size;
	unsigned int type, instance, size;
	u8 val;
	u16 reg;

	ret = request_firmware(&cfg, fn, dev);
	if (ret < 0) {
		dev_err(dev, "Failure to request config file %s\n", fn);
		//return 0;
		return 2; //Desmond ++ config file not exist case
	}

	mxt_read_current_crc(data);

	if (strncmp(cfg->data, MXT_CFG_MAGIC, strlen(MXT_CFG_MAGIC))) {
		dev_err(dev, "Unrecognised config file\n");
		ret = -EINVAL;
		goto release;
	}

	data_pos = strlen(MXT_CFG_MAGIC);

	/* Load information block and check */
	for (i = 0; i < sizeof(struct mxt_info); i++) {
		ret = sscanf(cfg->data + data_pos, "%hhx%n",
			     (unsigned char *)&cfg_info + i,
			     &offset);
		if (ret != 1) {
			dev_err(dev, "Bad format\n");
			ret = -EINVAL;
			goto release;
		}

		data_pos += offset;
	}

	/* Read CRCs */
	ret = sscanf(cfg->data + data_pos, "%x%n", &info_crc, &offset);
	if (ret != 1) {
		dev_err(dev, "Bad format\n");
		ret = -EINVAL;
		goto release;
	}
	data_pos += offset;

	ret = sscanf(cfg->data + data_pos, "%x%n", &config_crc, &offset);
	if (ret != 1) {
		dev_err(dev, "Bad format\n");
		ret = -EINVAL;
		goto release;
	}
	data_pos += offset;

	/* The Info Block CRC is calculated over mxt_info and the object table
	 * If it does not match then we are trying to load the configuration
	 * from a different chip or firmware version, so the configuration CRC
	 * is invalid anyway. */
	if (info_crc == data->info_block_crc) {
		if (config_crc == 0 || data->config_crc == 0) {
			dev_info(dev, "CRC zero, attempting to apply config\n");
		} else if (config_crc == data->config_crc) {
			dev_info(dev, "Config CRC 0x%06X: OK\n", data->config_crc);
			ret = 0;
			goto release;
		} else {
			dev_info(dev, "Config CRC 0x%06X: does not match file 0x%06X\n",
				 data->config_crc, config_crc);
		}
	} else {
		dev_warn(dev, "Info block CRC mismatch - attempting to apply config\n");
	}

	/* Malloc memory to store configuration */
	config_start_offset = MXT_OBJECT_START
		+ data->info.object_num * MXT_OBJECT_SIZE;
	config_mem_size = data->mem_size - config_start_offset;
	config_mem = kzalloc(config_mem_size, GFP_KERNEL);
	if (!config_mem) {
		dev_err(dev, "Failed to allocate memory\n");
		ret = -ENOMEM;
		goto release;
	}

	while (data_pos < cfg->size) {
		/* Read type, instance, length */
		ret = sscanf(cfg->data + data_pos, "%x %x %x%n",
			     &type, &instance, &size, &offset);
		if (ret == 0) {
			/* EOF */
			break;
		} else if (ret != 3) {
			dev_err(dev, "Bad format\n");
			ret = -EINVAL;
			goto release_mem;
		}
		data_pos += offset;

		object = mxt_get_object(data, type);
		if (!object) {
			ret = -EINVAL;
			goto release_mem;
		}

		if (instance >= object->instances) {
			dev_err(dev, "Object instances exceeded!\n");
			ret = -EINVAL;
			goto release_mem;
		}

		reg = object->start_address + object->size * instance;

		if (size > object->size) {
			/* Either we are in fallback mode due to wrong
			 * config or config from a later fw version,
			 * or the file is corrupt or hand-edited */
			dev_warn(dev, "Discarding %u bytes in T%u!\n",
				 size - object->size, type);

			size = object->size;
		} else if (object->size > size) {
			/* If firmware is upgraded, new bytes may be added to
			 * end of objects. It is generally forward compatible
			 * to zero these bytes - previous behaviour will be
			 * retained. However this does invalidate the CRC and
			 * will force fallback mode until the configuration is
			 * updated. We warn here but do nothing else - the
			 * malloc has zeroed the entire configuration. */
			dev_warn(dev, "Zeroing %d byte(s) in T%d\n",
				 object->size - size, type);
		}

		for (i = 0; i < size; i++) {
			ret = sscanf(cfg->data + data_pos, "%hhx%n",
				     &val,
				     &offset);
			if (ret != 1) {
				dev_err(dev, "Bad format\n");
				ret = -EINVAL;
				goto release_mem;
			}

			byte_offset = reg + i - config_start_offset;

			if ((byte_offset >= 0)
			    && (byte_offset <= config_mem_size)) {
				*(config_mem + byte_offset) = val;
			} else {
				dev_err(dev, "Bad object: reg:%d, T%d, ofs=%d\n",
					reg, object->type, byte_offset);
				ret = -EINVAL;
				goto release_mem;
			}

			data_pos += offset;
		}

	}

	/* calculate crc of the received configs (not the raw config file) */
	if (data->T7_address < config_start_offset) {
		dev_err(dev, "Bad T7 address, T7addr = %x, config offset %x\n",
				data->T7_address, config_start_offset);
		ret = 0;
		goto release_mem;
	}

	calculated_crc = mxt_calculate_config_crc(config_mem,
			data->T7_address - config_start_offset, config_mem_size);

	/* check the crc, calculated should same as what's in file */
	if (config_crc > 0 && (config_crc != calculated_crc)) {
		dev_err(dev, "CRC mismatch in config file, calculated=%06X, file=%06X\n",
				calculated_crc, config_crc);
		ret = 0;
		goto release_mem;
	}

	/* Write configuration as blocks */
	byte_offset = 0;
	while (byte_offset < config_mem_size) {
		size = config_mem_size - byte_offset;

		if (size > MXT_MAX_BLOCK_WRITE)
			size = MXT_MAX_BLOCK_WRITE;

		ret = mxt_write_block(data->client,
				      config_start_offset + byte_offset,
				      size, config_mem + byte_offset);
		if (ret != 0) {
			dev_err(dev, "Config write error, ret=%d\n", ret);
			goto release_mem;
		}

		byte_offset += size;
	}

	ret = 1; /* tell the caller config has been sent */

release_mem:
	kfree(config_mem);
release:
	release_firmware(cfg);
	return ret;
}

static int mxt_set_power_cfg(struct mxt_data *data, u8 mode)
{
	struct device *dev = &data->client->dev;
	int error;
	u8 actv_cycle_time;
	u8 idle_cycle_time;

	if (data->state != APPMODE) {
		dev_err(dev, "Not in APPMODE\n");
		return -EINVAL;
	}

	switch (mode) {
	case MXT_POWER_CFG_DEEPSLEEP:
		actv_cycle_time = 0;
		idle_cycle_time = 0;
		break;
	case MXT_POWER_CFG_RUN:
	default:
		actv_cycle_time = data->actv_cycle_time;
		idle_cycle_time = data->idle_cycle_time;
		break;
	}

	error = mxt_write_object(data, MXT_GEN_POWER_T7, MXT_POWER_ACTVACQINT,
				 actv_cycle_time);
	if (error)
		goto i2c_error;

	error = mxt_write_object(data, MXT_GEN_POWER_T7, MXT_POWER_IDLEACQINT,
				 idle_cycle_time);
	if (error)
		goto i2c_error;

	dev_dbg(dev, "Set ACTV %d, IDLE %d\n", actv_cycle_time, idle_cycle_time);

	data->is_stopped = (mode == MXT_POWER_CFG_DEEPSLEEP) ? 1 : 0;

	return 0;

i2c_error:
	dev_err(dev, "Failed to set power cfg\n");
	return error;
}

static int mxt_read_power_cfg(struct mxt_data *data, u8 *actv_cycle_time,
				u8 *idle_cycle_time)
{
	int error;

	error = mxt_read_object(data, MXT_GEN_POWER_T7,
				MXT_POWER_ACTVACQINT,
				actv_cycle_time);
	if (error)
		return error;

	error = mxt_read_object(data, MXT_GEN_POWER_T7,
				MXT_POWER_IDLEACQINT,
				idle_cycle_time);
	if (error)
		return error;

	return 0;
}

static int mxt_check_power_cfg_post_reset(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
	int error;

	error = mxt_read_power_cfg(data, &data->actv_cycle_time,
				   &data->idle_cycle_time);
	if (error)
		return error;

	/* Power config is zero, select free run */
	if (data->actv_cycle_time == 0 || data->idle_cycle_time == 0) {
		dev_dbg(dev, "Overriding power cfg to free run\n");
		data->actv_cycle_time = 255;
		data->idle_cycle_time = 255;

		error = mxt_set_power_cfg(data, MXT_POWER_CFG_RUN);
		if (error)
			return error;
	}

	return 0;
}

static int mxt_probe_power_cfg(struct mxt_data *data)
{
	int error;

	error = mxt_read_power_cfg(data, &data->actv_cycle_time,
				   &data->idle_cycle_time);
	if (error)
		return error;

	/* If in deep sleep mode, attempt reset */
	if (data->actv_cycle_time == 0 || data->idle_cycle_time == 0) {
		error = mxt_soft_reset(data, MXT_RESET_VALUE);
		if (error)
			return error;

		error = mxt_check_power_cfg_post_reset(data);
		if (error)
			return error;
	}

	return 0;
}

static int mxt_check_reg_init(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
	int timeout_counter = 0;
	int ret;
	u8 command_register;
	u32 cfg_crc = 0xE4EA8C;

	ret = mxt_download_config(data, MXT_CFG_NAME);
	if (ret < 0)
		return ret;
	else if (ret == 2) {
		//Desmond ++ No config file, initializing config manually
		mxt_read_current_crc(data);
		mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] %s: cfg crc 0x%06X\n", __func__, data->config_crc);

		if(data->config_crc > 0) {
			mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] controller already have configuration\n");
			if(data->config_crc != cfg_crc) {
				mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] but config crc is invalid, initializing config\n");
				mxt_init_config(data);
				//return 0;
			}else
				return 0;
		}else {
			mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] controller have no configuration, initializing config\n");
			mxt_init_config(data);
		}
		//Desmond --
	}
	else if (ret == 0)
		/* CRC matched, or config parse failure
		 * - no need to reset */
		return 0;

	/* Backup to memory */
	mxt_write_object(data, MXT_GEN_COMMAND_T6,
			MXT_COMMAND_BACKUPNV,
			MXT_BACKUP_VALUE);
	msleep(MXT_BACKUP_TIME);

	do {
		ret =  mxt_read_object(data, MXT_GEN_COMMAND_T6,
					MXT_COMMAND_BACKUPNV,
					&command_register);
		if (ret)
			return ret;
		msleep(20);
	} while ((command_register != 0) && (timeout_counter++ <= 100));

	if (timeout_counter > 100) {
		dev_err(dev, "No response after backup!\n");
		return -EIO;
	}

	ret = mxt_soft_reset(data, MXT_RESET_VALUE);
	if (ret)
		return ret;

	ret = mxt_check_power_cfg_post_reset(data);
	if (ret)
		return ret;

	return 0;
}

static int mxt_read_info_block_crc(struct mxt_data *data)
{
	int ret;
	u16 offset;
	u8 buf[3];
	offset = MXT_OBJECT_START + MXT_OBJECT_SIZE * data->info.object_num;

	ret = mxt_read_reg(data->client, offset, sizeof(buf), buf);
	if (ret)
		return ret;

	data->info_block_crc = (buf[2] << 16) | (buf[1] << 8) | buf[0];
	return 0;
}

static int mxt_get_object_table(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	struct device *dev = &data->client->dev;
	int ret;
	int i;
	u16 end_address;
	u8 reportid = 0;
	u8 buf[data->info.object_num][MXT_OBJECT_SIZE];

	data->mem_size = 0;

	data->object_table = kcalloc(data->info.object_num,
				     sizeof(struct mxt_object), GFP_KERNEL);
	if (!data->object_table) {
		dev_err(dev, "Failed to allocate object table\n");
		return -ENOMEM;
	}

	ret = mxt_read_reg(client, MXT_OBJECT_START, sizeof(buf), buf);
	if (ret)
		goto free_object_table;

	for (i = 0; i < data->info.object_num; i++) {
		struct mxt_object *object = data->object_table + i;

		object->type = buf[i][0];
		object->start_address = (buf[i][2] << 8) | buf[i][1];
		object->size = buf[i][3] + 1;
		object->instances = buf[i][4] + 1;
		object->num_report_ids = buf[i][5];

		if (object->num_report_ids) {
			reportid += object->num_report_ids * object->instances;
			object->max_reportid = reportid;
			object->min_reportid = object->max_reportid -
				object->instances * object->num_report_ids + 1;
		}

		end_address = object->start_address
			+ object->size * object->instances - 1;

		if (end_address >= data->mem_size)
			data->mem_size = end_address + 1;

		/* save data for objects used when processing interrupts */
		switch (object->type) {
		case MXT_TOUCH_MULTI_T9:
			data->T9_reportid_max = object->max_reportid;
			data->T9_reportid_min = object->min_reportid;
			data->num_touchids = object->num_report_ids * object->instances;
			break;
		case MXT_GEN_COMMAND_T6:
			data->T6_reportid = object->max_reportid;
			break;
		case MXT_GEN_MESSAGE_T5:
			if (data->info.family_id == 0x80) {
				/* On mXT224 must read and discard CRC byte
				 * otherwise DMA reads are misaligned */
				data->T5_msg_size = object->size;
			} else {
				/* CRC not enabled, therefore don't read last byte */
				data->T5_msg_size = object->size - 1;
			}
			data->T5_address = object->start_address;
			break;
		case MXT_GEN_POWER_T7:
			data->T7_address = object->start_address;
			break;
		case MXT_TOUCH_KEYARRAY_T15:
			data->T15_reportid_max = object->max_reportid;
			data->T15_reportid_min = object->min_reportid;
			break;
		case MXT_PROCI_TOUCHSUPPRESSION_T42:
			data->T42_reportid_max = object->max_reportid;
			data->T42_reportid_min = object->min_reportid;
			break;
		case MXT_SPT_MESSAGECOUNT_T44:
			data->T44_address = object->start_address;
			break;
                case MXT_SPT_NOISESUPPRESSION_T48:
                        data->T48_reportid = object->max_reportid;
			break;
		case MXT_PROCI_ACTIVE_STYLUS_T63:
			data->T63_reportid_max = object->max_reportid;
			data->T63_reportid_min = object->min_reportid;
			data->num_stylusids =
				object->num_report_ids * object->instances;
			break;
		}

		mxt_debug(ATMEL_PAD_DEBUG_INFO, "[Touch][mxt1664s] T%u, start:%u size:%u instances:%u "
			"min_reportid:%u max_reportid:%u\n",
			object->type, object->start_address, object->size,
			object->instances,
			object->min_reportid, object->max_reportid);
	}

	/* Store maximum reportid */
	data->max_reportid = reportid;

	/* If T44 exists, T9 position has to be directly after */
	if (data->T44_address && (data->T5_address != data->T44_address + 1)) {
		dev_err(dev, "Invalid T44 position\n");
		ret = -EINVAL;
		goto free_object_table;
	}

	/* Allocate message buffer */
	data->msg_buf = kcalloc(data->max_reportid, data->T5_msg_size, GFP_KERNEL);
	if (!data->msg_buf) {
		dev_err(dev, "Failed to allocate message buffer\n");
		ret = -ENOMEM;
		goto free_object_table;
	}

	return 0;

free_object_table:
	kfree(data->object_table);
	return ret;
}

static int mxt_read_resolution(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	int error;
	unsigned int x_range, y_range;
	unsigned char orient;
	unsigned char val;

	/* Update matrix size in info struct */
	error = mxt_read_reg(client, MXT_MATRIX_X_SIZE, 1, &val);
	if (error)
		return error;
	data->info.matrix_xsize = val;

	error = mxt_read_reg(client, MXT_MATRIX_Y_SIZE, 1, &val);
	if (error)
		return error;
	data->info.matrix_ysize = val;

	/* Read X/Y size of touchscreen */
	error =  mxt_read_object(data, MXT_TOUCH_MULTI_T9,
			MXT_TOUCH_XRANGE_MSB, &val);
	if (error)
		return error;
	x_range = val << 8;

	error =  mxt_read_object(data, MXT_TOUCH_MULTI_T9,
			MXT_TOUCH_XRANGE_LSB, &val);
	if (error)
		return error;
	x_range |= val;

	error =  mxt_read_object(data, MXT_TOUCH_MULTI_T9,
			MXT_TOUCH_YRANGE_MSB, &val);
	if (error)
		return error;
	y_range = val << 8;

	error =  mxt_read_object(data, MXT_TOUCH_MULTI_T9,
			MXT_TOUCH_YRANGE_LSB, &val);
	if (error)
		return error;
	y_range |= val;

	error =  mxt_read_object(data, MXT_TOUCH_MULTI_T9,
			MXT_TOUCH_ORIENT, &orient);

	if (error)
		return error;

	/* Handle default values */
	if (x_range == 0)
		x_range = 1023;

	if (y_range == 0)
		y_range = 1023;

	if (orient & MXT_XY_SWITCH) {
		data->max_x = y_range;
		data->max_y = x_range;
	} else {
		data->max_x = x_range;
		data->max_y = y_range;
	}

	dev_info(&client->dev,
			"Matrix Size X%uY%u Touchscreen size X%uY%u\n",
			data->info.matrix_xsize, data->info.matrix_ysize,
			data->max_x, data->max_y);

	return 0;
}

static int mxt_initialize(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	struct mxt_info *info = &data->info;
	int error;
	u8 retry_count = 0;

retry_probe:
	/* Read info block */
	error = mxt_read_reg(client, 0, sizeof(*info), info);
	if (error) {
		error = mxt_probe_bootloader(data);
		if (error) {
			/* Chip is not in appmode or bootloader mode */
			return error;
		} else {
			if (++retry_count > 10) {
				dev_err(&client->dev,
					"Could not recover device from "
					"bootloader mode\n");
				data->state = BOOTLOADER;
				/* this is not an error state, we can reflash
				 * from here */
				return 0;
			}

			/* Tell bootloader to enter app mode. Ignore errors
			 * since we're in a retry loop */
			mxt_send_bootloader_cmd(data, false);
			msleep(MXT_FWRESET_TIME);
			goto retry_probe;
		}
	}

	dev_info(&client->dev,
		"Family ID: %d Variant ID: %d Version: %d.%d.%02X "
		"Object Num: %d\n",
		info->family_id, info->variant_id,
		info->version >> 4, info->version & 0xf,
		info->build, info->object_num);

	data->state = APPMODE;

	/* Get object table information */
	error = mxt_get_object_table(data);
	if (error) {
		dev_err(&client->dev, "Error %d reading object table\n", error);
		return error;
	}

	/* Read information block CRC */
	error = mxt_read_info_block_crc(data);
	if (error) {
		dev_err(&client->dev, "Error %d reading info block CRC\n", error);
	}

	error = mxt_probe_power_cfg(data);
	if (error) {
		dev_err(&client->dev, "Failed to initialize power cfg\n");
		return error;
	}

	/* Check register init values */
	error = mxt_check_reg_init(data);
	if (error) {
		dev_err(&client->dev, "Failed to initialize config\n");
		return error;
	}

	error = mxt_read_resolution(data);
	if (error) {
		dev_err(&client->dev, "Failed to initialize screen size\n");
		return error;
	}

	return 0;
}

static int mxt_check_firmware_format(struct device *dev, const struct firmware *fw)
{
	unsigned int pos = 0;
	char c;

	while (pos < fw->size) {
		c = *(fw->data + pos);

		if (c < '0' || (c > '9' && c < 'A') || c > 'F')
			return 0;

		pos++;
	}

	/* To convert file try
	 * xxd -r -p mXTXXX__APP_VX-X-XX.enc > maxtouch.fw */
	dev_err(dev, "Aborting: firmware file must be in binary format\n");

	return -1;
}

static int mxt_load_fw(struct device *dev, const char *fn)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	const struct firmware *fw = NULL;
	unsigned int frame_size;
	unsigned int pos = 0;
	unsigned int retry = 0;
	unsigned int frame = 0;
	int ret;

	ret = request_firmware(&fw, fn, dev);
	if (ret < 0) {
		dev_err(dev, "Unable to open firmware %s\n", fn);
		return ret;
	}

	/* Check for incorrect enc file */
	ret = mxt_check_firmware_format(dev, fw);
	if (ret)
		goto release_firmware;

	if (data->state != BOOTLOADER) {
		/* Change to the bootloader mode */
		ret = mxt_soft_reset(data, MXT_BOOT_VALUE);
		if (ret)
			goto release_firmware;

		ret = mxt_get_bootloader_address(data);
		if (ret)
			goto release_firmware;

		data->state = BOOTLOADER;
	}

	ret = mxt_check_bootloader(data, MXT_WAITING_BOOTLOAD_CMD);
	if (ret) {
		mxt_wait_for_chg(data);
		/* Bootloader may still be unlocked from previous update
		 * attempt */
		ret = mxt_check_bootloader(data, MXT_WAITING_FRAME_DATA);
		if (ret) {
			data->state = FAILED;
			goto release_firmware;
		}
	} else {
		dev_info(dev, "Unlocking bootloader\n");

		/* Unlock bootloader */
		ret = mxt_send_bootloader_cmd(data, true);
		if (ret) {
			data->state = FAILED;
			goto release_firmware;
		}
	}

	while (pos < fw->size) {
		mxt_wait_for_chg(data);
		ret = mxt_check_bootloader(data, MXT_WAITING_FRAME_DATA);
		if (ret) {
			data->state = FAILED;
			goto release_firmware;
		}

		frame_size = ((*(fw->data + pos) << 8) | *(fw->data + pos + 1));

		/* Take account of CRC bytes */
		frame_size += 2;

		/* Write one frame to device */
		ret = mxt_bootloader_write(data, fw->data + pos, frame_size);
		if (ret) {
			data->state = FAILED;
			goto release_firmware;
		}

		mxt_wait_for_chg(data);
		ret = mxt_check_bootloader(data, MXT_FRAME_CRC_PASS);
		if (ret) {
			retry++;

			/* Back off by 20ms per retry */
			msleep(retry * 20);

			if (retry > 20) {
				data->state = FAILED;
				goto release_firmware;
			}
		} else {
			retry = 0;
			pos += frame_size;
			frame++;
		}

		if (frame % 10 == 0)
			dev_info(dev, "Updated %d frames, %d/%zd bytes\n",
				 frame, pos, fw->size);

	}

	dev_info(dev, "Finished, sent %d frames, %zd bytes\n", frame, pos);

	data->state = INIT;

release_firmware:
	release_firmware(fw);
	return ret;
}

static ssize_t mxt_update_fw_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	int error;

	disable_irq(data->irq);

	error = mxt_load_fw(dev, MXT_FW_NAME);
	if (error) {
		dev_err(dev, "The firmware update failed(%d)\n", error);
		count = error;
	} else {
		dev_info(dev, "The firmware update succeeded\n");

		/* Wait for reset */
		msleep(MXT_FWRESET_TIME);

		kfree(data->object_table);
		data->object_table = NULL;
		kfree(data->msg_buf);
		data->msg_buf = NULL;

		mxt_initialize(data);
	}

	if (data->state == APPMODE) {
		enable_irq(data->irq);
	}

	return count;
}

static ssize_t mxt_version_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	int count = 0;

	count += sprintf(buf + count, "%d", data->info.version);
	count += sprintf(buf + count, "\n");

	return count;
}

static ssize_t mxt_build_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	int count = 0;

	count += sprintf(buf + count, "%d", data->info.build);
	count += sprintf(buf + count, "\n");

	return count;
}

static ssize_t mxt_fw_full_version_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct mxt_data *data = dev_get_drvdata(dev);

	return sprintf(buf, "%d.%d.%02X\n", data->info.version >> 4, data->info.version & 0xf, data->info.build);
}

static ssize_t mxt_pause_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	ssize_t count;
	char c;

	c = data->driver_paused ? '1' : '0';
	count = sprintf(buf, "%c\n", c);

	return count;
}

static ssize_t mxt_pause_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	int i;

	if (sscanf(buf, "%u", &i) == 1 && i < 2) {
		data->driver_paused = (i == 1);
		dev_dbg(dev, "%s\n", i ? "paused" : "unpaused");
		return count;
	} else {
		dev_dbg(dev, "pause_driver write error\n");
		return -EINVAL;
	}
}

static ssize_t mxt_debug_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	int count;
	char c;

	c = data->debug_enabled ? '1' : '0';
	count = sprintf(buf, "%c\n", c);

	return count;
}

static ssize_t mxt_debug_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	int i;

	if (sscanf(buf, "%u", &i) == 1 && i < 2) {
		data->debug_enabled = (i == 1);

		dev_dbg(dev, "%s\n", i ? "debug enabled" : "debug disabled");
		return count;
	} else {
		dev_dbg(dev, "debug_enabled write error\n");
		return -EINVAL;
	}
}

static int mxt_check_mem_access_params(struct mxt_data *data, loff_t off,
				       size_t *count)
{
	if (data->state != APPMODE) {
		dev_err(&data->client->dev, "Not in APPMODE\n");
		return -EINVAL;
	}

	if (off >= data->mem_size)
		return -EIO;

	if (off + *count > data->mem_size)
		*count = data->mem_size - off;

	if (*count > MXT_MAX_BLOCK_WRITE)
		*count = MXT_MAX_BLOCK_WRITE;

	return 0;
}

static ssize_t mxt_mem_access_read(struct file *filp, struct kobject *kobj,
	struct bin_attribute *bin_attr, char *buf, loff_t off, size_t count)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct mxt_data *data = dev_get_drvdata(dev);
	int ret = 0;

	ret = mxt_check_mem_access_params(data, off, &count);
	if (ret < 0)
		return ret;

	if (count > 0)
		ret = mxt_read_reg(data->client, off, count, buf);

	return ret == 0 ? count : ret;
}

static ssize_t mxt_mem_access_write(struct file *filp, struct kobject *kobj,
	struct bin_attribute *bin_attr, char *buf, loff_t off,
	size_t count)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct mxt_data *data = dev_get_drvdata(dev);
	int ret = 0;

	ret = mxt_check_mem_access_params(data, off, &count);
	if (ret < 0)
		return ret;

	if (count > 0)
		ret = mxt_write_block(data->client, off, count, buf);

	return ret == 0 ? count : 0;
}

//Desmond ++ Device Attributes
static ssize_t mxt_cfg_show(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	struct mxt_object *object;
	int error, i;
	int target_obj;
	u8 val;

	sscanf(buf, "%d\n", &target_obj);

	switch (target_obj){
		case MXT_DEBUG_DIAGNOSTIC_T37:

		case MXT_GEN_MESSAGE_T5:
		case MXT_GEN_COMMAND_T6:
		case MXT_GEN_POWER_T7:
		case MXT_GEN_ACQUIRE_T8:

		case MXT_TOUCH_MULTI_T9:
		case MXT_TOUCH_KEYARRAY_T15:

		case MXT_PROCI_ONETOUCH_T24:
		case MXT_PROCI_TWOTOUCH_T27:
		case MXT_PROCI_GRIP_T40:
		case MXT_PROCI_TOUCHSUPPRESSION_T42:
		case MXT_PROCI_STYLUS_T47:
		//case MXT_PROCG_NOISESUPPRESSION_T48:
		case MXT_PROCI_ADAPTIVETHRESHOLD_T55:
		case MXT_PROCI_SHIELDLESS_T56:
		case MXT_PROCI_EXTRATOUCHDATA_T57:

		case MXT_SPT_COMMSCONFIG_T18:
		case MXT_SPT_SELFTEST_T25:
		case MXT_SPT_USERDATA_T38:
		case MXT_SPT_DIGITIZER_T43:
		case MXT_SPT_MESSAGECOUNT_T44:
		case MXT_SPT_CTECONFIG_T46:

			object = mxt_get_object(data, target_obj);
			if (object) {
				for(i=0; i < object->size; i++) {
					error = mxt_read_object(data, target_obj, i, &val);
					printk("T%d byte[%d]\t= %d\n", target_obj, i, val);
				}
			}

			break;

		default:
			printk("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
			"Debug Objects\n",
			"\t# Diagnostic Debug\t\tT37\n",
			"General Objects\n",
			"\t# Message Processor\t\tT5\n",
			"\t# Command Processor\t\tT6\n",
			"\t# Power Configuration\t\tT7\n",
			"\t# Acquisition Configuration\tT8\n",
			"Touch Objects\n",
			"\t# Multiple Touch Touchscreen\tT9\n",
			"\t# Key Array\t\t\tT15\n",
			"Signal Processing Objects\n",
			"\t# One-touch Gesture\t\tT24\n",
			"\t# Two-touch Gesture\t\tT27\n",
			"\t# Grip Suppression\t\tT40\n",
			"\t# Touch Suppression\t\tT42\n",
			"\t# Stylus\t\t\tT47\n",
			"\t# Noise Suppression\t\tT48\n",
			"\t# Adaptive Threshold\t\tT55\n",
			"\t# Shieldless\t\t\tT56\n",
			"\t# Extra Touchscreen Data\tT57\n",
			"Support Objects\n",
			"\t# Communications Configuration\tT18\n",
			"\t# Self Test\t\t\tT25\n",
			"\t# User Data\t\t\t38\n",
			"\t# Digitizer HID Configuration\tT43\n",
			"\t# Message Count\t\t\tT44\n",
			"\t# CTE Configuration\t\tT46\n"
			);
			break;
	}

	return count;
}

static ssize_t mxt_cfg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mxt_data *data = dev_get_drvdata(dev);
	int error;
	int cfg[3];

	sscanf(buf, "%d%d%d\n", &cfg[0], &cfg[1], &cfg[2]);

	switch(cfg[0]){
		case MXT_GEN_COMMAND_T6:
			printk("[Touch][mxt1664s] set T%d[%d] = %d\n", cfg[0], cfg[1], cfg[2]);
			if (cfg[1] == MXT_COMMAND_RESET) {
				
				error = mxt_soft_reset(data, cfg[2]);
				if (error < 0)
					printk("[Touch][mxt1664s] soft reset failed\n");
			}else
				mxt_write_object(data, cfg[0], cfg[1], cfg[2]);
			break;
		case MXT_GEN_POWER_T7:
		case MXT_GEN_ACQUIRE_T8:
		case MXT_TOUCH_MULTI_T9:
			printk("[Touch][mxt1664s] set T%d[%d] = %d\n", cfg[0], cfg[1], cfg[2]);
			mxt_write_object(data, cfg[0], cfg[1], cfg[2]);
			break;
		default:
			printk("[Touch][mxt1664s] T%d is not support for write register\n", cfg[0]);
			break;
	}

	return count;
}

static ssize_t mxt_T7_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	struct mxt_data *data = dev_get_drvdata(dev);

	int error, i;
	u8 val;
	char tmpstr[800];

	for(i=0; i < 6; i++) {
		error = mxt_read_object(data, MXT_GEN_COMMAND_T6, MXT_COMMAND_RESET+i, &val);
		sprintf(tmpstr,"T6 byte[%d] = %d\n",i ,val);
		strncat (buf,tmpstr,strlen(tmpstr));
	}

	for(i=0; i < 3; i++) {
		error = mxt_read_object(data, MXT_GEN_POWER_T7,	MXT_POWER_IDLEACQINT+i, &val);
		sprintf(tmpstr,"T7 byte[%d] = %d\n",i ,val);
		strncat (buf,tmpstr,strlen(tmpstr));
	}

	for(i=0; i < 10; i++) {
		error = mxt_read_object(data, MXT_GEN_ACQUIRE_T8, MXT_ACQUIRE_CHRGTIME+i, &val);
		sprintf(tmpstr,"T8 byte[%d] = %d\n",i ,val);
		strncat (buf,tmpstr,strlen(tmpstr));
	}

	for(i=0; i < 35; i++) {
		error = mxt_read_object(data, MXT_TOUCH_MULTI_T9, MXT_TOUCH_CTRL+i, &val);
		sprintf(tmpstr,"T9 byte[%d] = %d\n",i ,val);
		strncat (buf,tmpstr,strlen(tmpstr));
	}

	return strlen(buf);
}

static ssize_t mxt_load_config(struct device *dev, struct device_attribute *attr, char *buf)
{	
	struct mxt_data *data = dev_get_drvdata(dev);
	mxt_init_config(data);
	return sprintf(buf, "load config to flash\n");
}

static ssize_t mxt_read_chg(struct device *dev, struct device_attribute *attr, char *buf)
{	
	struct mxt_data *data = dev_get_drvdata(dev);

	int error;

	error = mxt_read_t9_messages_until_invalid(data);
	if(error < 0) {
		mxt_debug(ATMEL_PAD_DEBUG_ERR, "[Touch][mxt1664s] CHG abnormal\n");
	}

	return strlen(buf);
}
//Desmond --

static DEVICE_ATTR(update_fw, S_IWUSR, NULL, mxt_update_fw_store);
static DEVICE_ATTR(debug_enable, S_IWUSR | S_IRUSR, mxt_debug_enable_show,
		   mxt_debug_enable_store);
static DEVICE_ATTR(pause_driver, S_IWUSR | S_IRUSR, mxt_pause_show,
		   mxt_pause_store);
static DEVICE_ATTR(version, S_IRUGO, mxt_version_show, NULL);
static DEVICE_ATTR(build, S_IRUGO, mxt_build_show, NULL);
//Desmond ++
static DEVICE_ATTR(show_cfg, S_IWUSR, NULL, mxt_cfg_show);
static DEVICE_ATTR(store_cfg, S_IWUSR, NULL, mxt_cfg_store);
static DEVICE_ATTR(T7, S_IRUGO, mxt_T7_show, NULL);
static DEVICE_ATTR(read_chg, S_IRUGO, mxt_read_chg, NULL);
static DEVICE_ATTR(fw_id, S_IRUGO, mxt_fw_full_version_show, NULL);
//Desmond --
static DEVICE_ATTR(load_cfg, S_IRUGO, mxt_load_config, NULL);

static struct attribute *mxt_attrs[] = {
	&dev_attr_update_fw.attr,
	&dev_attr_debug_enable.attr,
	&dev_attr_pause_driver.attr,
	&dev_attr_version.attr,
	&dev_attr_build.attr,
	//Desmond ++
	&dev_attr_show_cfg.attr,
	&dev_attr_store_cfg.attr,
	&dev_attr_T7.attr,
	&dev_attr_read_chg.attr,
	&dev_attr_fw_id.attr,
	//Desmond --
	&dev_attr_load_cfg.attr,
	NULL
};

static const struct attribute_group mxt_attr_group = {
	.attrs = mxt_attrs,
};

static void mxt_start(struct mxt_data *data)
{
	int error;
	//struct device *dev = &data->client->dev;

	if (data->is_stopped == 0)
		return;

	error = mxt_set_power_cfg(data, MXT_POWER_CFG_RUN);
	if (error)
		return;

	/* At this point, it may be necessary to clear state
	 * by disabling/re-enabling the noise suppression object */

	/* Recalibrate since chip has been in deep sleep */
	error = mxt_write_object(data, MXT_GEN_COMMAND_T6,
		MXT_COMMAND_CALIBRATE, 1);

	if (!error)
		mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] MXT started\n");
}

static void mxt_stop(struct mxt_data *data)
{
	int error;
	//struct device *dev = &data->client->dev;

	if (data->is_stopped)
		return;

	error = mxt_set_power_cfg(data, MXT_POWER_CFG_DEEPSLEEP);

	if (!error)
		mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] MXT suspended\n");
}

static int mxt_input_open(struct input_dev *dev)
{
	struct mxt_data *data = input_get_drvdata(dev);

	mxt_start(data);

	return 0;
}

static void mxt_input_close(struct input_dev *dev)
{
	struct mxt_data *data = input_get_drvdata(dev);

	mxt_stop(data);
}

static int mxt_initialize_input_device(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
	struct input_dev *input_dev;
	int ret;
	int key;

	/* Initialize input device */
	input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(dev, "Failed to allocate input device\n");
		return -ENOMEM;
	}

	if (data->pdata->input_name) {
		input_dev->name = data->pdata->input_name;
	} else {
		input_dev->name = "atmel-maxtouch";
	}

	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = dev;
	input_dev->open = mxt_input_open;
	input_dev->close = mxt_input_close;

	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(BTN_TOUCH, input_dev->keybit);

	/* For single touch */
	input_set_abs_params(input_dev, ABS_X,
			     0, data->max_x, 0, 0);
	input_set_abs_params(input_dev, ABS_Y,
			     0, data->max_y, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE,
			     0, 255, 0, 0);

	/* For multi touch */
	input_mt_init_slots(input_dev,
		data->num_touchids + data->num_stylusids);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR,
			     0, MXT_MAX_AREA, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X,
			     0, data->max_x, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
			     0, data->max_y, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE,
			     0, 255, 0, 0);
	//input_set_abs_params(input_dev, ABS_MT_ORIENTATION, 0, 255, 0, 0); //Desmond++

	/* For T63 active stylus */
	if (data->T63_reportid_min) {
		__set_bit(BTN_STYLUS, input_dev->keybit);
		__set_bit(BTN_STYLUS2, input_dev->keybit);

		input_set_abs_params(input_dev, ABS_MT_TOOL_TYPE,
			0, MT_TOOL_MAX, 0, 0);
	}

	/* For T15 key array */
	mxt_t15_keystatus = 0;
	for (key = 0; key < ARRAY_SIZE(mxt_t15_keys); key++) {
		input_set_capability(input_dev, EV_KEY, mxt_t15_keys[key]);
	}

	input_set_drvdata(input_dev, data);
	i2c_set_clientdata(data->client, data);

	ret = input_register_device(input_dev);
	if (ret) {
		dev_err(dev, "Error %d registering input device\n", ret);
		input_free_device(input_dev);
		return ret;
	}

	data->input_dev = input_dev;

	return 0;
}

static int __devinit mxt_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	const struct mxt_platform_data *pdata = client->dev.platform_data;
	struct mxt_data *data;
	int error;

	mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] %s++\n", __FUNCTION__);

	if (!pdata)
		return -EINVAL;

	data = kzalloc(sizeof(struct mxt_data), GFP_KERNEL);
	if (!data) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		return -ENOMEM;
	}

	data_bak = data; //Desmond ++

	data->state = INIT;

	data->client = client;
	data->pdata = pdata;
	data->irq = client->irq;

	//Desmond ++ move to attach pad work
#if 0
	if(0){
		/* Initialize i2c device */
		error = mxt_initialize(data);
		if (error)
			goto err_free_data;
	
		error = mxt_initialize_input_device(data);
		if (error)
			goto err_free_object;
	
		error = request_threaded_irq(client->irq, NULL, mxt_interrupt,
				IRQF_TRIGGER_FALLING | IRQF_SHARED, client->dev.driver->name, data);
		if (error) {
			dev_err(&client->dev, "Error %d registering irq\n", error);
			goto err_free_input_device;
		}
	}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	data->early_suspend.suspend = mxt_early_suspend;
	data->early_suspend.resume = mxt_late_resume;
	register_early_suspend(&data->early_suspend);
#endif

	error = sysfs_create_group(&client->dev.kobj, &mxt_attr_group);
	if (error) {
		dev_err(&client->dev, "Failure %d creating sysfs group\n",
			error);
		goto err_free_irq;
	}

	sysfs_bin_attr_init(&data->mem_access_attr);
	data->mem_access_attr.attr.name = "mem_access";
	data->mem_access_attr.attr.mode = S_IRUGO | S_IWUSR;
	data->mem_access_attr.read = mxt_mem_access_read;
	data->mem_access_attr.write = mxt_mem_access_write;
	data->mem_access_attr.size = data->mem_size;

	if (sysfs_create_bin_file(&client->dev.kobj,
				  &data->mem_access_attr) < 0) {
		dev_err(&client->dev, "Failed to create %s\n",
			data->mem_access_attr.attr.name);
		goto err_remove_sysfs_group;
	}

	//Desmond++ add for I2C_STRESS_TEST
#ifdef CONFIG_I2C_STRESS_TEST
	mxt_debug(ATMEL_PAD_DEBUG_INFO, "[Touch][mxt1664s] add i2c test case\n");
	i2c_add_test_case(client, "mxtTouch", ARRAY_AND_SIZE(gTouchTestCaseInfo));
#endif
	//Desmond-- add for I2C_STRESS_TEST

	mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] %s--\n", __FUNCTION__);

	return 0;

err_remove_sysfs_group:
	sysfs_remove_group(&client->dev.kobj, &mxt_attr_group);
err_free_irq:
	//free_irq(client->irq, data);
//err_free_input_device:
	//input_unregister_device(data->input_dev);
//err_free_object:
	//kfree(data->msg_buf);
	//kfree(data->object_table);
//err_free_data:
	kfree(data);
	return error;
}

static int __devexit mxt_remove(struct i2c_client *client)
{
	struct mxt_data *data = i2c_get_clientdata(client);

	sysfs_remove_bin_file(&client->dev.kobj, &data->mem_access_attr);
	sysfs_remove_group(&client->dev.kobj, &mxt_attr_group);
	free_irq(data->irq, data);
	input_unregister_device(data->input_dev);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif
	kfree(data->msg_buf);
	data->msg_buf = NULL;
	kfree(data->object_table);
	data->object_table = NULL;
	kfree(data);
	data = NULL;

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int mxt_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mxt_data *data = i2c_get_clientdata(client);
	struct input_dev *input_dev;

	//Desmond++ If not in pad, return
	if(g_isPadMode) {
		g_isPadSleep = 1; //Desmond++

		input_dev = data->input_dev;
		mutex_lock(&input_dev->mutex);

		if (input_dev->users)
			mxt_stop(data);

		mutex_unlock(&input_dev->mutex);
	}

	return 0;
}

static int mxt_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mxt_data *data = i2c_get_clientdata(client);
	struct input_dev *input_dev;

	//Desmond++ If not in pad, return
	if(g_isPadMode) {
		g_isPadSleep = 0; //Desmond++

		input_dev = data->input_dev;
		mutex_lock(&input_dev->mutex);

		if (input_dev->users)
			mxt_start(data);

		mutex_unlock(&input_dev->mutex);
	}
	
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mxt_early_suspend(struct early_suspend *es)
{
	struct mxt_data *mxt;
	mxt = container_of(es, struct mxt_data, early_suspend);

	//Desmond++ If not in pad, return
	if(g_isPadMode) {
		if (mxt_suspend(&mxt->client->dev) != 0)
			dev_err(&mxt->client->dev, "%s: failed\n", __func__);
	}
}

static void mxt_late_resume(struct early_suspend *es)
{
	struct mxt_data *mxt;
	mxt = container_of(es, struct mxt_data, early_suspend);

	//Desmond++ If not in pad, return
	if(g_isPadMode) {
		if (mxt_resume(&mxt->client->dev) != 0)
			dev_err(&mxt->client->dev, "%s: failed\n", __func__);
	}
}
#endif

#endif

static SIMPLE_DEV_PM_OPS(mxt_pm_ops, mxt_suspend, mxt_resume);

/* Desmond ++ Redundant functions
static void mxt_shutdown(struct i2c_client *client)
{
	struct mxt_data *data = i2c_get_clientdata(client);

	disable_irq(data->irq);
	data->state = SHUTDOWN;
}*/

static const struct i2c_device_id mxt_id[] = {
	{ "qt602240_ts", 0 },
	{ "atmel_mxt_pad_ts", 0 },
	{ "mXT224", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, mxt_id);

static struct i2c_driver mxt_driver = {
	.driver = {
		.name	= "atmel_mxt_pad_ts",
		.owner	= THIS_MODULE,
		.pm	= &mxt_pm_ops,
	},
	.probe		= mxt_probe,
	.remove		= __devexit_p(mxt_remove),
	//.shutdown	= mxt_shutdown,
	.id_table	= mxt_id,
};

//Desmond++
#ifdef CONFIG_EEPROM_NUVOTON
static int touch_mp_event(struct notifier_block *this, unsigned long event, void *ptr);

static struct notifier_block touch_mp_notifier = {
        .notifier_call = touch_mp_event,
        .priority = TOUCH_MP_NOTIFY,
};
#endif //CONFIG_EEPROM_NUVOTON
//Desmond--

static int __init mxt_init(void)
{
	int support_atmel;	//Desmond ++

	mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] %s\n", __func__);

	//Desmond ++ check the machine version
	switch(g_A68_hwID){
		// not support mxt1664s driver in all sku of A68
		case A68_EVB:
		case A68_SR1_1:
		case A68_SR1_2:
		case A68_SR2:
		case A68_ER1:
		case A68_ER2:
		case A68_ER3:
		case A68_PR:
		case A68_MP:
			support_atmel = 0;
			mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] not support mxt1664s driver. A68_%d\n", g_A68_hwID);
			break;

		// not support mxt1664s driver after sku SR4 of A80
		case A80_SR4:
		case A80_SR5:
		case A80_ER:
		case A80_PR:
			support_atmel = 0;
			mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] not support mxt1664s driver. A80_%d\n", g_A68_hwID);
			break;

		case A80_EVB:
		case A80_SR1:
		case A80_SR2:
		case A80_SR3:
	        	support_atmel = 1;
			mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] A80_%d\n", g_A68_hwID);
			break;

		default:
	                support_atmel = 0;
			mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] Unknow HWID\n");
			break;
	}

	//Desmond ++ disable 1664s driver, if not support
	if(!support_atmel)
		return 0;

	g_sis_wq_attach_detach = create_singlethread_workqueue("g_atmel_wq_attach_detach_pad");
	if (!g_sis_wq_attach_detach) {
		mxt_debug(ATMEL_PAD_DEBUG_TRACE, "[Touch][mxt1664s] %s: create workqueue failed: g_atmel_wq_attach_detach_pad\n", __func__);
	}

#ifdef CONFIG_EEPROM_NUVOTON
	INIT_WORK(&g_mp_attach_work, attach_padstation_work);
	INIT_WORK(&g_mp_detach_work, detach_padstation_work);

	register_microp_notifier(&touch_mp_notifier);
	notify_register_microp_notifier(&touch_mp_notifier, "atmel_mxt_pad_ts"); //ASUS_BSP Lenter+
#endif //CONFIG_EEPROM_NUVOTON

	return i2c_add_driver(&mxt_driver);
}

static void __exit mxt_exit(void)
{
	i2c_del_driver(&mxt_driver);
#ifdef CONFIG_EEPROM_NUVOTON 
	unregister_microp_notifier(&touch_mp_notifier);
	notify_unregister_microp_notifier(&touch_mp_notifier, "atmel_mxt_pad_ts"); //ASUS_BSP Lenter+
#endif //CONFIG_EEPROM_NUVOTON
}

module_init(mxt_init);
module_exit(mxt_exit);

//Desmond++
#ifdef CONFIG_EEPROM_NUVOTON
static void attach_padstation_work(struct work_struct *work)
{
	int error;

	mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] %s++\n", __func__);
	msleep(gap_time_1);

	if(data_bak) {
		/* Initialize*/
		error = mxt_initialize(data_bak);
		if(error) {
			mxt_debug(ATMEL_PAD_DEBUG_ERR, "[Touch][mxt1664s] device initialization failed, error=%d\n", error);
			goto err_virtual_remove;
		}

		if(!g_isIrqRequest) {
			error = mxt_initialize_input_device(data_bak);
			if (error) {
				mxt_debug(ATMEL_PAD_DEBUG_ERR, "[Touch][mxt1664s] input device initialization failed, error=%d\n", error);
				goto err_free_object;
			}
	
			error = request_threaded_irq(data_bak->client->irq, NULL, mxt_interrupt,
					IRQF_TRIGGER_FALLING | IRQF_SHARED, data_bak->client->dev.driver->name, data_bak);
			if (error) {
				mxt_debug(ATMEL_PAD_DEBUG_ERR, "[Touch][mxt1664s] Error %d registering irq\n", error);
				goto err_free_input_device;
			}
			
			mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] Request irq successfuuly\n");
			g_isIrqRequest = 1;
		}

		msleep(gap_time_2);

		error = mxt_read_t9_messages_until_invalid(data_bak);
		if(error < 0) {
			mxt_debug(ATMEL_PAD_DEBUG_ERR, "[Touch][mxt1664s] CHG abnormal\n");
			goto err_virtual_remove;
		}

		g_isPadMode = 1;

	}else {
		mxt_debug(ATMEL_PAD_DEBUG_ERR, "[Touch][mxt1664s] data_bak is not exist\n");
	}
	
	mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] %s--\n", __func__);

	return;

err_free_input_device:
	input_unregister_device(data_bak->input_dev);
err_free_object:
	kfree(data_bak->msg_buf);
	kfree(data_bak->object_table);
err_virtual_remove:
	/*TODO*/
	//Trigger Virtual Remove
	mxt_debug(ATMEL_PAD_DEBUG_ERR, "[Touch][mxt1664s] Trigger Virtual Remove\n");
}

static void detach_padstation_work(struct work_struct *work)
{
	mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] %s++\n", __func__);

	g_isPadMode = 0;
	g_isPadSleep = 0;

	mxt_debug(ATMEL_PAD_DEBUG_POWER, "[Touch][mxt1664s] %s--\n", __func__);
	return;
}

static int touch_attach_padstation_pad(void)
{
	mxt_debug(ATMEL_PAD_DEBUG_TRACE, "[Touch][mxt1664s] touch_attach_padstation_pad()++\n");

	queue_work(g_sis_wq_attach_detach, &g_mp_attach_work);

	mxt_debug(ATMEL_PAD_DEBUG_TRACE, "[Touch][mxt1664s] touch_attach_padstation_pad()--\n");

	return 0;
}
//EXPORT_SYMBOL(touch_attach_padstation_pad);

static int touch_detach_padstation_pad(void)
{
	mxt_debug(ATMEL_PAD_DEBUG_TRACE, "[Touch][mxt1664s] touch_detach_padstation_pad()++\n");

	queue_work(g_sis_wq_attach_detach, &g_mp_detach_work);

	mxt_debug(ATMEL_PAD_DEBUG_TRACE, "[Touch][mxt1664s] touch_detach_padstation_pad()--\n");

	return 0;
}
//EXPORT_SYMBOL(touch_detach_padstation_pad);

static int touch_mp_event(struct notifier_block *this, unsigned long event, void *ptr)
{

        switch (event) {

        case P01_ADD:
                mxt_debug(ATMEL_PAD_DEBUG_TRACE, "[Touch][mxt1664s] P05_ADD++\r\n");

                touch_attach_padstation_pad();

                mxt_debug(ATMEL_PAD_DEBUG_TRACE, "[Touch][mxt1664s] P05_ADD--\r\n");

                return NOTIFY_DONE;

        case P01_REMOVE:
                mxt_debug(ATMEL_PAD_DEBUG_TRACE, "[Touch][mxt1664s] P05_REMOVE++\r\n");

                touch_detach_padstation_pad();

                mxt_debug(ATMEL_PAD_DEBUG_TRACE, "[Touch][mxt1664s] P05_REMOVE--\r\n");

                return NOTIFY_DONE;

        default:
                return NOTIFY_DONE;
        }
}
#endif //CONFIG_EEPROM_NUVOTON
//Desmond--

/* Module information */
MODULE_AUTHOR("Joonyoung Shim <jy0922.shim@samsung.com>");
MODULE_DESCRIPTION("Atmel maXTouch Touchscreen driver");
MODULE_LICENSE("GPL");
