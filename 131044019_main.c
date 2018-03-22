/**************************************************************************************************************/
/* Gozde DOGAN - 131044019                                                                                    */
/*                                                                                                            */
/* BIL 344 - System Programming Homework 1                                                                    */
/* tiff dosyasi okuma                                                                                         */
/*                                                                                                            */
/* 22 Mart 2018                                                                                               */
/*                                                                                                            */
/* test2.tiff dosyasi 260 - 266 satirlarini yoruma alinca calisiyor.                                          */
/* DEBUG mode eklenmistir. 24. Satir yorum icine alinirsa gereksiz seyler yazilmayacaktir.                    */
/**************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define INTEL "4949"
#define MOTOROLA_UPPER "4D4D"
#define MOTOROLA_LOWER "4d4d"
#define SIZE 10
#define MAXSIZE 100

#define DEBUG /* yazilmasi gerekmeyen bazi seyler icin debug modu kullanildi */

/* tag leri tutabilmek icin olusturulan struct yapisi */
typedef struct
{
    uint16_t   TagId;
    uint16_t   DataType; 
    uint32_t  DataCount;  
    uint32_t  DataOffset;
} TifTag;

/* header bilgilerini tutabilmek için olusturulmus struct yapisi */
typedef struct 
{
    uint16_t  sIdentifier; 
    uint16_t  sVersion;
    uint32_t sIFD_offset;
} TiffHeader;

/* dosyadaki taglerin ve bir sonraki IFD nin yer aldigi adresin bulundugu struct yapisi */
typedef struct 
{
    uint16_t    NumDirEntries; 
    TifTag TagList[MAXSIZE];
    uint32_t   NextIFDOffset; 
} TifDirectory;

/*Image Data'sinin okunmasi icin yazilmis bir struct yapisi */
typedef struct
{
    uint8_t first;
    uint8_t middle;
    uint8_t last;    
}ImageData;

/* Function Definition */
void readTiff(char *tiffFile, int **FileDatas);
/* tiff dosyasini ozelliklerine gore okuyan fonksiyon */

void printFileDatas(int **FileDatas, int row, int col);
/* dosyadan okunan verilerin 1 ve 0 seklinde dolduruldugu FileDatas arrayinin yazdirildigi fonksiyon */ 

void printTag(TifTag source);
/* Gelen tag'i ozelliklerine gore yazdiran fonksiyon */

void printTagList(TifTag TagList[MAXSIZE], int size);
/* Gelen tag listesini yazdiran fonksiyon */

void findByteOrder(char sIdentifier[SIZE]);
/* header daki identifier bilgisine gore dosya tipinin belirlendigi fonskiyon */

void findWidthAndHeight(FILE* tiff);
/* height ve width bulmak icin yazildi */

int hexadecimalToDecimal(char hexVal[SIZE]);
/* hexadecimal sayiyi decimal'e ceviren fonksyion */


int iWidth = 0; /* FileDatas arrayinin column(sutun) bilgisinin tutuldugu degisken */
int iHeight = 0; /* FileDatas arrayinin row(satir) bilgisinin tutuldugu degisken **/

char byteOrder[SIZE]; /* Dosya tipinin tutuldugu degisken */


/************************************** START OF MAIN **************************************/
int main(int argc, char* argv[]){
    
    if(argc != 2){
        fprintf(stdout, "-------------Usage-------------\n");
        fprintf(stdout, "./tiffProcessor testFile.tiff\n");
        fprintf(stdout, "-------------------------------\n");
        return 0;
    }
    
    #ifdef DEBUG
        fprintf(stdout, "tiffFile: %s\n\n", argv[1]);
    #endif

    int **FileDatas;

    readTiff(argv[1], FileDatas);
    //printFileDatas(FileDatas, row, col);

    return 0;
}
/************************************** END OF MAIN **************************************/

