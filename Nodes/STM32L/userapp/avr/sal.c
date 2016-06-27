/**
 * @file sal.c
 *
 * @brief Low-level crypto API for an AES unit implemented in AT86RF231
 *
 * This file implements the low-level crypto API based on an AES unit
 * implemented in an Atmel's radio transceiver AT86RF231.
 *
 * $Id: sal.c 12326 2008-11-28 08:53:44Z sschneid $
 *
 */
/**
 * @author
 *      Atmel Corporation: http://www.atmel.com
 *      Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2008, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> LICENSE.txt
 */

/* === Includes ============================================================ */

//#include "sal_types.h"
#include "rf_type.h"
#if ((RF_PLATFORM== ZGB_LINK_212) || (RF_PLATFORM== ZGB_LINK_231) )
#include <string.h>
#include "tat.h"
//#include "platform_types.h"
#include "hal.h"
#include "sal.h"

/* === Macros ============================================================== */

#define AES_RY_BIT                  (1)     /* AES_RY: poll on finished op */
#define AES_DIR_VOID                (AES_DIR_ENCRYPT + AES_DIR_DECRYPT + 1)
                                            /* Must be different from both summands */

/* === Types =============================================================== */


/* === Globals ============================================================= */

/* True after sal_aes_setup(). */
bool setup_flag;
/* True if decryption key is actual and was computed. */
bool dec_initialized = false;
/* Buffer written over SPI to AES unit. */
uint8_t aes_buf[AES_BLOCKSIZE+2];
/* Last value of "dir" parameter in sal_aes_setup(). */
uint8_t last_dir = AES_DIR_VOID;
/* Actual encryption key. */
uint8_t enc_key[AES_KEYSIZE];
/* Actual decryption key (valid if last_dir == AES_DIR_DECRYPT). */
uint8_t dec_key[AES_KEYSIZE];

//uint8_t aes_key[16]  = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

/* === Implementation ====================================================== */

/**
 * @brief Initialization of SAL.
 *
 * This functions initializes the SAL.
 * For chips with SPI, this function is empty.
 *
 */
void sal_init(void)
{
}



/**
 * @brief Setup AES unit
 *
 * This function perform the following tasks as part of the setup of the
 * AES unit: key initialization, set encryption direction and encryption mode.
 *
 * In general, the contents of SRAM buffer is destroyed. When using
 * sal_aes_wrrd(), sal_aes_read() needs to be called in order to get the result
 * of the last AES operation before you may call sal_aes_setup() again.
 *
 * @param[in] key AES key or NULL (NULL: use last key)
 * @param[in] enc_mode  AES_MODE_ECB or AES_MODE_CBC
 * @param[in] dir AES_DIR_ENCRYPT or AES_DIR_DECRYPT
 *
 * @return  False if some parameter was illegal, true else
 */
