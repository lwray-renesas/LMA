#include "r_cg_cgc.h"

#include "r_cg_macrodriver.h"
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
* Function Name: R_CGC_Create
* Description  : This function initializes the clock generator.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_CGC_Create(void)
{
    volatile uint32_t w_count;

    /* Set fMX */
    CMC = _00_CGC_HISYS_PORT | _10_CGC_SYSOSC_PERMITTED | _00_CGC_SYSOSC_UNDER10M;
    MSTOP = 1U;     /* X1 oscillator/external clock stopped */

    /* Set fIM */
    MIOEN = 0U;     /* middle-speed on-chip oscillator stopped */

    /* Change the waiting time according to the system */
    for (w_count = 0U; w_count <= CGC_FIMWAITTIME; w_count++)
    {
        NOP();
    }


    /* Set fSX */
    OSMC = _00_CGC_CLK_ENABLE | _00_CGC_IT_CLK_fSX_CLK;
    VRTCEN = 1U;    /* enables input clock supply */
    SCMC = _10_CGC_SUB_OSC | _00_CGC_LOW_OSCILLATION;
    XTSTOP = 0U;    /* XT1 oscillator operating */

    /* Change the waiting time according to the system */
    for (w_count = 0U; w_count <= CGC_SUBWAITTIME; w_count++)
    {
        NOP();
    }

    XT1SELDIS = 0U; /* enables clock supply */

    /* Set fSUB */
    SELLOSC = 0U;   /* sub clock (fSX) */
    /* Set fCLK */
    CSS = 0U;       /* main system clock (fMAIN) */
    /* Set fMAIN */
    MCM0 = 0U;      /* selects the main on-chip oscillator clock (fOCO) as the main system clock (fMAIN) */
    /* Set fMAIN Control */
    MCM1 = 0U;      /* high-speed on-chip oscillator clock */
    VRTCEN = 0U;    /* stops input clock supply */
}


/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