/*****************************************************************************/
/* tiffFile : Dosya adi
/* FileDatas: Dosya iceriginin 1 ve 0'lar seklinde tutuldugu array
/*
/* tiff dosyasinin iceriginin okunup 1 ve 0'lar seklinde tutuldugu fonksiyon
/*****************************************************************************/
void readTiff(char *tiffFile, int **FileDatas){

    FILE *tiff;

    tiff = fopen(tiffFile, "rb");
    if(!tiff)
        perror("fopen :");

    #ifdef DEBUG
    else
        fprintf(stdout, "Tiff file opened\n\n");
    #endif

    findWidthAndHeight(tiff);
    fprintf(stdout, "FONKSIYONDAN SONRA>>>> width: %d\theight: %d\n", iWidth, iHeight);

    rewind(tiff);
    /*************************** HEADER OKUNUYOR ***************************/
    TiffHeader header;
    int iStatus = 0;
    char sIdentifier[SIZE];
    char sVersion[SIZE];
    char sIFD_offset[SIZE];

    
    iStatus = fread(&header, sizeof(TiffHeader), 1, tiff);
    if(iStatus == -1)
        perror("fread: ");

    #ifdef DEBUG     
        fprintf(stdout, "header: %xh\n", header);
        fprintf(stdout, "--------------------------------------------------------------------\n");
        fprintf(stdout, "sIdentifier: %xh\t", header.sIdentifier);
        fprintf(stdout, "sVersion: %xh\t", header.sVersion);
        fprintf(stdout, "sIFD_offset: %xh\t\n", header.sIFD_offset);

        fprintf(stdout, "--------------------------------------------------------------------\n");
    #endif

    sprintf(sIdentifier, "%x", header.sIdentifier);
    sprintf(sVersion, "%x", header.sVersion);
    sprintf(sIFD_offset, "%x", header.sIFD_offset);

    #ifdef DEBUG  
        fprintf(stdout, "sIdentifier: %sh\t", sIdentifier);
        fprintf(stdout, "sVersion: %sh\t", sVersion);
        fprintf(stdout, "sIFD_offset: %sh\t\n", sIFD_offset); 
        fprintf(stdout, "--------------------------------------------------------------------\n");
    #endif
    /**************************** HEADER OKUNDU ****************************/

    /************************************* BYTE ORDER BELIRLENIYOR *************************************/
    findByteOrder(sIdentifier);
    /************************************** BYTE ORDER BELIRLENDI **************************************/

    /******************************* ADRESE GIDILIP NumDirEntries OKUNUYOR *******************************/
    if (fseek(tiff, header.sIFD_offset, SEEK_SET) != 0)
        perror("fseek:");


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    char cNextIFDOffset[SIZE];
    
    do {

        TifDirectory directory;
        char sNumDirEntries[SIZE];
        int iNumDirEntries;

        TifTag tempTag;

        iStatus = fread(&(directory.NumDirEntries), sizeof(directory.NumDirEntries), 1, tiff);
        if(iStatus == -1)
            perror("fread: ");

        /***************************** ADRESE GIDILIP NumDirEntries OKUNACAK *****************************/
        sprintf(sNumDirEntries, "%x", directory.NumDirEntries);
        iNumDirEntries = hexadecimalToDecimal(sNumDirEntries);

        #ifdef DEBUG 
            fprintf(stdout, "sNumDirEntries: %sh\t", sNumDirEntries);
            fprintf(stdout, "iNumDirEntries: %d\n", iNumDirEntries); 
        #endif
        /******************************* ADRESE GIDILIP NumDirEntries OKUNDU *******************************/


        /*********************************** TAGLIST DOLDURULUYOR ***********************************/
        for(int i = 0; i < iNumDirEntries; i++){
            fread(&tempTag, sizeof(tempTag), 1, tiff);

            /*#ifdef DEBUG
                fprintf(stdout, "\n\ntempTag: ");
                printTag(tempTag);
            #endif*/

            directory.TagList[i] = tempTag;

            /*#ifdef DEBUG
                fprintf(stdout, "TagList[%d]:\n", i);
                printTag(directory.TagList[i]);
            #endif*/
        }

        #ifdef DEBUG 
            fprintf(stdout, "\n\n"); 
        #endif
        /************************************ TAGLIST DOLDURULDU ************************************/


        /******************************* TAGLIST YAZILIYOR *******************************/
        #ifdef DEBUG
            printTagList(directory.TagList, iNumDirEntries);
        #endif
        /******************************** TAGLIST YAZILDI ********************************/


        /*********************** DataOffset + TagID KONTROLU YAPILIYOR *********************/
        char cDataOffset[10];
        char cDataCount[10];
        char cTagID[10];
        int iTagID;

        for(int i = 0; i < iNumDirEntries; ++i){
            /************************** DataOffset **************************/
            sprintf(cDataOffset, "%x", directory.TagList[i].DataOffset);
            sprintf(cDataCount, "%x", directory.TagList[i].DataCount);
            fprintf(stdout, "\niDataCount: %d\t\t cDataOffset: %s\t\t iDataOffset: %d\t\t ", hexadecimalToDecimal(cDataCount), cDataOffset, hexadecimalToDecimal(cDataOffset));

            if(strlen(cDataOffset) < 4)
                fprintf(stdout, "DON'T GO DATAOFFSET\t");
            else if(strlen(cDataOffset) > 4)
                fprintf(stdout, "NO DATA\t");
            else
                fprintf(stdout, "GO DATAOFFSET\t");
            /************************** DataOffset **************************/


            /**************************** TagID ****************************/
            sprintf(cTagID, "%x", directory.TagList[i].TagId);
            iTagID = hexadecimalToDecimal(cTagID);

            fprintf(stdout, "iTagID: %d\t", iTagID);

            // Burada belirtilen adreslere gidip width ve height'i bulacaktim!!!
            // Ama fseek i yorumdan cikarinca sonsuz donguye giriyor.
            if(iTagID == 256){
                fprintf(stdout, "width\n"); 
                //if (fseek(tiff, directory.TagList[i].DataOffset, SEEK_CUR) != 0)
                 //   perror("fseek:");

                //fread

            }else if(iTagID == 257)
                fprintf(stdout, "height\n");
            else
                fprintf(stdout, "NOTHING\n");

            /**************************** TagID ****************************/
        }
        /*********************** DataOffset + TagID KONTROLU YAPILDI ***********************/


        /***************************** NextIFDOffset OKUNUYOR *****************************/ 
        uint32_t hNextIFDOffset;

        fread(&hNextIFDOffset, sizeof(hNextIFDOffset), 1, tiff);
        sprintf(cNextIFDOffset, "%x", hNextIFDOffset);

        #ifdef DEBUG
            fprintf(stdout, "\nhNextIFDOffset: %xh\t", hNextIFDOffset);
            fprintf(stdout, "cNextIFDOffset: %s\n", cNextIFDOffset);
        #endif
        /***************************** NextIFDOffset OKUNDU *****************************/


        if (strcmp(cNextIFDOffset, "0") != 0 && fseek(tiff, hNextIFDOffset, SEEK_CUR) != 0)
            perror("fseek:");


    }while(strcmp(cNextIFDOffset, "0") != 0);

    if(strcmp(cNextIFDOffset, "0") == 0)
        fprintf(stdout, "\nEnd of the file\n");

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    fclose(tiff);
}