/*bool sal_aes_setup(uint8_t *key,
                   uint8_t enc_mode,
                   uint8_t dir)
{
    if (key != NULL)
    {
        // Setup key. 
        dec_initialized = false;

        last_dir = AES_DIR_VOID;

        // Save key for later use after decryption or sleep. 
        memcpy(enc_key, key, AES_KEYSIZE);

        // Set subregister AES_MODE (Bits 4:6 in AES_CON) to 1: KEY SETUP. 
        aes_buf[0] = AES_MODE_KEY;

        // Fill in key. 
        memcpy(aes_buf+1, key, AES_KEYSIZE);

        // Write to SRAM in one step. 
        hal_sram_write(AES_CON, AES_BLOCKSIZE+1, aes_buf);
    }

    // Set encryption direction. 
    switch(dir)
    {
        case AES_DIR_ENCRYPT:
            if (last_dir == AES_DIR_DECRYPT)
            {
                 // If the last operation was decryption, the encryption
                 //key must be stored in enc_key, so re-initialize it.
               
                aes_buf[0] = AES_MODE_KEY;

                // Fill in key. 
                memcpy(aes_buf+1, enc_key, AES_KEYSIZE);

                // Write to SRAM in one step. 
                hal_sram_write(AES_CON, AES_BLOCKSIZE+1, aes_buf);
            }
            break;

        case AES_DIR_DECRYPT:
            if (last_dir != AES_DIR_DECRYPT)
            {
                aes_buf[0] = AES_MODE_KEY;

                if (!dec_initialized)
                {
                    uint8_t dummy[AES_BLOCKSIZE];

                    // Compute decryption key and initialize unit with it. 

                    // Dummy ECB encryption. 
                    aes_buf[0] = AES_MODE_ECB;
                    aes_buf[AES_BLOCKSIZE+1] = AES_MODE_ECB | AES_REQUEST;
                    setup_flag = true;  // Needed in sal_aes_wrrd(). 
                    sal_aes_wrrd(dummy, NULL);

                    // Read last round key:

                    // Set to key mode. 
                    aes_buf[0] = AES_MODE_KEY;
                    hal_sram_write(AES_CON, 1, aes_buf);

                    // Read the key. 
                    hal_sram_read(AES_STATE_KEY, AES_KEYSIZE, dec_key);
                }

              
                 // Now the decryption key is computed resp. known,
                 // simply re-initialize the unit;
               // aes_buf[0] is AES_MODE_KEY
                

                // Fill in key. 
                memcpy(aes_buf+1, dec_key, AES_KEYSIZE);

                // Write to SRAM in one step. 
                hal_sram_write(AES_CON, AES_BLOCKSIZE+1, aes_buf);

                dec_initialized = true;
            }
            break;

        default:
            return false;
    }

    last_dir = dir;

    // Set encryption mode. 
    switch(enc_mode)
    {
        case AES_MODE_ECB:
        case AES_MODE_CBC:
            {
                aes_buf[0] = enc_mode | dir;
                aes_buf[AES_BLOCKSIZE+1] = enc_mode | dir | AES_REQUEST;
            }
            break;

        default:
            return (false);
    }

    setup_flag = true;

    return (true);
}*/

//bool sal_aes_setup(uint8_t *key, uint8_t enc_mode,uint8_t dir)
bool sal_aes_setup(uint8_t *key)
{
	
	if (key != NULL)
	{
	    // Save key for later use after decryption or sleep. 
	    memcpy(enc_key, key, AES_KEYSIZE);

	    // Set subregister AES_MODE (Bits 4:6 in AES_CON) to 1: KEY SETUP. 
	    aes_buf[0] = AES_MODE_KEY;

	    // Fill in key. 
	    memcpy(aes_buf+1, key, AES_KEYSIZE);

	    // Write to SRAM in one step. 
	    hal_sram_write(AES_CON, AES_BLOCKSIZE+1, aes_buf);
	}

   	return true;
}



/**
 * @brief Re-inits key and state after a sleep or TRX reset
 *
 * This function re-initializes the AES key and the state of the
 * AES engine after TRX sleep or reset.
 * The contents of AES registers AES_CON and AES_CON_MIRROR
 * are restored, the next AES operation started with sal_aes_wrrd()
 * will be executed correctly.
 * However, the contents of SRAM buffers is destroyed, in general.
 * When using sal_aes_wrrd(), call sal_aes_read() to get the result
 * of the last AES operation BEFORE you put the transceiver unit to
 * sleep state!
 */
void sal_aes_restart(void)
{
    uint8_t *keyp;
    uint8_t save_cmd;

    if (last_dir == AES_DIR_ENCRYPT)
    {
        keyp = enc_key;
    }
    else
    {
        keyp = dec_key;
    }

    save_cmd = aes_buf[0];
    aes_buf[0] = AES_MODE_KEY;

    /* Fill in key. */
    memcpy(aes_buf+1, keyp, AES_KEYSIZE);

    /* Write to SRAM in one step. */
    hal_sram_write(AES_CON, AES_BLOCKSIZE+1, aes_buf);

    aes_buf[0] = save_cmd;
    setup_flag = true;
}



