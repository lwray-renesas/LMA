/* generated vector header file - do not edit */
#ifndef VECTOR_DATA_H
#define VECTOR_DATA_H
#ifdef __cplusplus
        extern "C" {
        #endif
/* Number of interrupts allocated */
#ifndef VECTOR_DATA_IRQ_COUNT
#define VECTOR_DATA_IRQ_COUNT    (7)
#endif
/* ISR prototypes */
void rtc_alarm_periodic_isr(void);
void sdadc_b_adi_isr(void);
void sdadc_b_adi2_isr(void);
void rtc_carry_isr(void);
void agt_int_isr(void);

/* Vector table allocations */
#define VECTOR_NUMBER_RTC_ALARM_0 ((IRQn_Type) 1) /* RTC ALARM 0 (Alarm 0 interrupt) */
#define RTC_ALARM_0_IRQn          ((IRQn_Type) 1) /* RTC ALARM 0 (Alarm 0 interrupt) */
#define VECTOR_NUMBER_SDADC0_ADI ((IRQn_Type) 5) /* SDADC0 ADI (End of SD A/D conversion (type 1)) */
#define SDADC0_ADI_IRQn          ((IRQn_Type) 5) /* SDADC0 ADI (End of SD A/D conversion (type 1)) */
#define VECTOR_NUMBER_RTC_PERIOD ((IRQn_Type) 6) /* RTC PERIOD (Periodic interrupt) */
#define RTC_PERIOD_IRQn          ((IRQn_Type) 6) /* RTC PERIOD (Periodic interrupt) */
#define VECTOR_NUMBER_SDADC0_ADI2 ((IRQn_Type) 7) /* SDADC0 ADI2 (End of SD A/D conversion (type 2)) */
#define SDADC0_ADI2_IRQn          ((IRQn_Type) 7) /* SDADC0 ADI2 (End of SD A/D conversion (type 2)) */
#define VECTOR_NUMBER_RTC_ALARM_1 ((IRQn_Type) 10) /* RTC ALARM 1 (Alarm 1 interrupt) */
#define RTC_ALARM_1_IRQn          ((IRQn_Type) 10) /* RTC ALARM 1 (Alarm 1 interrupt) */
#define VECTOR_NUMBER_RTC_CARRY ((IRQn_Type) 11) /* RTC CARRY (Carry interrupt) */
#define RTC_CARRY_IRQn          ((IRQn_Type) 11) /* RTC CARRY (Carry interrupt) */
#define VECTOR_NUMBER_AGT7_INT ((IRQn_Type) 58) /* AGT7 INT (AGT interrupt) */
#define AGT7_INT_IRQn          ((IRQn_Type) 58) /* AGT7 INT (AGT interrupt) */
#ifdef __cplusplus
        }
        #endif
#endif /* VECTOR_DATA_H */
