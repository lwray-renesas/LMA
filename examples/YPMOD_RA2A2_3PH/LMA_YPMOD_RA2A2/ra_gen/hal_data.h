/* generated HAL header file - do not edit */
#ifndef HAL_DATA_H_
#define HAL_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "common_data.h"
#include "r_agt.h"
#include "r_timer_api.h"
#include "r_sdadc_b.h"
#include "r_adc_api.h"
#include "r_rtc.h"
#include "r_rtc_api.h"
FSP_HEADER
/** AGT Timer Instance */
extern const timer_instance_t g_timer0;

/** Access the AGT instance using these structures when calling API functions directly (::p_api is not used). */
extern agt_instance_ctrl_t g_timer0_ctrl;
extern const timer_cfg_t g_timer0_cfg;

#ifndef TMR_Callback
void TMR_Callback(timer_callback_args_t *p_args);
#endif
/** AGT Timer Instance */
extern const timer_instance_t g_working_timer;

/** Access the AGT instance using these structures when calling API functions directly (::p_api is not used). */
extern agt_instance_ctrl_t g_working_timer_ctrl;
extern const timer_cfg_t g_working_timer_cfg;

#ifndef NULL
void NULL(timer_callback_args_t *p_args);
#endif
/** AGT Timer Instance */
extern const timer_instance_t g_idle_timer;

/** Access the AGT instance using these structures when calling API functions directly (::p_api is not used). */
extern agt_instance_ctrl_t g_idle_timer_ctrl;
extern const timer_cfg_t g_idle_timer_cfg;

#ifndef NULL
void NULL(timer_callback_args_t *p_args);
#endif
/** ADC on ADC Instance. */
extern const adc_instance_t g_adc0;
extern sdadc_b_instance_ctrl_t g_adc0_ctrl;
extern const adc_cfg_t g_adc0_cfg;
#ifndef DSADC_Callback
void DSADC_Callback(adc_callback_args_t *p_args);
#endif
/* rtc Instance. */
extern const rtc_instance_t g_rtc0;

/** Access the rtc instance using these structures when calling API functions directly (::p_api is not used). */
extern rtc_instance_ctrl_t g_rtc0_ctrl;
extern const rtc_cfg_t g_rtc0_cfg;

#ifndef RTC_Callback
void RTC_Callback(rtc_callback_args_t *p_args);
#endif
void hal_entry(void);
void g_hal_init(void);
FSP_FOOTER
#endif /* HAL_DATA_H_ */
