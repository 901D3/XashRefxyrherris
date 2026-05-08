
# _Main_

## 1. GetRefAPI

This is the first and only exported function for getting the renderer API.\
The engine's functions is passed to the renderer and the renderer's function is also passed to the engine.

Not just that, the engine also passes `ref_globals_t*`.\
I don't know what's the usage of it but for now, the WIP renderer for Xash3D FWGS with buildnum 4049 haven't use it 

We also save the params structs like `ref_client_t` and `ref_host_t`, note that the function signature don't have `ref_client_t*` and `ref_host_t*` ptrs so we use the engine's function `EngineGetParm` to get the client and host data.\
`PARM_GET_CLIENT_PTR` and `PARM_GET_HOST_PTR` is from `ref_parm_e` enum
```
// save those to a ptr variable with the same ptr type
reinterpret_cast<ref_client_t *>(ref_api_t::EngineGetParm(PARM_GET_CLIENT_PTR, 0))
reinterpret_cast<ref_host_t *>(ref_api_t::EngineGetParm(PARM_GET_HOST_PTR, 0))
```


For now, the renderer is WIP so we return the ref_soft REF_API_VERSION from Xash3D FWGS so we can "trick" the engine into thinking the renderer is valid, we'll just ignore the version number that the engine passed to the renderer.
