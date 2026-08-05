
#include <string>
#include <stdint.h>

#include "xash3d_types.h"

extern "C" {
#include "xash3d_mathlib.h"
}

#include "com_model.h"
#include "cl_entity.h"
#include "cvardef.h"
#include "r_studioint.h"
#include "studio.h"
#include "crtlib.h"
#include "enginefeatures.h"

#include "engine_stuff.h"

#include "rx_base.h"
#include "rx_image.h"
#include "rx_3d.h"
#include "rx_triapi.h"
#include "rx_studioapi.h"
#include "rx_blend.h"
#include "rx_image_manager.h"
#include "rx_logsutils.h"

#include "xyrherris-mathlib.h"

#include "rx_memmgr.h"

#include "rx_macros.h"

STUDIOAPI_EXPORT void StudioAPI_SetHeader( void* header ) { studioAPI.modelHeader = reinterpret_cast<studiohdr_t*>( header ); }

STUDIOAPI_EXPORT void StudioAPI_SetCurrentModel( model_s* model ) { render3D.currModel = model; }

STUDIOAPI_EXPORT model_s* StudioAPI_SetupPlayerModel( int index ) {
  // index is the first person player's index, we check if the index is out of range(check if index is beyond max player in a sv too)
  if ( index < 0 || index >= refClient->maxclients ) return nullptr;

  studioAPI.currPlayerInfo = engineFuncs->pfnPlayerInfo( index );

  rx_playermodel_t* playerModel = &studioAPI.playerModel[ index ];
  memset( playerModel, 0, sizeof( rx_playermodel_t ) );

  playerModel->model = nullptr; // g-cont: force for "dev-mode", non-local games and menu preview

  if ( refGlobals->developer || !engineFuncs->EngineGetParm( PARM_SINGLEPLAYER_GAME, 0 ) || render3D.viewpassFlags & RF_DRAW_WORLD ) {
    if ( strcmp( playerModel->modelName, studioAPI.currPlayerInfo->model ) ) {
      strncpy( playerModel->name, studioAPI.currPlayerInfo->model, sizeof( playerModel->name ) - 1 ); // assign a path name
      snprintf( playerModel->modelName, sizeof( playerModel->modelName ),
        "models/player/%s/%s.mdl",
        studioAPI.currPlayerInfo->model, studioAPI.currPlayerInfo->model ); // check if the targeted model exists

      if ( engineFuncs->fsapi->FileExists( playerModel->modelName, false ) )
        // fallback to current entity's model
        playerModel->model = engineFuncs->Mod_ForName( playerModel->modelName, false, true );

      if ( !playerModel->model ) playerModel->model = render3D.currEntity->model;
    }
  }
  else { // use the current model
    playerModel->model = render3D.currModel;
    playerModel->name[ 0 ] = '\0';
  }

  return playerModel->model;
}

STUDIOAPI_EXPORT void StudioAPI_SetModelBodyPart( int bodyParti, void** bodyPart, void** subModel ) {
  if ( bodyParti > studioAPI.modelHeader->numbodyparts )
    bodyParti = 0;

  studioAPI.bodyPart = reinterpret_cast<mstudiobodyparts_t*>(
    studioAPI.modelHeader + studioAPI.modelHeader->bodypartindex + bodyParti );

  int index = render3D.currEntity->curstate.body / studioAPI.bodyPart->base;
  index %= studioAPI.bodyPart->nummodels;

  studioAPI.subModel = reinterpret_cast<mstudiomodel_t*>(
    studioAPI.modelHeader + studioAPI.bodyPart->modelindex + index );

  if ( bodyPart ) *bodyPart = studioAPI.bodyPart;
  if ( subModel ) *subModel = studioAPI.subModel;
}

