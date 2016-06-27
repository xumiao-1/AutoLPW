/**
 * @file sal_types.h
 *
 * @brief This file contains defines for SAL types.
 *
 * $Id: sal_types.h 12326 2008-11-28 08:53:44Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2008, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> LICENSE.txt
 */

/* Prevent double inclusion */
#ifndef STAL_TYPES_H
#define STAL_TYPES_H

/* === INCLUDES ============================================================ */

/* SAL types for transceiver based security: */
#define AT86RF2xx                       (0x01)


#define SAL_TYPE  AT86RF2xx

/* SAL types for MCU based security: */
#define ATXMEGA                         (0x11)

/* === PROTOTYPES ========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* STAL_TYPES_H */
/* EOF */
