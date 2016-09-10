/*! @file FIFO.c
 *
 *  @brief Routines to implement a FIFO buffer.
 *
 *  This contains the structure and "methods" for accessing a byte-wide FIFO.
 *
 *  @author A.Pope
 *  @date 10-09-2016
 */
#include "FIFO.h"

void FIFO_Init(TFIFO * const FIFO)
{
  FIFO->Start = 0;
  FIFO->End = 0;
  FIFO->NbBytes = 0;
}

bool FIFO_Put(TFIFO * const FIFO, const uint8_t data)
{
  bool success = false;

  if (FIFO->NbBytes < FIFO_SIZE) //Ensure that the buffer is not full before putting data within it
  {
    FIFO->NbBytes++;                        //Increment data counter
    FIFO->Buffer[FIFO->End] = data;         //store data in the buffer
    FIFO->End = (FIFO->End + 1) % FIFO_SIZE;//increment end pointer within FIFO_SIZE

    success = true;
  }

  return success;
}

bool FIFO_Get(TFIFO * const FIFO, uint8_t * const dataPtr)
{
  bool success = false;

  if (FIFO->NbBytes > 0)
  {
    FIFO->NbBytes--;
    *dataPtr = FIFO->Buffer[FIFO->Start];
    FIFO->Start = (FIFO->Start + 1) % FIFO_SIZE;

    success = true;
  }

  return success;
}