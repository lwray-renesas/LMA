#include "r_cg_macrodriver.h"
#include "r_cg_port.h"
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
* Function Name: R_PORT_Create
* Description  : This function initializes the Port I/O.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_PORT_Create(void)
{
    PFSEG0 = _80_PFSEG07_SEG | _00_PFSEG06_PORT | _20_PFSEG05_SEG | _10_PFSEG04_SEG;
    PFSEG1 = _08_PFSEG11_SEG | _04_PFSEG10_SEG | _02_PFSEG09_SEG | _01_PFSEG08_SEG | _F0_PFSEG1_DEFAULT_VALUE;
    PFSEG2 = _10_PFSEG20_SEG | _08_PFSEG19_SEG | _04_PFSEG18_SEG | _02_PFSEG17_SEG | _01_PFSEG16_SEG | 
             _E0_PFSEG2_DEFAULT_VALUE;
    PFSEG3 = _02_PFSEG25_SEG | _01_PFSEG24_SEG | _FC_PFSEG3_DEFAULT_VALUE;
    P1 = _00_Pn2_OUTPUT_0;
    POM1 = _00_POMn2_NCH_OFF;
    PM1 = _00_PMn2_MODE_OUTPUT;
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
