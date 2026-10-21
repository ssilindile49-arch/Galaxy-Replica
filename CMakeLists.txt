cmake_minimum_required(VERSION 3.22)
project(OrbitTexture VERSION 1.0.0)

# Set JUCE_DIR to your local JUCE checkout, or use -DJUCE_DIR=...
if(NOT DEFINED JUCE_DIR)
  message(FATAL_ERROR "Set JUCE_DIR to your JUCE folder, e.g. C:/JUCE")
endif()
add_subdirectory("${JUCE_DIR}" JUCE)

juce_add_plugin(OrbitTexture
    COMPANY_NAME "Independent Audio Tools"
    BUNDLE_ID "com.independentaudiotools.orbittexture"
    PLUGIN_MANUFACTURER_CODE InAu
    PLUGIN_CODE OrTx
    FORMATS VST3 Standalone
    PRODUCT_NAME "Orbit Texture"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(OrbitTexture PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)
target_compile_features(OrbitTexture PRIVATE cxx_std_17)
target_link_libraries(OrbitTexture PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
target_compile_definitions(OrbitTexture PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
