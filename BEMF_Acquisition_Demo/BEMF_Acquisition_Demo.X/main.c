 /*
 * MAIN Generated Driver File
 * 
 * @file main.c
 * 
 * @defgroup main MAIN
 * 
 * @brief This is the generated driver implementation file for the MAIN driver.
 *
 * @version MAIN Driver Version 1.0.0
*/

/*
© [2023] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/

/* Number of pole pairs of a BLDC motor */
#define MOTOR_PAIR_POLES                    4

/* MOSFET switching frequency in Hz */
#define F_SAMPLING                          20000.0

/* uint16_t range mapping: 0 - 359.99 electrical degrees -> 0 - 65535 */
#define DEGREES_TO_U16(DEG)                 (uint16_t)( (float)(DEG) * 65536.0 / 360.0 + 0.5)

/* Speed conversion from RPM to LUT scrolling speed */
#define RPM_TO_U16(RPM)                     (uint16_t)(((float)(RPM) * 65536.0 * (float)(MOTOR_PAIR_POLES)) / ((float)(F_SAMPLING) * 60.0) + 0.5)

/* Sets the amplitude of the sine wave signals, and thus the scaling values of duty cycle in U.Q.1.15 format, ranging from 0 to 1.00 
 * Duty cycle scaling is done in hardware using the hardware accelerator of TCE.*/
#define AMP_TO_U16(X)                       (uint16_t)(32768.0*(X) + 0.5)

/* Speed of the motor - 120 RPM */
#define SPEED                               RPM_TO_U16(120)

/* Amplitude of the sine wave - 10% */
#define AMPLITUDE                           AMP_TO_U16(0.1)


#include "mcc_generated_files/system/system.h"

typedef enum
{
    MUX_PHASE_A = (AC_MUXPOS_AINP5_gc | AC_MUXNEG_AINN1_gc),
    MUX_PHASE_B = (AC_MUXPOS_AINP6_gc | AC_MUXNEG_AINN1_gc),
    MUX_PHASE_C = (AC_MUXPOS_AINP3_gc | AC_MUXNEG_AINN1_gc),
} mux_t;

/* LUT that is used to generate a sinusoidal drive */
static const uint16_t sine_lookup_table[] = 
{
  16384, 16786, 17187, 17589, 17989, 18389, 18788, 19185, 19580, 19973, 20364, 
  20753, 21140, 21523, 21903, 22280, 22653, 23023, 23389, 23750, 24107, 24459, 24807, 
  25149, 25486, 25818, 26143, 26463, 26777, 27085, 27386, 27681, 27969, 28250, 28523, 
  28790, 29049, 29300, 29543, 29779, 30006, 30226, 30437, 30639, 30833, 31018, 31194, 
  31362, 31520, 31670, 31810, 31941, 32062, 32174, 32276, 32369, 32453, 32526, 32590, 
  32644, 32689, 32723, 32748, 32763, 32768, 32763, 32748, 32723, 32689, 32644, 32590, 
  32526, 32453, 32369, 32276, 32174, 32062, 31941, 31810, 31670, 31520, 31362, 31194, 
  31018, 30833, 30639, 30437, 30226, 30006, 29779, 29543, 29300, 29049, 28790, 28523, 
  28250, 27969, 27681, 27386, 27085, 26777, 26463, 26143, 25818, 25486, 25149, 24807, 
  24459, 24107, 23750, 23389, 23023, 22653, 22280, 21903, 21523, 21140, 20753, 20364, 
  19973, 19580, 19185, 18788, 18389, 17989, 17589, 17187, 16786, 16384, 15981, 15580, 
  15178, 14778, 14378, 13979, 13582, 13187, 12794, 12403, 12014, 11627, 11244, 10864, 
  10487, 10114, 9744, 9378, 9017, 8660, 8308, 7960, 7618, 7281, 6949, 6624, 6304, 5990, 
  5682, 5381, 5086, 4798, 4517, 4244, 3977, 3718, 3467, 3224, 2988, 2761, 2541, 2330, 
  2128, 1934, 1749, 1573, 1405, 1247, 1097, 957, 826, 705, 593, 491, 398, 314, 241, 
  177, 123, 78, 44, 19, 4, 0, 4, 19, 44, 78, 123, 177, 241, 314, 398, 491, 593, 705, 
  826, 957, 1097, 1247, 1405, 1573, 1749, 1934, 2128, 2330, 2541, 2761, 2988, 3224, 
  3467, 3718, 3977, 4244, 4517, 4798, 5086, 5381, 5682, 5990, 6304, 6624, 6949, 7281, 
  7618, 7960, 8308, 8660, 9017, 9378, 9744, 10114, 10487, 10864, 11244, 11627, 12014, 
  12403, 12794, 13187, 13582, 13979, 14378, 14778, 15178, 15580, 15981
};

