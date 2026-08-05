#! /usr/bin/env python
# encoding: utf-8
# mittorn, 2018

from waflib import Logs
import os

top = "."

glewIncludes = "3rdparty/glew/include"

BACKEND_SOFT = 0
BACKEND_GL21 = 1
BACKEND_GLES2 = 2  # TODO: compile the same gl backend for android

compileGL21 = True

rSpeedsEnabled = True
msvcDebug = False


def options(opt):
    return


def configure(conf):
    # feel free to change those paths if your workspace layouts is different
    # you should reconfigure after change paths
    glew_libdir = conf.path.find_dir("3rdparty/glew/lib").abspath()

    conf.env.LIBPATH += [glew_libdir]
    conf.env.INCLUDES += [conf.path.find_dir(glewIncludes).abspath()]

    conf.check(lib="glew32", uselib_store="GLEW")

    # for Windows only
    # append OpenGL32, OpenGL32.Lib comes from Windows 10 SDK (maybe Windows 11 SDK too?)
    conf.env.append_value("LIB", ["OpenGL32"])


def makeBase():
    macros = "REF_DLL=1, HAVE_STRNICMP=1, HAVE_STRICMP=1, XASH_VIDEO=2, "

    libs = ["engine_includes", "werror", "public", "M"]

    includes = [
        ".",
        "dll_init/",
        "engine_ref_includes/",
        "engine_ref_includes/common/",
        "engine_ref_includes/engine/",
        "engine_ref_includes/engine/common/",
        "engine_ref_includes/filesystem/",
        "engine_ref_includes/public/",
        "engine_ref_includes/pm_shared/",
        # frontend
        "src/frontend/",
        "src/frontend/3d",
        "src/frontend/api",
        "src/frontend/image",
        "src/frontend/video",
        # GL
        "src/backend/gl/",
        "src/backend/gl/3d/",
        "src/backend/gl/api/",
        "src/backend/gl/image/",
        # GL shaders
        "src/backend/gl/shader/out/",
        "src/backend/gl/shader/out/3d/",
        "src/backend/gl/shader/out/api/",
        "src/backend/gl/shader/out/draw2d/",
        # soft
        "src/backend/soft/",
        "src/backend/soft/3d/",
        "src/backend/soft/api/",
        "src/backend/soft/image/",
        "src/backend/soft/video/",
        # 3rdparty
        "3rdparty/glew/include/",
    ]

    sourceFiles = [
        "*.cpp",
        "dll_init/**/*.cpp",
        "src/frontend/**/*.cpp",
        "engine_ref_includes/**/*.c",
    ]

    return macros, libs, includes, sourceFiles


def build_variant(bld, env, name, libs, includes, sourceFiles, macros, backend):
    if rSpeedsEnabled:
        macros += "R_SPEEDS=1, "

    if backend == BACKEND_SOFT:
        sourceFiles += ["src/backend/soft/**/*.cpp"]

    elif backend == BACKEND_GL21:
        macros += "GL=1, "
        libs += ["GLEW"]
        sourceFiles += ["src/backend/gl/**/*.cpp"]

    if msvcDebug:
        macros += "_DEBUG=1, "

    bld.shlib(
        source=bld.path.ant_glob(sourceFiles),
        target=name,
        includes=includes,
        defines=macros,
        use=libs,
        install_path=bld.env.LIBDIR,
        env=env,
    )


def build(bld):
    soft_env = bld.env.derive()
    gl_env = bld.env.derive()

    if msvcDebug:
        for env in (soft_env, gl_env):
            env.append_value("CXXFLAGS", ["/MDd", "/fsanitize=address"])
            env.append_value("CFLAGS", ["/MDd", "/fsanitize=address"])
            env.append_value("LINKFLAGS", ["/fsanitize=address"])

    macros, libs, includes, sourceFiles = makeBase()

    build_variant(
        bld=bld,
        env=soft_env,
        name="ref_xashrefxyr_soft",
        libs=libs,
        includes=includes,
        sourceFiles=sourceFiles,
        macros=macros,
        backend=BACKEND_SOFT,
    )

    if compileGL21:
        macros, libs, includes, sourceFiles = makeBase()

        build_variant(
            bld=bld,
            env=gl_env,
            name="ref_xashrefxyr_gl21",
            libs=libs,
            includes=includes,
            sourceFiles=sourceFiles,
            macros=macros,
            backend=BACKEND_GL21,
        )
