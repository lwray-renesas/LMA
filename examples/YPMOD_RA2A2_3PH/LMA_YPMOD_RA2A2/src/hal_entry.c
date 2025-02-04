#include <Benchmark/Benchmark.h>
#include "hal_data.h"
#include "LMA_Core.h"

FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event);
FSP_CPP_FOOTER

/* Configuration required for configuring the library 4500 ws/imp = 800 imp/kwh (3,600,000 = [ws/imp] * [kwh/imp])*/
static LMA_Config config =
{
 .gcalib =
 {
  .fs = 3906.25f,
  .fline_coeff = 97650.0000f,
  .deg_per_sample = 4.608f
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

/* Calibration data to load at startup*/
static LMA_PhaseCalibration default_calib =
{
 .vrms_coeff = 21177.2051f,
 .irms_coeff = 53685.3828f,
 .vi_phase_correction = 0.0f,
 .p_coeff = 1136906368.0000f
};

/* Define our system energy struct*/
static LMA_SystemEnergy system_energy =
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
static LMA_Phase phase;

/* Utility structs for data output*/
static LMA_Measurements measurements;
static LMA_ConsumptionData energy_consumed;

/* Benchmarking*/
static benchmark_t benchmark;

void hal_entry(void)
{
    /* Turn the MACL on*/
    R_MSTP->MSTPCRC_b.MSTPC15 = 0U;
    R_MACL->MULC_b.MULSM = 1;

    /* Init benchmarking system*/
    Benchmark_init(&benchmark, 5);

    /* Initialise the framework*/
    LMA_Init(&config);

    /* Set system energy structure*/
    LMA_EnergySet(&system_energy);

    /* Initialise the phase(s)*/
    LMA_PhaseRegister(&phase);

    /* Load calibration data*/
    LMA_PhaseLoadCalibration(&phase, &default_calib);

    /* Adjust phase correction for SDADC on RA2A2*/
    if(phase.calib.vi_phase_correction >= 0)
    {
        /* I leads V, so I need to be delayed*/
        /* 0.012 comes from HW UM 31.1.5*/
        R_SDADC_B->SDADPHCR0 = (uint16_t) (phase.calib.vi_phase_correction / 0.012f);
        R_SDADC_B->SDADPHCR4 = 0;
    }
    else
    {
        /* I lags V, so V need to be delayed*/
        /* 0.012 comes from HW UM 31.1.5*/
        R_SDADC_B->SDADPHCR0 = 0;
        R_SDADC_B->SDADPHCR4 = (uint16_t) ((phase.calib.vi_phase_correction * -1.00f) / 0.012f);
    }

    /* Start the ADC threads*/
    LMA_Start();

    while(1)
    {
        Benchmark_run(&benchmark);

        /* Check if there are measurements ready, if so read them and print to screen.*/
        if(LMA_MeasurementsReady(&phase))
        {
            /* Get current snapshot of measurements*/
            LMA_MeasurementsGet(&phase, &measurements);

            /* Get current snapshot of system energy*/
            LMA_EnergyGet(&system_energy);

            /* Convert system energy to energy consumed*/
            LMA_ConsumptionDataGet(&system_energy, &energy_consumed);
        }
    }
}

/*******************************************************************************************************************//**
 * This function is called at various points during the startup process.  This implementation uses the event that is
 * called right before main() to set up the pins.
 *
 * @param[in]  event    Where at in the start up process the code is currently at
 **********************************************************************************************************************/
void R_BSP_WarmStart(bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_RESET == event)
    {
#if BSP_FEATURE_FLASH_LP_VERSION != 0

        /* Enable reading from data flash. */
        R_FACI_LP->DFLCTL = 1U;

        /* Would normally have to wait tDSTOP(6us) for data flash recovery. Placing the enable here, before clock and
         * C runtime initialization, should negate the need for a delay since the initialization will typically take more than 6us. */
#endif
    }

    if (BSP_WARM_START_POST_C == event)
    {
        /* C runtime environment and system clocks are setup. */

        /* Configure pins. */
        R_IOPORT_Open (&IOPORT_CFG_CTRL, &IOPORT_CFG_NAME);
    }
}

void DSADC_Callback(adc_callback_args_t *p_args)
{
    (void)p_args;

    Benchmark_work_begin(&benchmark);
    phase.ws.samples.voltage = (spl_t)R_SDADC_B->SDADCR4;
    phase.ws.samples.current = (spl_t)R_SDADC_B->SDADCR0;
    phase.ws.samples.voltage90 = LMA_PhaseShift90(phase.ws.samples.voltage);
    LMA_CB_ADC();
    Benchmark_work_end(&benchmark);
}

/* Callback function */
void TMR_Callback(timer_callback_args_t *p_args)
{
    (void)p_args;

    Benchmark_work_begin(&benchmark);
    LMA_CB_TMR();
    Benchmark_work_end(&benchmark);

    /* We'll use the computation timer as the benchmarking tick*/
    Benchmark_cb_period(&benchmark);
}

void RTC_Callback(rtc_callback_args_t *p_args)
{
    /* Handle the RTC event */
    switch (p_args->event)
    {
        /* Alarm 0 interrupt */
        case RTC_EVENT_ALARM_IRQ:
        {
            break;
        }
        /* Alarm 1 interrupt */
        case RTC_EVENT_ALARM1_IRQ:
        {
            break;
        }
        /* Periodic interrupt event */
        case RTC_EVENT_PERIODIC_IRQ:
        {
            LMA_CB_RTC();
            break;
        }
        default:
        {
            break;
        }
    }
}
