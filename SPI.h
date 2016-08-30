/*! @file SPI.h
 *
 *  @brief Serial Port Interface.
 *
 *  This module is a HAL (Hardware abstraction Layer) for the SPI on the PIC.
 *  It facilitates communication between hardware and software for other modules
 *  such as SM, IR, and EEPROM.
 *
 *  @author Andrew.P
 *  @date 02-08-2016
 */
#ifndef SPI_H
#define	SPI_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "types.h"

/* TSPI_MODE is used to select different SPI modes */
typedef enum {
  SPI_NONE = 0,
  SPI_EEPROM = 2,
  SPI_SM = 4
}TSPI_MODE;

/*!@brief Sets up the SPI interface before first use.
 *
 * @return BOOL - true if the SPI was successfully initialized.
 */
bool SPI_Init(void);

/*!@brief Changes the module (mode) SPI is referencing.
 *
 * @param mode - The mode to change to
 */
void SPI_SelectMode(TSPI_MODE mode);

/*!@brief Sends data to the currently selected SPI module.
 *
 * @param txData - The data to transmit
 * @return rxData - The data that is potentially received during this process
 */
uint8_t SPI_SendData(uint8_t txData);

#ifdef	__cplusplus
}
#endif

#endif	/* SPI_H */