STUDIOAPI_DRAW_EXPORT int StudioAPI_DrawModel( int flags ) {
  cl_entity_t* clEntity = render3D.currEntity;
  entity_state_t* currentState = &clEntity->curstate;

  // if the curent entity is a dead player
  if ( currentState->renderfx == kRenderFxDeadPlayer ) {
    // entity is out of range (server max client limit), dont draw
    if ( currentState->renderamt <= 0 || currentState->renderamt > refClient->maxclients )
      return 0;

    entity_state_t deadPlayer;
    memcpy( &deadPlayer, StudioAPI_GetPlayerState( currentState->renderamt - 1 ), sizeof( entity_state_t ) );

    deadPlayer.number = currentState->renderamt;
    deadPlayer.weaponmodel = 0;
    deadPlayer.gaitsequence = 0;
    deadPlayer.movetype = MOVETYPE_NONE;

    VectorCopy( currentState->angles, deadPlayer.angles );
    VectorCopy( currentState->origin, deadPlayer.origin );

    int result = StudioAPI_DrawPlayer( flags, &deadPlayer );

    return result;
  }

  return 1;
}

STUDIOAPI_DRAW_EXPORT int StudioAPI_DrawPlayer( int flags, entity_state_s* playerState ) {
  int playerIdx = playerState->number - 1;
  if ( playerIdx < 0 || playerIdx >= refClient->maxclients ) return 0;

  model_s* playerModel = StudioAPI_SetupPlayerModel( playerIdx );
  if ( !playerModel ) return 0;

  render3D.currModel = playerModel;
  entity_state_s* currentEntityState = &render3D.currEntity->curstate;

  StudioAPI_SetHeader( reinterpret_cast<studiohdr_t*>( engineFuncs->Mod_Extradata( mod_studio, playerModel ) ) );

  if ( playerState->gaitsequence ) {
    studioAPI.currPlayerInfo = StudioAPI_GetPlayerInfo( playerIdx );
    float originalAngle[ 3 ] = Vec3( render3D.currEntity->angles );

    // StudioAPI_ProcessGait( playerState );

    // studioAPI.currentPlayerInfo->gaitsequence = playerState->gaitsequence;
    studioAPI.currPlayerInfo = nullptr;

    StudioAPI_SetupModelTransform(
      render3D.currEntity->origin,
      render3D.currEntity->angles,
      render3D.currEntity->curstate.movetype == MOVETYPE_STEP,
      render3D.currEntity->player,
      render3D.flipViewModel );
    VectorCopy( originalAngle, render3D.currEntity->angles );
  }
  else {
    Vector4Set( currentEntityState->controller, 127, 127, 127, 127 );
    Vector4Copy( currentEntityState->controller, render3D.currEntity->latched.prevcontroller );

    studioAPI.currPlayerInfo = StudioAPI_GetPlayerInfo( playerIdx );
    studioAPI.currPlayerInfo->gaitsequence = 0;

    StudioAPI_SetupModelTransform(
      render3D.currEntity->origin,
      render3D.currEntity->angles,
      render3D.currEntity->curstate.movetype == MOVETYPE_STEP,
      render3D.currEntity->player,
      render3D.flipViewModel );
  }

  if ( flags & STUDIO_RENDER ) {
    if ( StudioAPI_CheckBoundingBox( ) ) return 0;

    studioAPI.frameCount++;

    if ( studioAPI.modelHeader->numbodyparts == 0 ) return 1;
  }

  studioAPI.currPlayerInfo = StudioAPI_GetPlayerInfo( playerIdx );
  StudioAPI_SetupEntityBonesTransforms( render3D.currEntity );

  studioAPI.currPlayerInfo->renderframe = render3D.frameCount;
  studioAPI.currPlayerInfo = nullptr;

  if ( flags & STUDIO_RENDER ) {
    if ( render3D.currModel != render3D.currEntity->model || !( render3D.viewpassFlags & RF_DRAW_WORLD ) )
      render3D.currEntity->curstate.body = 255;

    if ( render3D.currModel == render3D.currEntity->model )
      render3D.currEntity->curstate.body = 1;

    studioAPI.currPlayerInfo = engineFuncs->pfnPlayerInfo( playerIdx );

    StudioAPI_RenderModelWrapper( );
    studioAPI.currPlayerInfo = nullptr;
  }

  return 0;
}

