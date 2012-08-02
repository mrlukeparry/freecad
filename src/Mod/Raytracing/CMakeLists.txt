
add_subdirectory(App)
if(FREECAD_BUILD_GUI)
    add_subdirectory(Gui)
endif(FREECAD_BUILD_GUI)

INSTALL(
    FILES
        Init.py
        InitGui.py
        RaytracingExample.py
    DESTINATION
        Mod/Raytracing
)

INSTALL(
    DIRECTORY
      Materials
    DESTINATION
        ${CMAKE_INSTALL_DATADIR}/Mod/Raytracing
    PATTERN "Makefile*" EXCLUDE
)


INSTALL(
    DIRECTORY
      Presets
    DESTINATION
        ${CMAKE_INSTALL_DATADIR}/Mod/Raytracing
    PATTERN "Makefile*" EXCLUDE
)

INSTALL(
    DIRECTORY
        Templates
    DESTINATION
        ${CMAKE_INSTALL_DATADIR}/Mod/Raytracing
    FILES_MATCHING PATTERN "*.pov*"
)

