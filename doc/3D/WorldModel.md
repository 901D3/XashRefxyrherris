# _WorldModel_

## 1. Loading? world model

Yeah, a world is a model.

When we start a new game, Hazard Course for example. the engine will load the Hazard Course world model and we use `ref_client_t::models[1]` to access the world model data, which we will use to render the world model, maybe manipulate some data.

_**Note**_: The world model might be BSP2 format, and the renderer currently doesn't support loading maps in BSP2 format.

A model can have:

- Nodes, `mnode_t`, we will call it mnode.
- Leaf nodes, `mleaf_t`, we will call it mleaf, which is also a mnode. We can reinterpret cast to mleaf ptr if the mnode is guaranteed leaf node.
- Surfaces, `msurface_t`, we will call it msurface.
- Planes, `mplane_t`, we will call it mplane.
- Vertices, `mvertex_t`, we will call it mvertex or mvertices.
- Edges, either `medges16_t` or `medges32_t`, we will call it medge. Currently we only use `medge16_t`.
- Some extra data in `cache_user_t`.
- Light data `color24`, baked lights? Currently unused.
- Submodels aka dmodels `dmodel_t`. Currently unused
- Texture infos,`mtexinfo_t`. Currently unused.
- Clip nodes, `mclipnode16_t`. Currently unused.

A msurface have a sample size. We will use `ref_api_t::Mod_SampleSizeForFace` to get the sample size. \
 For speed, we can save the sample size value to a variable and later when rendering, we don't have to get the sample size for a surface. \
 This only works for case where all of the surfaces in the world model have same sample size. Otherwise, some surfaces might be [luxels](https://developer.valvesoftware.com/wiki/Luxel), has `TEX_EXTRA_LIGHTMAP` flag,.. So those surfaces' sample size value differs from other surfaces, and we can't be sure that a sample size is the same for all surfaces, that means we must get sample size for every surface for rendering. \
 `ref_api_t::Mod_SampleSizeForFace` is just some branches and it's negligible in terms of performance.
