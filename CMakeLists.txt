cmake_minimum_required(VERSION 3.15)

project(GrooveSeq VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED JUCE_DIR)
  set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/JUCE" CACHE PATH "Path to JUCE")
endif()

if(NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
  message(FATAL_ERROR "JUCE not found. Set JUCE_DIR or add JUCE as a subfolder.")
endif()

add_subdirectory(${JUCE_DIR} JUCE)

juce_add_plugin(GrooveSeq
    COMPANY_NAME "Roddotcom"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Rodo
    PLUGIN_CODE Gsq1
    FORMATS VST3
    PRODUCT_NAME "GrooveSeq"
)

target_sources(GrooveSeq
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/PluginEntry.cpp
        Source/Sequencer.cpp
        Source/Sequencer.h
        Source/SequencerGrid.cpp
        Source/SequencerGrid.h
        Source/SamplePad.cpp
        Source/SamplePad.h
)

target_compile_definitions(GrooveSeq
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(GrooveSeq
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_formats
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
