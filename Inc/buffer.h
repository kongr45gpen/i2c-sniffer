#ifndef BUFFER_H_
#define BUFFER_H_

// ******************************************************
// * A circular buffer for the data received by I2C     *
// *                                                    *
// * Instead of printing the I2C data as soon as it is  *
// * received, we store them in the memory and print    *
// * them when the processor has time to spare.         *
// *                                                    *
// * This buffer is circular, meaning that whenever we  *
// * run out of space, we go back to the start, erasing *
// * any previous data.                                 *
// ******************************************************

#define I2C_BUFFER_SIZE 7000 // size in bytes (1 byte = 1 received I2C bit or START/STOP condition)
uint8_t buffer[I2C_BUFFER_SIZE];
uint16_t bufferPos = 0;   // the current writing position inside the buffer
uint16_t bufferStart = 0; // the current reading position inside the buffer

#endif /* BUFFER_H_ */
