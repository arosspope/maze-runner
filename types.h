/*! @file types.h
 *
 *  @brief Defines variables and libaries used for all modules.
 *
 *  @author Andrew Pope (sn: 11655949)
 *  @date 02-08-2016
 */
#ifndef TYPES_H
#define	TYPES_H

#ifdef	__cplusplus
extern "C" {
#endif
#include <pic.h>
#include <xc.h>
#include <stdint.h>
#include <stdbool.h> //Used for boolean definitions true = 1; false = 0;

#define _XTAL_FREQ 20000000 //Must re-define the XTAL_FREQ here for __delay_ms    

typedef union
{
  int16_t l;
  struct
  {
    int8_t Lo;
    int8_t Hi;
  } s;
} int16union_t;

typedef union
{
  uint16_t l;
  struct
  {
    uint8_t Lo;
    uint8_t Hi;
  } s;
} uint16union_t;

// Union to efficiently access hi and lo parts of a long integer
typedef union
{
  uint32_t l;
  struct
  {
    uint16_t Lo;
    uint16_t Hi;
  } s;
} uint32union_t;

// Union to efficiently access hi and lo parts of a "phrase" (8 bytes)
typedef union
{
  uint64_t l;
  struct
  {
    uint32_t Lo;
    uint32_t Hi;
  } s;
} uint64union_t;

#ifdef	__cplusplus
}
#endif

#endif	/* TYPES_H */

