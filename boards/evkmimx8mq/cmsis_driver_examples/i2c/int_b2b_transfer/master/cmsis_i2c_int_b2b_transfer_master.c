/*
 * The Clear BSD License
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*  Standard C Included Files */
#include <string.h>
/*  SDK Included Files */
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_i2c.h"
#include "Driver_I2C.h"
#include "fsl_i2c_cmsis.h"

#include "pin_mux.h"
#include "clock_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_I2C_MASTER Driver_I2C2
#define I2C_MASTER_SLAVE_ADDR 0x7EU
#define I2C_DATA_LENGTH 32U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint8_t g_master_txBuff[I2C_DATA_LENGTH];
uint8_t g_master_rxBuff[I2C_DATA_LENGTH];
volatile bool g_MasterCompletionFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t I2C2_GetFreq(void)
{
    return  CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / (CLOCK_GetRootPreDivider(kCLOCK_RootI2c2)) / \
        (CLOCK_GetRootPostDivider(kCLOCK_RootI2c2)) / 5 ;/* SYSTEM PLL1 DIV5 */
}


void I2C_MasterSignalEvent_t(uint32_t event)
{
    if (event == ARM_I2C_EVENT_TRANSFER_DONE)
    {
        g_MasterCompletionFlag = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Board specific RDC settings */
    BOARD_RdcInit();

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitMemory();

    CLOCK_SetRootMux(kCLOCK_RootI2c2, kCLOCK_I2cRootmuxSysPll1Div5); /* Set I2C source to SysPLL1 Div5 160MHZ */
    CLOCK_SetRootDivider(kCLOCK_RootI2c2, 1U, 4U);                /* Set root clock to 160MHZ / 4 = 40MHZ */

    /*Init I2C*/
    EXAMPLE_I2C_MASTER.Initialize(I2C_MasterSignalEvent_t);

    /* Enable i2c moduole and i2c clock */
    EXAMPLE_I2C_MASTER.PowerControl(ARM_POWER_FULL);

    /*config transmit speed*/
    EXAMPLE_I2C_MASTER.Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);

    PRINTF("\r\nCMSIS I2C board2board interrupt example -- Master transfer.\r\n");

    /*Set up i2c master to send data to master*/
    for (uint32_t i = 0U; i < I2C_DATA_LENGTH; i++)
    {
        g_master_txBuff[i] = i;
    }
    PRINTF("Master will send data :");

    for (uint32_t i = 0U; i < I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_txBuff[i]);
    }
    PRINTF("\r\n\r\n");

    /*start transfer*/
    EXAMPLE_I2C_MASTER.MasterTransmit(I2C_MASTER_SLAVE_ADDR, g_master_txBuff, I2C_DATA_LENGTH, false);

    /*wait for master complete*/
    while (!g_MasterCompletionFlag)
    {
    }

    /*  Reset master completion flag to false. */
    g_MasterCompletionFlag = false;

    PRINTF("Receive sent data from slave :");

    EXAMPLE_I2C_MASTER.MasterReceive(I2C_MASTER_SLAVE_ADDR, g_master_rxBuff, I2C_DATA_LENGTH, false);

    /*wait for master complete*/
    while (!g_MasterCompletionFlag)
    {
    }
    /*  Reset master completion flag to false. */
    g_MasterCompletionFlag = false;

    for (uint32_t i = 0U; i < I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_rxBuff[i]);
    }
    PRINTF("\r\n\r\n");

    /* Transfer completed. Check the data.*/
    for (uint32_t i = 0U; i < I2C_DATA_LENGTH; i++)
    {
        if (g_master_rxBuff[i] != g_master_txBuff[i])
        {
            PRINTF("\r\nError occured in the transfer ! \r\n");
            break;
        }
    }

    PRINTF("\r\nEnd of I2C example .\r\n");
    while (1)
    {
    }
}
