#include "BatteryIcon.h"
#include "Layout_Component_JSONKeys.h"
#include "Theme_Image_JSONKeys.h"
#include "Utils.h"

/* Battery update frequency in milliseconds. */
static const constexpr int timerFrequency = 2000;

/* Number of battery percentages to average to get the reported percent. */
static const constexpr int percentageCount = 10;

BatteryIcon::BatteryIcon() : WindowFocusedTimer("BatteryIcon"),
batteryImage(Theme::Image::JSONKeys::batteryIcon),
batteryImageLayout(&batteryImage, Layout::Component::JSONKeys::batteryIcon),
batteryPercentLayout(&batteryPercent, 
        Layout::Component::JSONKeys::batteryPercent)
{
#    if JUCE_DEBUG
    setName("BatteryIcon");
#    endif
    setInterceptsMouseClicks(false, false);
    setWantsKeyboardFocus(false);
    batteryPercent.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(batteryPercent);
    addAndMakeVisible(batteryImage);
    startTimer(1);
}

/*
 * Runs applyConfigBounds on all child components, and updates bounds to fit the
 * battery text and icon.
 */
void BatteryIcon::applyConfigBounds()
{
    using namespace juce;
    batteryImageLayout.applyConfigBounds();
    batteryPercentLayout.applyConfigBounds();
    Rectangle<int> childBounds = batteryImage.getBounds()
            .getUnion(batteryPercent.getBounds());
    childBounds.setLeft(0);
    childBounds.setTop(0);
    if (childBounds != getBounds())
    {
        setBounds(childBounds);
    }
}

/*
 * Set the icon's new display status.
 */
void BatteryIcon::setStatus
(BatteryIconImage imageSelection, juce::String percent)
{
    using namespace juce;
    batteryImage.setImageAssetIndex((int) imageSelection);
    batteryPercent.setText(percent, dontSendNotification);
}

/*
 * Enables battery updates when this component becomes visible, and disables 
 * them when it's hidden.
 */
void BatteryIcon::visibilityChanged()
{
    if (isVisible())
    {
        if (!isTimerRunning())
        {
            startTimer(1);
        }
    }
    else
    {
        stopTimer();
    }
}

/**
 * Clears cached battery percentages when the timer is disabled, so that
 * the values will catch up more quickly on resume.
 */
void BatteryIcon::onSuspend()
{
    batteryPercents.clear();
}

/*
 * Updates the battery percentage.
 */
void BatteryIcon::timerCallback()
{
    BatteryMonitor::BatteryStatus batteryStatus =
            batteryMonitor.getBatteryStatus();
    int batteryPercent = batteryStatus.percent;
    if (batteryPercent < 0)
    {
        //no battery detected
        setStatus(noBattery, "");
    }
    else
    {
        batteryPercents.add(batteryPercent);
        batteryPercent = 0;
        for (const int& percent : batteryPercents)
        {
            batteryPercent += percent;
        }
        batteryPercent /= batteryPercents.size();
        if (batteryPercents.size() > percentageCount)
        {
            batteryPercents.remove(0);
        }
        int status = round(((float) batteryPercent) / 100.0f * 3.0f);
        jassert(status >= 0 && status < 4);
        if (batteryStatus.isCharging)
        {
            status += (int) charging0;
        }
        setStatus((BatteryIconImage) status,
                juce::String(batteryPercent) + juce::String("%"));
    }
    if (getTimerInterval() != timerFrequency)
    {
        startTimer(timerFrequency);
    }
}