/*********************************************************************************/
/* tiff: input dosyasi                                                           */
/* width ve height'i bulmak icin yazildi.                                        */
/*                                                                               */
/* https://stackoverflow.com/questions/16980088/how-to-read-tiff-file-headers-in */
/* -c?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa   */
/* sitesiden alindi.                                                             */
/*********************************************************************************/
void findWidthAndHeight(FILE* tiff){
    char sWidth[10];
    char sHeight[10];
    unsigned char info[500];
    fread(info, sizeof(unsigned char), 500, tiff); 

    long int width = *(long int*)&info[256];
    short int height = *(short int*)&info[257];

    printf("width : %d \t", width);
    printf("height : %d \n", height);

    sprintf(sWidth, "%d", width);
    sprintf(sHeight, "%d", height);

    iWidth = hexadecimalToDecimal(sWidth);
    iHeight = hexadecimalToDecimal(sHeight);

    rewind(tiff);
}


/*********************************************************************************/
/* FileDatas: dosya iceriginin 1 ve 0'lar seklinde tutuldugu array
/* row: FileDatas arrayinin satir sayisi
/* col: FileDatas arrayinin sutun sayisi
/*
/* FileDatas arrayinin ekrana bastirildigi fonksiyon
/**********************************************************************************/
void printFileDatas(int **FileDatas, int row, int col){
    int i = 0, j = 0;

    fprintf(stdout, "Width: %d\n", iWidth);
    fprintf(stdout, "Height: %d\n", iHeight);

    fprintf(stdout, "%s\n", byteOrder);

    for(i = 0; i < row; i++){
        for(j = 0; j < col; j++){
            fprintf(stdout, " %d", FileDatas[i][j]);
        }
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "\n");

}


