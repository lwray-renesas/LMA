/* generated vector header file - do not edit */
#ifndef VECTOR_DATA_H
#define VECTOR_DATA_H
#ifdef __cplusplus
        extern "C" {
        #endif
/* Number of interrupts allocated */
#ifndef VECTOR_DATA_IRQ_COUNT
#define VECTOR_DATA_IRQ_COUNT    (12)
#endif
/* ISR prototypes */
void rtc_alarm_periodic_isr(void);
void rtc_carry_isr(void);
void sci_uart_rxi_isr(void);
void sci_uart_txi_isr(void);
void sci_uart_tei_isr(void);
void sci_uart_eri_isr(void);
void sdadc_b_adi_isr(void);
void fcu_frdyi_isr(void);
void sdadc_b_adi2_isr(void);
void agt_int_isr(void);

/* Vector table allocations */
#define VECTOR_NUMBER_RTC_ALARM_0 ((IRQn_Type) 1) /* RTC ALARM 0 (Alarm 0 interrupt) */
#define RTC_ALARM_0_IRQn          ((IRQn_Type) 1) /* RTC ALARM 0 (Alarm 0 interrupt) */
#define VECTOR_NUMBER_RTC_ALARM_1 ((IRQn_Type) 2) /* RTC ALARM 1 (Alarm 1 interrupt) */
#define RTC_ALARM_1_IRQn          ((IRQn_Type) 2) /* RTC ALARM 1 (Alarm 1 interrupt) */
#define VECTOR_NUMBER_RTC_CARRY ((IRQn_Type) 3) /* RTC CARRY (Carry interrupt) */
#define RTC_CARRY_IRQn          ((IRQn_Type) 3) /* RTC CARRY (Carry interrupt) */
#define VECTOR_NUMBER_SCI9_RXI ((IRQn_Type) 4) /* SCI9 RXI (Receive data full) */
#define SCI9_RXI_IRQn          ((IRQn_Type) 4) /* SCI9 RXI (Receive data full) */
#define VECTOR_NUMBER_SCI9_TXI ((IRQn_Type) 5) /* SCI9 TXI (Transmit data empty) */
#define SCI9_TXI_IRQn          ((IRQn_Type) 5) /* SCI9 TXI (Transmit data empty) */
#define VECTOR_NUMBER_SCI9_TEI ((IRQn_Type) 6) /* SCI9 TEI (Transmit end) */
#define SCI9_TEI_IRQn          ((IRQn_Type) 6) /* SCI9 TEI (Transmit end) */
#define VECTOR_NUMBER_SCI9_ERI ((IRQn_Type) 7) /* SCI9 ERI (Receive error) */
#define SCI9_ERI_IRQn          ((IRQn_Type) 7) /* SCI9 ERI (Receive error) */
#define VECTOR_NUMBER_RTC_PERIOD ((IRQn_Type) 10) /* RTC PERIOD (Periodic interrupt) */
#define RTC_PERIOD_IRQn          ((IRQn_Type) 10) /* RTC PERIOD (Periodic interrupt) */
#define VECTOR_NUMBER_SDADC0_ADI ((IRQn_Type) 13) /* SDADC0 ADI (End of SD A/D conversion (type 1)) */
#define SDADC0_ADI_IRQn          ((IRQn_Type) 13) /* SDADC0 ADI (End of SD A/D conversion (type 1)) */
#define VECTOR_NUMBER_FCU_FRDYI ((IRQn_Type) 14) /* FCU FRDYI (Flash ready interrupt) */
#define FCU_FRDYI_IRQn          ((IRQn_Type) 14) /* FCU FRDYI (Flash ready interrupt) */
#define VECTOR_NUMBER_SDADC0_ADI2 ((IRQn_Type) 15) /* SDADC0 ADI2 (End of SD A/D conversion (type 2)) */
#define SDADC0_ADI2_IRQn          ((IRQn_Type) 15) /* SDADC0 ADI2 (End of SD A/D conversion (type 2)) */
#define VECTOR_NUMBER_AGT7_INT ((IRQn_Type) 58) /* AGT7 INT (AGT interrupt) */
#define AGT7_INT_IRQn          ((IRQn_Type) 58) /* AGT7 INT (AGT interrupt) */
/* The number of entries required for the ICU vector table. */
#define BSP_ICU_VECTOR_NUM_ENTRIES (59)

#ifdef __cplusplus
        }
        #endif
#endif /* VECTOR_DATA_H */
