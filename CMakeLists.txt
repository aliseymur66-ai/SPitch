cmake_minimum_required(VERSION 3.22)
project(SPitch VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(JUCE)

juce_add_plugin(SPitch
    COMPANY_NAME "SeymurAudio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Seym
    PLUGIN_CODE Spch
    FORMATS VST3 Standalone
    PRODUCT_NAME "SPitch")

target_sources(SPitch
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

target_compile_definitions(SPitch
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_WIN_WEBVIEW2=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(SPitch
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
