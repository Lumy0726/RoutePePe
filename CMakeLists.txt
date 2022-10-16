cmake_minimum_required(VERSION 3.14.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12 CACHE STRING "")

set(vst3sdk_SOURCE_DIR vst3sdk)
if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(RoutePePe
    # This is your plug-in version number. Change it here only.
    # Version number symbols usable in C++ can be found in
    # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
    VERSION 1.1.0.0
    DESCRIPTION "RoutePePe VST 3 Plug-in"
)

set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(RoutePePe
    source/version.h
    source/myplugincids.h
    source/mypluginprocessor.h
    source/mypluginprocessor.cpp
    source/myplugincontroller.h
    source/myplugincontroller.cpp
    source/mypluginentry.cpp
    source/AudioBuffer.h
    source/AudioBuffer.cpp
    source/SharedM.h
    source/SharedM.cpp
)

#- VSTGUI Wanted ----
if(SMTG_ADD_VSTGUI)
    target_sources(RoutePePe
        PRIVATE
            resource/myplugineditor.uidesc
    )
    target_link_libraries(RoutePePe
        PRIVATE
            vstgui_support
    )
    smtg_target_add_plugin_resources(RoutePePe
        RESOURCES
            "resource/myplugineditor.uidesc"
    )
endif(SMTG_ADD_VSTGUI)
# -------------------

smtg_target_add_plugin_snapshots (RoutePePe
    RESOURCES
        resource/7ED7EBC62F205D52A7ACA1782952D7B2_snapshot.png
        resource/7ED7EBC62F205D52A7ACA1782952D7B2_snapshot_2.0x.png
)

target_link_libraries(RoutePePe
    PRIVATE
        sdk
)

smtg_target_configure_version_file(RoutePePe)

if(SMTG_MAC)
    smtg_target_set_bundle(RoutePePe
        BUNDLE_IDENTIFIER com.lumylumystudio.routepepe
        COMPANY_NAME "LumyLumyStudio"
    )
    smtg_target_set_debug_executable(RoutePePe
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
elseif(SMTG_WIN)
    target_sources(RoutePePe PRIVATE 
        resource/win32resource.rc
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT RoutePePe)

        smtg_target_set_debug_executable(RoutePePe
            "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()
endif(SMTG_MAC)
