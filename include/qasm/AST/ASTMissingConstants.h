/* -*- coding: utf-8 -*-
 *
 * Copyright 2021 IBM RESEARCH. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * =============================================================================
 */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__APPLE__)

#include <math.h>

/* 128-bit precision math constants that are unavailble on Apple iOS. */

#ifndef M_El
#define M_El           2.718281828459045235360287471352662498L /* e */
#endif

#ifndef M_LOG2El
#define M_LOG2El       1.442695040888963407359924681001892137L /* log_2 e */
#endif

#ifndef M_LOG10El
#define M_LOG10El      0.434294481903251827651128918916605082L /* log_10 e */
#endif

#ifndef M_LN2l
#define M_LN2l         0.693147180559945309417232121458176568L /* log_e 2 */
#endif

#ifndef M_LN10l
#define M_LN10l        2.302585092994045684017991454684364208L /* log_e 10 */
#endif

#ifndef M_PIl
#define M_PIl          3.141592653589793238462643383279502884L /* pi */
#endif

#ifndef M_PI_2l
#define M_PI_2l        1.570796326794896619231321691639751442L /* pi/2 */
#endif

#ifndef M_PI_4l
#define M_PI_4l        0.785398163397448309615660845819875721L /* pi/4 */
#endif

#ifndef M_1_PIl
#define M_1_PIl        0.318309886183790671537767526745028724L /* 1/pi */
#endif

#ifndef M_2_PIl
#define M_2_PIl        0.636619772367581343075535053490057448L /* 2/pi */
#endif

#ifndef M_2_SQRTPIl
#define M_2_SQRTPIl    1.128379167095512573896158903121545172L /* 2/sqrt(pi) */
#endif

#ifndef M_SQRT2l
#define M_SQRT2l       1.414213562373095048801688724209698079L /* sqrt(2) */
#endif

#ifndef M_SQRT1_2l
#define M_SQRT1_2l     0.707106781186547524400844362104849039L /* 1/sqrt(2) */
#endif

#endif // __APPLE__

#ifdef __cplusplus
} // extern "C"
#endif


