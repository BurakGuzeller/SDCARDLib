# STM32 SD KART YAPILANDIRMASI
---
1. System clock ayarlamasını istediğiniz şekilde ayarlayabilirsiniz, buraları atlıyorum.

---
---
2. SD kartın haberleşmesini sağlamak için SPI enable yapılmadı full master şeklinde aşağıdaki görselde inceleyebilirsiniz.

![resim1](/images/Ekran%20Görüntüsü%20-%202024-11-26%2010-35-45.png)

---
---
3. SD kart için debug mesjalarını dinlemek ve yazılan verileri ve okunan verileri gözlemlemek için uartı aktif ettim (opsiyonel).

![resim2](/images/Ekran%20Görüntüsü%20-%202024-11-26%2010-35-52.png)

---

---
4. Pinout konfigrasyonlarım bu şekildedir. CS pini ise SPI ın hangi birimle ya da hangi slave adfresi ile haberleşmeye geçmesini istediği bacaktır. SPI CS pini low durumunda slave enable olup high durumunda disable olmaktadır. Bunu stm32'nin bize sağladığı kütüphane dosyaları sağlamakta ama SD için.

![resim3](/images/Ekran%20Görüntüsü%20-%202024-11-26%2010-36-40.png)

---
5. Middleware and Software Packs sekmesi altında FatFS dosya sistemini enable yapmayı unutmayınız aşağıdaki görseldeki gibi.

![resim4](/images/Ekran%20Görüntüsü%20-%202024-11-26%2010-37-12.png)


> ayarlamaları aşağıda belirtildiği gibi yapınız fark ederseniz 2 paramterede değişiklik var sadece.

![resim5](/images/Ekran%20Görüntüsü%20-%202024-11-26%2010-37-23.png)

---

Kurulum bu kadar ki kodlama kısmına sıra.


## Yazılım Kısmı

 * mountSDCard
> SD kartı monte edilip edilmeme durumunu kontrol eder.

```C
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


```
> Örnek kullanım bu şekildedir.

```C
if (mountSDCard()) {
    debugMessage("SD kart monte edildi.");
} else {
    debugMessage("SD kart monte edilemedi.");
}
```

* SDCardIsCapacity

> void SDCardIsCapacity(void)
SD kartın toplam ve boş kapasitesini hesaplar ve raporlar.

```C
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
```

> Örnek kullanım bu şekildedir.

```C
SDCardIsCapacity();

```

*  createSDFile
 
> bool createSDFile(const char *fileName)
Belirtilen isimle bir dosya oluşturur. Başarılıysa true, aksi takdirde false döner.

```C
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

```
> Örnek kullanım bu şekildedir.

```C
if (createSDFile("example.txt")) {
    debugMessage("Dosya oluşturuldu.");
}
```

* createAndWriteSDFile

> void createAndWriteSDFile(const char *fileName, const char *data)
Dosya oluşturur ve içine veri yazar.

```C
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
```

> Örnek kullanım bu şekildedir.

```C
createAndWriteSDFile("log.txt", "Bu bir test verisidir.");

```

* LoggerSDFile

> void LoggerSDFile(const char *fileName, const char *data)
Var olan dosyanın sonuna veri ekler.

```C
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

```


> Örnek kullanım bu şekildedir.

```C
LoggerSDFile("log.txt", "Yeni bir log girdisi.");

```

* readSDFileData

> void readSDFileData(const char *fileName)

```C
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
```

> Örnek kulanım bu şekildedir.

```C
readSDFileData("log.txt");

```

* removingSDFile

> void removingSDFile(const char* fileName)

> Belirtilen dosyayı siler.

```C
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
```

> Örnek kullanım bu şekildeir.

```C
removingSDFile("log.txt");
```