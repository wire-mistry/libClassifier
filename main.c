#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <linux/limits.h>
#include <elf.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include "colours.h"
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

typedef struct sLibInfo {

	char libName[PATH_MAX];
	char isELF;
	char cpuBitSize;
	char endianness;
	char os_abi;
	uint16_t objFileType;
	uint16_t machineType;
	uint32_t objVersionType;
} LibInfo;

extern char objFileTypeStrings[PATH_MAX/8][PATH_MAX/8];
extern char machineTypeStrings[PATH_MAX/8][PATH_MAX/8];
int getFileContentParsed(char *fileContentBuff, LibInfo *libinfo);
int printFinalResults(LibInfo *libinfo,int nos,int noLibFiles);
int printInfo(LibInfo *libinfo,int nos);
#define READ_SIZE 4096*4
//#define READ_SIZE 4096
#define FILE_FORMAT_STRING "file format "
#define ARCH "architecture"
#define MAX_NO_OF_FILES PATH_MAX




int main(int argc, char *argv[]) {
	struct dirent *dirContent=NULL;
	char completePathName[PATH_MAX]={0};
	//char objDumpCmd[PATH_MAX*2]={0};
	char fileContentBuff[READ_SIZE]={0};
	LibInfo mLibInfo[256] = {0};
	int totalEntrs=0;
	int noLibFiles = 0;
	int fileCount = 0;
  //printf("argc is : %d\n",argc);
	if(argc > 1) {
		struct stat dirStatbuf;
		if (stat(argv[1], &dirStatbuf) != 0) {
				printf(RED"Please enter a valid investigation path\n"RESET);
				exit(0);
			}
		if(S_ISDIR(dirStatbuf.st_mode)) {
				//printf("Investigating path %s\n",argv[1]);
				DIR *libDir = opendir(argv[1]);

				if (libDir == NULL) {
        	perror("Could not open current directory" );
        	return 0;
    		}
				while ((dirContent = readdir(libDir)) != NULL) {
            //printf("Investigating on %s\n", dirContent->d_name);
						memset(completePathName,0,sizeof(completePathName));
						snprintf(completePathName,strlen(argv[1])+strlen(dirContent->d_name)+1,"%s%s",argv[1],dirContent->d_name);
						/*if(strcmp(completePathName,""))
						{
							printf("complete path name: %s\n",completePathName);
						}*/
						if (stat(completePathName, &dirStatbuf) != 0){
       					perror("..");
								printf("Not able to stat %s \n",completePathName );
							}
						if(S_ISDIR(dirStatbuf.st_mode)) {
							/*printf("%s seems to be directory, Program still need to figure out a way for this.\n"
										 "Till then request you to rerun the program for this specific path again\n"
										 "Skipping recursive directory, proceeding for further files...\n", dirContent->d_name);*/
										 totalEntrs++;

						}else {
							totalEntrs++;

							//TODO: what to do with .a or static libs.
							FILE *filePtr = fopen(completePathName,"r");
							if(filePtr) {
								char	*retval = fgets(fileContentBuff,sizeof(fileContentBuff),filePtr);
								if(retval == NULL)
								{
									printf("filecontent is empty\n");
								}

								if(retval){
										 memcpy(mLibInfo[fileCount].libName,completePathName,sizeof(mLibInfo[fileCount].libName));
										 //printf("FILE IN mLibInfo:%s\n",mLibInfo[fileCount].libName);
										 if(getFileContentParsed(fileContentBuff,&mLibInfo[fileCount++])) {
											 noLibFiles++;
										 }

		 							}
								}
							}
				}
				//printInfo(mLibInfo,fileCount-1);
				printFinalResults(mLibInfo,fileCount-1, noLibFiles);
		}else {
			printf(RED"Please enter a valid investigation path\n"RESET);
		}
	}else {
		printf(RED"Please enter a valid investigation path\n"RESET);
	}
}
char ELF[3][8] = {
	"No",
	"Yes"
};
int printInfo(LibInfo *libinfo,int nos){
	int looper=0;
	for(;looper<nos;looper++) {
		printf("******FILE NAME: %s*********\n",libinfo[looper].libName);
	  printf("IsELF:\t%s\n",ELF[libinfo[looper].isELF]);
		if(libinfo[looper].isELF) {
			printf("CPU Bit Size:\t%d\n",libinfo[looper].cpuBitSize);
			printf("Endianness:\t%c\n",libinfo[looper].endianness);
			printf("OS_abi:\t%d\n",libinfo[looper].os_abi);
			printf("ObjFileType:\t%s\n",objFileTypeStrings[libinfo[looper].objFileType]);
			printf("MachineType:\t%s\n",machineTypeStrings[libinfo[looper].machineType]);
			printf("ObjVersionType:\t%d\n",libinfo[looper].objVersionType);
		}
		if(!(looper == (nos-1))){
		printf("Enter X for exit any other key for next element\n");
		char cinput=0;
		cinput = fgetc(stdin);
		if((cinput == 'X') || (cinput == 'x'))
		exit(0);
		}
	}
}
int printFinalResults(LibInfo *libinfo,int nos,int noLibFiles){
	int looper=0;
	printf("Total number of libraries %d\n",noLibFiles);
	printf("File ArchType\n"
					"==== ===========\n");
	for(;looper<nos;looper++) {

		//printf("IsELF:\t%s\n",ELF[libinfo[looper].isELF]);
		if(libinfo[looper].isELF) {
			printf("%s\t",libinfo[looper].libName);
			//printf("CPU Bit Size:\t%d\n",libinfo[looper].cpuBitSize);
			//printf("Endianness:\t%c\n",libinfo[looper].endianness);
			//printf("OS_abi:\t%d\n",libinfo[looper].os_abi);
			//printf("ObjFileType:\t%s\n",objFileTypeStrings[libinfo[looper].objFileType]);
			//printf("MachineType:\t%s\n",machineTypeStrings[libinfo[looper].machineType]);
			//printf("ObjVersionType:\t%d\n",libinfo[looper].objVersionType);
			if(strstr(machineTypeStrings[libinfo[looper].machineType],"Intel")) {
				printf("x86_%d\n",libinfo[looper].cpuBitSize);
			}else {
				char cmd[2048]={0};
				snprintf(cmd,sizeof("readelf -A ")+sizeof(libinfo[looper].libName)+sizeof(" > ./tempArchData"),"readelf -A %s  %s",libinfo[looper].libName," > ./tempArchData");
				system(cmd);
				FILE *fp = fopen("./tempArchData","r");
				if(fp) {
					char attr[512]={0};
					if(fgets(attr,512,fp)){
						char *ptrAtt = strstr(attr,"Attribute Section");
						if(ptrAtt){
							ptrAtt += strlen("Attribute Section")+1;
							char *ptrAtt_c = strstr(ptrAtt,"\n");
							*ptrAtt_c = ' ';
							printf("\"%s",ptrAtt);
						}
					}
					fgets(attr,512,fp);//discard this line
					if(fgets(attr,512,fp)){
						char *ptrAtt = strstr(attr,"Tag_CPU_name");
						if(ptrAtt){
							ptrAtt += strlen("Tag_CPU_name")+3;
							printf("%s\n",ptrAtt);
					}
					}
				}
				else
					printf("Unexpected Situation\n");

			}
		}

	}
}
int getFileContentParsed(char *fileContentBuff, LibInfo *libinfo) {
	int retVal=0;
	if(fileContentBuff){

		//Entry Byte at 0th index or 1st position = 0x7F
		//1st index or 2st position = 'E'
		//2nd index or 3rd Pos = 'L'
		//3rd index or 4th Pos = 'F'
		if((*(fileContentBuff + EI_MAG0) == ELFMAG0) && (*(fileContentBuff+EI_MAG1) == ELFMAG1) &&
		   (*(fileContentBuff+EI_MAG2) == ELFMAG2) && (*(fileContentBuff+EI_MAG3) == ELFMAG3)) {
				 libinfo->isELF = 1;//truee
				 //CPU bit size at 4 index or 5th position 2=64bit 1=32bit
		 		if((*(fileContentBuff+EI_CLASS) == ELFCLASS64)) {
		 				libinfo->cpuBitSize = 64;//true
		 			}else if((*(fileContentBuff+EI_CLASS) == ELFCLASS32)) {
		 				libinfo->cpuBitSize = 32;
		 			}else {
		 				libinfo->cpuBitSize = 0;
		 			}
		 			//Data-Endeanness 5th Index or 6th Pos 2=Big 1=Little
		 			if((*(fileContentBuff+EI_DATA) == ELFDATA2MSB)) {
		 					libinfo->endianness = 'B';//true
		 				}else if((*(fileContentBuff+EI_DATA) == ELFDATA2LSB)) {
		 					libinfo->endianness = 'L';
		 				}else {
		 					libinfo->endianness = 0;
		 				}
		 			// EI_VERSION 6th Index or 7th Pos is version Currently being ignored.
		 			//EI_OSABI 7th Index or 8th Pos is OS/ABI
		 			libinfo->os_abi = *(fileContentBuff+EI_OSABI);//assign and print later
		 			//next 	8 bytes are not defined yet in elf.h

		 			//next bytes in header can further tell  machine type and adress details
		 			//EI_OSABI+8 NOT defined
		 			//EI_OSABI+9 and uint16 type for all architecture 32 or 64
		 			libinfo->objFileType = (uint16_t)*(fileContentBuff+EI_OSABI+9);
					memcpy(&(libinfo->objFileType),(fileContentBuff+EI_OSABI+9),sizeof(uint16_t));
		 			libinfo->machineType = (uint16_t)*(fileContentBuff+EI_OSABI+11);
					memcpy(&(libinfo->machineType),(fileContentBuff+EI_OSABI+11),sizeof(uint16_t));
		 			libinfo->objVersionType = (uint32_t)*(fileContentBuff+EI_OSABI+13);
					memcpy(&(libinfo->objVersionType),(fileContentBuff+EI_OSABI+13),sizeof(uint16_t));
					retVal = 1;

			 }else {
				 libinfo->isELF = 0;
			 }

	}
	return retVal;
}
