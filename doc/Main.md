
# _Main_

## 1. GetRefAPI

This is the first and only exported function for getting the renderer API.\
The engine's functions is passed to the renderer and the renderer's function is also passed to the engine.

Not just that, the engine also passes `ref_globals_t*`.\
I don't know what's the usage of it but for now, the WIP renderer for Xash3D FWGS with buildnum 4049 haven't use it 

We also save the params structs like `ref_client_t` and `ref_host_t`, note that the function signature don't have `ref_client_t*` and `ref_host_t*` ptrs so we use the engine's function `EngineGetParm` to get the client and host data.

For now, the renderer is WIP so we use the ref_soft REF_API_VERSION from Xash3D FWGS so we can "trick" the engine into thinking the renderer is valid, we'll just ignore the version number that the engine passed to the renderer.

```
// have some structs
#include "ref_api.h" // ref_interface_t, ref_api_t, ref_globals_t, ref_client_t, ref_host_t

#include "xash3d_types.h" // EXPORT macro, for convenient

// we use the ref API version from engine
#define REF_API_VERSION 16

ref_api_t *engineFunctions;
ref_globals_t *refGlobals;
ref_client_t *refClient;
ref_host_t *refHost;

static const ref_interface_t exportedRefAPI = {
  // exported renderer functions here
};
```

Now we will define the body for `GetRefAPI`, and also export it

```
extern "C" EXPORT int GetRefAPI(int version, ref_interface_t *outRefFunctions, ref_api_t *inEngineFunctions, ref_globals_t *inRefGlobals);
extern "C" EXPORT int GetRefAPI(int version, ref_interface_t *outRefFunctions, ref_api_t *inEngineFunctions, ref_globals_t *inRefGlobals) {
  // fill in our callbacks
  *outRefFunctions = exportedRefAPI;

  // save ptr
  engineFunctions = inEngineFunctions;
  refGlobals      = inRefGlobals;

  // save client and host data
  refClient = reinterpret_cast<ref_client_t *>(engineFunctions->EngineGetParm(PARM_GET_CLIENT_PTR, 0));
  refHost   = reinterpret_cast<ref_host_t *>(engineFunctions->EngineGetParm(PARM_GET_HOST_PTR, 0));

  // return the correct API version
  return REF_API_VERSION;
}
```
