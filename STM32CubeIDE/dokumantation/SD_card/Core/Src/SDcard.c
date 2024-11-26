/*
 * SDcard.c
 *
 *  Created on: Nov 22, 2024
 *      Author: Muhammed Burak Guzeller
 */

#include "SDcard.h"

FATFS fs; // file system
FIL fil;  // file
FRESULT fresult;  // to store the result

#define MAX_SIZE  256
char buffer[MAX_SIZE];

UINT br, bw; //file read/write count

FATFS *pfs;
DWORD fre_clust;

/*
 * @brief : SD karta baglanma kontrolu saglar.
 *
 */
bool mountSDCard(void) {
	fresult = f_mount(&fs, "/", 1);
	if (fresult != FR_OK) {
		debugMessage (SD_CARD_MOUNT_ERROR);
		return false;
	}
	else {
		debugMessage(SD_CARD_MOUNT_SUCCESS);
		return true;
	}
}

/*
 * @brief : SD kart kapasite hesabi yapar.
 *
 */
void SDCardIsCapacity(void) {
	uint32_t total, free_space;
	memset(buffer,0,sizeof(buffer));
	if(mountSDCard()) {
		f_getfree("", &fre_clust, &pfs);
		 total = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
		sprintf(buffer, "SD CARD Total Size: \t%lu\r\n",total);
		debugMessage(buffer);
		memset(buffer,0,sizeof(buffer));
		free_space = (uint32_t)(fre_clust * pfs->csize * 0.5);
		sprintf(buffer,"SD CARD Free Space: \t%lu\r\n", free_space);
		debugMessage(buffer);
		memset(buffer,0,sizeof(buffer));
	}
}

/**
 * @brief  : SD karta sadece dosya oluşturur.
 *           Herhangi bir yazma ya da okuma işlevine sahip değildir.
 *
 * @param  : fileName - Oluşturulacak dosyanın ismi (const char*).
 *
 * @return : bool - Dosya oluşturulursa true, aksi takdirde false döner.
 */
