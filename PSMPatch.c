
#include <taihen.h>
#include <vitasdk.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <psp2/pss.h>


static SceUID LoadModuleHook = -1;
static tai_hook_ref_t LoadModuleHook_ref;

static SceUID cOpenHook = -1;
static tai_hook_ref_t cOpenHook_ref;

static SceUID cReadHook = -1;
static tai_hook_ref_t cReadHook_ref;

static SceUID cCloseHook = -1;
static tai_hook_ref_t cCloseHook_ref;

static SceUID ccReadHook = -1;
static tai_hook_ref_t ccReadHook_ref;

static SceUID ccOpenHook = -1;
static tai_hook_ref_t ccOpenHook_ref;

//For Assemblys:

static SceUID pssCryptoOpenHook = -1;
static tai_hook_ref_t pssCryptoOpenHook_ref;

static SceUID pssCryptoReadHook = -1;
static tai_hook_ref_t pssCryptoReadHook_ref;

static SceUID pssCryptoCloseHook = -1;
static tai_hook_ref_t pssCryptoCloseHook_ref;

int ReplaceFile = 0xFF;

int UpdateSize = 0x00;
int SizeToSet = 0xFFFFF;

int getFileSize(const char *file) {
	SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
	if (fd < 0)
		return fd;
	int fileSize = sceIoLseek(fd, 0, SCE_SEEK_END);
	sceIoClose(fd);
	return fileSize;
}

int ReadFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	int written = sceIoRead(fd, buf, size);

	sceIoClose(fd);
	return written;
}

int WriteFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	if (fd < 0)
		return fd;

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}


typedef struct Storage{
SceUID fd;
char FilePath[1024];
} Storage;

Storage fdToPath[128];

void StoreFD(SceUID fd, char *path)
{
	for(int i = 0;i<128;i++)
	{
		if(fdToPath[i].fd == 0x00)
		{
			fdToPath[i].fd = (uint32_t)fd;
			strcpy(fdToPath[i].FilePath,path);
		}
		
	}
}
void RemoveFD(SceUID fd)
{
	for(int i = 0;i<128;i++)
	{
		if(fdToPath[i].fd == (uint32_t)fd)
		{
			fdToPath[i].fd = 0x00;
			memset(fdToPath[i].FilePath,0x00,1024);
		}
		
	}
}
int GetFD(SceUID fd)
{
	for(int i = 0;i<128;i++)
	{
		if(fdToPath[i].fd == (uint32_t)fd)
		{
			return i;
		}
	}
	return 0xFF;
}
void GetNewPath(char *buf,char *OldPath)
{
	memset(buf,0x00,1024);
	char *NewPath;
	NewPath = OldPath + sizeof("pss0:/top/Application/") - 1;
	sprintf(buf,"pss0:/top/Documents/p/%s",NewPath);
}
int pss_crypto_open_patch(ScePssCryptoHandle *handle, char *path)
{
		
	sceClibPrintf("[PSMPatch] pss_crypto_open(%x %s)\n",handle,path);
	
	int ret;
	ret = TAI_CONTINUE(int, pssCryptoOpenHook_ref, handle, path);
	
		
	char NewPath[1028];
	GetNewPath(NewPath,(char *)path);
	
	if(getFileSize(NewPath) > 0)
	{
		handle->size = getFileSize(NewPath);
		sceClibPrintf("[PSMPatch] Updating ScePssCryptoHandle size! to: %x\n",handle->size);
		StoreFD((uint32_t)handle,(char *)path);
		if(ret != 1)
		{
			sceClibPrintf("[PSMPatch] File exists in PATCH but NOT in APPLICATION, - Faking SUCCESS\n");
			ret = 0x01;
			handle->unk3 = 1;
			handle->unk0 = 0;
			handle->unk1 = 1;
			
		}
	}
	
	sceClibPrintf("[PSMPatch] Handle: unk0 %lx",handle->unk0);
	sceClibPrintf(" unk1 %lx",handle->unk1);
	sceClibPrintf(" Size %lx",handle->size);
	sceClibPrintf(" unk3 %lx\n",handle->unk3);
	
	sceClibPrintf("[PSMPatch] pss_crypto_open ret: %x\n",ret);
	
	return ret;

}


int psmIoOpen_p(const char *path, int flags, SceMode mode, ScePssCryptoHandle *handle, int arg5) //ScePssCryptoHandle is in VitaSDK now :D
{
	sceClibPrintf("[PSMPatch] psmIoOpen(%s,%x,%x,%x,%x);\n",path,flags,mode,handle,arg5);
	
	int ret;
	ret = TAI_CONTINUE(int, ccOpenHook_ref, path, flags, mode,handle, arg5);

	char NewPath[1028];
	GetNewPath(NewPath,(char *)path);
	
	if(getFileSize(NewPath) > 0)
	{

		if((!strstr(path,".exe")) || (!strstr(path,".dll"))) //Ignore for Assemblys.
		{
			sceClibPrintf("[PSMPatch] This is an assembly, Passing on to pss_crypto_open!\n");
			return ret;
		}
		
		handle->size = getFileSize(NewPath);
		sceClibPrintf("[PSMPatch] Updating ScePssCryptoHandle size! to: %x\n",handle->size);
	}
	sceClibPrintf("[PSMPatch] psmIoOpen ret: %x\n",ret);
	
	return ret;
}

