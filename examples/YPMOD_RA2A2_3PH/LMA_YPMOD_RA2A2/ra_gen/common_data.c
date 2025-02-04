/* generated common source file - do not edit */
#include "common_data.h"
#if BSP_FEATURE_CGC_HAS_OSTDCSE
const cgc_extended_cfg_t g_cgc0_cfg_extend =
{
    .ostd_enable = (1),
    .mostd_enable = (0),
    .sostd_enable = (0),
#if defined(VECTOR_NUMBER_CGC_MOSTD_STOP)
    .mostd_irq            = VECTOR_NUMBER_CGC_MOSTD_STOP,
#else
    .mostd_irq            = FSP_INVALID_VECTOR,
#endif
    .mostd_ipl            = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_CGC_SOSC_STOP)
    .sostd_irq            = VECTOR_NUMBER_CGC_SOSC_STOP,
#else
    .sostd_irq            = FSP_INVALID_VECTOR,
#endif
    .sostd_ipl            = (BSP_IRQ_DISABLED),
    .sdadc_b_clock_switch_enable = (0),
    .mostd_detection_time = 0,
    .sostd_detection_time = 0,
};
#endif

#if BSP_TZ_NONSECURE_BUILD
 #if defined(BSP_CFG_CLOCKS_SECURE) && BSP_CFG_CLOCKS_SECURE
  #error "The CGC registers are only accessible in the TrustZone Secure Project."
 #endif
#endif

const cgc_cfg_t g_cgc0_cfg = { .p_callback = NULL,
#if BSP_FEATURE_CGC_HAS_OSTDCSE
    .p_extend   = &g_cgc0_cfg_extend,
#else
		.p_extend = NULL,
#endif
		};

cgc_instance_ctrl_t g_cgc0_ctrl;
const cgc_instance_t g_cgc0 = { .p_api = &g_cgc_on_cgc, .p_ctrl = &g_cgc0_ctrl,
		.p_cfg = &g_cgc0_cfg, };
ioport_instance_ctrl_t g_ioport_ctrl;
const ioport_instance_t g_ioport = { .p_api = &g_ioport_on_ioport, .p_ctrl =
		&g_ioport_ctrl, .p_cfg = &g_bsp_pin_cfg, };
void g_common_init(void) {
}
