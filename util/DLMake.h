/**
 * in linux DLMakeInit DLMakeDestroy getFunc can compile c code to so ,and load func from it
 * other plateforme  only support load func from dynamic library file
 */
#ifdef __cplusplus
extern "C"
{
#endif
#if (defined _LINUX ) && ((!defined _ANDROID) || (!defined _IOS))
void DLMakeInit();
void DLMakeDestroy();
void *getFunc(const char *code,size_t size,const char *funcName);
#endif
/*open  dynamic library file and create dllHandle*/
int createDllHandle(void **dllHandle,const char *dllPath);
/*load func from  dllHandle*/
void * getFuncFromDLL(const char * funcName,void *dllHandle);

void destroyDllHandle(void *dllHandle);
#ifdef __cplusplus
}
#endif
