#include "../PocketHomeApplication.h"
#include "../AppLauncher.h"
#include "InputSettingsPage.h"

InputSettingsPage::InputSettingsPage() :
PageComponent("InputSettingsPage",{
    {3,
        {
            {&title, 1}
        }},
    {2,
        {
            {&cursorVisible, 5},
            {&choosemode, 2}
        }},
    {2,
        {
            {&calibrating, 1}
        }},
    {2,
        {
            {&fnmapping, 1}
        }}
}, true),
title("settings", "Input settings"),
choosemode("choosemode"),
calibrating("Calibrate the screen"),
fnmapping("Remap keyboard (FN key fix)"),
cursorVisible("cursorvisible", "Select the visibility of the cursor:")
{
    title.setJustificationType(Justification::centred);
    //ComboBox
    choosemode.addItem("Not visible", 1);
    choosemode.addItem("Visible", 2);
    choosemode.addListener(this);
    MainConfigFile& config = PocketHomeApplication::getInstance()->getConfig();
    if (config.getConfigBool(MainConfigFile::showCursorKey))
    {
        choosemode.setSelectedId(2);
    }
    else
    {
        choosemode.setSelectedId(1);
    }
    calibrating.addListener(this);
    fnmapping.addListener(this);
    addAndShowLayoutComponents();
}

InputSettingsPage::~InputSettingsPage() { }

void InputSettingsPage::pageButtonClicked(Button* button)
{
    if (button == &calibrating)
    {
        AppLauncher launcher;
        launcher.startOrFocusApp("XInput Calibrator", "xinput_calibrator");
    }
}

void InputSettingsPage::comboBoxChanged(ComboBox* c)
{
    if (c != &choosemode) return;
    MainConfigFile& config = PocketHomeApplication::getInstance()->getConfig();
    bool cursorVisible = (c->getSelectedId() == 2);
    config.setConfigBool(MainConfigFile::showCursorKey, cursorVisible);

    LookAndFeel& laf = getLookAndFeel();
    PokeLookAndFeel* mc = (PokeLookAndFeel*) & laf;
    mc->setCursorVisible(cursorVisible);
}

