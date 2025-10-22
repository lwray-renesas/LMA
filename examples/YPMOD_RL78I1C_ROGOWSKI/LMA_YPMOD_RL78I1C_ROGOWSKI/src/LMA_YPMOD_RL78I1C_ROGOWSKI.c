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

#include "LMA_Core.h"
#include "Menu.h"
#include "Storage.h"
#include "r_cg_macrodriver.h"
#include "stdio.h"
#include "string.h"

#define PHASE_CALIB_ID (1U)
#define NEUTRAL_CALIB_ID (2U)
#define GLOBAL_CALIB_ID (3U)
#define ENERGY_LOG_ID (4U)

/* Configuration required for configuring the library 4500 ws/imp = 800 imp/kwh (3,600,000 = [ws/imp] * [kwh/imp])*/
static LMA_Config config = {.gcalib = {.fs = 0.0f, .fline_coeff = 0.0f, .deg_per_sample = 0.0f},
                            .update_interval = 25,
                            .fline_tol_low = 25.00f,
                            .fline_tol_high = 75.00f,
                            .meter_constant = 4500.00f,
                            .no_load_i = 0.01f,
                            .no_load_p = 2.00f,
                            .v_sag = 50.00f,
                            .v_swell = 280.00f};

/* Calibration data to load at startup*/
static LMA_GlobalCalibration default_global_calib = {.fs = 3906.25f, .fline_coeff = 97650.0000f, .deg_per_sample = 4.608f};

/* Calibration data to load at startup*/
static LMA_PhaseCalibration default_phase_calib = {
    .vrms_coeff = 21177.2051f, .irms_coeff = 53685.3828f, .vi_phase_correction = 0.0f, .p_coeff = 1136906368.0000f};

/* Calibration data to load at startup*/
static LMA_NeutralCalibration default_neutral_calib = {.irms_coeff = 53685.3828f};

/* Define our system energy struct*/
static LMA_SystemEnergy system_energy = {
    .energy =
        {
            .unit = {.act = 0.0f, .react = 0.0f, .app = 0.0f},
            .accumulator =
                {
                    .act_imp_ws = 0.0f,
                    .act_exp_ws = 0.0f,
                    .app_imp_ws = 0.0f,
                    .app_exp_ws = 0.0f,
                    .c_react_imp_ws = 0.0f,
                    .c_react_exp_ws = 0.0f,
                    .l_react_imp_ws = 0.0f,
                    .l_react_exp_ws = 0.0f,
                },
            .counter =
                {
                    .act_imp = (uint64_t)0,
                    .act_exp = (uint64_t)0,
                    .app_imp = (uint64_t)0,
                    .app_exp = (uint64_t)0,
                    .c_react_imp = (uint64_t)0,
                    .c_react_exp = (uint64_t)0,
                    .l_react_imp = (uint64_t)0,
                    .l_react_exp = (uint64_t)0,
                },
        },
    .impulse = {.led_on_count = 39, /* 10ms at 3906 Hz sampling frequency*/
                .active_counter = 0,
                .reactive_counter = 0,
                .apparent_counter = 0,
                .active_on = false,
                .reactive_on = false,
                .apparent_on = false},
};

/* Now define our phase pointing to our data structure*/
LMA_Phase phase;
LMA_Neutral neutral;

/** @brief Modifies ADC phase shift registers based on phase errors in calibration data
 * Must be called after loading calibration data to phases.
 */
static void Calibrate_adc_phase(void);

/** @brief Stores calibration data of all phases and global system*/
static void Store_calibration_data(void);

/** @brief Perform CPU Load test and display results*/
static void Cpu_load(char *p_args);
/** @brief Calibrate chip and store & display results*/
static void Calibrate(char *p_args);
/** @brief Restores default calibration to device and displays results*/
static void Restore_default_calibration(char *p_args);
/** @brief Stores current snapshot of energy log*/
static void Energy_log(char *p_args);
/** @brief resets energy log (library and veeprom)*/
static void Energy_clear(char *p_args);
/** @brief Display measurement results*/
static void Display_measurements(char *p_args);
/** @brief Dumps dataflash contents to terminal*/
static void Mem_dump(char *p_args);
/** @brief Direct call to NVIC_SystemReset*/
static void System_reset(char *p_args);

