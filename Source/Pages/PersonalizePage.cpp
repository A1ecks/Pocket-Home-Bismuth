#include "../PocketHomeApplication.h"
#include "PersonalizePage.h"

PersonalizePage::PersonalizePage() :
PageComponent("PersonalizePage",{
    {
        { 3,
            {
                {&title, 1}
            }},
        { 2,
            {
                {&bgTitle, 1},
                {&bgTypePicker, 1}
            }},
        { 2,
            {
                {&bgLabel, 1},
                {&bgEditor, 1}
            }},
        { 2,
            {
                {&menuPickerLabel, 1},
                {&menuTypePicker, 1}
            }},
        { 5,
            {
                {nullptr, 1}
            }}
    }
}, true),
title("personalizeTitle", "Homepage Settings"),
bgTitle("bgTitle", "Background:"),
bgTypePicker("bgTypePicker"),
bgLabel("bgLabel", ""),
bgEditor("Choose the new background",
        "Please choose your new background image"),
menuPickerLabel("menuPickerLabel", "Application menu:"),
menuTypePicker("menuTypePicker")
{
    title.setJustificationType(Justification::centred);
    bgTypePicker.addItem("Default", 1);
    bgTypePicker.addItem("Color", 2);
    bgTypePicker.addItem("Image", 3);
    bgTypePicker.setSelectedId(1);
    bgTypePicker.addListener(this);

    bgEditor.setColour(TextEditor::ColourIds::textColourId,
            Colour::greyLevel(0.f));
    bgEditor.addFileSelectListener(this);

    menuTypePicker.addItem("Scrolling menu", 1);
    menuTypePicker.setSelectedId(1);
    menuTypePicker.addListener(this);
    updateComboBox();
    addAndShowLayoutComponents();
}

PersonalizePage::~PersonalizePage() { }

void PersonalizePage::updateComboBox()
{
    MainConfigFile& config = PocketHomeApplication::getInstance()->getConfig();
    /* Checking the current configuration */
    String background = config.getConfigString(MainConfigFile::backgroundKey);
    bool display = false;
    if (background.length() == 0);
    else if (background.length() == 6 &&
             background.containsOnly("0123456789ABCDEF"))
    {
        bgTypePicker.setSelectedItemIndex(1, sendNotificationSync);
        display = true;
        bgEditor.setText(background, false);
    }
    else
    {
        bgTypePicker.setSelectedItemIndex(2, sendNotificationSync);
        display = true;
        bgEditor.setText(background, false);
    }

    bgEditor.setVisible(display);
    bgLabel.setVisible(display);
}

void PersonalizePage::comboBoxChanged(ComboBox * box)
{
    MainConfigFile& config = PocketHomeApplication::getInstance()->getConfig();
    if (box == &bgTypePicker)
    {
        bgEditor.setText("",NotificationType::dontSendNotification);
        switch (box->getSelectedId())
        {
            case 1:
                config.setConfigString(MainConfigFile::backgroundKey, "4D4D4D");
                bgEditor.setVisible(false);
                bgLabel.setVisible(false);
                return;
            case 2:
                bgLabel.setText("Hex value:", dontSendNotification);
                bgEditor.showFileSelectButton(false);
                break;
            case 3:
                bgLabel.setText("Image path:", dontSendNotification);
                bgEditor.showFileSelectButton(true);
        }
        bgEditor.setVisible(true);
        bgLabel.setVisible(true);
    }
    else if (box == &menuTypePicker)
    {
        config.setConfigString(MainConfigFile::menuTypeKey, box->getText());
    }
}

void PersonalizePage::fileSelected(FileSelectTextEditor * edited)
{
    MainConfigFile& config = PocketHomeApplication::getInstance()->getConfig();
    String value = edited->getText();
    //color value
    if (bgTypePicker.getSelectedId() == 2)
    {
        value = value.toUpperCase();
        if (value.length() != 6 || !value.containsOnly("0123456789ABCDEF"))
            bgEditor.setText("Invalid color", false);
        else
        {
            config.setConfigString(MainConfigFile::backgroundKey, value);
        }
    }
    else if (bgTypePicker.getSelectedId() == 3)
    {
        config.setConfigString(MainConfigFile::backgroundKey, value);
    }
}