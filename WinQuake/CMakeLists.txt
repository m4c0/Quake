add_executable(quake
    cl_demo.c cl_input.c cl_main.c cl_parse.c cl_tent.c chase.c cmd.c common.c console.c crc.c cvar.c

    gl_draw.c gl_mesh.c gl_model.c gl_refrag.c gl_rlight.c gl_rmain.c gl_rmisc.c gl_rsurf.c gl_screen.c gl_test.c gl_warp.c

    host.c host_cmd.c keys.c menu.c mathlib.c net_dgrm.c net_loop.c net_main.c net_vcr.c net_udp.c net_bsd.c pr_cmds.c pr_edict.c pr_exec.c r_part.c sbar.c sv_main.c sv_phys.c sv_move.c sv_user.c zone.c view.c wad.c world.c cd_null.c snd_dma.c snd_mem.c snd_mix.c

    snd_osx.c sys_glut.c gl_vidglut.c
    )
target_compile_definitions(quake PUBLIC GLQUAKE)
target_link_libraries(quake PUBLIC "-framework AudioUnit")
target_link_libraries(quake PUBLIC "-framework GLUT")
target_link_libraries(quake PUBLIC "-framework OpenGL")

