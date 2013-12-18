/*
 * crc8.h
 * Karl Zeilhofer
 * 15.2.2010
 *
 * Header-File zu crc8.c
 */

#ifndef __CRC8_H
#define __CRC8_H

#include <stdlib.h>
#include <inttypes.h>

#define CRC8POLY ((uint8_t)(1<<5 | 1<<4 | 1<<0)) // CRC-8 Polynom ohne MSB. Quelle: Wikipedia

uint8_t CRC8_run(const uint8_t* data, uint16_t length); // Nutzdaten

#endif // __CRC8_H