/* Function that is called every 50us to update the drive */
void Motor_Drive(void)
{
/* Counters that scroll through the LUT at runtime. These counter are used to create the 120 degrees 
 * phase shift between each of the motor's phases */
    static uint16_t     phase_a = DEGREES_TO_U16(0.0);
    static uint16_t     phase_b = DEGREES_TO_U16(120.0);
    static uint16_t     phase_c = DEGREES_TO_U16(240.0);
    static const uint16_t speed = SPEED;

    /* Values that will be written in the CMP channels of TCE */
    uint16_t drive_a, drive_b, drive_c;
    
    /* Updating the counters */
    phase_a += speed;
    phase_b += speed;
    phase_c += speed;
    
    /* Select new variables from the LUT for each CMP channel */
    drive_a = sine_lookup_table[(phase_a >> 8)];
    drive_b = sine_lookup_table[(phase_b >> 8)];
    drive_c = sine_lookup_table[(phase_c >> 8)];
    
    /* Update the values from CMP channels with new ones */
    TCE0_CompareChannels012BufferedSet(drive_a, drive_b, drive_c);
}

/* Functions that switches the MUX to monitor all 3 phases at runtime */
void Mux_Set(uint8_t mode)
{
    uint8_t temp;
    temp = AC0.MUXCTRL;
    temp &= ~(AC_MUXPOS_gm | AC_MUXNEG_gm);
    temp |= mode;
    AC0.MUXCTRL = temp;
}

/* Function that is called during the enlarged dead time to read the BEMF state */
void BEMF_Read(void)
{
    bool  bemf_state;
    static mux_t mux = MUX_PHASE_A;

    /* BEMF sampling point marked by an IO toggling */
    IO_PF4_SetHigh();
    bemf_state = ((AC0.STATUS & AC_CMPSTATE_bm) != 0 );
    IO_PF4_SetLow();
    
    /* Switching AC0 MUX from one phase to another one */
    switch(mux)
    {
        case MUX_PHASE_A: if(bemf_state) {IO_PF1_SetHigh();} else {IO_PF1_SetLow();} mux = MUX_PHASE_B; break;
        case MUX_PHASE_B: if(bemf_state) {IO_PF2_SetHigh();} else {IO_PF2_SetLow();} mux = MUX_PHASE_C; break;
        case MUX_PHASE_C: if(bemf_state) {IO_PF3_SetHigh();} else {IO_PF3_SetLow();} mux = MUX_PHASE_A; break;
        default: mux = MUX_PHASE_A; break;
    } 
    
    /* Update Analog Comparator MUX to monitor another phase of the motor */
    Mux_Set(mux);
    
    /* Update drive to keep the motor spinning */
    Motor_Drive();
}

int main(void)
{
    SYSTEM_Initialize();
    
    /* Register the BEMF sampling function as a callback */
    TCE0_Compare3CallbackRegister(BEMF_Read);
    
    /* Enable hardware scaling accelerator after initialization to avoid messing up 
     * the timing for reading BEMF with CMP3 channel of TCE */
    TCE0_ScaleEnable(true);
    
    /* Set the Amplitude level */
    TCE0_AmplitudeSet(AMPLITUDE);
    
    while(1)
    {
    }    
}