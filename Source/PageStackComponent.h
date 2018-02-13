/**
 * @file PageStackComponent.h
 * 
 * TODO: documentation, organization
 */
#pragma once

class PageStackComponent : public Component {
public:
  int transitionDurationMillis = 200;

  PageStackComponent();
  ~PageStackComponent();

  void paint(Graphics &) override;
  void resized() override;

  enum Transition { kTransitionNone, kTransitionTranslateHorizontal, kTransitionTranslateHorizontalLeft };

  void pushPage(Component *page, Transition transition);
  void swapPage(Component *page, Transition transition);
  void popPage(Transition transition);
  void insertPage(Component *page, int idx);
  void removePage(int idx);
  void clear(Transition transition);

  int getDepth() const;

  Component *getCurrentPage();

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PageStackComponent)

  Array<Component *> stack;

  void transitionIn(Component *component, Transition transition, int durationMillis,
                    bool reverse = false);
  void transitionOut(Component *component, Transition transition, int durationMillis,
                     bool reverse = false);
};