bool createSDFile(const char *fileName) {
    if (!mountSDCard()) {
        return false;
    }

    fresult = f_open(&fil, fileName, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
    if (fresult != FR_OK) {
        debugMessage("FILE CREATION FAILED. ERROR CODE: %d\r\n", fresult);
        return false;
    }

    fresult = f_close(&fil);
    if (fresult != FR_OK) {
        debugMessage("FILE CLOSE FAILED. ERROR CODE: %d\r\n", fresult);
        return false;
    }

    return true;
}


/*
 * @brief : eger hem dosya olusturup hem de dosya icerisine veri
 * yazilmasi gerektiginde kullanilir.
 *
 * @param : const char turunden olusturulacak dosya ismi.
 * @param : fileName dosyasi icerisine yazilacak veri.
 */
void createAndWriteSDFile(const char *fileName, const char *data) {
	if(mountSDCard()) {
		fresult = f_open(&fil, fileName, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
	  	f_puts(data, &fil);
	  	fresult = f_close(&fil);
	  	if(fresult == FR_OK) {
	  		debugMessage("FILE IS SUCCES CREATE AND WRITE\r\n");
	  	}
	  	else {
	        debugMessage("FILE IS NOT CREATE AND WROTE, ERROR CODE : %d\r\n", fresult);
	  	}
	}
}

/*
 * @brief : SD karta loglama islemini gerceklestiren metottur.
 * her veri eklenende dosyanin souna ekler.
 *
 * @param : const char turunden dosya ismi.
 * @param : const char turunden log verisi.
 */
void LoggerSDFile(const char *fileName, const char *data) {
    if (!mountSDCard()) {
        return;
    }

    fresult = f_open(&fil, fileName, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
    if (fresult != FR_OK) {
        return;
    }

    // Dosya sonuna git
    fresult = f_lseek(&fil, f_size(&fil));
    if (fresult != FR_OK) {
        debugMessage("FILE COULD NOT BE LOCATED. ERROR CODE: %d\r\n", fresult);
        f_close(&fil);
        return;
    }

    fresult = f_write(&fil, data, strlen(data), &bw);
    if (fresult == FR_OK && bw == strlen(data)) {
        debugMessage("FILE IS SUCCESFULLY WRITEN.\r\n");
    } else {
        debugMessage("FILE IS NOT WRITEN, ERROR CODE: %d\r\n", fresult);
    }
    fresult = f_close(&fil);
    if (fresult != FR_OK) {
        debugMessage("FILE IS NOT CLOSED, ERROR CODE: %d\r\n", fresult);
    }
}


/**
 * @brief  : Dosyadan veri okunmasını sağlayan metot.
 *
 * @param  : fileName - Okunması istenilen dosyanın ismi (const char*).
 *
 * @return : void
 */
void readSDFileData(const char *fileName) {
    if (!mountSDCard()) {
        return;
    }

    fresult = f_open(&fil, fileName, FA_READ);
    if (fresult == FR_NO_FILE) {
        debugMessage("%s THERE IS NO FILE NAMED\r\n", fileName);
        return;
    } else if (fresult != FR_OK) {
        debugMessage("FILE IS NOT OPEN. ERROR CODE: %d\r\n", fresult);
        return;
    }

    // Dosya boyutunu al
    UINT fileSize = f_size(&fil);
    if (fileSize == 0) {
        debugMessage("FILE IS EMPTY\r\n");
    }
    else if (fileSize <= sizeof(buffer)) {
        memset(buffer, 0, sizeof(buffer));
        fresult = f_read(&fil, buffer, fileSize, &br);
        if (fresult != FR_OK || br == 0) {
            debugMessage("DATA FROM FILE COULD NOT BE READ. ERROR CODE: %d\r\n", fresult);
        }
        else {
            debugMessage(buffer);
        }
    }
    else {
        readLargeSDFile(fileName);
    }

    f_close(&fil);
}

/**
 * @brief  : Büyük dosyaların okunmasını sağlayan fonksiyon. Dosya boyutu,
 *           buffer'ın kapasitesinden büyükse, dosya veri kaybı olmadan okunur.
 *
 * @param  : fileName - Okunmak istenilen dosyanın ismi (const char*).
 *           Dosya SD kart üzerinde mevcut olmalıdır.
 *
 * @return : void
 *
 * Dosya, SD karttan okunmaya başlanır ve dosya boyutuna göre veriler buffer'a
 * bölünerek okunur. Dosya başarıyla okunursa, buffer içeriği her okuma işleminde
 * debugMessage ile yazdırılır. Dosya sonunda, SD kart dosyası kapanır.
 */
void readLargeSDFile(const char *fileName) {
    if (!mountSDCard()) {
        return;
    }

    fresult = f_open(&fil, fileName, FA_READ);
    if (fresult != FR_OK) {
        debugMessage("FILE IS NOT OPEN. ERROR CODE: %d\r\n", fresult);
        return;
    }

    uint32_t fileSize = f_size(&fil); // Dosya boyutunu al
    uint32_t bytesRead = 0;           // Okunan toplam bayt sayısı
    UINT toRead = 0;                  // O an okunacak bayt sayısı

    while (fileSize > bytesRead) {
        // Kalan dosya boyutunu kontrol et ve okuma boyutunu belirle
        toRead = (fileSize - bytesRead > sizeof(buffer)) ? sizeof(buffer) : (fileSize - bytesRead);

        memset(buffer, 0, sizeof(buffer));
        fresult = f_read(&fil, buffer , toRead, &br); // Dosyadan buffer boyutunda veri oku
        if (fresult != FR_OK) {
            debugMessage("FILE READING ERROR, ERROR CODE %d\r\n", fresult);
            break;
        }

        if (sizeof(buffer) > MAX_SIZE - 1) {
            buffer[MAX_SIZE - 1] = '\0';
        }

        debugMessage(buffer);
        bytesRead += br;
    }

    f_close(&fil);
}

/*
 * @brief : SD karttan dosya silinmesini saglar.
 *
 * @param : const char turunden silinmesi istenilen dosya ismi.
 *
 */
void removingSDFile(const char* fileName) {
    char removingFileName[50] = { 0 };
    sprintf(removingFileName, "%s", fileName); // Dosya adı formatı tutarlı

    fresult = f_unlink(removingFileName);
    if (fresult == FR_OK) {
        debugMessage("%s FILE SUCCESSFULLY DELETED.\r\n", fileName);
    } else {
        debugMessage("%s FILE IS NOT DELETED, ERROR CODE: %d\r\n", fileName, fresult);
    }
}