char *pss_crypto_read_patch(ScePssCryptoHandle *handle, int ctx) 
{
	sceClibPrintf("[PSMPatch] pss_crypto_read(%x %x)\n",handle,ctx);
	char *ret;
	ret = TAI_CONTINUE(char*, pssCryptoReadHook_ref, handle, ctx);
	
	int index = GetFD((uint32_t)handle);
	
	if(index != 0xFF)
	{
		free(ret);
		
		char Path[1024];
		strcpy(Path,fdToPath[index].FilePath);
		char NewPath[1024];
		memset(NewPath,0x00,1024);
		GetNewPath(NewPath,Path);
		
		char *buf = malloc(handle->size);
		
		
		
		sceClibPrintf("[PSMPatch] Replacing %s with %s\n",Path,NewPath);
		
		ReadFile(NewPath,buf,handle->size);
		
		return buf;
	}
	return ret;
	
}

int psmIoRead_p(int arg1, char *buf, int size, int arg4, int arg5)
{

	
	int ret;
	ret = TAI_CONTINUE(int, ccReadHook_ref, arg1, buf, size, arg4, arg5);
	sceClibPrintf("[PSMPatch] psmIoRead(%x,%x,%x,%x,%x);\n",arg1,buf,size,arg4,arg5);
	
	if(ReplaceFile != 0xFF)
	{
		
		free(buf);

		char Path[1024];
		strcpy(Path,fdToPath[ReplaceFile].FilePath);
		
		if((!strstr(Path,".exe")) || (!strstr(Path,".dll"))) //Ignore for Assemblys. (Let it be handled by pss_crypto_read)
		{
			sceClibPrintf("[PSMPatch] This is an assembly, Passing on to pss_crypto_read!\n");
			return ret;
		}
		
		char NewPath[1024];
		memset(NewPath,0x00,1024);
		GetNewPath(NewPath,Path);
		size = getFileSize(NewPath);
		
		char *buf = malloc(size);
		
		ReadFile(NewPath,buf,size);
		
		sceClibPrintf("[PSMPatch] Replacing %s with %s\n",Path,NewPath);
		ReplaceFile = 0xFF;
	}
	sceClibPrintf("[PSMPatch] PsmIoRed ret: %x");
	return ret;
	}
	
SceUID sceIoOpen_p(const char *file, int flags, SceMode mode)
{
		SceUID ret;
		ret = TAI_CONTINUE(SceUID,cOpenHook_ref, file,flags,mode);

		char NewPath[1028];
		GetNewPath(NewPath,(char *)file);
		
		if(getFileSize(NewPath) > 0)
		{
					
			if((!strstr(file,".exe")) || (!strstr(file,".dll"))) //Ignore for Assemblys.
			{
				sceClibPrintf("[PSMPatch] This is an assembly, Passing on to pss_crypto_open!\n");
				return ret;
			}
			
			sceClibPrintf("[PSMPatch] OPEN fd %x = %s\n",ret,file);
			
			if(ret < 0)
			{
				sceClibPrintf("[PSMPatch] File exists in PATCH but NOT in APPLICATION, - Faking valid File Descriptor\n");
				ret = rand(); // Generate random positive number as File Descriptor.
			}
			
			sceClibPrintf("[PSMPatch] NewPath: %s Storing %x\n",NewPath,ret);
			StoreFD(ret,(char *)file);
			UpdateSize = 0x01;
			SizeToSet = getFileSize(NewPath);
		}
		else
		{
			UpdateSize = 0x00;
			SizeToSet = 0xFFFFF;
		}
		
		return ret;
	}
int sceIoRead_p(SceUID fd, void *data, SceSize size)
{
		int ret;
		
		if(GetFD(fd) != 0xFF)
		{
			sceClibPrintf("[PSMPatch] READ %x\n",fd);
			sceClibPrintf("[PSMPatch] Found file to replace! %x\n",fd);
			ReplaceFile = GetFD(fd);
			
		}
		ret = TAI_CONTINUE(int, cReadHook_ref, fd,data,size);
		return ret;
	}
int sceIoClose_p(SceUID fd)
{
		if(GetFD(fd) != 0xFF)
		{
			RemoveFD(fd);
		}
		int ret;
		ret = TAI_CONTINUE(int, cCloseHook_ref,fd);
		return ret;
}