/**********************************************************************************/
/* TagList: tag arrayi
/* size: tag arrayinin boyutu
/*
/* Tag listesinin yazdirildigi fonksiyon
/**********************************************************************************/
void printTagList(TifTag TagList[MAXSIZE], int size){
    fprintf(stdout, "TAGLIST:\n");
    for (int i = 0; i < size; ++i){
        fprintf(stdout, "TagList[%d]:\t", i);
        printTag(TagList[i]);
        fprintf(stdout, "\n");
    }
}


/**********************************************************************************/
/* source: TifTag tipindeki arguman
/*
/* Tag'i ozelliklerine gore yazan fonksiyon
/**********************************************************************************/
void printTag(TifTag source){
    char sTagID[SIZE];
    char sDataType[SIZE];
    char sDataCount[SIZE];
    char sDataOffset[SIZE];

    sprintf(sTagID, "%x", source.TagId);
    sprintf(sDataType, "%x", source.DataType);
    sprintf(sDataCount, "%x", source.DataCount);
    sprintf(sDataOffset, "%x", source.DataOffset);

    fprintf(stdout, "TagID: %sh\t", sTagID);
    fprintf(stdout, "DataType: %sh\t", sDataType);
    fprintf(stdout, "DataCount: %sh\t", sDataCount);
    fprintf(stdout, "DataOffset: %sh\t", sDataOffset);
    fprintf(stdout, "\n");
}

/**********************************************************************************/
/* sIdentifier: dosya tipinin tutuldugu degisken
/*
/* sIdentifier'a gore dosya tipinin belirlendigi fonksiyon
/**********************************************************************************/
void findByteOrder(char sIdentifier[SIZE]){
    fprintf(stdout, "in findByteOrder\n");
    //fprintf(stdout, "sIdentifier: %s\tINTEL: %s\tMOTOROLA_UPPER: %s\n", sIdentifier, INTEL, MOTOROLA_UPPER);
    if(strcmp(sIdentifier, INTEL) == 0)
        sprintf(byteOrder, "INTEL");
    else if(strcmp(sIdentifier, MOTOROLA_UPPER) == 0)
        sprintf(byteOrder, "MOTOROLA");
    else if(strcmp(sIdentifier, MOTOROLA_LOWER) == 0)
        sprintf(byteOrder, "MOTOROLA");
    else{
        perror("byteOrder:");
        fprintf(stdout, "Invalid File\n");
    }

    #ifdef DEBUG
        fprintf(stdout, "\nbyte order: %s\n\n", byteOrder);
    #endif
}


/**********************************************************************************/
/* hexVal: hexadecimal verinin string seklinde tutuldugu degisken
/*
/* gelen string'i decimal sayiya ceviren fonksiyon
/**********************************************************************************/
int hexadecimalToDecimal(char hexVal[])
{   
    int len = strlen(hexVal);
    int base = 1;
    int dec_val = 0;

    /************************* HARFLERE BUYUK HARFLERE CEVRILIYOR ***************************/
    for(int i=0; i < strlen(hexVal); ++i){
        if(hexVal[i]>='a' && hexVal[i]<='z')
            hexVal[i] = hexVal[i] - 32;
    }
    /***************************************************************************************/

    for (int i=len-1; i>=0; i--){   
        if (hexVal[i]>='0' && hexVal[i]<='9'){
            dec_val += (hexVal[i] - 48)*base;
            base = base * 16;
        } else if (hexVal[i]>='A' && hexVal[i]<='F'){
            dec_val += (hexVal[i] - 55)*base;
            base = base*16;
        }
    }    
    return dec_val;
}
