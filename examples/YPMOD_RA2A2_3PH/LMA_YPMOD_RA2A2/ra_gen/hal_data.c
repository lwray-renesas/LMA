/* generated HAL source file - do not edit */
#include "hal_data.h"
flash_lp_instance_ctrl_t g_flash0_ctrl;
const flash_cfg_t g_flash0_cfg = { .data_flash_bgo = true, .p_callback =
		rm_vee_flash_callback, .p_context = &g_vee0_ctrl, .ipl = (3),
#if defined(VECTOR_NUMBER_FCU_FRDYI)
    .irq                 = VECTOR_NUMBER_FCU_FRDYI,
#else
		.irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const flash_instance_t g_flash0 = { .p_ctrl = &g_flash0_ctrl, .p_cfg =
		&g_flash0_cfg, .p_api = &g_flash_on_flash_lp };
rm_vee_flash_instance_ctrl_t g_vee0_ctrl;

const rm_vee_flash_cfg_t g_vee0_cfg_ext = { .p_flash = &g_flash0 };

static uint16_t g_vee0_record_offset[16 + 1] = { 0 };

const rm_vee_cfg_t g_vee0_cfg = { .start_addr =
		BSP_FEATURE_FLASH_DATA_FLASH_START, .num_segments = 2, .total_size =
		BSP_DATA_FLASH_SIZE_BYTES, .ref_data_size = 0, .record_max_id = 16,
		.rec_offset = &g_vee0_record_offset[0], .p_callback = VEE_Callback,
		.p_context = NULL, .p_extend = &g_vee0_cfg_ext };

/* Instance structure to use this module. */
const rm_vee_instance_t g_vee0 = { .p_ctrl = &g_vee0_ctrl, .p_cfg = &g_vee0_cfg,
		.p_api = &g_rm_vee_on_flash };
dtc_instance_ctrl_t g_transfer1_ctrl;

#if (1 == 1)
transfer_info_t g_transfer1_info DTC_TRANSFER_INFO_ALIGNMENT =
		{ .transfer_settings_word_b.dest_addr_mode =
				TRANSFER_ADDR_MODE_INCREMENTED,
				.transfer_settings_word_b.repeat_area =
						TRANSFER_REPEAT_AREA_DESTINATION,
				.transfer_settings_word_b.irq = TRANSFER_IRQ_END,
				.transfer_settings_word_b.chain_mode =
						TRANSFER_CHAIN_MODE_DISABLED,
				.transfer_settings_word_b.src_addr_mode =
						TRANSFER_ADDR_MODE_FIXED,
				.transfer_settings_word_b.size = TRANSFER_SIZE_1_BYTE,
				.transfer_settings_word_b.mode = TRANSFER_MODE_NORMAL, .p_dest =
						(void*) NULL, .p_src = (void const*) NULL, .num_blocks =
						(uint16_t) 0, .length = (uint16_t) 0, };

#elif (1 > 1)
/* User is responsible to initialize the array. */
transfer_info_t g_transfer1_info[1] DTC_TRANSFER_INFO_ALIGNMENT;
#else
/* User must call api::reconfigure before enable DTC transfer. */
#endif

const dtc_extended_cfg_t g_transfer1_cfg_extend = { .activation_source =
		VECTOR_NUMBER_SCI9_RXI, };

const transfer_cfg_t g_transfer1_cfg = {
#if (1 == 1)
		.p_info = &g_transfer1_info,
#elif (1 > 1)
    .p_info              = g_transfer1_info,
#else
    .p_info = NULL,
#endif
		.p_extend = &g_transfer1_cfg_extend, };

/* Instance structure to use this module. */
const transfer_instance_t g_transfer1 = { .p_ctrl = &g_transfer1_ctrl, .p_cfg =
		&g_transfer1_cfg, .p_api = &g_transfer_on_dtc };
dtc_instance_ctrl_t g_transfer0_ctrl;

#if (1 == 1)
transfer_info_t g_transfer0_info DTC_TRANSFER_INFO_ALIGNMENT =
		{ .transfer_settings_word_b.dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
				.transfer_settings_word_b.repeat_area =
						TRANSFER_REPEAT_AREA_SOURCE,
				.transfer_settings_word_b.irq = TRANSFER_IRQ_END,
				.transfer_settings_word_b.chain_mode =
						TRANSFER_CHAIN_MODE_DISABLED,
				.transfer_settings_word_b.src_addr_mode =
						TRANSFER_ADDR_MODE_INCREMENTED,
				.transfer_settings_word_b.size = TRANSFER_SIZE_1_BYTE,
				.transfer_settings_word_b.mode = TRANSFER_MODE_NORMAL, .p_dest =
						(void*) NULL, .p_src = (void const*) NULL, .num_blocks =
						(uint16_t) 0, .length = (uint16_t) 0, };

#elif (1 > 1)
/* User is responsible to initialize the array. */
transfer_info_t g_transfer0_info[1] DTC_TRANSFER_INFO_ALIGNMENT;
#else
/* User must call api::reconfigure before enable DTC transfer. */
#endif

const dtc_extended_cfg_t g_transfer0_cfg_extend = { .activation_source =
		VECTOR_NUMBER_SCI9_TXI, };

const transfer_cfg_t g_transfer0_cfg = {
#if (1 == 1)
		.p_info = &g_transfer0_info,
#elif (1 > 1)
    .p_info              = g_transfer0_info,
#else
    .p_info = NULL,
#endif
		.p_extend = &g_transfer0_cfg_extend, };

/* Instance structure to use this module. */
const transfer_instance_t g_transfer0 = { .p_ctrl = &g_transfer0_ctrl, .p_cfg =
		&g_transfer0_cfg, .p_api = &g_transfer_on_dtc };
sci_uart_instance_ctrl_t g_uart9_ctrl;

baud_setting_t g_uart9_baud_setting = {
/* Baud rate calculated with 0.160% error. */.semr_baudrate_bits_b.abcse = 1,
		.semr_baudrate_bits_b.abcs = 0, .semr_baudrate_bits_b.bgdm = 0,
		.cks = 0, .brr = 25, .mddr = (uint8_t) 256, .semr_baudrate_bits_b.brme =
				false };

/** UART extended configuration for UARTonSCI HAL driver */
const sci_uart_extended_cfg_t g_uart9_cfg_extend = {
		.clock = SCI_UART_CLOCK_INT, .rx_edge_start =
				SCI_UART_START_BIT_FALLING_EDGE, .noise_cancel =
				SCI_UART_NOISE_CANCELLATION_DISABLE, .rx_fifo_trigger =
				SCI_UART_RX_FIFO_TRIGGER_MAX, .p_baud_setting =
				&g_uart9_baud_setting,
		.flow_control = SCI_UART_FLOW_CONTROL_RTS,
#if 0xFF != 0xFF
                .flow_control_pin       = BSP_IO_PORT_FF_PIN_0xFF,
                #else
		.flow_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
		.rs485_setting = { .enable = SCI_UART_RS485_DISABLE, .polarity =
				SCI_UART_RS485_DE_POLARITY_HIGH,
#if 0xFF != 0xFF
                    .de_control_pin = BSP_IO_PORT_FF_PIN_0xFF,
                #else
				.de_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
				}, .irda_setting = { .ircr_bits_b.ire = 0,
				.ircr_bits_b.irrxinv = 0, .ircr_bits_b.irtxinv = 0, }, };

/** UART interface configuration */
const uart_cfg_t g_uart9_cfg = { .channel = 9, .data_bits = UART_DATA_BITS_8,
		.parity = UART_PARITY_OFF, .stop_bits = UART_STOP_BITS_1, .p_callback =
				Uart9_callback, .p_context = NULL, .p_extend =
				&g_uart9_cfg_extend,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == g_transfer0)
                .p_transfer_tx       = NULL,
#else
		.p_transfer_tx = &g_transfer0,
#endif
#if (RA_NOT_DEFINED == g_transfer1)
                .p_transfer_rx       = NULL,
#else
		.p_transfer_rx = &g_transfer1,
#endif
#undef RA_NOT_DEFINED
		.rxi_ipl = (2), .txi_ipl = (2), .tei_ipl = (2), .eri_ipl = (2),
#if defined(VECTOR_NUMBER_SCI9_RXI)
                .rxi_irq             = VECTOR_NUMBER_SCI9_RXI,
#else
		.rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI9_TXI)
                .txi_irq             = VECTOR_NUMBER_SCI9_TXI,
#else
		.txi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI9_TEI)
                .tei_irq             = VECTOR_NUMBER_SCI9_TEI,
#else
		.tei_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI9_ERI)
                .eri_irq             = VECTOR_NUMBER_SCI9_ERI,
#else
		.eri_irq = FSP_INVALID_VECTOR,
#endif
		};

/* Instance structure to use this module. */
const uart_instance_t g_uart9 = { .p_ctrl = &g_uart9_ctrl,
		.p_cfg = &g_uart9_cfg, .p_api = &g_uart_on_sci };
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
		.p_context = (void*) &NULL,
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
				.p_context = (void*) &NULL,
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
				.p_context = (void*) &NULL,
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
