add_library(WinQuake STATIC
    cl_demo.cpp cl_input.cpp cl_main.cpp cl_parse.cpp cl_tent.cpp chase.cpp cmd.cpp common.cpp console.cpp crc.cpp

    gl_draw.cpp gl_mesh.cpp gl_model.cpp gl_refrag.cpp gl_rlight.cpp gl_rmain.cpp gl_rmisc.cpp gl_rsurf.cpp gl_screen.cpp gl_warp.cpp

    host.cpp host_cmd.cpp keys.cpp menu.cpp mathlib.cpp net_dgrm.cpp net_loop.cpp net_main.cpp net_vcr.cpp net_udp.cpp net_bsd.cpp pr_cmds.cpp pr_edict.cpp pr_exec.cpp r_part.cpp sbar.cpp sv_main.cpp sv_phys.cpp sv_move.cpp sv_user.cpp zone.cpp view.cpp world.cpp cd_null.cpp snd_dma.cpp snd_mem.cpp snd_mix.cpp

    snd_osx.cpp sys_glut.cpp gl_vidglut.cpp
    )
target_compile_definitions(WinQuake PUBLIC GLQUAKE)
target_compile_options(WinQuake PRIVATE "-Wunused-variable" "-Werror")
target_link_libraries(WinQuake PUBLIC "-framework AudioUnit")
target_link_libraries(WinQuake PUBLIC "-framework GLUT")
target_link_libraries(WinQuake PUBLIC "-framework OpenGL")

set_source_files_properties(gl_vidglut.cpp sys_glut.cpp PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations")
