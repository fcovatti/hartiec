/*
 *  hal.c
 *
 *  Copyright 2013 Michael Zillgith
 *
 *  This file is part of libIEC61850.
 *
 *  libIEC61850 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libIEC61850 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with libIEC61850.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  See COPYING file for the complete license text.
 */

#ifndef HAL_C_
#define HAL_C_
#include <stdint.h>

/*! \addtogroup hal
   *
   *  @{
   */

/**
 * Get the system time in milliseconds.
 *
 * The time value returned as 64-bit unsigned integer should represent the milliseconds
 * since the UNIX epoch (1970/01/01 00:00 UTC).
 *
 * \return the system time with millisecond resolution.
 */
uint64_t Hal_getTimeInMs();

/*! @} */

#endif /* HAL_C_ */
