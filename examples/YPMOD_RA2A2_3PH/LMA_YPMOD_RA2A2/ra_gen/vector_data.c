/* generated vector source file - do not edit */
#include "bsp_api.h"
/* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
#if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_NUM_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [1] = rtc_alarm_periodic_isr, /* RTC ALARM 0 (Alarm 0 interrupt) */
            [2] = rtc_alarm_periodic_isr, /* RTC ALARM 1 (Alarm 1 interrupt) */
            [3] = rtc_carry_isr, /* RTC CARRY (Carry interrupt) */
            [4] = sci_uart_rxi_isr, /* SCI9 RXI (Receive data full) */
            [5] = sci_uart_txi_isr, /* SCI9 TXI (Transmit data empty) */
            [6] = sci_uart_tei_isr, /* SCI9 TEI (Transmit end) */
            [7] = sci_uart_eri_isr, /* SCI9 ERI (Receive error) */
            [10] = rtc_alarm_periodic_isr, /* RTC PERIOD (Periodic interrupt) */
            [13] = sdadc_b_adi_isr, /* SDADC0 ADI (End of SD A/D conversion (type 1)) */
            [14] = fcu_frdyi_isr, /* FCU FRDYI (Flash ready interrupt) */
            [15] = sdadc_b_adi2_isr, /* SDADC0 ADI2 (End of SD A/D conversion (type 2)) */
            [58] = agt_int_isr, /* AGT7 INT (AGT interrupt) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES] =
        {
            [1] = BSP_PRV_VECT_ENUM(EVENT_RTC_ALARM_0,GROUP1), /* RTC ALARM 0 (Alarm 0 interrupt) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_RTC_ALARM_1,GROUP2), /* RTC ALARM 1 (Alarm 1 interrupt) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_RTC_CARRY,GROUP3), /* RTC CARRY (Carry interrupt) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_SCI9_RXI,GROUP4), /* SCI9 RXI (Receive data full) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_SCI9_TXI,GROUP5), /* SCI9 TXI (Transmit data empty) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_SCI9_TEI,GROUP6), /* SCI9 TEI (Transmit end) */
            [7] = BSP_PRV_VECT_ENUM(EVENT_SCI9_ERI,GROUP7), /* SCI9 ERI (Receive error) */
            [10] = BSP_PRV_VECT_ENUM(EVENT_RTC_PERIOD,GROUP2), /* RTC PERIOD (Periodic interrupt) */
            [13] = BSP_PRV_VECT_ENUM(EVENT_SDADC0_ADI,GROUP5), /* SDADC0 ADI (End of SD A/D conversion (type 1)) */
            [14] = BSP_PRV_VECT_ENUM(EVENT_FCU_FRDYI,GROUP6), /* FCU FRDYI (Flash ready interrupt) */
            [15] = BSP_PRV_VECT_ENUM(EVENT_SDADC0_ADI2,GROUP7), /* SDADC0 ADI2 (End of SD A/D conversion (type 2)) */
            [58] = BSP_PRV_VECT_ENUM(EVENT_AGT7_INT,FIXED), /* AGT7 INT (AGT interrupt) */
        };
        #endif
        #endif
