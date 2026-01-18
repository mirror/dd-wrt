#include <stdint.h>
#include "dpiImpl.h"
#include "token.h"

void *godrorWrapHandle(uintptr_t handle) {
  godrorHwrap *a1 = calloc(1, sizeof(godrorHwrap));
  a1->handle = handle;
  return (void *)a1;
}

void TokenCallbackHandler(uintptr_t handle, dpiAccessToken *access_token);

int godrorTokenCallbackHandlerDebug(void *context, dpiAccessToken *acToken) {
  godrorHwrap *a1 = (godrorHwrap*)context;
  if (a1->token) {
    free((void *)a1->token);
  }
  if (a1->privateKey) {
    free((void *)a1->privateKey);
  }
  TokenCallbackHandler(a1->handle, acToken);
  a1->token = acToken->token;
  a1->privateKey = acToken->privateKey;
  return 0;
}