void StudioAPI_SetupModelTransform( float position[], float angle[], bool lerpMovement, bool isPlayer, bool flipViewModel ) {
  if ( lerpMovement && ( refHost->features & ENGINE_COMPUTE_STUDIO_LERP ) )
    ; // unimplemented

  if ( isPlayer ) // dont pitch if entity is a player
    angle[ PITCH ] = 0.f;
  else if ( !( refHost->features, ENGINE_COMPENSATE_QUAKE_BUG ) )
    angle[ PITCH ] = -angle[ PITCH ];

  Matrix3x4_CreateFromEntity( studioAPI.modelTransf.rotationMatrix, angle, position, 1.f );

  // negate y if flip viewmodel is on
  if ( flipViewModel ) {
    studioAPI.modelTransf.rotationMatrix[ 0 ][ 1 ] = -studioAPI.modelTransf.rotationMatrix[ 0 ][ 1 ];
    studioAPI.modelTransf.rotationMatrix[ 1 ][ 1 ] = -studioAPI.modelTransf.rotationMatrix[ 1 ][ 1 ];
    studioAPI.modelTransf.rotationMatrix[ 2 ][ 1 ] = -studioAPI.modelTransf.rotationMatrix[ 2 ][ 1 ];
  }
}

void StudioAPI_RenderModelFinal( ) {
  int renderMode = StudioAPI_GetForceFaceFlags( ) ? kRenderTransAdd : render3D.currEntity->curstate.rendermode;
  StudioAPI_SetupRenderer( renderMode );

  {
    for ( int i = 0; i < studioAPI.modelHeader->numbodyparts; i++ ) {
      StudioAPI_SetModelBodyPart(
        i,
        reinterpret_cast<void**>( &studioAPI.bodyPart ),
        reinterpret_cast<void**>( &studioAPI.subModel ) );

      StudioAPI_SetRenderMode( renderMode );
      StudioAPI_RenderModel( );
      // GL_StudioDrawShadow( );
    }
  }
}

void StudioAPI_LoadModelImage( model_t* model, studiohdr_t* modelMetadata, mstudiotexture_t* modelTexture ) {
  size_t bufferSize;

  char name[ 128 ];
  char imageName[ 128 ];
  char modelName[ 128 ];

  texture_t* texture;

  unsigned int flags = 0;

  if ( modelTexture->flags & STUDIO_NF_NORMALMAP )
    flags |= TF_NORMALMAP;

  bool isDMBaseName = !Q_strnicmp( modelTexture->name, "DM_Base", 7 );

  if ( isDMBaseName || Q_strnicmp( modelTexture->name, "remap", 5 ) ) {
    int imageCount = model->numtextures;
    char value[ 6 ];

    model->textures = reinterpret_cast<texture_t**>(
      ENGINE_REALLOC( model->mempool, model->textures, ( imageCount + 1 ) * sizeof( texture_t* ) ) );

    bufferSize = modelTexture->width * modelTexture->height + 768;

    texture = reinterpret_cast<texture_t*>( ENGINE_ALLOC( model->mempool, sizeof( texture_t ) + bufferSize ) );

    model->textures[ imageCount ] = texture;

    if ( !isDMBaseName ) {
      strcpy( texture->name, "DM_Base" );

      texture->anim_min = PLATE_HUE_START;
      texture->anim_max = PLATE_HUE_END;
      texture->anim_total = SUIT_HUE_END;
    }
    else {
      strcpy( texture->name, "DM_User" );
      strncpy( value + 0, modelTexture->name + 7, 4 );

      texture->anim_min = M_bound( 0, atoi( value ), 255 );
      texture->anim_max = texture->anim_min;
      texture->anim_total = texture->anim_min;
    }

    texture->width = modelTexture->width;
    texture->height = modelTexture->height;

    memcpy( texture + 1, reinterpret_cast<unsigned char*>( modelMetadata ) + modelTexture->index, bufferSize );

    modelTexture->flags |= STUDIO_NF_COLORMAP;
    flags |= TF_FORCE_COLOR;

    model->numtextures++;
  }

  strcpy( modelName, model->name );
  COM_StripExtension( modelName );

  COM_FileBase( modelTexture->name, name, sizeof( name ) );

  if ( modelTexture->flags & STUDIO_NF_NOMIPS ) flags |= TF_NOMIPMAP;

  bufferSize = sizeof( mstudiotexture_t ) + modelTexture->width * modelTexture->height + 768;

  engineFuncs->Image_SetMDLPointer( reinterpret_cast<unsigned char*>( modelMetadata ) + modelTexture->index );
  snprintf( imageName, sizeof( imageName ), "#%s/%s.mdl", modelName, name );
  modelTexture->index = ImageMGR_LoadImage_( imageName, reinterpret_cast<uint8_t*>( modelTexture ), bufferSize, flags );

  if ( !modelTexture->index ) {
    dlog( "can not load model image %s, fall back to default texture", imageName );
    modelTexture->index = refctx.image.defaultImageIdx;
  }

  if ( texture )
    texture->gl_texturenum = modelTexture->index;
}

