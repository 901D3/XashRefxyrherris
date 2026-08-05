
@echo off

:: assume XashRefXyrherris is in 3rdparty/ of xash3d-fwgs/ folder
:: xash3d-fwgs
:: └──3rdparty
::    └──XashRefXyrherris
::       └──scripts
::           .bat

cd /d %~dp0

cd ..\..\..

xcopy /y .\common\backends.h       .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\beamdef.h        .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\bspfile.h        .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\cl_entity.h      .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\com_image.h      .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\com_model.h      .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\const.h          .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\con_nprint.h     .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\cvardef.h        .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\defaults.h       .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\demo_api.h       .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\dlight.h         .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\enginefeatures.h .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\entity_state.h   .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\entity_types.h   .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\event_api.h      .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\event_args.h     .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\event_flags.h    .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\gameinfo.h       .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\hltv.h           .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\ivoicetweak.h    .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\kbutton.h        .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\lightstyle.h     .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\mathlib.h        .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\netadr.h         .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\net_api.h        .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\particledef.h    .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\pmove.h          .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\pmtrace.h        .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\port.h           .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\qfont.h          .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\q_client.h       .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\ref_params.h     .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\render_api.h     .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\r_efx.h          .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\r_studioint.h    .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\screenfade.h     .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\studio_event.h   .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\synctype.h       .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\triangleapi.h    .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\usercmd.h        .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\wadfile.h        .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\weaponinfo.h     .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\wrect.h          .\3rdparty\XashRefXyrherris\engine_ref_includes\common\
xcopy /y .\common\xash3d_types.h   .\3rdparty\XashRefXyrherris\engine_ref_includes\common\

xcopy /y .\engine\alias.h        .\3rdparty\XashRefXyrherris\engine_ref_includes\engine\
xcopy /y .\engine\anorms.h       .\3rdparty\XashRefXyrherris\engine_ref_includes\engine\
xcopy /y .\engine\custom.h       .\3rdparty\XashRefXyrherris\engine_ref_includes\engine\
xcopy /y .\engine\customentity.h .\3rdparty\XashRefXyrherris\engine_ref_includes\engine\
xcopy /y .\engine\edict.h        .\3rdparty\XashRefXyrherris\engine_ref_includes\engine\
xcopy /y .\engine\eiface.h       .\3rdparty\XashRefXyrherris\engine_ref_includes\engine\
xcopy /y .\engine\mobility_int.h .\3rdparty\XashRefXyrherris\engine_ref_includes\engine\
xcopy /y .\engine\physint.h      .\3rdparty\XashRefXyrherris\engine_ref_includes\engine\
xcopy /y .\engine\progdefs.h     .\3rdparty\XashRefXyrherris\engine_ref_includes\engine\
xcopy /y .\engine\ref_api.h      .\3rdparty\XashRefXyrherris\engine_ref_includes\engine\
xcopy /y .\engine\shake.h        .\3rdparty\XashRefXyrherris\engine_ref_includes\engine\
xcopy /y .\engine\sprite.h       .\3rdparty\XashRefXyrherris\engine_ref_includes\engine\
xcopy /y .\engine\studio.h       .\3rdparty\XashRefXyrherris\engine_ref_includes\engine\
xcopy /y .\engine\warpsin.h      .\3rdparty\XashRefXyrherris\engine_ref_includes\engine\

xcopy /y .\public\crclib.c         .\3rdparty\XashRefXyrherris\engine_ref_includes\public\
xcopy /y .\public\crclib.h         .\3rdparty\XashRefXyrherris\engine_ref_includes\public\
xcopy /y .\public\crtlib.c         .\3rdparty\XashRefXyrherris\engine_ref_includes\public\
xcopy /y .\public\crtlib.h         .\3rdparty\XashRefXyrherris\engine_ref_includes\public\
xcopy /y .\public\matrixlib.c      .\3rdparty\XashRefXyrherris\engine_ref_includes\public\
xcopy /y .\public\miniz.c          .\3rdparty\XashRefXyrherris\engine_ref_includes\public\
xcopy /y .\public\miniz.h          .\3rdparty\XashRefXyrherris\engine_ref_includes\public\
xcopy /y .\public\xash3d_mathlib.c .\3rdparty\XashRefXyrherris\engine_ref_includes\public\
xcopy /y .\public\xash3d_mathlib.h .\3rdparty\XashRefXyrherris\engine_ref_includes\public\