/* Menuing*/
Menu main_menu = {.p_name = "Main Menu"};
Menu_option cpu_load_option = {
    .p_cmd = "cpu", .p_help = "Currently Unsupported", .option_type = ACTION, .option.action = &Cpu_load};

Menu_option calib_option = {.p_cmd = "calib",
                            .p_help = "Calibrates device, stores in lib and VEEPROM\r\n"
                                      "\t\t\t Arguments (space seperated): vrms irms fline line_cycles\r\n"
                                      "\t\t\t - vrms = Target RMS Voltage\r\n"
                                      "\t\t\t - irms = Target RMS Current\r\n"
                                      "\t\t\t - fline = Target Operating Frequency\r\n"
                                      "\t\t\t - line_cycles = Number of line cycles to calibrate over\r\n"
                                      "\t\t\t e.g., calib 230 5 50 25",
                            .option_type = ACTION,
                            .option.action = &Calibrate};

Menu_option default_calib_option = {.p_cmd = "defcalib",
                                    .p_help = "Restores default calibration to lib and VEEPROM",
                                    .option_type = ACTION,
                                    .option.action = &Restore_default_calibration};

Menu_option energy_log_option = {.p_cmd = "elog",
                                 .p_help = "Stores a snapshot of the current energy log in VEEPROM without stopping LMA",
                                 .option_type = ACTION,
                                 .option.action = &Energy_log};

Menu_option energy_clear_option = {.p_cmd = "eclear",
                                   .p_help = "Stops LMA, resets the energy log (in library and VEEPROM) and restarts LMA",
                                   .option_type = ACTION,
                                   .option.action = &Energy_clear};

Menu_option display_option = {.p_cmd = "display",
                              .p_help = "Displays current measurements",
                              .option_type = ACTION,
                              .option.action = &Display_measurements};

Menu_option memdump_option = {
    .p_cmd = "memdump", .p_help = "Dumps dataflash contents to terminal", .option_type = ACTION, .option.action = &Mem_dump};

Menu_option reset_option = {.p_cmd = "reset",
                            .p_help = "Performs illegal address access & immediately resets MCU",
                            .option_type = ACTION,
                            .option.action = &System_reset};

static LMA_PhaseCalibration veeprom_phase_calib;
static LMA_NeutralCalibration veeprom_neutral_calib;
static LMA_GlobalCalibration veeprom_global_calib;
static LMA_SystemEnergy veeprom_sys_energy;

