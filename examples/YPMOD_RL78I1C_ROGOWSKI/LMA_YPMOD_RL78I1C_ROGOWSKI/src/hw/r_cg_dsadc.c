
#include "r_cg_macrodriver.h"
#include "r_cg_dsadc.h"
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_DSADC_Create
* Description  : This function initializes the DSAD converter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_DSADC_Create(void)
{
    DSADCK = 0U;
    DSADRES = 1U;   /* reset DSAD converter */
    DSADRES = 0U;   /* reset release of DSAD converter */
    DSADCEN = 1U;   /* enables input clock supply */
    DSAMK = 1U;     /* disable INTDSAD interrupt */
    DSAIF = 0U;     /* clear INTDSAD interrupt flag */
    DSAZMK0 = 1U;   /* disable INTDSADZC0 interrupt */
    DSAZIF0 = 0U;   /* clear INTDSADZC0 interrupt flag */
    DSAZMK1 = 1U;   /* disable INTDSADZC1 interrupt */
    DSAZIF1 = 0U;   /* clear INTDSADZC1 interrupt flag */
    /* Set INTDSAD high priority */
    DSAPR1 = 0U;
    DSAPR0 = 0U;
    DSADMR = _0000_DSAD_SAMPLING_FREQUENCY_0 | _0000_DSAD_RESOLUTION_24BIT;
    DSADGCR0 = _00_DSAD_CH1_PGAGAIN_1 | _00_DSAD_CH0_PGAGAIN_1;
    DSADGCR1 = _00_DSAD_CH3_PGAGAIN_1;
    DSADHPFCR = _80_DSAD_CUTOFF_FREQUENCY_2 | _00_DSAD_CH3_HIGHPASS_FILTER_ENABLE |
                _00_DSAD_CH1_HIGHPASS_FILTER_ENABLE | _00_DSAD_CH0_HIGHPASS_FILTER_ENABLE;
    DSADPHCR0 = _0000_DSAD_PHCR0_VALUE;
    DSADPHCR1 = _0000_DSAD_PHCR1_VALUE;
    DSADPHCR3 = _0000_DSAD_PHCR3_VALUE;
}
/***********************************************************************************************************************
* Function Name: R_DSADC_Start
* Description  : This function starts the DSAD converter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_DSADC_Start(void)
{
	uint8_t delay = 50U;

    DSAMK = 1U;     /* disable INTDSAD interrupt */
    DSAIF = 0U;     /* clear INTDSAD interrupt flag */

    DSADMR &= (uint16_t)~(_0008_DSAD_CH3_OPERATION | _0002_DSAD_CH1_OPERATION | _0001_DSAD_CH0_OPERATION);
    while(delay > 0U)
    {
    	--delay;
    	__nop();
    }

    DSADMR |= _0008_DSAD_CH3_OPERATION | _0002_DSAD_CH1_OPERATION | _0001_DSAD_CH0_OPERATION;

    DSAIF = 0U;     /* clear INTDSAD interrupt flag */
    DSAMK = 0U;     /* enable INTDSAD interrupt */
}
/***********************************************************************************************************************
* Function Name: R_DSADC_Stop
* Description  : This function stops the DSAD converter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_DSADC_Stop(void)
{
	uint8_t delay = 50U;

    DSAMK = 1U;     /* disable INTDSAD interrupt */
    DSAIF = 0U;     /* clear INTDSAD interrupt flag */
    DSADMR &= (uint16_t)~(_0008_DSAD_CH3_OPERATION | _0002_DSAD_CH1_OPERATION | _0001_DSAD_CH0_OPERATION);

    while(delay > 0U)
    {
    	--delay;
    	__nop();
    }
}
/***********************************************************************************************************************
* Function Name: R_DSADC_Set_OperationOn
* Description  : This function power-on control.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_DSADC_Set_OperationOn(void)
{
    DSADMR |= _0800_DSAD_CH3_POWER_ON | _0200_DSAD_CH1_POWER_ON | _0100_DSAD_CH0_POWER_ON;
}
/***********************************************************************************************************************
* Function Name: R_DSADC_Set_OperationOff
* Description  : This function power-down control.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_DSADC_Set_OperationOff(void)
{
    DSADMR &= (uint16_t)~(_0800_DSAD_CH3_POWER_ON | _0200_DSAD_CH1_POWER_ON | _0100_DSAD_CH0_POWER_ON);
}
/***********************************************************************************************************************
* Function Name: R_DSADC_Channel0_Get_Result
* Description  : This function returns the conversion result in the buffer.
* Arguments    : buffer -
*                    the address where to write the conversion result
* Return Value : None
***********************************************************************************************************************/
void R_DSADC_Channel0_Get_Result(uint32_t * const buffer)
{
    *buffer = DSADCR0H;
    *buffer = (uint32_t)((*buffer << 16U) + DSADCR0);
}
/***********************************************************************************************************************
* Function Name: R_DSADC_Channel1_Get_Result
* Description  : This function returns the conversion result in the buffer.
* Arguments    : buffer -
*                    the address where to write the conversion result
* Return Value : None
***********************************************************************************************************************/
void R_DSADC_Channel1_Get_Result(uint32_t * const buffer)
{
    *buffer = DSADCR1H;
    *buffer = (uint32_t)((*buffer << 16U) + DSADCR1);
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
