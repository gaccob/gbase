/* sha1.h */
/*
    This file is part of the AVR-Crypto-Lib.
    Copyright (C) 2008  Daniel Otte (daniel.otte@rub.de)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * \file    sha1.h
 * \author  Daniel Otte
 * \email   daniel.otte@rub.de
 * \date    2006-10-08
 * \license GPLv3 or later
 * \brief   SHA-1 declaration.
 * \ingroup SHA-1
 *
 */

#ifndef SHA1_H_
#define SHA1_H_

#include <stdint.h>
/** \def SHA1_HASH_BITS
 * definees the size of a SHA-1 hash in bits
 */

/** \def SHA1_HASH_BYTES
 * definees the size of a SHA-1 hash in bytes
 */

/** \def SHA1_BLOCK_BITS
 * definees the size of a SHA-1 input block in bits
 */

/** \def SHA1_BLOCK_BYTES
 * definees the size of a SHA-1 input block in bytes
 */
#define SHA1_HASH_BITS  160
#define SHA1_HASH_BYTES (SHA1_HASH_BITS/8)
#define SHA1_BLOCK_BITS 512
#define SHA1_BLOCK_BYTES (SHA1_BLOCK_BITS/8)

/** \typedef sha1_ctx_t
 * \brief SHA-1 context type
 *
 * A vatiable of this type may hold the state of a SHA-1 hashing process
 */
typedef struct {
    uint32_t h[5];
    uint64_t length;
} sha1_ctx_t;


/** \fn sha1(sha1_hash_t *dest, const void* msg, uint32_t length_b)
 * \brief hashing a message which in located entirely in RAM
 * This function automatically hashes a message which is entirely in RAM with
 * the SHA-1 hashing algorithm.
 * \param dest pointer to the hash value destination
 * \param msg  pointer to the message which should be hashed
 * \param length_b length of the message in bits
 */
void sha1(void *dest, const void* msg, uint32_t length_b);

#endif /*SHA1_H_*/