void StudioAPI_RenderModelWrapper( ) {
  StudioAPI_SetChromePosition( );
  StudioAPI_SetForceFaceFlags( 0 );

  if ( render3D.currEntity->curstate.renderfx == kRenderFxGlowShell ) {
    render3D.currEntity->curstate.renderfx = kRenderFxNone;
  }

  StudioAPI_RenderModelFinal( );
}

void StudioAPI_SetupEntityBonesTransforms( cl_entity_t* clEntity ) {
  static vec3_t pos1[ MAXSTUDIOBONES ];
  static vec4_t q1[ MAXSTUDIOBONES ];
  static vec3_t pos2[ MAXSTUDIOBONES ];
  static vec4_t q2[ MAXSTUDIOBONES ];

  if ( clEntity->curstate.sequence >= studioAPI.modelHeader->numseq )
    clEntity->curstate.sequence = 0;

  // sequence desc of the entity's sequence
  mstudioseqdesc_t* sequenceDesc = reinterpret_cast<mstudioseqdesc_t*>(
    &reinterpret_cast<byte*>( studioAPI.modelHeader )[ studioAPI.modelHeader->seqindex + clEntity->curstate.sequence ] );

  float frame = StudioAPI_EstimateFrame( clEntity, sequenceDesc, studioAPI.time );

  mstudioanim_t* animationList = reinterpret_cast<mstudioanim_t*>( engineFuncs->R_StudioGetAnim( studioAPI.modelHeader, render3D.currModel, sequenceDesc ) );
  StudioAPI_CalcRotations( clEntity, sequenceDesc, animationList, frame, pos1, q1 );

  // animation blending
  // if (sequenceDesc->numblends>1) {}

  // animation interp

  clEntity->latched.prevframe = frame;

  mstudiobone_t* headerBone = reinterpret_cast<mstudiobone_t* >( &reinterpret_cast<byte*>( studioAPI.modelHeader )[ studioAPI.modelHeader->boneindex ] );

  if ( studioAPI.currPlayerInfo && studioAPI.currPlayerInfo->gaitsequence != 0 ) {
    bool copyBones = true;

    if ( studioAPI.currPlayerInfo->gaitsequence >= studioAPI.modelHeader->numseq )
      studioAPI.currPlayerInfo->gaitsequence = 0;

    // sequence desc of the player gait sequence
    sequenceDesc = reinterpret_cast<mstudioseqdesc_t*>(
      &reinterpret_cast<byte*>( studioAPI.modelHeader )[ studioAPI.modelHeader->seqindex + studioAPI.currPlayerInfo->gaitsequence ] );

    // get animation for the new sequence desc
    animationList = reinterpret_cast<mstudioanim_t*>( engineFuncs->R_StudioGetAnim( studioAPI.modelHeader, render3D.currModel, sequenceDesc ) );
    StudioAPI_CalcRotations( clEntity, sequenceDesc, animationList, frame, pos1, q1 );

    for ( int i = 0; i < studioAPI.modelHeader->numbones; i++ ) {
      if ( Q_strcmp( headerBone[ i ].name, "Bip01 Spine" ) )
        copyBones = false;
      else if ( Q_strcmp( headerBone[ headerBone[ i ].parent ].name, "Bip01 Pelvis" ) )
        copyBones = true;

      if ( copyBones ) {
        VectorCopy( pos2[ i ], pos1[ i ] );
        Vector4Copy( q2[ i ], q1[ i ] );
      }
    }
  }

  for ( int i = 0; i < studioAPI.modelHeader->numbones; i++ ) {
    rx_studiobonetransf_t* currBoneTransf = &studioAPI.boneTransf[ i ];
    rx_studiobonetransf_t* parentBoneTransf = &studioAPI.boneTransf[ headerBone[ i ].parent ];

    matrix3x4 boneMatrix;
    Matrix3x4_FromOriginQuat( boneMatrix, q1[ i ], pos1[ i ] );

    if ( headerBone[ i ].parent == -1 ) {
      Matrix3x4_ConcatTransforms( currBoneTransf->boneTransf, studioAPI.rotationMatrix, boneMatrix );
      Matrix3x4_Copy( currBoneTransf->lightTransf, currBoneTransf->lightTransf );

      // apply client-side effects to the transformation matrix
      StudioAPI_FxTransform( clEntity, currBoneTransf->boneTransf );
    }
    else {
      Matrix3x4_ConcatTransforms( currBoneTransf->boneTransf, parentBoneTransf->boneTransf, boneMatrix );
      Matrix3x4_ConcatTransforms( currBoneTransf->lightTransf, parentBoneTransf->lightTransf, boneMatrix );
    }
  }
}

