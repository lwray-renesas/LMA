/***********************************************************************/
/*                                                                     */
/*  FILE        :Main.c                                                */
/*  DATE        :                                                      */
/*  DESCRIPTION :Main Program                                          */
/*  CPU TYPE    :                                                      */
/*                                                                     */
/*  NOTE:THIS IS A TYPICAL EXAMPLE.                                    */
/*                                                                     */
/***********************************************************************/

#include "r_cg_macrodriver.h"
#include "r_cg_sau.h"

#include "LMA_Core.h"


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
LMA_Phase phase;
LMA_Phase neutral;

/* Utility structs for data output*/
static LMA_Measurements measurements;
static LMA_ConsumptionData energy_consumed;

void main(void)
{
	EI();

    /* Initialise the framework*/
    LMA_Init(&config);

    /* Set system energy structure*/
    LMA_EnergySet(&system_energy);

    /* Initialise the phase(s)*/
    LMA_PhaseRegister(&phase);
    LMA_PhaseRegister(&neutral);

    /* Load calibration data*/
    LMA_PhaseLoadCalibration(&phase, &default_calib);
    LMA_PhaseLoadCalibration(&neutral, &default_calib);

    /* Adjust phase correction for SDADC on RL78/I1C*/
    if(phase.calib.vi_phase_correction >= 0)
    {
        /* I leads V, so I (SDADC0) needs to be delayed*/
        /* 0.012 comes from HW UM 31.1.5 - sampling frequency of 3906.25Hz and target AC line frequency of 50Hz*/
    	DSADPHCR0 = (uint16_t) (phase.calib.vi_phase_correction / 0.012f);
    	DSADPHCR3 = 0;
    }
    else
    {
        /* I lags V, so V (SDADC4) needs to be delayed*/
        /* 0.012 comes from HW UM 31.1.5 - sampling frequency of 3906.25Hz and target AC line frequency of 50Hz*/
    	DSADPHCR0 = 0;
    	DSADPHCR3 = (uint16_t) (phase.calib.vi_phase_correction / -0.012f);
    }

    LMA_Start();


	while(1)
	{
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
