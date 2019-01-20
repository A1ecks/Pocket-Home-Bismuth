#pragma once
/**
 * @file  Theme_Image_JSONKeys.h
 *
 * @brief  Provides the keys for all configurable component image resources
 *         defined in the imageResource.json configuration file.
 */

#include "JuceHeader.h"

namespace Theme { namespace Image { namespace JSONKeys {

//######################### UI Component Data ##############################
//Defines all component types managed in the config file
static const juce::Identifier menuFrame
        ("menu frame");
static const juce::Identifier batteryIcon
        ("battery");
static const juce::Identifier wifiIcon
        ("wifi");
static const juce::Identifier powerButton
        ("power button");
static const juce::Identifier settingsButton
        ("settings button");
static const juce::Identifier popupMenu
        ("popup menu");
static const juce::Identifier pageLeft
        ("left arrow button");
static const juce::Identifier pageRight
        ("right arrow button");
static const juce::Identifier pageUp
        ("up arrow button");
static const juce::Identifier pageDown
        ("down arrow button");
static const juce::Identifier settingsListBtn
        ("settings list button");
static const juce::Identifier spinner
        ("loading spinner");

static const juce::Array<juce::Identifier> components =
{
    menuFrame,
    batteryIcon,
    wifiIcon,
    powerButton,
    settingsButton,
    popupMenu,
    pageLeft,
    pageRight,
    pageUp,
    pageDown,
    settingsListBtn,
    spinner
};
}}}