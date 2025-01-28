#include "LMA_Port.h"
#include "LMA_Core.h"

#define ACC_SIGN_SHIFT ((acc_t)(32))
#define ACC_SIGN_MASK ((uint32_t)(0x80000000))
#define ACC_MSB_MASK ((acc_t)(0x8000000000000000))

#define FLOAT_SIGN_MASK ((uint32_t)(0x80000000))
#define FLOAT_EXP_MASK ((uint32_t)(0x7F800000))
#define FLOAT_MANTISSA_MASK ((uint32_t)(0x7FFFFF))
#define FLOAT_EXP_SHIFT ((uint32_t)(23))
#define FLOAT_MANTISSA_BITS ((uint32_t)(23))

/** @brief Counts leading zeroes uint32_t type
 * @param num - number to compute leading zeroes on.
 * @return number of leading zeroes.
 */
static inline uint32_t LMA_CLZ(uint32_t num)
{
    static const uint8_t clz_table[256] = {
                                           8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
                                           3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                           2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                           2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                           1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t index = 0;
    uint32_t count = 0;

    if(0 == (num & 0xFFFF0000))
    {
        index = (uint8_t)((num & 0xFF00)>>8);
        count += clz_table[index] + 16;
        if(8 == clz_table[index])
        {
            index = (uint8_t)(num & 0xFF);
            count += clz_table[index];
        }
    }
    else
    {
        index = (uint8_t)((num & 0xFF000000)>>24);
        count += clz_table[index];
        if(8 == count)
        {
            index = (uint8_t)((num & 0xFF0000)>>16);
            count += clz_table[index];
        }
    }


    return count;
}

/** @brief Accepts uint32_t (assuming its ieee 754 single precision float) and returns its recirpocal
 * @details https://stackoverflow.com/questions/9011161/how-to-implement-floating-point-division-in-binary-with-no-division-hardware-and
 * @param a - ieee 754 single precision float to compute reciprocal of.
 * @return ~ 1/a
 */
static inline float LMA_FloatReciprocal(uint32_t a)
{
    static const uint16_t reciprocal_lut[256] =
    {
     0x1ff, 0x1fd, 0x1fb, 0x1f9, 0x1f7, 0x1f5, 0x1f3, 0x1f1, 0x1f0, 0x1ee, 0x1ec, 0x1ea, 0x1e8, 0x1e6, 0x1e5, 0x1e3,
     0x1e1, 0x1df, 0x1dd, 0x1dc, 0x1da, 0x1d8, 0x1d7, 0x1d5, 0x1d3, 0x1d2, 0x1d0, 0x1ce, 0x1cd, 0x1cb, 0x1c9, 0x1c8,
     0x1c6, 0x1c5, 0x1c3, 0x1c2, 0x1c0, 0x1bf, 0x1bd, 0x1bc, 0x1ba, 0x1b9, 0x1b7, 0x1b6, 0x1b4, 0x1b3, 0x1b1, 0x1b0,
     0x1ae, 0x1ad, 0x1ac, 0x1aa, 0x1a9, 0x1a7, 0x1a6, 0x1a5, 0x1a3, 0x1a2, 0x1a1, 0x19f, 0x19e, 0x19d, 0x19c, 0x19a,
     0x199, 0x198, 0x196, 0x195, 0x194, 0x193, 0x191, 0x190, 0x18f, 0x18e, 0x18d, 0x18b, 0x18a, 0x189, 0x188, 0x187,
     0x186, 0x184, 0x183, 0x182, 0x181, 0x180, 0x17f, 0x17e, 0x17c, 0x17b, 0x17a, 0x179, 0x178, 0x177, 0x176, 0x175,
     0x174, 0x173, 0x172, 0x171, 0x170, 0x16f, 0x16e, 0x16d, 0x16c, 0x16b, 0x16a, 0x169, 0x168, 0x167, 0x166, 0x165,
     0x164, 0x163, 0x162, 0x161, 0x160, 0x15f, 0x15e, 0x15d, 0x15c, 0x15b, 0x15a, 0x159, 0x158, 0x158, 0x157, 0x156,
     0x155, 0x154, 0x153, 0x152, 0x151, 0x151, 0x150, 0x14f, 0x14e, 0x14d, 0x14c, 0x14b, 0x14b, 0x14a, 0x149, 0x148,
     0x147, 0x146, 0x146, 0x145, 0x144, 0x143, 0x142, 0x142, 0x141, 0x140, 0x13f, 0x13f, 0x13e, 0x13d, 0x13c, 0x13b,
     0x13b, 0x13a, 0x139, 0x138, 0x138, 0x137, 0x136, 0x135, 0x135, 0x134, 0x133, 0x133, 0x132, 0x131, 0x130, 0x130,
     0x12f, 0x12e, 0x12e, 0x12d, 0x12c, 0x12c, 0x12b, 0x12a, 0x12a, 0x129, 0x128, 0x128, 0x127, 0x126, 0x126, 0x125,
     0x124, 0x124, 0x123, 0x122, 0x122, 0x121, 0x120, 0x120, 0x11f, 0x11e, 0x11e, 0x11d, 0x11d, 0x11c, 0x11b, 0x11b,
     0x11a, 0x11a, 0x119, 0x118, 0x118, 0x117, 0x117, 0x116, 0x115, 0x115, 0x114, 0x114, 0x113, 0x112, 0x112, 0x111,
     0x111, 0x110, 0x110, 0x10f, 0x10f, 0x10e, 0x10d, 0x10d, 0x10c, 0x10c, 0x10b, 0x10b, 0x10a, 0x10a, 0x109, 0x109,
     0x108, 0x107, 0x107, 0x106, 0x106, 0x105, 0x105, 0x104, 0x104, 0x103, 0x103, 0x102, 0x102, 0x101, 0x101, 0x100
    };

    uint32_t result = (a & 0x80000000) | ((252 << FLOAT_EXP_SHIFT)-(a & FLOAT_EXP_MASK));
    uint32_t y = a & 0x007fffff;

    a = reciprocal_lut[y >> 15];  /* approx */
    y |= 0x00800000;

    y = (uint32_t)-(int32_t)(y * a); /* y = 1 - arg * approx */

    /* y = y * y + y */
    R_MACL->MULC = 0; /* Multiply Only, Unsigned Integer*/
    R_MACL->MUL32U = y;
    R_MACL->MULB23 = y;
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    /* approx = y * approx + approx */
    a <<= 15;
    R_MACL->MUL32U = R_MACL->MULR23.MULRH + y;
    R_MACL->MULR23.MULRL = 0;
    R_MACL->MULR23.MULRH = 0;
    R_MACL->MULB23 = a;
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    result |= (R_MACL->MULR23.MULRH + a);

    return *(float*)&result;
}

float LMA_AccToFloat(acc_t acc)
{
    volatile uint32_t result = (uint32_t) ((*((uint32_t*)(&acc)+1)) & ACC_SIGN_MASK);
    volatile uint32_t shift_count;

    /* Handle Sign*/
    if(result > 0)
    {
        *((uint32_t*)(&acc)+1) ^= 0xFFFFFFFF;
        *(uint32_t*)(&acc) ^= 0xFFFFFFFF;
        ++acc;
    }

    if(*((uint32_t*)(&acc)+1) == 0)
    {
        if(*((uint32_t*)&acc) == 0)
        {
            return 0.0f;
        }

        /* 32b number*/
        /* Find leading one*/
        shift_count = LMA_CLZ(*(uint32_t*)(&acc));

        if(shift_count > 8)
        {
            acc <<= shift_count - 8; /* Align*/
            ++shift_count; /* Add leading 1*/
        }
        else
        {
            acc >>= 8-shift_count; /* Align*/
            ++acc; /* Add leading 1*/
        }

        result |= (uint32_t)((127 + (32 - shift_count)) << 23) | (uint32_t)(*(uint32_t*)(&acc) & 0x7FFFFF);
    }
    else
    {
        /* 64b number*/
        /* Find leading one*/
        shift_count = LMA_CLZ(*((uint32_t*)(&acc)+1));

        if(shift_count > 39)
        {
            acc <<= shift_count-39; /* Align*/
            ++shift_count; /* Add leading 1*/
        }
        else
        {
            acc >>= 39-shift_count; /* Align*/
            ++acc; /* Add leading 1*/
        }

        result |= (uint32_t)((127 + (64 - shift_count)) << 23) | (uint32_t)(*(uint32_t*)(&acc) & 0x7FFFFF);
    }

    return *((float*)&result);
}

float LMA_FPMul_Fast(float a, float b)
{
    uint32_t result = ((*(uint32_t*)&a) & FLOAT_SIGN_MASK) ^ ((*(uint32_t*)&b) & FLOAT_SIGN_MASK);
    result |= ((*(uint32_t*)&a) & FLOAT_EXP_MASK) + ((*(uint32_t*)&b) & FLOAT_EXP_MASK) - (127 << FLOAT_EXP_SHIFT);

    R_MACL->MULC = 0; /* Multiply Only, Unsigned Integer*/
    R_MACL->MUL32U = (*((uint32_t*)&(a)) & FLOAT_MANTISSA_MASK) | (1 << FLOAT_MANTISSA_BITS);
    R_MACL->MULB23 = (*((uint32_t*)&(b)) & FLOAT_MANTISSA_MASK) | (1 << FLOAT_MANTISSA_BITS);
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    if (R_MACL->MULR23.MULRH >= 0x8000)
    {
        /* Take least significant 16bits from High & most significant 8 bits from Low */
        result |= (((R_MACL->MULR23.MULRH & 0xFFFF) << 8) | ((R_MACL->MULR23.MULRL & 0xFF000000) >> 24)) & 0x7FFFFF;
        result += (1 << FLOAT_EXP_SHIFT);
    }
    else
    {
        /* Take least significant 15bits from High & most significant 9 bits from Low */
        result |= (((R_MACL->MULR23.MULRH & 0x7FFF) << 9) | ((R_MACL->MULR23.MULRL & 0xFF800000) >> 23)) & 0x7FFFFF;
    }

    return *((float*)&result);
}

float LMA_FPDiv_Fast(float a, float b)
{
    return LMA_FPMul_Fast(a, LMA_FloatReciprocal(*(uint32_t*)&b));
}

float LMA_FPSqrt_Fast(float a)
{
    uint32_t approx = 0x1fbd3f7d + ((*(uint32_t*)&a) >> 1);

    /* Newton-Rapson iteration
     * approx = (((approx * approx) + a)/(approx)) * 0.5;
     */
    float f_approx = LMA_FPMul_Fast(LMA_FPDiv_Fast(LMA_FPMul_Fast(*(float*)&approx, *(float*)&approx) + a, *(float*)&approx), 0.5f);
    f_approx = LMA_FPMul_Fast(LMA_FPDiv_Fast(LMA_FPMul_Fast(f_approx, f_approx) + a, f_approx), 0.5f);
    return f_approx;
}

float LMA_FPAbs_Fast(float a)
{
    uint32_t tmp = *(uint32_t*)(&a) & 0x7FFFFFFF;
    return *(float*)&tmp;
}

void LMA_AccReset(LMA_Workspace *const p_ws, const uint32_t phase_id)
{
    (void) phase_id;
    R_MACL->MULC = 0xC0; /* MAC, Signed Integer*/
    R_MACL->MAC32S = *((uint32_t*)&(p_ws->samples.current));
    R_MACL->MULR0.MULRL = 0;
    R_MACL->MULR0.MULRH = 0;
    R_MACL->MULR1.MULRL = 0;
    R_MACL->MULR1.MULRH = 0;
    R_MACL->MULR2.MULRL = 0;
    R_MACL->MULR2.MULRH = 0;

    R_MACL->MULB0 = *((uint32_t*)&(p_ws->samples.current));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    R_MACL->MULB1 = *((uint32_t*)&(p_ws->samples.voltage));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    R_MACL->MULB2 = *((uint32_t*)&(p_ws->samples.voltage90));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    R_MACL->MAC32S = *((uint32_t*)&(p_ws->samples.voltage));
    R_MACL->MULR3.MULRL = 0;
    R_MACL->MULR3.MULRH = 0;
    R_MACL->MULB3 = *((uint32_t*)&(p_ws->samples.voltage));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    *((uint32_t*)(&p_ws->accs.iacc)) = R_MACL->MULR0.MULRL;
    *((uint32_t*)(&p_ws->accs.iacc)+1) = R_MACL->MULR0.MULRH;
    *((uint32_t*)(&p_ws->accs.pacc)) = R_MACL->MULR1.MULRL;
    *((uint32_t*)(&p_ws->accs.pacc)+1) = R_MACL->MULR1.MULRH;
    *((uint32_t*)(&p_ws->accs.qacc)) = R_MACL->MULR2.MULRL;
    *((uint32_t*)(&p_ws->accs.qacc)+1) = R_MACL->MULR2.MULRH;
    *((uint32_t*)(&p_ws->accs.vacc)) = R_MACL->MULR3.MULRL;
    *((uint32_t*)(&p_ws->accs.vacc)+1) = R_MACL->MULR3.MULRH;

    p_ws->accs.sample_count = 1;
}

void LMA_AccRun(LMA_Workspace *const p_ws, const uint32_t phase_id)
{
    (void) phase_id;
    R_MACL->MULC = 3; /* MAC, Signed Integer*/
    R_MACL->MAC32S = *((uint32_t*)&(p_ws->samples.current));
    R_MACL->MULB0 = *((uint32_t*)&(p_ws->samples.current));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    R_MACL->MULB1 = *((uint32_t*)&(p_ws->samples.voltage));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    R_MACL->MULB2 = *((uint32_t*)&(p_ws->samples.voltage90));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    R_MACL->MAC32S = *((uint32_t*)&(p_ws->samples.voltage));
    R_MACL->MULB3 = *((uint32_t*)&(p_ws->samples.voltage));
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();

    ++p_ws->accs.sample_count;
}

void LMA_AccLoad(LMA_Workspace *const p_ws, LMA_Accumulators *const p_accs, const uint32_t phase_id)
{
    (void)p_ws;
    (void)phase_id;
    *((uint32_t*)(&p_accs->iacc)) = R_MACL->MULR0.MULRL;
    *((uint32_t*)(&p_accs->iacc)+1) = R_MACL->MULR0.MULRH;
    *((uint32_t*)(&p_accs->pacc)) = R_MACL->MULR1.MULRL;
    *((uint32_t*)(&p_accs->pacc)+1) = R_MACL->MULR1.MULRH;
    *((uint32_t*)(&p_accs->qacc)) = R_MACL->MULR2.MULRL;
    *((uint32_t*)(&p_accs->qacc)+1) = R_MACL->MULR2.MULRH;
    *((uint32_t*)(&p_accs->vacc)) = R_MACL->MULR3.MULRL;
    *((uint32_t*)(&p_accs->vacc)+1) = R_MACL->MULR3.MULRH;

    p_accs->sample_count = p_ws->accs.sample_count;
}

void LMA_ADC_Init(void)
{
    R_SDADC_B_Open(&g_adc0_ctrl, &g_adc0_cfg);
}

void LMA_ADC_Start(void)
{
    R_SDADC_B_ScanStart(&g_adc0_ctrl);
}

void LMA_ADC_Stop(void)
{
    R_SDADC_B_ScanStop(&g_adc0_ctrl);
}

void LMA_TMR_Init(void)
{
    R_AGT_Open(&g_timer0_ctrl, &g_timer0_cfg);
}

void LMA_TMR_Start(void)
{
    R_AGT_Start(&g_timer0_ctrl);
}

void LMA_TMR_Stop(void)
{
    R_AGT_Stop(&g_timer0_ctrl);
}

void LMA_RTC_Init(void)
{
    R_RTC_Open(&g_rtc0_ctrl, &g_rtc0_cfg);
}

void LMA_RTC_Start(void)
{
    rtc_time_t set_time =
    {
     .tm_sec  = 0,
     .tm_min  = 0,
     .tm_hour = 0,
     .tm_mday = 15,
     .tm_wday = 3,
     .tm_mon  = 1,
     .tm_year = 125,
    };
    R_RTC_PeriodicIrqRateSet(&g_rtc0_ctrl, RTC_PERIODIC_IRQ_SELECT_1_DIV_BY_2_SECOND);
    R_RTC_CalendarTimeSet(&g_rtc0_ctrl, &set_time);
}

void LMA_RTC_Stop(void)
{
    /* TODO: Populate*/
}

void LMA_IMP_ActiveOn(void)
{
    R_PORT1->PODR_b.PODR2 = 1; /* P102 = HIGH*/
}

void LMA_IMP_ActiveOff(void)
{
    R_PORT1->PODR_b.PODR2 = 0; /* P102 = HIGH*/
}

void LMA_IMP_ReactiveOn(void)
{
    /* TODO: Populate*/
}

void LMA_IMP_ReactiveOff(void)
{
    /* TODO: Populate*/
}

void LMA_IMP_ApparentOn(void)
{
    /* TODO: Populate*/
}

void LMA_IMP_ApparentOff(void)
{
    /* TODO: Populate*/
}