void StudioAPI_CalcRotations( cl_entity_t* clEntity, mstudioseqdesc_t* sequenceDesc, mstudioanim_t* animationList, float frame, vec3_t* outpos, vec4_t* outq ) {
  float adj[ MAXSTUDIOCONTROLLERS ];

  // clamp for safety
  if ( frame > sequenceDesc->numframes - 1 )
    frame = 0.f;
  else if ( frame < -0.01f )
    frame = -0.01f;

  int framei = M_floor( frame );

  // if we dont want to interpolate animation, set this to 0
  float frac = frame - framei;

  mstudiobone_t* bone = reinterpret_cast< mstudiobone_t*>( studioAPI.modelHeader + studioAPI.modelHeader->boneindex );

  StudioAPI_CalcBoneAdjust( clEntity->curstate.controller, static_cast<float>( clEntity->mouth.mouthopen ), adj );

  for ( int i = 0; i < studioAPI.modelHeader->numbones; i++ )
    R_StudioCalcBones( framei, frac, &bone[ i ], &animationList[ i ], adj, outpos[ i ], outq[ i ] );

  if ( sequenceDesc->motiontype & STUDIO_X ) outpos[ sequenceDesc->motionbone ][ 0 ] = 0.f;
  if ( sequenceDesc->motiontype & STUDIO_Y ) outpos[ sequenceDesc->motionbone ][ 1 ] = 0.f;
  if ( sequenceDesc->motiontype & STUDIO_Z ) outpos[ sequenceDesc->motionbone ][ 2 ] = 0.f;
}

