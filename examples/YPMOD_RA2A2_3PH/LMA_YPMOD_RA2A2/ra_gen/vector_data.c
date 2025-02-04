/* generated vector source file - do not edit */
#include "bsp_api.h"
/* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
#if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_MAX_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [1] = rtc_alarm_periodic_isr, /* RTC ALARM 0 (Alarm 0 interrupt) */
            [5] = sdadc_b_adi_isr, /* SDADC0 ADI (End of SD A/D conversion (type 1)) */
            [6] = rtc_alarm_periodic_isr, /* RTC PERIOD (Periodic interrupt) */
            [7] = sdadc_b_adi2_isr, /* SDADC0 ADI2 (End of SD A/D conversion (type 2)) */
            [10] = rtc_alarm_periodic_isr, /* RTC ALARM 1 (Alarm 1 interrupt) */
            [11] = rtc_carry_isr, /* RTC CARRY (Carry interrupt) */
            [58] = agt_int_isr, /* AGT7 INT (AGT interrupt) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_MAX_ENTRIES] =
        {
            [1] = BSP_PRV_VECT_ENUM(EVENT_RTC_ALARM_0,GROUP1), /* RTC ALARM 0 (Alarm 0 interrupt) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_SDADC0_ADI,GROUP5), /* SDADC0 ADI (End of SD A/D conversion (type 1)) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_RTC_PERIOD,GROUP6), /* RTC PERIOD (Periodic interrupt) */
            [7] = BSP_PRV_VECT_ENUM(EVENT_SDADC0_ADI2,GROUP7), /* SDADC0 ADI2 (End of SD A/D conversion (type 2)) */
            [10] = BSP_PRV_VECT_ENUM(EVENT_RTC_ALARM_1,GROUP2), /* RTC ALARM 1 (Alarm 1 interrupt) */
            [11] = BSP_PRV_VECT_ENUM(EVENT_RTC_CARRY,GROUP3), /* RTC CARRY (Carry interrupt) */
            [58] = BSP_PRV_VECT_ENUM(EVENT_AGT7_INT,FIXED), /* AGT7 INT (AGT interrupt) */
        };
        #endif
        #endif
