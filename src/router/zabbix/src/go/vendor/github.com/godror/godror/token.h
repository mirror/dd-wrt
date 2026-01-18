#ifndef GODROR_TOKEN
#define GODROR_TOKEN

struct godrorHwrap {
  uintptr_t handle;
  const char *token;
  const char *privateKey;
};
typedef struct godrorHwrap godrorHwrap;

void *godrorWrapHandle(uintptr_t handle);
int godrorTokenCallbackHandlerDebug(void* context, dpiAccessToken *token);

#endif
