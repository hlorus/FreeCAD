include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Boost_INCLUDE_DIRS}
    ${OCC_INCLUDE_DIR}
    ${COIN3D_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS}
    ${XercesC_INCLUDE_DIRS}
)

set(MeasureGui_LIBS
    Measure
    FreeCADGui
)


SET(MeasureGui_SRCS
    ${Measure_QRC_SRCS}
    # ${MeasureGui_UIC_SRCS}
    # DlgMeasurePreferences.ui
    # DlgMeasurePreferencesImp.cpp
    # DlgMeasurePreferencesImp.h
    AppMeasureGui.cpp
    Command.cpp
    PreCompiled.cpp
    PreCompiled.h
    ViewProviderMeasureDistancePoints.cpp
    ViewProviderMeasureDistancePoints.h
    ViewProviderMeasureAngle.cpp
    ViewProviderMeasureAngle.h
    # Workbench.cpp
    # Workbench.h
)

set(MeasureGui_Scripts
    ../InitGui.py
)

if(FREECAD_USE_PCH)
    add_definitions(-D_PreComp_)
    GET_MSVC_PRECOMPILED_SOURCE("PreCompiled.cpp" PCH_SRCS ${MeasureGui_SRCS})
    ADD_MSVC_PRECOMPILED_HEADER(MeasureGui PreCompiled.h PreCompiled.cpp PCH_SRCS)
endif(FREECAD_USE_PCH)

add_library(MeasureGui SHARED ${MeasureGui_SRCS} ${MeasureGui_Scripts}) #  ${MeasureGuiIcon_SVG}
target_link_libraries(MeasureGui ${MeasureGui_LIBS})

SET_BIN_DIR(MeasureGui MeasureGui /Mod/Measure)
SET_PYTHON_PREFIX_SUFFIX(MeasureGui)


INSTALL(TARGETS MeasureGui DESTINATION ${CMAKE_INSTALL_LIBDIR})