void StudioAPI_CalcBoneAdjust( byte* controller1, float mouthOpenValue, float* out ) {
  mstudiobonecontroller_t* bcontroller = reinterpret_cast<mstudiobonecontroller_t*>( &reinterpret_cast<byte*>( studioAPI.modelHeader )[ studioAPI.modelHeader->bonecontrollerindex ] );

  for ( int j = 0; j < studioAPI.modelHeader->numbonecontrollers; j++ ) {
    float value = 0.f;
    int i = bcontroller[ j ].index;

    if ( i == STUDIO_MOUTH ) { // mouth hardcoded at controller 4
      value = M_bound( 0.f, mouthOpenValue / 64.f, 1.f );

      // do lerp
      value = bcontroller[ j ].start * ( 1.f - value ) + bcontroller[ j ].end * value;
    }
    else if ( i < 4 ) { // not mouth controller
      // check for 360% wrapping
      if ( FBitSet( bcontroller[ j ].type, STUDIO_RLOOP ) )
        value = controller1[ i ] * ( 360.f / 256.f ) + bcontroller[ j ].start;
      else {
        value = bound( 0.f, controller1[ i ] / 255.f, 1.f );

        // do lerp
        value = bcontroller[ j ].start * ( 1.f - value ) + bcontroller[ j ].end * value;
      }
    }

    int bcontrollerType = bcontroller[ j ].type & STUDIO_TYPES;

    if ( bcontrollerType == STUDIO_XR || bcontrollerType == STUDIO_YR || bcontrollerType == STUDIO_ZR )
      out[ j ] = DEG2RAD( value );
    else if ( bcontrollerType == STUDIO_X || bcontrollerType == STUDIO_Y || bcontrollerType == STUDIO_Z )
      out[ j ] = value;
  }
}

void StudioAPI_FxTransform( cl_entity_t* clEntity, matrix3x4 transform ) {
  if ( clEntity->curstate.renderfx == kRenderFxDistort || clEntity->curstate.renderfx == kRenderFxHologram ) {
    int random = engineFuncs->COM_RandomFloat( 0, 49 );

    if ( !random ) {
      int axis = !!random;
      if ( axis == 1 ) axis = 2;

      VectorScale( transform[ axis ], engineFuncs->COM_RandomFloat( 1.f, 1.484f ), transform[ axis ] );
    }
    else if ( !( random = engineFuncs->COM_RandomFloat( 0, 49 ) ) ) {
      int axis = !!random;
      if ( axis == 1 ) axis = 2;

      transform[ engineFuncs->COM_RandomLong( 0, 2 ) ][ 3 ] += engineFuncs->COM_RandomFloat( -10.f, 10.f );
    }
  }
  else if ( clEntity->curstate.renderfx == kRenderFxExplode ) {
    float scale = 1.f + ( studioAPI.time - clEntity->curstate.animtime ) * 10.f;
    if ( scale > 2.f ) scale = 2.f;

    transform[ 0 ][ 1 ] *= scale;
    transform[ 1 ][ 1 ] *= scale;
    transform[ 2 ][ 1 ] *= scale;
  }
}

int StudioAPI_SetupMeshSkin( studiohdr_t* header, int textureIndex ) {
  if ( !header || studioAPI.forceFaceFlags & STUDIO_NF_CHROME ) return 0;

  mstudiotexture_t* texture = nullptr;

  if ( !texture )
    texture = reinterpret_cast<mstudiotexture_t*>( &reinterpret_cast<byte*>( header )[ header->textureindex ] );

  return texture[ textureIndex ].index;
}