int pss_crypto_close_patch(ScePssCryptoHandle *handle)
{
	int ret;
	ret = TAI_CONTINUE(int, pssCryptoCloseHook_ref, handle);
	
	if(GetFD((uint32_t)handle) != 0xFF)
	{
			RemoveFD((uint32_t)handle);
	}
	
	return ret;
}

SceUID sceKernelLoadStartModule_p(char *path, SceSize args, void *argp, int flags, SceKernelLMOption *option, int *status)
{
	sceClibPrintf("[PSMPatch] Starting Module: %s\n",path);
	
	SceUID ret;
	ret = TAI_CONTINUE(SceUID, LoadModuleHook_ref, path, args, argp, flags, option, status);
	
	if(!strcmp(path,"app0:/module/libmono_bridge.suprx"))
	{
		sceClibPrintf("[PSMPatch] SceLibMonoBridge Detected!\n");

		ccReadHook = taiHookFunctionExport(&ccReadHook_ref, 
										  "SceLibMonoBridge",
										  TAI_ANY_LIBRARY, 
										  0x32553C73, //SceLibMonoBridge_32553C73 "psmIoRead"
										  psmIoRead_p);
										  
		ccOpenHook = taiHookFunctionExport(&ccOpenHook_ref,
										  "SceLibMonoBridge",
										  TAI_ANY_LIBRARY, 
										  0xC8E1B6B3, //SceLibMonoBridge_C8E1B6B3 "psmIoOpen"
										  psmIoOpen_p);
										  

		pssCryptoOpenHook = taiHookFunctionExport(&pssCryptoOpenHook_ref, 
										  "SceLibMonoBridge",
										  TAI_ANY_LIBRARY, 
										  0x6B4125E4, //pss_crypto_open
										  pss_crypto_open_patch);

		pssCryptoReadHook = taiHookFunctionExport(&pssCryptoReadHook_ref, 
										  "SceLibMonoBridge",
										  TAI_ANY_LIBRARY, 
										  0x32BA8444, //pss_crypto_read
										  pss_crypto_read_patch);
										  
		pssCryptoCloseHook = taiHookFunctionExport(&pssCryptoCloseHook_ref, 
										  "SceLibMonoBridge",
										  TAI_ANY_LIBRARY, 
										  0x37483E03, //pss_crypto_read
										  pss_crypto_close_patch);
										  
										  
		cOpenHook = taiHookFunctionImport(&cOpenHook_ref, 
										  "SceLibMonoBridge",
										  TAI_ANY_LIBRARY, 
										  0x6C60AC61, //sceIoOpen
										  sceIoOpen_p);
		cReadHook = taiHookFunctionImport(&cReadHook_ref, 
										  "SceLibMonoBridge",
										  TAI_ANY_LIBRARY, 
										  0xFDB32293, //sceIoRead
										  sceIoRead_p);
										  

								  
		//SceLibMonoBridge_32553C73 PsmIoRead
		//SceLibMonoBridge_C8E1B6B3 PsmIoOpen

		sceClibPrintf("[PSMPatch] cOpenHook %x, %x\n",cOpenHook,cOpenHook_ref);
		sceClibPrintf("[PSMPatch] cReadHook %x, %x\n",cReadHook,cReadHook_ref);
		sceClibPrintf("[PSMPatch] cCloseHook %x, %x\n",cCloseHook,cCloseHook_ref);
		
		sceClibPrintf("[PSMPatch] pssCryptoOpenHook %x, %x\n",pssCryptoOpenHook,pssCryptoOpenHook_ref);
		sceClibPrintf("[PSMPatch] pssCryptoReadHook %x, %x\n",pssCryptoReadHook,pssCryptoReadHook_ref);
		sceClibPrintf("[PSMPatch] pssCryptoCloseHook %x, %x\n",pssCryptoCloseHook,pssCryptoCloseHook_ref);
	}
	return ret;
}


void _start() __attribute__ ((weak, alias ("module_start"))); 

void module_start(SceSize argc, const void *args) {
	char titleid[12];
	sceAppMgrAppParamGetString(0, 12, titleid, 256);

	if(!strcmp(titleid,"PCSI00011")) // PSM Runtime
	{
		sceClibPrintf("[PSMPatch] Silca: I like to see girls die :3\n");
		memset(fdToPath,0x00,sizeof(fdToPath));
		sceClibPrintf("[PSMPatch] Loaded!\n");
		sceClibPrintf("[PSMPatch] Running on %s\n",titleid);

		LoadModuleHook = taiHookFunctionImport(&LoadModuleHook_ref, 
										  TAI_MAIN_MODULE,
										  TAI_ANY_LIBRARY,
										  0x2DCC4AFA, //sceKernelLoadStartModule
										  sceKernelLoadStartModule_p);

		
		sceClibPrintf("[PSMPatch] LoadModuleHook %x, %x\n",LoadModuleHook,LoadModuleHook_ref);
	}
}

int module_stop(SceSize argc, const void *args) {
	return SCE_KERNEL_STOP_SUCCESS;
}