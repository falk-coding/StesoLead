cmake_minimum_required(VERSION 3.22)

# Universal binary på macOS (Apple Silicon + Intel). Måste sättas före project().
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
endif()
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minsta macOS-version")

project(StesoLead VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows: statisk C++-runtime så att pluginet fungerar utan Visual C++ Redistributable.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# JUCE: använd lokal kopia om JUCE_DIR är satt, annars hämta från GitHub.
if(DEFINED JUCE_DIR)
    add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/juce)
else()
    include(FetchContent)
    FetchContent_Declare(juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.10
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(juce)
endif()

set(STESO_FORMATS VST3)
if(APPLE)
    list(APPEND STESO_FORMATS AU)
endif()

juce_add_plugin(StesoLead
    COMPANY_NAME "Falk"
    COMPANY_WEBSITE "https://github.com/falk-coding"
    BUNDLE_ID "se.falk.stesolead"
    PLUGIN_MANUFACTURER_CODE Falk
    PLUGIN_CODE Stsl
    FORMATS ${STESO_FORMATS}
    PRODUCT_NAME "StesoLead"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES Instrument Synth
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
    MICROPHONE_PERMISSION_ENABLED FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

set(STESO_SOURCES
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Parameters.cpp
    Source/Presets.cpp
    Source/dsp/SynthEngine.cpp
    Source/dsp/Effects.cpp)

target_sources(StesoLead PRIVATE ${STESO_SOURCES})

set(STESO_DEFINES
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    STESO_VERSION="${PROJECT_VERSION}")

target_compile_definitions(StesoLead PUBLIC ${STESO_DEFINES})

target_link_libraries(StesoLead
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Tester: renderar alla presets, kollar NaN/klipp/CPU/röster, sparar WAV + skärmdump.
option(STESO_BUILD_TESTS "Bygg testprogrammet" ON)
if(STESO_BUILD_TESTS)
    juce_add_console_app(StesoLeadTests PRODUCT_NAME "StesoLeadTests")
    target_sources(StesoLeadTests PRIVATE tests/RenderTest.cpp ${STESO_SOURCES})
    target_compile_definitions(StesoLeadTests PRIVATE ${STESO_DEFINES} STESO_TESTS=1)
    target_link_libraries(StesoLeadTests
        PRIVATE
            juce::juce_audio_utils
            juce::juce_dsp
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)
endif()
