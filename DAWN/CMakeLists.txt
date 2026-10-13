cmake_minimum_required(VERSION 3.22)
project(DAWN VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(dawn_core
    src/model/Project.cpp
    src/model/MidiClip.cpp
    src/engine/GraphSnapshot.cpp
    src/engine/AudioProcessingBlock.cpp
    src/engine/AudioEngine.cpp
    src/engine/PlaybackSnapshot.cpp
    src/engine/MidiScheduler.cpp
    src/engine/AnalogSynth.cpp
    src/engine/LiveMidiQueue.cpp
    src/engine/GamepadMap.cpp
    src/model/Paths.cpp
    src/model/DataLayout.cpp
    src/engine/OfflineRender.cpp
    src/catalog/PluginCatalog.cpp
)
target_include_directories(dawn_core PUBLIC src)

add_executable(dawn_tests tests/model_test.cpp tests/midi_test.cpp tests/input_test.cpp tests/final_test.cpp tests/workflow_test.cpp tests/portable_test.cpp)
target_link_libraries(dawn_tests PRIVATE dawn_core)
target_compile_options(dawn_tests PRIVATE -Wall -Wextra -Werror)

# DAWN is GPLv3. These flags turn off the JUCE splash and any usage reporting.
# juce::URL is compiled in, but the only callers are the user-started plugin hub.
#   cmake -S DAWN -B DAWN/build -DDAWN_WITH_JUCE=ON
option(DAWN_WITH_JUCE "Build the desktop and Android application shell" OFF)

if(DAWN_WITH_JUCE)
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.8
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)

    juce_add_gui_app(DAWN
        PRODUCT_NAME "DAWN"
        COMPANY_NAME "DAWN"
        BUNDLE_ID app.dawn.audio
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
    )
    target_sources(DAWN PRIVATE
        src/app/Main.cpp
        src/app/DawnApplication.cpp
        src/juce/InternalAnalogSynth.cpp
        src/juce/Vst3Wrapper.cpp
        src/juce/ProjectState.cpp
        src/juce/LiveMidiInjector.cpp
        src/juce/InputProfile.cpp
        src/juce/InputMapper.cpp
        src/juce/InputMappingComponent.cpp
        src/juce/CatalogManager.cpp
        src/juce/PluginInstaller.cpp
        src/juce/PluginScanner.cpp
        src/juce/GamepadPoller.cpp
        src/juce/PluginRack.cpp
        src/juce/ProjectSerializer.cpp
        src/juce/OfflineRenderEngine.cpp
        src/juce/DawnMainComponent.cpp
        src/juce/VisualizerDataQueue.cpp
        src/juce/MasterVisualizers.cpp
        src/juce/CyberdeckLookAndFeel.cpp
        src/juce/DawnAboutPanel.cpp
        src/juce/InspectorPanel.cpp
        src/juce/AppPaths.cpp
        src/juce/CompingComponent.cpp
    )
    target_include_directories(DAWN PRIVATE src)
    target_link_libraries(DAWN PRIVATE
        dawn_core
        juce::juce_audio_utils
        juce::juce_audio_formats
        juce::juce_audio_devices
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_basics
    )
    target_compile_definitions(DAWN PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_DISABLE_AUDIOPROCESSOR_BEGIN_AND_END_GESTURE_CHECKING=1
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_PLUGINHOST_VST3=1
        JUCE_PLUGINHOST_AU=$<BOOL:${APPLE}>
        $<$<BOOL:${DAWN_ASIO_SDK}>:JUCE_ASIO=1>
    )

    install(TARGETS DAWN
        BUNDLE DESTINATION .
        RUNTIME DESTINATION .
    )
    install(FILES packaging/DAWN_PORTABLE.txt DESTINATION .)
    install(FILES packaging/DAWN_DATA/demo.dawn DESTINATION DAWN_DATA)

    set(CPACK_PACKAGE_NAME "DAWN")
    set(CPACK_PACKAGE_VENDOR "DAWN")
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "DRM-free DAW. No portals. GPLv3.")
    set(CPACK_PACKAGE_FILE_NAME "DAWN-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/LICENSE.txt")
    set(CPACK_PACKAGE_EXECUTABLES "DAWN;DAWN")
    set(CPACK_WIX_UPGRADE_GUID "7F3C1A90-6E2B-4D58-9C11-0A77B6E4D201")
    set(CPACK_NSIS_DISPLAY_NAME "DAWN")
    set(CPACK_NSIS_PACKAGE_NAME "DAWN ${PROJECT_VERSION}")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_DMG_VOLUME_NAME "DAWN")
    set(CPACK_DMG_SLA_USE_RESOURCE_FILE_LICENSE OFF)
    if(WIN32)
        set(CPACK_GENERATOR "NSIS;WIX")
    elseif(APPLE)
        set(CPACK_GENERATOR "DragNDrop")
    else()
        set(CPACK_GENERATOR "TGZ")
    endif()
    include(CPack)

    add_custom_target(dawn_appimage
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/packaging/make-appimage.sh "${CMAKE_BINARY_DIR}"
        DEPENDS DAWN
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Build a Linux AppImage"
        VERBATIM
    )
endif()
