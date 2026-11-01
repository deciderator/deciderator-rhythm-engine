cmake_minimum_required(VERSION 3.22)
project(DECIDERATOR VERSION 0.2.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "DECIDERATOR requires Windows x64")
endif()
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 29396c22c93392d6738e021b83196283d6e4d850)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(Deciderator
    COMPANY_NAME "DECIDERATOR" BUNDLE_ID "audio.deciderator.rhythm"
    PLUGIN_MANUFACTURER_CODE Dcdr PLUGIN_CODE Dcrh
    FORMATS VST3 PRODUCT_NAME "DECIDERATOR Rhythm Engine"
    IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE VST3_CATEGORIES Fx Tools)
target_sources(Deciderator PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(Deciderator PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
juce_add_binary_data(DecideratorLegal SOURCES LICENSE THIRD_PARTY_NOTICES.md third_party/aquanode/NOTICE.md)
target_link_libraries(Deciderator PRIVATE juce::juce_audio_utils DecideratorLegal
    PRIVATE juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
include(CTest)
# Track our shared class layouts explicitly, including with localised MSVC/NMake
# versions whose /showIncludes output is not decoded by the dependency scanner.
set(deciderator_headers
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/PluginProcessor.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/PluginEditor.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/CreativeDSP.h")
set_property(SOURCE Source/PluginProcessor.cpp Source/PluginEditor.cpp
    APPEND PROPERTY OBJECT_DEPENDS "${deciderator_headers}")
if(BUILD_TESTING)
    juce_add_console_app(DecideratorTests PRODUCT_NAME "DECIDERATOR Tests")
    set_target_properties(DecideratorTests PROPERTIES OUTPUT_NAME "DecideratorValidation")
    target_sources(DecideratorTests PRIVATE Tests/ProcessorTests.cpp Tests/CreativeTests.cpp Tests/ExtensionTests.cpp)
    target_link_libraries(DecideratorTests PRIVATE Deciderator juce::juce_audio_utils)
    set_property(SOURCE Tests/ProcessorTests.cpp Tests/CreativeTests.cpp Tests/ExtensionTests.cpp
        APPEND PROPERTY OBJECT_DEPENDS "${deciderator_headers}")
    add_test(NAME processor COMMAND DecideratorTests)
endif()
