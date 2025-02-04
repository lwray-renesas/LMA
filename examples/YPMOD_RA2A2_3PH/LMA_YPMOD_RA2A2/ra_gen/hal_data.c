/* generated HAL source file - do not edit */
#include "hal_data.h"
agt_instance_ctrl_t g_timer0_ctrl;
const agt_extended_cfg_t g_timer0_extend = { .count_source = AGT_CLOCK_PCLKB,
		.agto = AGT_PIN_CFG_DISABLED, .agtoab_settings_b.agtoa =
				AGT_PIN_CFG_DISABLED, .agtoab_settings_b.agtob =
				AGT_PIN_CFG_DISABLED, .measurement_mode = AGT_MEASURE_DISABLED,
		.agtio_filter = AGT_AGTIO_FILTER_NONE, .enable_pin =
				AGT_ENABLE_PIN_NOT_USED,
		.trigger_edge = AGT_TRIGGER_EDGE_RISING, .counter_bit_width =
				AGT_COUNTER_BIT_WIDTH_16, };
const timer_cfg_t g_timer0_cfg = { .mode = TIMER_MODE_PERIODIC,
/* Actual period: 0.02 seconds. Actual duty: 50%. */.period_counts =
		(uint32_t) 0x7530, .duty_cycle_counts = 0x3a98, .source_div =
		(timer_source_div_t) 0, .channel = 7, .p_callback = TMR_Callback,
/** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
		.p_context = &NULL,
#endif
		.p_extend = &g_timer0_extend, .cycle_end_ipl = (2),
#if defined(VECTOR_NUMBER_AGT7_INT)
    .cycle_end_irq       = VECTOR_NUMBER_AGT7_INT,
#else
		.cycle_end_irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const timer_instance_t g_timer0 = { .p_ctrl = &g_timer0_ctrl, .p_cfg =
		&g_timer0_cfg, .p_api = &g_timer_on_agt };
agt_instance_ctrl_t g_working_timer_ctrl;
const agt_extended_cfg_t g_working_timer_extend = { .count_source =
		AGT_CLOCK_PCLKB, .agto = AGT_PIN_CFG_DISABLED,
		.agtoab_settings_b.agtoa = AGT_PIN_CFG_DISABLED,
		.agtoab_settings_b.agtob = AGT_PIN_CFG_DISABLED, .measurement_mode =
				AGT_MEASURE_DISABLED, .agtio_filter = AGT_AGTIO_FILTER_NONE,
		.enable_pin = AGT_ENABLE_PIN_NOT_USED, .trigger_edge =
				AGT_TRIGGER_EDGE_RISING, .counter_bit_width =
				AGT_COUNTER_BIT_WIDTH_16, };
const timer_cfg_t g_working_timer_cfg =
		{ .mode = TIMER_MODE_PERIODIC,
				/* Actual period: 0.04369066666666667 seconds. Actual duty: 50%. */.period_counts =
						(uint32_t) 0x10000, .duty_cycle_counts = 0x8000,
				.source_div = (timer_source_div_t) 0, .channel = 1,
				.p_callback = NULL,
				/** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
				.p_context = &NULL,
#endif
				.p_extend = &g_working_timer_extend, .cycle_end_ipl =
						(BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_AGT1_INT)
    .cycle_end_irq       = VECTOR_NUMBER_AGT1_INT,
#else
				.cycle_end_irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const timer_instance_t g_working_timer = { .p_ctrl = &g_working_timer_ctrl,
		.p_cfg = &g_working_timer_cfg, .p_api = &g_timer_on_agt };
agt_instance_ctrl_t g_idle_timer_ctrl;
const agt_extended_cfg_t g_idle_timer_extend = {
		.count_source = AGT_CLOCK_PCLKB, .agto = AGT_PIN_CFG_DISABLED,
		.agtoab_settings_b.agtoa = AGT_PIN_CFG_DISABLED,
		.agtoab_settings_b.agtob = AGT_PIN_CFG_DISABLED, .measurement_mode =
				AGT_MEASURE_DISABLED, .agtio_filter = AGT_AGTIO_FILTER_NONE,
		.enable_pin = AGT_ENABLE_PIN_NOT_USED, .trigger_edge =
				AGT_TRIGGER_EDGE_RISING, .counter_bit_width =
				AGT_COUNTER_BIT_WIDTH_16, };
const timer_cfg_t g_idle_timer_cfg =
		{ .mode = TIMER_MODE_PERIODIC,
				/* Actual period: 0.04369066666666667 seconds. Actual duty: 50%. */.period_counts =
						(uint32_t) 0x10000, .duty_cycle_counts = 0x8000,
				.source_div = (timer_source_div_t) 0, .channel = 0,
				.p_callback = NULL,
				/** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
				.p_context = &NULL,
#endif
				.p_extend = &g_idle_timer_extend, .cycle_end_ipl =
						(BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_AGT0_INT)
    .cycle_end_irq       = VECTOR_NUMBER_AGT0_INT,
#else
				.cycle_end_irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const timer_instance_t g_idle_timer = { .p_ctrl = &g_idle_timer_ctrl, .p_cfg =
		&g_idle_timer_cfg, .p_api = &g_timer_on_agt };
sdadc_b_instance_ctrl_t g_adc0_ctrl;

const sdadc_b_scan_cfg_t g_adc0_channel_cfg = { .scan_cfg_mask = ((1) | (1 << 1)
		| (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (0 << 7)),
		.hpf_cutoff = SDADC_B_CUTOFF_10B, .gain_setting = {
				(sdadc_b_channel_gain_t) 0, (sdadc_b_channel_gain_t) 0,
				(sdadc_b_channel_gain_t) 0, (sdadc_b_channel_gain_t) 0,
				(sdadc_b_channel_gain_t) 0, (sdadc_b_channel_gain_t) 0,
				(sdadc_b_channel_gain_t) 0, }, .hpf_setting = {
				(sdadc_b_channel_hpf_t) 0, (sdadc_b_channel_hpf_t) 0,
				(sdadc_b_channel_hpf_t) 0, (sdadc_b_channel_hpf_t) 0,
				(sdadc_b_channel_hpf_t) 0, (sdadc_b_channel_hpf_t) 0,
				(sdadc_b_channel_hpf_t) 0, (sdadc_b_channel_hpf_t) 1, },
		.phase_adjustment = { 0, 0, 0, 0, 0, 0, 0, 0, }, };

const sdadc_b_extended_cfg_t g_adc0_cfg_extend = { .oper_clk =
		SDADC_B_CLOCK_IS_12MHZ, .sampling_mode = SDADC_B_4KHZ_SAMPLING_MODE,
		.p_channel_cfg = &g_adc0_channel_cfg, .conv_end_ipl = (0),
		.conv_end_irq = VECTOR_NUMBER_SDADC0_ADI, .conv_end_irq2 =
				VECTOR_NUMBER_SDADC0_ADI2, .zc_ipl = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_SDADC0_ADZC0)
    .zc_irq          = VECTOR_NUMBER_SDADC0_ADZC0,
#else
		.zc_irq = FSP_INVALID_VECTOR,
#endif
		.zc_ipl2 = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_SDADC0_ADZC1)
    .zc_irq2         = VECTOR_NUMBER_SDADC0_ADZC1,
#else
		.zc_irq2 = FSP_INVALID_VECTOR,
#endif
		.zc_detection_setting = (0 | (SDADC_B_ZC_LEVEL_OUTPUT_MODE << 1) | 0 | 0
				| 0 | (SDADC_B_ZC_LEVEL_OUTPUT_MODE << 5) | 0 | 0), };

const adc_cfg_t g_adc0_cfg = { .unit = 0, .mode = (adc_mode_t) 0, .resolution =
		ADC_RESOLUTION_24_BIT, .alignment = ADC_ALIGNMENT_RIGHT, .trigger =
		ADC_TRIGGER_SOFTWARE, .scan_end_irq = FSP_INVALID_VECTOR,
		.scan_end_ipl = 0, .scan_end_b_irq = FSP_INVALID_VECTOR,
		.scan_end_b_ipl = 0, .p_callback = DSADC_Callback, .p_context = NULL,
		.p_extend = &g_adc0_cfg_extend, };

/* Instance structure to use this module. */
const adc_instance_t g_adc0 = { .p_ctrl = &g_adc0_ctrl, .p_cfg = &g_adc0_cfg,
		.p_channel_cfg = NULL, .p_api = &g_adc_on_sdadc_b };
rtc_instance_ctrl_t g_rtc0_ctrl;
const rtc_error_adjustment_cfg_t g_rtc0_err_cfg = { .adjustment_mode =
		RTC_ERROR_ADJUSTMENT_MODE_AUTOMATIC, .adjustment_period =
		RTC_ERROR_ADJUSTMENT_PERIOD_10_SECOND, .adjustment_type =
		RTC_ERROR_ADJUSTMENT_NONE, .adjustment_value = 0, };
const rtc_extended_cfg_t g_rtc0_cfg_extend = { .alarm1_ipl = (3),
#if defined(VECTOR_NUMBER_RTC_ALARM_1)
    .alarm1_irq               = VECTOR_NUMBER_RTC_ALARM_1,
#else
		.alarm1_irq = FSP_INVALID_VECTOR,
#endif
		};

const rtc_cfg_t g_rtc0_cfg = { .clock_source = RTC_CLOCK_SOURCE_SUBCLK,
		.p_err_cfg = &g_rtc0_err_cfg, .p_callback = RTC_Callback, .p_context =
				NULL, .p_extend = &g_rtc0_cfg_extend, .alarm_ipl = (3),
		.periodic_ipl = (3), .carry_ipl = (3),
#if defined(VECTOR_NUMBER_RTC_ALARM_0)
    .alarm_irq               = VECTOR_NUMBER_RTC_ALARM_0,
#else
		.alarm_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_RTC_PERIOD)
    .periodic_irq            = VECTOR_NUMBER_RTC_PERIOD,
#else
		.periodic_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_RTC_CARRY)
    .carry_irq               = VECTOR_NUMBER_RTC_CARRY,
#else
		.carry_irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const rtc_instance_t g_rtc0 = { .p_ctrl = &g_rtc0_ctrl, .p_cfg = &g_rtc0_cfg,
		.p_api = &g_rtc_on_rtc };
void g_hal_init(void) {
	g_common_init();
}
