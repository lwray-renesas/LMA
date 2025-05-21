#include "LMA_Port.h"
#include "LMA_Core.h"
#include "iodefine.h"
#include "math.h"

#include "r_cg_macrodriver.h"
#include "r_cg_tau.h"
#include "r_cg_dsadc.h"
#include "r_cg_rtc.h"

float LMA_AccToFloat(acc_t acc)
{
    return (float)acc;
}

float LMA_FPMul_Fast(float a, float b)
{
	return a * b;
}

float LMA_FPDiv_Fast(float a, float b)
{
    return a / b;
}

float LMA_FPSqrt_Fast(float a)
{
    return sqrt(a);
}

float LMA_FPAbs_Fast(float a)
{
    uint32_t tmp = *(uint32_t*)(&a) & 0x7FFFFFFF;
    return *(float*)&tmp;
}

void LMA_AccReset(LMA_Workspace *const p_ws, const uint32_t phase_id)
{
    (void) phase_id;

    /* Signed multiplication mode*/
	MULC = 0x40U;

	/* Irms*/
	MUL32SL = *((uint16_t *) &(p_ws->samples.current));
	MUL32SH = *(((uint16_t *) &(p_ws->samples.current)) + 1);
	MULBL = *((uint16_t *) &(p_ws->samples.current));
	MULBH = *(((uint16_t *) &(p_ws->samples.current)) + 1);
	__nop();
	__nop();
	*((uint8_t *) &(p_ws->accs.iacc)) = MULR0;
	__nop();
	*(((uint8_t *) &(p_ws->accs.iacc)) + 1) = MULR1;
	*(((uint8_t *) &(p_ws->accs.iacc)) + 2) = MULR2;
	*(((uint8_t *) &(p_ws->accs.iacc)) + 3) = MULR3;


	/* P*/
	MULBL = *((uint16_t *) &(p_ws->samples.voltage));
	MULBH = *(((uint16_t *) &(p_ws->samples.voltage)) + 1);
	__nop();
	__nop();
	*((uint8_t *) &(p_ws->accs.pacc)) = MULR0;
	__nop();
	*(((uint8_t *) &(p_ws->accs.pacc)) + 1) = MULR1;
	*(((uint8_t *) &(p_ws->accs.pacc)) + 2) = MULR2;
	*(((uint8_t *) &(p_ws->accs.pacc)) + 3) = MULR3;

	/* Q*/
	MULBL = *((uint16_t *) &(p_ws->samples.voltage90));
	MULBH = *(((uint16_t *) &(p_ws->samples.voltage90)) + 1);
	__nop();
	__nop();
	*((uint8_t *) &(p_ws->accs.qacc)) = MULR0;
	__nop();
	*(((uint8_t *) &(p_ws->accs.qacc)) + 1) = MULR1;
	*(((uint8_t *) &(p_ws->accs.qacc)) + 2) = MULR2;
	*(((uint8_t *) &(p_ws->accs.qacc)) + 3) = MULR3;

	/* Vrms*/
	MUL32SL = *((uint16_t *) &(p_ws->samples.voltage));
	MUL32SH = *(((uint16_t *) &(p_ws->samples.voltage)) + 1);
	MULBL = *((uint16_t *) &(p_ws->samples.voltage));
	MULBH = *(((uint16_t *) &(p_ws->samples.voltage)) + 1);
	__nop();
	__nop();
	*((uint8_t *) &(p_ws->accs.vacc)) = MULR0;
	__nop();
	*(((uint8_t *) &(p_ws->accs.vacc)) + 1) = MULR1;
	*(((uint8_t *) &(p_ws->accs.vacc)) + 2) = MULR2;
	*(((uint8_t *) &(p_ws->accs.vacc)) + 3) = MULR3;

    p_ws->accs.sample_count = 1;
}

