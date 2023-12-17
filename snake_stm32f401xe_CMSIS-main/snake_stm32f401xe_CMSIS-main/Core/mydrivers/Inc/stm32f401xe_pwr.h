/*
 * stm32401xe_pwr.h
 *
 *  Created on: 27 sty 2022
 *      Author: ROJEK
 */
#include "stm32f401xe.h"

#ifndef MYDRIVERS_INC_STM32F401XE_PWR_H_
#define MYDRIVERS_INC_STM32F401XE_PWR_H_

typedef enum PvdThresholdLevel_t
{
	kPvdLevel22V,
	kPvdLevel23V,
	kPvdLevel24V,
	kPvdLevel25V,
	kPvdLevel26V,
	kPvdLevel27V,
	kPvdLevel28V,
	kPvdLevel29V

} PvdThresholdLevel_t;

typedef enum PvdMode_t
{
	kPvdModeNormal,
	kPvdModeIrqRT,
	kPvdModeIrqFT
} PvdMode_t;

typedef enum PwrEntry_t
{
	kWFI,
	kWFE
} PwrEntry_t;

// there are multiple choices for what has to be disabled during stop mode
// bits that are set/reset
// MRLV - main regulator low voltage
// LPLV - low-power regulator low voltage
// FPDS - flash power-down in stop mode
// LPDS - low-power deep sleep

typedef enum StopModes_t
{
	kStopMR,
	kStopMRFPD,
	kStopLP,
	kStopLPFPD,
	kStopMRLV,
	kStopLPLV
} StopModes_t;

void Pwr_EnablePvd(PvdThresholdLevel_t pvd_level, PvdMode_t mode);
void Pwr_EnterSleepMode(PwrEntry_t entry);
void Pwr_EnterStopMode(PwrEntry_t entry, StopModes_t stop_mode);
#endif /* MYDRIVERS_INC_STM32F401XE_PWR_H_ */
