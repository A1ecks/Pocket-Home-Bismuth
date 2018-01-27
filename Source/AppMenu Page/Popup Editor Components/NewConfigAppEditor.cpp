/*
  ==============================================================================

    NewConfigAppEditor.cpp
    Created: 23 Jan 2018 9:54:29pm
    Author:  Anthony Brown

  ==============================================================================
 */
#include "NewConfigAppEditor.h"

NewConfigAppEditor::NewConfigAppEditor(AppConfigFile& config,
        IconThread& iconThread, std::function<void() > onConfirm) :
AppMenuPopupEditor("New favorite application link", iconThread,
[this, &config, onConfirm](AppMenuPopupEditor* editor)
{
    AppConfigFile::AppItem newApp;
    newApp.name = editor->getNameField();
    newApp.icon = editor->getIconField();
    newApp.shell = editor->getCommandField();
    newApp.launchInTerminal = editor->launchInTerm();
    config.addFavoriteApp(newApp, config.getFavorites().size());
    onConfirm();
},
[]()
{
}
, false, true, false)
{
}