void LMA_AccRun(LMA_Workspace *const p_ws, const uint32_t phase_id)
{
    (void) phase_id;

	MULC = 0xC0U;

	/* Irms*/
	MULR0 = *((uint8_t *) &(p_ws->accs.iacc));
	MULR1 = *(((uint8_t *) &(p_ws->accs.iacc)) + 1);
	MULR2 = *(((uint8_t *) &(p_ws->accs.iacc)) + 2);
	MULR3 = *(((uint8_t *) &(p_ws->accs.iacc)) + 3);

	MAC32SL = *((uint16_t *) &(p_ws->samples.current));
	MAC32SH = *(((uint16_t *) &(p_ws->samples.current)) + 1);
	MULBL = *((uint16_t *) &(p_ws->samples.current));
	MULBH = *(((uint16_t *) &(p_ws->samples.current)) + 1);
	__nop();
	__nop();
	*((uint8_t *) &(p_ws->accs.iacc)) = MULR0;
	__nop();
	*(((uint8_t *) &(p_ws->accs.iacc)) + 1) = MULR1;
	*(((uint8_t *) &(p_ws->accs.iacc)) + 2) = MULR2;
	*(((uint8_t *) &(p_ws->accs.iacc)) + 3) = MULR3;


	/* P*/
	MULR0 = *((uint8_t *) &(p_ws->accs.pacc));
	MULR1 = *(((uint8_t *) &(p_ws->accs.pacc)) + 1);
	MULR2 = *(((uint8_t *) &(p_ws->accs.pacc)) + 2);
	MULR3 = *(((uint8_t *) &(p_ws->accs.pacc)) + 3);

	MULBL = *((uint16_t *) &(p_ws->samples.voltage));
	MULBH = *(((uint16_t *) &(p_ws->samples.voltage)) + 1);
	__nop();
	__nop();
	*((uint8_t *) &(p_ws->accs.pacc)) = MULR0;
	__nop();
	*(((uint8_t *) &(p_ws->accs.pacc)) + 1) = MULR1;
	*(((uint8_t *) &(p_ws->accs.pacc)) + 2) = MULR2;
	*(((uint8_t *) &(p_ws->accs.pacc)) + 3) = MULR3;

	/* Q*/
	MULR0 = *((uint8_t *) &(p_ws->accs.qacc));
	MULR1 = *(((uint8_t *) &(p_ws->accs.qacc)) + 1);
	MULR2 = *(((uint8_t *) &(p_ws->accs.qacc)) + 2);
	MULR3 = *(((uint8_t *) &(p_ws->accs.qacc)) + 3);

	MULBL = *((uint16_t *) &(p_ws->samples.voltage90));
	MULBH = *(((uint16_t *) &(p_ws->samples.voltage90)) + 1);
	__nop();
	__nop();
	*((uint8_t *) &(p_ws->accs.qacc)) = MULR0;
	__nop();
	*(((uint8_t *) &(p_ws->accs.qacc)) + 1) = MULR1;
	*(((uint8_t *) &(p_ws->accs.qacc)) + 2) = MULR2;
	*(((uint8_t *) &(p_ws->accs.qacc)) + 3) = MULR3;

	/* Vrms*/
	MULR0 = *((uint8_t *) &(p_ws->accs.vacc));
	MULR1 = *(((uint8_t *) &(p_ws->accs.vacc)) + 1);
	MULR2 = *(((uint8_t *) &(p_ws->accs.vacc)) + 2);
	MULR3 = *(((uint8_t *) &(p_ws->accs.vacc)) + 3);

	MAC32SL = *((uint16_t *) &(p_ws->samples.voltage));
	MAC32SH = *(((uint16_t *) &(p_ws->samples.voltage)) + 1);
	MULBL = *((uint16_t *) &(p_ws->samples.voltage));
	MULBH = *(((uint16_t *) &(p_ws->samples.voltage)) + 1);
	__nop();
	__nop();
	*((uint8_t *) &(p_ws->accs.vacc)) = MULR0;
	__nop();
	*(((uint8_t *) &(p_ws->accs.vacc)) + 1) = MULR1;
	*(((uint8_t *) &(p_ws->accs.vacc)) + 2) = MULR2;
	*(((uint8_t *) &(p_ws->accs.vacc)) + 3) = MULR3;

    ++p_ws->accs.sample_count;
}

void LMA_AccLoad(LMA_Workspace *const p_ws, LMA_Accumulators *const p_accs, const uint32_t phase_id)
{
    (void)phase_id;
    p_accs->iacc = p_ws->accs.iacc;
    p_accs->pacc = p_ws->accs.pacc;
    p_accs->qacc = p_ws->accs.qacc;
    p_accs->vacc = p_ws->accs.vacc;

    p_accs->sample_count = p_ws->accs.sample_count;
}

spl_t LMA_PhaseShift90(spl_t new_voltage)
{
    static spl_t voltage_buffer[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static uint8_t buffer_index = 0;

    uint8_t buffer_index_19 = buffer_index + 2;
    uint8_t buffer_index_20 = buffer_index + 1;

    if(buffer_index_19 > 21)
    {
        buffer_index_19 -= 21;
    }

    if(buffer_index_20 > 21)
    {
        buffer_index_20 -= 21;
    }

    /* Append new voltage*/
    voltage_buffer[buffer_index] = new_voltage;

    /* Interpolate 19.53 samples - just take the mid point*/
    int32_t interpolated_value =
            ((voltage_buffer[buffer_index_19] * 60) >> 7) + ((voltage_buffer[buffer_index_20] * 68) >> 7);

    buffer_index = buffer_index_20;

    /* Convert back to its 32b value*/
    return interpolated_value;
}

void LMA_ADC_Init(void)
{
	/* Create called at system init*/
}

void LMA_ADC_Start(void)
{
	R_DSADC_Set_OperationOn();
	R_DSADC_Start();
}

void LMA_ADC_Stop(void)
{
	R_DSADC_Set_OperationOff();
	R_DSADC_Stop();
}

void LMA_TMR_Init(void)
{
	/* Create called at system init*/
}

void LMA_TMR_Start(void)
{
	R_TAU0_Channel0_Start();
}

void LMA_TMR_Stop(void)
{
	R_TAU0_Channel0_Stop();
}

void LMA_RTC_Init(void)
{
	/* Create called at system init*/
}

void LMA_RTC_Start(void)
{
	R_RTC_Start();
}

void LMA_RTC_Stop(void)
{
    /* TODO: Populate*/
}

void LMA_IMP_ActiveOn(void)
{
    P1_bit.no2 = 1U;
}

void LMA_IMP_ActiveOff(void)
{
	P1_bit.no2 = 0U;
}

void LMA_IMP_ReactiveOn(void)
{
    /* TODO: Populate*/
}

void LMA_IMP_ReactiveOff(void)
{
    /* TODO: Populate*/
}

void LMA_IMP_ApparentOn(void)
{
    /* TODO: Populate*/
}

void LMA_IMP_ApparentOff(void)
{
    /* TODO: Populate*/
}
