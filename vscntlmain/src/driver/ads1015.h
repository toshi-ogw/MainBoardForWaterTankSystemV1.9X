/*
 * ads1015.h
 *
 *  Created on: 2015/10/21
 *      Author: toshio
 */

#ifndef ADS1015_H_
#define ADS1015_H_

#include "i2c.h"

#define	ADS1015_DEVICEID	0x48

#define	ADS1015_CONVERSION_REGISTER	0
#define	ADS1015_CONFIGN_REGISTER	1
#define	ADS1015_LO_THRESH_REGISTER	2
#define	ADS1015_HI_THRESH_REGISTER	3

#define	ADS1015_POINTER_REGISTER	0


#define	ADS1015_CONFIG_BEGIN_CONV		(1	<< 15)
#define	ADS1015_CONFIG_PERFOMMING		(0	<< 15)

#define	ADS1015_CONFIG_AIN0_AIN1		(0	<< 12)
#define	ADS1015_CONFIG_AIN0_AIN3		(1	<< 12)
#define	ADS1015_CONFIG_AIN1_AIN3		(2	<< 12)
#define	ADS1015_CONFIG_AIN2_AIN3		(3	<< 12)
#define	ADS1015_CONFIG_AIN0_GND			(4	<< 12)
#define	ADS1015_CONFIG_AIN1_GND			(5	<< 12)
#define	ADS1015_CONFIG_AIN2_GND			(6	<< 12)
#define	ADS1015_CONFIG_AIN3_GND			(7	<< 12)

#define	ADS1015_CONFIG_FS_6144			(0	<< 	9)
#define	ADS1015_CONFIG_FS_4096			(1	<< 	9)
#define	ADS1015_CONFIG_FS_2048			(2	<< 	9)
#define	ADS1015_CONFIG_FS_1024			(3	<< 	9)
#define	ADS1015_CONFIG_FS_0512			(4	<< 	9)
#define	ADS1015_CONFIG_FS_0256			(5	<< 	9)

#define	ADS1015_CONFIG_CONT_MODE		(0  <<  8)
#define	ADS1015_CONFIG_SINGLE_MODE		(1  <<  8)

#define	ADS1015_CONFIG_FS_128SPS		(0	<< 	5)
#define	ADS1015_CONFIG_FS_250SPS		(1	<< 	5)
#define	ADS1015_CONFIG_FS_490SPS		(2	<< 	5)
#define	ADS1015_CONFIG_FS_920SPS		(3	<< 	5)
#define	ADS1015_CONFIG_FS_1600SPS		(4	<< 	5)
#define	ADS1015_CONFIG_FS_2400SPS		(5	<< 	5)
#define	ADS1015_CONFIG_FS_3300SPS		(6	<< 	5)

#define	ADS1015_CONFIG_TRAD_COMP		(0  <<  4)
#define	ADS1015_CONFIG_WIND_COMP		(1  <<  4)

#define	ADS1015_CONFIG_ACTIVE_LO		(0  <<  3)
#define	ADS1015_CONFIG_ACTIVE_HI		(1  <<  3)

#define	ADS1015_CONFIG_NON_LATCH_COMP	(0  <<  2)
#define	ADS1015_CONFIG_LATCH_COMP		(1  <<  2)

#define	ADS1015_CONFIG_ASSERT_AFTER_1	0
#define	ADS1015_CONFIG_ASSERT_AFTER_2	1
#define	ADS1015_CONFIG_ASSERT_AFTER_4	2
#define	ADS1015_CONFIG_DSIABLE_COMP		3

typedef union {
	struct {
		uint16_t	comp_que:2;
		uint16_t	comp_lat:1;
		uint16_t	comp_pol:1;
		uint16_t	comp_mode:1;
		uint16_t	dr:3;
		uint16_t	mode:1;
		uint16_t	pga:3;
		uint16_t	mux:3;
		uint16_t	os:1;

	} bit;
	uint16_t	value;

} ADS1015_CONFIG, *PADS1015_CONFIG;

int Read_Adc(uint8_t reg, uint16_t *pvalue);
int Write_Adc(uint8_t reg, uint16_t value);
int16_t StartConversion_Adc(uint16_t config);

#define	READ_ADS1015_CONVERSION_REGISTE(pvalue)	Read_Adc(ADS1015_CONVERSION_REGISTER, pvalue)
#define	READ_ADS1015_CONFIGN_REGISTER(pvalue)	Read_Adc(ADS1015_CONFIGN_REGISTER, pvalue)
#define	READ_ADS1015_LO_THRESH_REGISTER(pvalue)	Read_Adc(ADS1015_LO_THRESH_REGISTER, pvalue)
#define	READ_ADS1015_HI_THRESH_REGISTER(pvalue)	Read_Adc(ADS1015_HI_THRESH_REGISTER, pvalue)

#define	WRITE_ADS1015_CONVERSION_REGISTE(value)	Write_Adc(ADS1015_CONVERSION_REGISTER, value)
#define	WRITE_ADS1015_CONFIGN_REGISTER(value)	Write_Adc(ADS1015_CONFIGN_REGISTER, value)
#define	WRITE_ADS1015_LO_THRESH_REGISTER(value)	Write_Adc(ADS1015_LO_THRESH_REGISTER, value)
#define	WRITE_ADS1015_HI_THRESH_REGISTER(value)	Write_Adc(ADS1015_HI_THRESH_REGISTER, value)

#endif /* ADS1015_H_ */
