#ifndef DATETIME_H
#define DATETIME_H

#include "../../JuceLibraryCode/JuceHeader.h"

class DateTimePage : public Component, public Button::Listener, private ComboBox::Listener{
public:
  DateTimePage();
  ~DateTimePage();
  void buttonClicked(Button*) override;
  void paint(Graphics&) override;
  void resized() override;
  void comboBoxChanged(ComboBox*) override;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DateTimePage)
  //BackButton
  ScopedPointer<ImageButton> backButton;
  //Background color
  Colour bg_color;
  //Title of the pane
  Label title;
  //Combobox
  Label datemode;
  ComboBox choosemode;
  
  //Button for reconfiguring
  TextButton reconfigure;
};

#endif