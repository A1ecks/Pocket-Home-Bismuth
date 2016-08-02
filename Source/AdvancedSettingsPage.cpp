#include "AdvancedSettingsPage.h"

AdvancedSettingsPage::AdvancedSettingsPage(LauncherComponent* lc):
bg_color(0xffd23c6d),
title("settings", "Advanced Settings"),
addLogin("Change your password"), 
removeLogin("Remove your password"),
personalizeButton("Personalize your homepage"),
spl(new SettingsPageLogin),
ppc(new PersonalizePageComponent(lc))
{
  //Title font
  title.setFont(Font(27.f));
  //Back button
  backButton = createImageButton("Back", createImageFromFile(assetFile("backIcon.png")));
  backButton->addListener(this);
  backButton->setAlwaysOnTop(true);
  
  
  addLogin.addListener(this);
  removeLogin.addListener(this);
  personalizeButton.addListener(this);
  addAndMakeVisible(title);
  addAndMakeVisible(backButton);
  addAndMakeVisible(addLogin);
  addAndMakeVisible(removeLogin);
  addAndMakeVisible(personalizeButton);
}

AdvancedSettingsPage::~AdvancedSettingsPage(){ }

void AdvancedSettingsPage::resized(){
  auto bounds = getLocalBounds();
  int btn_height = 30;
  int btn_width = 370;
  
  backButton->setBounds(bounds.getX(), bounds.getY(), 60, bounds.getHeight());
  title.setBounds(bounds.getX()+160, bounds.getY()+10, btn_width, btn_height);
  
  TextButton* buttons[] = {&personalizeButton, &addLogin, &removeLogin};
  int y = 75;
  for(int i=0; i < 3; i++){
    buttons[i]->setBounds(bounds.getX()+70, bounds.getY()+y, btn_width, btn_height);
    y += 50;
  }
}

void AdvancedSettingsPage::deleteIcon(String name, String shell){
  ppc->deleteIcon(name, shell);
}

void AdvancedSettingsPage::displayNoPassword(){
AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon,
                                  "Error", 
                                  "No password is set, cannot remove",
                                  "Ok");
}

void AdvancedSettingsPage::buttonClicked(Button* button){
  if (button == backButton)
    getMainStack().popPage(PageStackComponent::kTransitionTranslateHorizontal);
  else if(button == &addLogin){
    spl->switchToModify();
    getMainStack().pushPage(spl, PageStackComponent::kTransitionTranslateHorizontal);
  }
  else if(button == &removeLogin){
    if(spl->hasPassword()){
      spl->switchToRemove();
      getMainStack().pushPage(spl, PageStackComponent::kTransitionTranslateHorizontal);
    }
    else displayNoPassword();
  } 
  else if (button == &personalizeButton)
    getMainStack().pushPage(ppc, PageStackComponent::kTransitionTranslateHorizontal);    
}

void AdvancedSettingsPage::paint(Graphics& g){
    auto bounds = getLocalBounds();
    g.fillAll(bg_color);
}