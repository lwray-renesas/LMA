#include "r_cg_macrodriver.h"
#include "r_cg_mac32bit.h"
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
* Function Name: R_MAC32Bit_Create
* Description  : This function initializes the 32bitMultiply module.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_MAC32Bit_Create(void)
{
    MACEN = 1U;     /* enables input clock supply */
    MACMK = 1U;     /* disable INTMACLOF interrupt */
    MACIF = 0U;     /* clear INTMACLOF interrupt flag */
    MULFRAC = 0U;   /* fixed point mode disabled */
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