void main(void)
{
  EI();

  Menu_init(&main_menu);
  Menu_register_option(&main_menu, &cpu_load_option);
  Menu_register_option(&main_menu, &calib_option);
  Menu_register_option(&main_menu, &default_calib_option);
  Menu_register_option(&main_menu, &energy_log_option);
  Menu_register_option(&main_menu, &energy_clear_option);
  Menu_register_option(&main_menu, &display_option);
  Menu_register_option(&main_menu, &memdump_option);
  Menu_register_option(&main_menu, &reset_option);

  /* Clear Screen & Home cursor*/
  Menu_printf("\033[2J");
  Menu_printf("\033[H");
  Menu_printf("Initialising LMA...\r\n");

  /* Initialise the framework*/
  LMA_Init(&config);

  /* Init EEL*/
  Storage_init();

  /* Prepare*/
  Storage_startup();

  if (Storage_read(GLOBAL_CALIB_ID, (__near uint8_t *)&veeprom_global_calib))
  {
    Menu_printf("Global Calibration Found in VEEPROM!\r\n");
    LMA_GlobalLoadCalibration(&veeprom_global_calib);
  }
  else
  {
    Menu_printf("Global Calibration NOT Found in VEEPROM - using defaults!\r\n");
    LMA_GlobalLoadCalibration(&default_global_calib);
  }

  if (Storage_read(ENERGY_LOG_ID, (__near uint8_t *)&veeprom_sys_energy))
  {
    Menu_printf("System Energy Log found in VEEPROM!\r\n");
    LMA_EnergySet(&veeprom_sys_energy);
  }
  else
  {
    Menu_printf("System Energy Log NOT found in VEEPROM - using defaults!\r\n");
    LMA_EnergySet(&system_energy);
  }

  /* Initialise the phase(s)*/
  LMA_PhaseRegister(&phase);

  /* Initialise Neutral*/
  LMA_NeutralRegister(&phase, &neutral);

  if (Storage_read(PHASE_CALIB_ID, (__near uint8_t *)&veeprom_phase_calib))
  {
    Menu_printf("Phase Calibration Found in VEEPROM!\r\n");
    LMA_PhaseLoadCalibration(&phase, &veeprom_phase_calib);
  }
  else
  {
    Menu_printf("Phase Calibration NOT Found in VEEPROM - using defaults!\r\n");
    LMA_PhaseLoadCalibration(&phase, &default_phase_calib);
  }

  if (Storage_read(NEUTRAL_CALIB_ID, (__near uint8_t *)&veeprom_phase_calib))
  {
    Menu_printf("Neutral Calibration Found in VEEPROM!\r\n");
    LMA_NeutralLoadCalibration(&neutral, &veeprom_neutral_calib);
  }
  else
  {
    Menu_printf("Neutral Calibration NOT Found in VEEPROM - using defaults!\r\n");
    LMA_NeutralLoadCalibration(&neutral, &default_neutral_calib);
  }

  /* Clearup*/
  Storage_shutdown();

  /* Calibrate ADC*/
  Calibrate_adc_phase();

  Menu_printf("Initialisation Successful!\r\n");

  Menu_printf("Starting LMA...");
  LMA_Start();
  Menu_printf("Done!\r\n");

  Menu_print_help();

  while (1)
  {
    Menu_handler();
  }
}

static void Calibrate_adc_phase(void)
{
  if (phase.calib.vi_phase_correction >= 0)
  {
    /* I leads V, so I (SDADC1) needs to be delayed*/
    /* 0.012 comes from HW UM 31.1.5 - sampling frequency of 3906.25Hz and target AC line frequency of 50Hz*/
    DSADPHCR1 = (uint16_t)(phase.calib.vi_phase_correction / 0.012f);
    DSADPHCR3 = 0;
  }
  else
  {
    /* I lags V, so V (SDADC4) needs to be delayed*/
    /* 0.012 comes from HW UM 31.1.5 - sampling frequency of 3906.25Hz and target AC line frequency of 50Hz*/
    DSADPHCR1 = 0;
    DSADPHCR3 = (uint16_t)(phase.calib.vi_phase_correction / -0.012f);
  }
}

static void Store_calibration_data(void)
{
  /* Prepare*/
  Storage_startup();

  Storage_write(PHASE_CALIB_ID, (__near uint8_t *)&phase.calib);
  Storage_write(NEUTRAL_CALIB_ID, (__near uint8_t *)&neutral.calib);
  Storage_write(GLOBAL_CALIB_ID, (__near uint8_t *)&config.gcalib);

  /* Clearup*/
  Storage_shutdown();
}

static void Cpu_load(char *p_args)
{
  (void)(p_args);
  Menu_printf("\r\nCurrently Unsupported!");
}

