#include "Benchmark.h"
#include "LMA_Core.h"
#include "Menu.h"
#include "Storage.h"
#include "hal_data.h"
#include "stdio.h"

FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event);
FSP_CPP_FOOTER

#define PHASE1_CALIB_ID (0U)
#define PHASE2_CALIB_ID (1U)
#define PHASE3_CALIB_ID (2U)
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
static LMA_Phase phase1;
static LMA_Phase phase2;
static LMA_Phase phase3;

/* Benchmarking*/
static benchmark_t benchmark;

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
    .p_cmd = "cpu", .p_help = "Performs CPU Load Test", .option_type = ACTION, .option.action = &Cpu_load};

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

Menu_option reset_option = {
    .p_cmd = "reset", .p_help = "Immediately calls NVIC_SystemReset()", .option_type = ACTION, .option.action = &System_reset};

static LMA_PhaseCalibration veeprom_phase_calib;
static LMA_GlobalCalibration veeprom_global_calib;
static LMA_SystemEnergy veeprom_sys_energy;

void hal_entry(void)
{
  /* Turn the MACL on*/
  R_MSTP->MSTPCRC_b.MSTPC15 = 0U;
  R_MACL->MULC_b.MULSM = 1;

  /* Init Menu system*/
  Menu_init(&main_menu);
  Menu_register_option(&main_menu, &cpu_load_option);
  Menu_register_option(&main_menu, &calib_option);
  Menu_register_option(&main_menu, &default_calib_option);
  Menu_register_option(&main_menu, &energy_log_option);
  Menu_register_option(&main_menu, &energy_clear_option);
  Menu_register_option(&main_menu, &display_option);
  Menu_register_option(&main_menu, &memdump_option);
  Menu_register_option(&main_menu, &reset_option);

  /* Init benchmarking system*/
  Benchmark_init(&benchmark, 5);

  /* Clear Screen & Home cursor*/
  Menu_printf("\033[2J");
  Menu_printf("\033[H");
  Menu_printf("Initialising LMA...\r\n");

  /* Initialise the framework*/
  LMA_Init(&config);

  /* Prepare*/
  Storage_startup();

  /* Load Global Calibration*/
  if (Storage_read(GLOBAL_CALIB_ID, (uint8_t *)&veeprom_global_calib))
  {
    Menu_printf("Global Calibration Found in VEEPROM!\r\n");
    LMA_GlobalLoadCalibration(&veeprom_global_calib);
  }
  else
  {
    Menu_printf("Global Calibration NOT Found in VEEPROM - using defaults!\r\n");
    LMA_GlobalLoadCalibration(&default_global_calib);
  }

  /* Load System Energy*/
  if (Storage_read(ENERGY_LOG_ID, (uint8_t *)&veeprom_sys_energy))
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
  LMA_PhaseRegister(&phase1);
  LMA_PhaseRegister(&phase2);
  LMA_PhaseRegister(&phase3);

  /* Load calibration data*/

  /* Phase 1 calibration*/
  if (Storage_read(PHASE1_CALIB_ID, (uint8_t *)&veeprom_phase_calib))
  {
    Menu_printf("Phase 1 Calibration Found in VEEPROM!\r\n");
    LMA_PhaseLoadCalibration(&phase1, &veeprom_phase_calib);
  }
  else
  {
    Menu_printf("Phase 1 Calibration NOT Found in VEEPROM - using defaults!\r\n");
    LMA_PhaseLoadCalibration(&phase1, &default_phase_calib);
  }

  /* Phase 2 calibration*/
  if (Storage_read(PHASE2_CALIB_ID, (uint8_t *)&veeprom_phase_calib))
  {
    Menu_printf("Phase 2 Calibration Found in VEEPROM!\r\n");
    LMA_PhaseLoadCalibration(&phase2, &veeprom_phase_calib);
  }
  else
  {
    Menu_printf("Phase 2 Calibration NOT Found in VEEPROM - using defaults!\r\n");
    LMA_PhaseLoadCalibration(&phase2, &default_phase_calib);
  }

  /* Phase 3 calibration*/
  if (Storage_read(PHASE3_CALIB_ID, (uint8_t *)&veeprom_phase_calib))
  {
    Menu_printf("Phase 3 Calibration Found in VEEPROM!\r\n");
    LMA_PhaseLoadCalibration(&phase3, &veeprom_phase_calib);
  }
  else
  {
    Menu_printf("Phase 3 Calibration NOT Found in VEEPROM - using defaults!\r\n");
    LMA_PhaseLoadCalibration(&phase3, &default_phase_calib);
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

void R_BSP_WarmStart(bsp_warm_start_event_t event)
{
  if (BSP_WARM_START_RESET == event)
  {
#if BSP_FEATURE_FLASH_LP_VERSION != 0

    /* Enable reading from data flash. */
    R_FACI_LP->DFLCTL = 1U;

    /* Would normally have to wait tDSTOP(6us) for data flash recovery. Placing the enable here, before clock and

     * * C runtime initialization, should negate the need for a delay since the initialization will typically take more than
     * 6us. */
#endif
  }

  if (BSP_WARM_START_POST_C == event)
  {
    /* C runtime environment and system clocks are setup. */

    /* Configure pins. */
    R_IOPORT_Open(&IOPORT_CFG_CTRL, &IOPORT_CFG_NAME);
  }
}

static void Calibrate_adc_phase(void)
{
  /* Adjust phase correction for SDADC on RA2A2*/
  if (phase1.calib.vi_phase_correction >= 0)
  {
    /* I leads V, so I (SDADC0) needs to be delayed*/
    /* 0.012 comes from HW UM 31.1.5 - sampling frequency of 3906.25Hz and target AC line frequency of 50Hz*/
    R_SDADC_B->SDADPHCR0 = (uint16_t)(phase1.calib.vi_phase_correction / 0.012f);
    R_SDADC_B->SDADPHCR4 = 0;
  }
  else
  {
    /* I lags V, so V (SDADC4) needs to be delayed*/
    /* 0.012 comes from HW UM 31.1.5 - sampling frequency of 3906.25Hz and target AC line frequency of 50Hz*/
    R_SDADC_B->SDADPHCR0 = 0;
    R_SDADC_B->SDADPHCR4 = (uint16_t)(phase1.calib.vi_phase_correction / -0.012f);
  }

  if (phase2.calib.vi_phase_correction >= 0)
  {
    /* I leads V, so I (SDADC0) needs to be delayed*/
    /* 0.012 comes from HW UM 31.1.5 - sampling frequency of 3906.25Hz and target AC line frequency of 50Hz*/
    R_SDADC_B->SDADPHCR1 = (uint16_t)(phase2.calib.vi_phase_correction / 0.012f);
    R_SDADC_B->SDADPHCR5 = 0;
  }
  else
  {
    /* I lags V, so V (SDADC4) needs to be delayed*/
    /* 0.012 comes from HW UM 31.1.5 - sampling frequency of 3906.25Hz and target AC line frequency of 50Hz*/
    R_SDADC_B->SDADPHCR1 = 0;
    R_SDADC_B->SDADPHCR5 = (uint16_t)(phase2.calib.vi_phase_correction / -0.012f);
  }

  if (phase3.calib.vi_phase_correction >= 0)
  {
    /* I leads V, so I (SDADC0) needs to be delayed*/
    /* 0.012 comes from HW UM 31.1.5 - sampling frequency of 3906.25Hz and target AC line frequency of 50Hz*/
    R_SDADC_B->SDADPHCR2 = (uint16_t)(phase3.calib.vi_phase_correction / 0.012f);
    R_SDADC_B->SDADPHCR6 = 0;
  }
  else
  {
    /* I lags V, so V (SDADC4) needs to be delayed*/
    /* 0.012 comes from HW UM 31.1.5 - sampling frequency of 3906.25Hz and target AC line frequency of 50Hz*/
    R_SDADC_B->SDADPHCR2 = 0;
    R_SDADC_B->SDADPHCR6 = (uint16_t)(phase3.calib.vi_phase_correction / -0.012f);
  }
}

static void Store_calibration_data(void)
{
  /* Prepare*/
  Storage_startup();

  Storage_write(PHASE1_CALIB_ID, (uint8_t *)&phase1.calib, sizeof(LMA_PhaseCalibration));
  Storage_write(PHASE2_CALIB_ID, (uint8_t *)&phase2.calib, sizeof(LMA_PhaseCalibration));
  Storage_write(PHASE3_CALIB_ID, (uint8_t *)&phase3.calib, sizeof(LMA_PhaseCalibration));
  Storage_write(GLOBAL_CALIB_ID, (uint8_t *)&config.gcalib, sizeof(LMA_GlobalCalibration));

  /* Clearup*/
  Storage_shutdown();
}

static void Cpu_load(char *p_args)
{
  (void)(p_args);
  Menu_printf("\r\nRunning CPU Load Test...");
  Benchmark_run(&benchmark);
  Menu_printf("Done!\r\n");
  Menu_printf("CPU Peak Load: %.2f [%%]", benchmark.cpu_utilisation_pk);
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

  Menu_printf("\r\nCalibrating Phase 1...");
  ca.p_phase = &phase1;
  ca.vrms_tgt = l_vrms;
  ca.irms_tgt = l_irms;
  ca.line_cycles = l_line_cycles;
  LMA_PhaseCalibrate(&ca);
  Menu_printf("Done!\r\n");
  Menu_printf("Irms Coefficient: %.4f\r\n", phase1.calib.irms_coeff);
  Menu_printf("Vrms Coefficient: %.4f\r\n", phase1.calib.vrms_coeff);
  Menu_printf("Power Coefficient: %.4f\r\n", phase1.calib.p_coeff);
  Menu_printf("V-I Phase Error: %.4f\r\n", phase1.calib.vi_phase_correction);

  Menu_printf("\r\nCalibrating Second Phase...");
  ca.p_phase = &phase2;
  LMA_PhaseCalibrate(&ca);
  Menu_printf("Done!\r\n");
  Menu_printf("Irms Coefficient: %.4f\r\n", phase2.calib.irms_coeff);
  Menu_printf("Vrms Coefficient: %.4f\r\n", phase2.calib.vrms_coeff);
  Menu_printf("Power Coefficient: %.4f\r\n", phase2.calib.p_coeff);
  Menu_printf("V-I Phase Error: %.4f\r\n", phase2.calib.vi_phase_correction);

  Menu_printf("\r\nCalibrating Third Phase...");
  ca.p_phase = &phase3;
  LMA_PhaseCalibrate(&ca);
  Menu_printf("Done!\r\n");
  Menu_printf("Irms Coefficient: %.4f\r\n", phase3.calib.irms_coeff);
  Menu_printf("Vrms Coefficient: %.4f\r\n", phase3.calib.vrms_coeff);
  Menu_printf("Power Coefficient: %.4f\r\n", phase3.calib.p_coeff);
  Menu_printf("V-I Phase Error: %.4f\r\n", phase3.calib.vi_phase_correction);

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
  LMA_PhaseLoadCalibration(&phase1, &default_phase_calib);
  LMA_PhaseLoadCalibration(&phase2, &default_phase_calib);
  LMA_PhaseLoadCalibration(&phase3, &default_phase_calib);
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

  Storage_write(ENERGY_LOG_ID, (uint8_t *)&system_energy, sizeof(LMA_SystemEnergy));

  /* Clearup*/
  Storage_shutdown();

  Menu_printf("Done!\r\n");
}

static void Energy_clear(char *p_args)
{
  fsp_err_t err = FSP_SUCCESS;
  (void)p_args;

  Menu_printf("\r\nStopping LMA...");
  LMA_Stop();
  Menu_printf("Done!\r\n");

  Menu_printf("\r\nClearing energy log...");
  memset(&system_energy, 0, sizeof(LMA_SystemEnergy));
  LMA_EnergySet(&system_energy);
  Menu_printf("Done!\r\n");

  err = RM_VEE_FLASH_Open(&g_vee0_ctrl, &g_vee0_cfg);
  if (FSP_SUCCESS != err)
  {
    __BKPT(0);
  }

  Menu_printf("\r\nWriting empty log to VEEPROM...");

  /* Prepare*/
  Storage_startup();

  Storage_write(ENERGY_LOG_ID, (uint8_t *)&system_energy, sizeof(LMA_SystemEnergy));

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
  static LMA_Measurements measurements1;
  static LMA_Measurements measurements2;
  static LMA_Measurements measurements3;
  static LMA_ConsumptionData energy_consumed;

  /* Get current snapshot of measurements*/
  LMA_MeasurementsGet(&phase1, &measurements1);

  Menu_printf("\r\n- Phase 1\r\n");
  Menu_printf("\tIrms: %.2f [A]\r\n", measurements1.irms);
  Menu_printf("\tVrms: %.2f [V]\r\n", measurements1.vrms);
  Menu_printf("\tP: %.2f [W]\r\n", measurements1.p);
  Menu_printf("\tQ: %.2f [VAR]\r\n", measurements1.q);
  Menu_printf("\tS: %.2f [VA]\r\n", measurements1.s);
  Menu_printf("\tF: %.2f [Hz]\r\n", measurements1.fline);

  /* Get current snapshot of measurements*/
  LMA_MeasurementsGet(&phase2, &measurements2);

  Menu_printf("\r\n- Phase 2\r\n");
  Menu_printf("\tIrms: %.2f [A]\r\n", measurements2.irms);
  Menu_printf("\tVrms: %.2f [V]\r\n", measurements2.vrms);
  Menu_printf("\tP: %.2f [W]\r\n", measurements2.p);
  Menu_printf("\tQ: %.2f [VAR]\r\n", measurements2.q);
  Menu_printf("\tS: %.2f [VA]\r\n", measurements2.s);
  Menu_printf("\tF: %.2f [Hz]\r\n", measurements2.fline);

  /* Get current snapshot of measurements*/
  LMA_MeasurementsGet(&phase3, &measurements3);

  Menu_printf("\r\n- Phase 3\r\n");
  Menu_printf("\tIrms: %.2f [A]\r\n", measurements3.irms);
  Menu_printf("\tVrms: %.2f [V]\r\n", measurements3.vrms);
  Menu_printf("\tP: %.2f [W]\r\n", measurements3.p);
  Menu_printf("\tQ: %.2f [VAR]\r\n", measurements3.q);
  Menu_printf("\tS: %.2f [VA]\r\n", measurements3.s);
  Menu_printf("\tF: %.2f [Hz]\r\n", measurements3.fline);

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

  for (uint8_t *ptr = (uint8_t *)0x40101000; ptr < (uint8_t *)0x40102000; ptr += 16)
  {
    Menu_printf("0x%X: 0x%02X%02X, 0x%02X%02X, 0x%02X%02X, 0x%02X%02X, 0x%02X%02X, 0x%02X%02X, 0x%02X%02X, 0x%02X%02X\r\n",
                (uint32_t)ptr, (uint8_t)(*(ptr)), (uint8_t)(*(ptr + 1)), (uint8_t)(*(ptr + 2)), (uint8_t)(*(ptr + 3)),
                (uint8_t)(*(ptr + 4)), (uint16_t)(*(ptr + 5)), (uint8_t)(*(ptr + 6)), (uint8_t)(*(ptr + 7)),
                (uint8_t)(*(ptr + 8)), (uint8_t)(*(ptr + 9)), (uint8_t)(*(ptr + 10)), (uint8_t)(*(ptr + 11)),
                (uint8_t)(*(ptr + 12)), (uint8_t)(*(ptr + 13)), (uint8_t)(*(ptr + 14)), (uint8_t)(*(ptr + 15)));
  }
}

static void System_reset(char *p_args)
{
  (void)p_args;
  NVIC_SystemReset();
}

/** @brief phase shifts voltage signal
 * @details
 * - 50Hz signal is 20ms.
 * - 50Hz signal being 360degree of period, to get 90degree we divide by 4.
 * - 20ms divided by 4 = 5ms.
 * - to delay 5ms with a 3906Hz clock we can do 0.005/(1/3906) = 19.53 samples - so we do 20
 * samples.
 *
 * @param[in] new_voltage - new voltage to store in the buffer
 * @return voltage sample 90degree (20 samples) ago.
 */
static spl_t PhaseShift90(spl_t new_voltage)
{
  static spl_t voltage_buffer[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  static uint8_t buffer_index = 0;

  uint8_t buffer_index_19 = buffer_index + 2;
  uint8_t buffer_index_20 = buffer_index + 1;

  if (buffer_index_19 > 21)
  {
    buffer_index_19 -= 21;
  }

  if (buffer_index_20 > 21)
  {
    buffer_index_20 -= 21;
  }

  /* Append new voltage*/
  voltage_buffer[buffer_index] = new_voltage;

  /* Interpolate 19.53 samples - just take the mid point*/
  int32_t interpolated_value = ((voltage_buffer[buffer_index_19] * 60) >> 7) + ((voltage_buffer[buffer_index_20] * 68) >> 7);

  buffer_index = buffer_index_20;

  /* Convert back to its 32b value*/
  return interpolated_value;
}

void DSADC_Callback(adc_callback_args_t *p_args)
{
  (void)p_args;

  Benchmark_work_begin(&benchmark);
  phase1.inputs.v_sample = (spl_t)R_SDADC_B->SDADCR4;
  phase1.inputs.i_sample = (spl_t)R_SDADC_B->SDADCR0;
  phase1.inputs.v90_sample = PhaseShift90(phase1.inputs.v_sample);

  phase2.inputs.v_sample = (spl_t)R_SDADC_B->SDADCR5;
  phase2.inputs.i_sample = (spl_t)R_SDADC_B->SDADCR1;
  phase2.inputs.v90_sample = PhaseShift90(phase2.inputs.v_sample);

  phase3.inputs.v_sample = (spl_t)R_SDADC_B->SDADCR6;
  phase3.inputs.i_sample = (spl_t)R_SDADC_B->SDADCR2;
  phase3.inputs.v90_sample = PhaseShift90(phase3.inputs.v_sample);
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

  /* We'll use the TMR as the benchmarking tick*/
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
