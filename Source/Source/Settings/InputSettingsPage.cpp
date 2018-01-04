#include "../Configuration/ConfigFile.h"
#include "../PocketHomeApplication.h"
#include "InputSettingsPage.h"

InputSettingsPage::InputSettingsPage() :
bg_color(0xffd23c6d),
title("settings", "Input settings"),
choosemode("choosemode"),
calibrating("Calibrate the screen"),
fnmapping("Remap keyboard (FN key fix)"),
cursorvisible("cursorvisible", "Select the visibility of the cursor:")
{
    //Title font
    title.setFont(Font(27.f));
    cursorvisible.setFont(Font(20.f));
    //Back button
    backButton = createImageButton("Back", createImageFromFile(assetFile("backIcon.png")));
    backButton->addListener(this);
    backButton->setAlwaysOnTop(true);
    //ComboBox
    choosemode.addItem("Not visible", 1);
    choosemode.addItem("Visible", 2);
    choosemode.addListener(this);

    //Let's check whether there is an option for time format in the config
    ConfigFile * config = ConfigFile::getInstance();
    if (config->getConfigBool(SHOW_CURSOR))
    {
        choosemode.setSelectedId(2);
    } else
    {
        choosemode.setSelectedId(1);
    }
    calibrating.addListener(this);
    fnmapping.addListener(this);

    addAndMakeVisible(cursorvisible);
    addAndMakeVisible(calibrating);
    addAndMakeVisible(fnmapping);
    addAndMakeVisible(choosemode);
    addAndMakeVisible(title);
    addAndMakeVisible(backButton);
}

InputSettingsPage::~InputSettingsPage()
{
}

void InputSettingsPage::buttonClicked(Button* but)
{
    if (but == backButton)
        PocketHomeApplication::getInstance()
        ->getMainStack().popPage(PageStackComponent::kTransitionTranslateHorizontal);
    else if (but == &calibrating)
    {
        int ret = system("vala-terminal -fs 8 -g 20 20 -e 'xinput_calibrator ; exit'");
        if (ret == -1)
            AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Error", "Failed launching vala-terminal, is it installed ?");
    }
}

void InputSettingsPage::comboBoxChanged(ComboBox* c)
{
    if (c != &choosemode) return;
    ConfigFile * config = ConfigFile::getInstance();
    bool cursorVisible = (c->getSelectedId() == 2);
    config->setConfigBool(SHOW_CURSOR, cursorVisible);

    LookAndFeel& laf = getLookAndFeel();
    PokeLookAndFeel* mc = (PokeLookAndFeel*) & laf;
    mc->setCursorVisible(cursorVisible);
}

void InputSettingsPage::paint(Graphics& g)
{
    auto bounds = getLocalBounds();
    g.fillAll(bg_color);
}

void InputSettingsPage::resized()
{
    auto bounds = getLocalBounds();
    int btn_height = 30;
    int btn_width = 345;

    int titlewidth = title.getFont().getStringWidth(title.getText());
    titlewidth /= 2;
    title.setBounds(bounds.getX() + 240 - titlewidth, bounds.getY() + 10, btn_width, btn_height);

    backButton->setBounds(bounds.getX(), bounds.getY(), 60, bounds.getHeight());

    int datewidth = cursorvisible.getFont().getStringWidth(cursorvisible.getText());
    cursorvisible.setBounds(bounds.getX() + 60, bounds.getY() + 70, datewidth, btn_height);
    int combowidth = 360 - datewidth;
    choosemode.setBounds(bounds.getX() + 60 + datewidth, bounds.getY() + 70, combowidth, btn_height);

    int middle = 240 - btn_width / 2;
    calibrating.setBounds(bounds.getX() + middle, bounds.getY() + 150, btn_width, btn_height);

    fnmapping.setBounds(bounds.getX() + middle, bounds.getY() + 200, btn_width, btn_height);
}