static void Calibrate(char *p_args)
{
  float l_vrms, l_irms, l_fline = 0.0f;
  uint32_t l_line_cycles = 0;

  sscanf(p_args, "%f %f %f %lu", &l_vrms, &l_irms, &l_fline, &l_line_cycles);

  Menu_printf("\r\nVRMS Target: %.4f", l_vrms);
  Menu_printf("\r\nIRMS Target: %.4f", l_irms);
  Menu_printf("\r\nFline Target: %.4f", l_fline);
  Menu_printf("\r\nLine Cycles Target: %d", l_line_cycles);

  LMA_PhaseCalibArgs ca;
  LMA_GlobalCalibArgs gca;

  Menu_printf("\r\nCalibrating Phase & neutral...");
  ca.p_phase = &phase;
  ca.vrms_tgt = l_vrms;
  ca.irms_tgt = l_irms;
  ca.line_cycles = l_line_cycles;
  LMA_PhaseCalibrate(&ca);
  Menu_printf("Done!\r\n");
  Menu_printf("Irms Coefficient: %.4f\r\n", phase.calib.irms_coeff);
  Menu_printf("Vrms Coefficient: %.4f\r\n", phase.calib.vrms_coeff);
  Menu_printf("Power Coefficient: %.4f\r\n", phase.calib.p_coeff);
  Menu_printf("V-I Phase Error: %.4f\r\n", phase.calib.vi_phase_correction);
  Menu_printf("Irms Neutral Coefficient: %.4f\r\n", neutral.calib.irms_coeff);

  Menu_printf("\r\nCalibrating Global Parameters (fs)...");
  gca.rtc_period = 1.0f;
  gca.rtc_cycles = 3;
  gca.fline_target = l_fline;
  LMA_GlobalCalibrate(&gca);
  Menu_printf("Done!\r\n");
  Menu_printf("Sampling Frequency: %.4f\r\n", config.gcalib.fs);
  Menu_printf("Line Frequency Coefficient: %.4f\r\n", config.gcalib.fline_coeff);
  Menu_printf("Degrees per ADC sample: %.4f\r\n\r\n", config.gcalib.deg_per_sample);

  Menu_printf("Backing up calibration data in veeprom...");
  Store_calibration_data();
  Menu_printf("Done!\r\n");
}

static void Restore_default_calibration(char *p_args)
{
  (void)p_args;

  Menu_printf("\r\nStopping LMA...");
  LMA_Stop();
  Menu_printf("Done!\r\n");

  Menu_printf("Loading default calibration data into libs...");
  LMA_PhaseLoadCalibration(&phase, &default_phase_calib);
  LMA_NeutralLoadCalibration(&neutral, &default_neutral_calib);
  LMA_GlobalLoadCalibration(&default_global_calib);
  Menu_printf("Done!\r\n");

  Menu_printf("Backing up calibration data in veeprom...");
  Store_calibration_data();
  Menu_printf("Done!\r\n");

  Menu_printf("Starting LMA...");
  LMA_Start();
  Menu_printf("Done!\r\n");
}

static void Energy_log(char *p_args)
{
  (void)p_args;

  Menu_printf("\r\nRetrieving energy log...");
  LMA_EnergyGet(&system_energy);
  Menu_printf("Done!\r\n");

  Menu_printf("\r\nWriting log to VEEPROM...");

  /* Prepare*/
  Storage_startup();

  Storage_write(ENERGY_LOG_ID, (__near uint8_t *)&system_energy);

  /* Clearup*/
  Storage_shutdown();

  Menu_printf("Done!\r\n");
}

static void Energy_clear(char *p_args)
{
  (void)p_args;

  Menu_printf("\r\nStopping LMA...");
  LMA_Stop();
  Menu_printf("Done!\r\n");

  Menu_printf("\r\nClearing energy log...");
  memset(&system_energy, 0, sizeof(LMA_SystemEnergy));
  LMA_EnergySet(&system_energy);
  Menu_printf("Done!\r\n");

  Menu_printf("\r\nWriting empty log to VEEPROM...");

  /* Prepare*/
  Storage_startup();

  Storage_write(ENERGY_LOG_ID, (__near uint8_t *)&system_energy);

  /* Clearup*/
  Storage_shutdown();

  Menu_printf("Done!\r\n");

  Menu_printf("\r\nStarting LMA...");
  LMA_Start();
  Menu_printf("Done!\r\n");
}