void StudioAPI_RenderModel( ) {
  if ( !studioAPI.modelHeader ) return;

  float shellScale = 0.f;

  mstudiotexture_t* modelTexture = reinterpret_cast<mstudiotexture_t*>( &reinterpret_cast<byte*>( studioAPI.modelHeader )[ studioAPI.modelHeader->textureindex ] );
  mstudiomesh_t* modelMesh = reinterpret_cast<mstudiomesh_t*>( &reinterpret_cast<byte*>( studioAPI.modelHeader )[ studioAPI.subModel->meshindex ] );

  byte* boneVerticesInfo = &reinterpret_cast<byte*>( studioAPI.modelHeader )[ studioAPI.subModel->vertinfoindex ];
  byte* boneNormalsInfo = &reinterpret_cast<byte*>( studioAPI.modelHeader )[ studioAPI.subModel->norminfoindex ];

  // for transforming bones
  vec3_t* boneVertexTransfs = reinterpret_cast<vec3_t*>( &reinterpret_cast<byte*>( studioAPI.modelHeader )[ studioAPI.subModel->vertindex ] );
  vec3_t* boneNormals = reinterpret_cast<vec3_t*>( &reinterpret_cast<byte*>( studioAPI.modelHeader )[ studioAPI.subModel->normindex ] );

  int entitySkinCount = render3D.currEntity->curstate.skin;
  short* modelSkin = reinterpret_cast<short*>( &reinterpret_cast<byte*>( studioAPI.modelHeader )[ studioAPI.modelHeader->skinindex ] );
  if ( entitySkinCount > 0 && entitySkinCount < studioAPI.modelHeader->numskinfamilies )
    modelSkin = &modelSkin[ entitySkinCount * studioAPI.modelHeader->numskinref ];

  // transform bones
  if ( studioAPI.modelHeader->flags & STUDIO_HAS_BONEWEIGHTS ) {
  }
  else {
    for ( int i = 0; i < studioAPI.subModel->numverts; i++ ) {
      Matrix3x4_VectorTransform( studioAPI.boneTransf[ boneVerticesInfo[ i ] ].boneTransf, boneVertexTransfs[ i ], &studioAPI.modelVertices[ i ] );
    }
  }

  vec3_t lightColor;

  // compute light value, saved to studioAPI.lightValues
  for ( int i = 0; i < studioAPI.subModel->nummesh; i++ ) {
    int faceFlags = modelTexture[ modelSkin[ modelMesh->skinref ] ].flags | studioAPI.forceFaceFlags;

    rx_sortedmesh_t* currmesh = studioAPI.modelMeshes[ i ];

    currmesh->flags = faceFlags;
    currmesh->mesh = &modelMesh[ i ];

    // addition color blending
    if ( render3D.currEntity->curstate.rendermode == kRenderTransAdd ) {
      for ( int j = 0; j < currmesh[ i ].mesh->numnorms; j++ ) {
        VectorSet( &studioAPI.lightValues[ j ], triapi.blendValue, triapi.blendValue, triapi.blendValue );
      }
    }
    else { // normal color blending

      for ( int j = 0; j < currmesh[ i ].mesh->numnorms; j++ ) {
        // 0 is full dark
        // 1 is full bright
        // if fullbright mode is on, lightValue is always 1
        float lightValue = 1.f;

        VectorScale( studioAPI.light.color, lightValue, &studioAPI.lightValues[ j ] );
      }
    }
  }

  rx_studiotricmd_t* triCommands;

  for ( int i = 0; i < studioAPI.subModel->nummesh; i++ ) {
    float oldBlendMode = triapi.blendValue;

    rx_sortedmesh_t* currmesh = studioAPI.modelMeshes[ i ];

    int faceFlags = modelTexture[ modelSkin[ currmesh->mesh->skinref ] ].flags | studioAPI.forceFaceFlags;

    triCommands = reinterpret_cast<rx_studiotricmd_t*>( &reinterpret_cast<byte*>( studioAPI.modelHeader )[ currmesh->mesh->triindex ] );

    if ( faceFlags & STUDIO_NF_MASKED ) {
      // is opaque
      if ( render3D.currEntity->curstate.rendermode & kRenderNormal )
        triapi.blendValue = 1.f;
    }
    else if ( faceFlags & STUDIO_NF_ADDITIVE ) {

      // is opaque
      if ( render3D.currEntity->curstate.rendermode & kRenderNormal ) {
      }
      else
        TriAPI_SetRenderMode_( BLEND_ADD );
    }

    int imageIdx = StudioAPI_SetupMeshSkin( studioAPI.modelHeader, modelSkin[ currmesh->mesh->skinref ] );

    //    if ( faceFlags & STUDIO_NF_CHROME )
    //      else if ( faceFlags & STUDIO_NF_UV_COORDS )
  }
}
