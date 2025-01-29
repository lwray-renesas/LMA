# LMA
A Light Weight AC Metrology Framework written completely in C (C99) targetting 32bit MCU's.

## Introduction
TODO: Top level intro - go into typical initialisation (main.c)

```
#include "LMA_Core.h"

/* Configuration required for configuring the library 4500 ws/imp = 800 imp/kwh*/
static LMA_Config config =
{
 .gcalib =
 {
  .fs = 3906.25f,
  .fline_coeff = 97650.0000f
 },
 .update_interval = 25,
 .fline_target = 50.00f,
 .fline_tol_low = 25.00f,
 .fline_tol_high = 75.00f,
 .meter_constant = 4500.00f,
 .no_load_i = 0.01f,
 .no_load_p = 2.00f,
 .v_sag = 50.00f,
 .v_swell = 280.00f
};

LMA_PhaseCalibration default_calib =
{
 .vrms_coeff = 21177.2051f,
 .irms_coeff = 53685.3828f,
 .p_coeff = 1136906368.0000f
};

/* Define our system energy struct*/
LMA_SystemEnergy system_energy =
{
 .impulse =
 {
  .led_on_count = 39, /* 10ms at 3906 Hz sampling frequency*/
  .active_counter = 0,
  .reactive_counter = 0,
  .active_on = false,
  .reactive_on = false
 }
};

/* Now define our phase pointing to our data structure*/
LMA_Phase phase;

void main(void)
{
    /* Initialise the framework*/
    LMA_Init(&config);

    /* Set system energy structure*/
    LMA_EnergySet(&system_energy);

    /* Initialise the phase(s)*/
    LMA_PhaseRegister(&phase);

    /* Load calibration data*/
    LMA_PhaseLoadCalibration(&phase, &default_calib);

    /* Start the ADC threads*/
    LMA_Start();

    while(1)
    {
		/* Run Application*/
    }
}

void ADC_Callback(adc_callback_args_t *p_args)
{
    phase.ws.samples.voltage = (spl_t)ADC_RESULT_REGISTER0;
    phase.ws.samples.current = (spl_t)ADC_RESULT_REGISTER1;
    phase.ws.samples.voltage90 = LMA_PhaseShift90(phase.ws.samples.voltage);

    LMA_CB_ADC();
}

void TMR_Callback(void)
{
	/* Periodic interrupt event*/
    LMA_CB_TMR();
}

void RTC_Callback(void)
{
    /* Periodic interrupt event */
    LMA_CB_RTC();
}
```

## Architecture
TODO: System level explanation of how to programatically construct a meter.

## Drivers
TODO: Description of drivers required and each ones responsibilities.

## Computations
TODO: The signal chain for each measured metric.

## Impulse Control
TODO: Explain the impulse control