static void Display_measurements(char *p_args)
{
  (void)(p_args);

  /* Utility structs for data output*/
  static LMA_Measurements measurements;
  static LMA_ConsumptionData energy_consumed;

  /* Get current snapshot of measurements*/
  LMA_MeasurementsGet(&phase, &measurements);

  Menu_printf("\r\n- Phase\r\n");
  Menu_printf("\tIrms: %.2f [A]\r\n", measurements.irms);
  Menu_printf("\tIrms Neutral: %.2f [A]\r\n", measurements.irms_neutral);
  Menu_printf("\tVrms: %.2f [V]\r\n", measurements.vrms);
  Menu_printf("\tP: %.2f [W]\r\n", measurements.p);
  Menu_printf("\tQ: %.2f [VAR]\r\n", measurements.q);
  Menu_printf("\tS: %.2f [VA]\r\n", measurements.s);
  Menu_printf("\tF: %.2f [Hz]\r\n", measurements.fline);

  /* Get current snapshot of system energy*/
  LMA_EnergyGet(&system_energy);
  /* Convert system energy to energy consumed*/
  LMA_ConsumptionDataGet(&system_energy, &energy_consumed);

  Menu_printf("\r\n- System Energy\r\n");
  Menu_printf("\tActive Import: %.4f [Wh]\r\n", energy_consumed.act_imp_energy_wh);
  Menu_printf("\tActive Export: %.4f [Wh]\r\n", energy_consumed.act_exp_energy_wh);
  Menu_printf("\tApparent Import: %.4f [Wh]\r\n", energy_consumed.app_imp_energy_wh);
  Menu_printf("\tApparent Export: %.4f [Wh]\r\n", energy_consumed.app_exp_energy_wh);
  Menu_printf("\tReactive Import (C): %.4f [VARh]\r\n", energy_consumed.c_imp_energy_wh);
  Menu_printf("\tReactive Export (C): %.4f [VARh]\r\n", energy_consumed.c_exp_energy_wh);
  Menu_printf("\tReactive Import (L): %.4f [VARh]\r\n", energy_consumed.l_imp_energy_wh);
  Menu_printf("\tReactive Export (L): %.4f [VARh]\r\n", energy_consumed.l_exp_energy_wh);
}

static void Mem_dump(char *p_args)
{
  (void)p_args;

  Menu_printf("\r\nDataflash Contents:\r\n");

  for (__near uint8_t *ptr = (__near uint8_t *)0x1000; ptr < (__near uint8_t *)0x1800; ptr += 16)
  {
    Menu_printf("0x%X: 0x%02X%02X, 0x%02X%02X, 0x%02X%02X, 0x%02X%02X, 0x%02X%02X, 0x%02X%02X, 0x%02X%02X, 0x%02X%02X\r\n",
                (uint16_t)ptr, (uint8_t)(*(ptr)), (uint8_t)(*(ptr + 1)), (uint8_t)(*(ptr + 2)), (uint8_t)(*(ptr + 3)),
                (uint8_t)(*(ptr + 4)), (uint16_t)(*(ptr + 5)), (uint8_t)(*(ptr + 6)), (uint8_t)(*(ptr + 7)),
                (uint8_t)(*(ptr + 8)), (uint8_t)(*(ptr + 9)), (uint8_t)(*(ptr + 10)), (uint8_t)(*(ptr + 11)),
                (uint8_t)(*(ptr + 12)), (uint8_t)(*(ptr + 13)), (uint8_t)(*(ptr + 14)), (uint8_t)(*(ptr + 15)));
  }
}

static void System_reset(char *p_args)
{
  (void)p_args;

  __far uint16_t *ptr;

  ptr = (__far uint16_t *)0x00011800;

  /* Illegal address access */
  *ptr = 0xFFFF;
}