/**
 * @brief Writes data, reads previous result and does the AES en/decryption
 *
 * The function returns after the AES operation is finished.
 *
 * When sal_aes_wrrd() is called several times in sequence, from the
 * second call onwards, odata contains the result of the previous operation.
 * To obtain the last result, you must call sal_aes_read() at the end.
 * Please note that any call of sal_aes_setup() as well as putting
 * the transceiver to sleep state destroys the SRAM contents,
 * i.e. the next call of sal_aes_wrrd() yields no meaningful result.
 *
 * @param[in]  idata  AES block to be en/decrypted
 * @param[out] odata  Result of previous operation
 *                    (odata may be NULL or equal to idata)
 */
/*void sal_aes_wrrd(uint8_t *idata, uint8_t *odata)
{
    uint8_t save_cmd;

     // Write data and start the operation.
     // AES_MODE in aes_buf[0] and aes_buf[AES_BLOCKSIZE+1] as well as
     // AES_REQUEST in aes_buf[AES_BLOCKSIZE+1]
     // were set before in sal_aes_setup()
    
 
    memcpy(aes_buf+1, idata, AES_BLOCKSIZE);

    // pal_trx_aes_wrrd() overwrites aes_buf, the last byte must be saved. 
    //save_cmd = aes_buf[AES_BLOCKSIZE+1];
    

  //  if (setup_flag)
  // {
   //    hal_trx_aes_wrrd(AES_CON, aes_buf, AES_BLOCKSIZE+2);
    //   setup_flag = false;
   // }
   // else
   // {
        hal_trx_aes_wrrd(AES_STATE_KEY, aes_buf+1, AES_BLOCKSIZE+1);
   // }

    // Restore the result. 
   if (odata != NULL)
    {
        memcpy(odata, aes_buf+1, AES_BLOCKSIZE);
    }

    aes_buf[AES_BLOCKSIZE+1] = save_cmd;

    // Wait for the operation to finish for 24 us. 
    delay_us(24);
}*/

void sal_aes_wrrd(uint8_t *idata,uint8_t enc_mode,uint8_t dir)
{
	/*if(setup_flag&&(dir==AES_DIR_DECRYPT))
	{
		sal_aes_setup(dec_key);
		setup_flag=false;
	}*/
	aes_buf[0]=(enc_mode|dir);
	memcpy(aes_buf+1, idata, AES_BLOCKSIZE);
	aes_buf[AES_BLOCKSIZE+1]=(enc_mode | dir | AES_REQUEST);
	hal_trx_aes_wrrd(AES_CON, aes_buf, AES_BLOCKSIZE+2);
	delay_us(24);
	
	/*if(dir==AES_DIR_ENCRYPT)
	{
		aes_buf[0]=AES_MODE_KEY;
		hal_sram_read(AES_STATE_KEY, AES_KEYSIZE, dec_key);
		setup_flag = true;
	}*/
	//hal_sram_write(0x94, 1, 0x80);
	
}

void sal_aes_dec_key(uint8_t dir)
{
	uint8_t i;
	if(dir==AES_DIR_ENCRYPT)
	{
		aes_buf[0]=AES_MODE_KEY;
		for(i=1;i<AES_KEYSIZE;i++)
		{
			aes_buf[i]=0x00;
		}
		 hal_sram_write(AES_CON, AES_BLOCKSIZE+1, aes_buf);
		 hal_sram_read(AES_STATE_KEY, AES_KEYSIZE, dec_key);
		 setup_flag = true;
	}
	else if((dir==AES_DIR_DECRYPT)&&setup_flag)
	{
		sal_aes_setup(dec_key);
		setup_flag=false;
	}
}

/**
 * @brief Reads the result of previous AES en/decryption
 *
 * This function returns the result of the previous AES operation,
 * so this function is needed in order to get the last result
 * of a series of sal_aes_wrrd() calls.
 *
 * @param[out] data     - result of previous operation
 */
void sal_aes_read(uint8_t *data)
{
    hal_sram_read(AES_STATE_KEY, AES_BLOCKSIZE, data);
}

#endif /* AT86RF231 */

/* EOF */
