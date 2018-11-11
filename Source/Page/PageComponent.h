#pragma once

#include "LayoutManager.h"
#include "NavButton.h"
#include "TransitionAnimator.h"
#include "JuceHeader.h"

/**
 * @file PageComponent.h
 * 
 * PageComponent is a container component that organizes and manages one screen
 * of the program UI.  PageComponents fill the entire program window, so that
 * only one can be visible at a time.
 *
 * Active pages are held in a stack, with the topmost page visible. The top page
 * on the stack may add another page to the stack above it. It may also remove
 * itself to reveal the page beneath it, as long as it's not the only page on
 * the stack.  The stack takes ownership of all pages it holds, deleting
 * them when they're removed from the stack.  The page stack is managed by
 * an object implementing PageComponent::PageStackInterface, that is linked to
 * the PageComponent after the page is added to the stack.
 * 
 * All pages are aware of all other page types that they can create through the 
 * PageComponent::PageType enum.  To be able to create pages, a page must have
 * a pointer to a PageFactoryInterface object assigned to it on construction.  
 * If it has a PageFactoryInterface, the page on the top of the page stack can
 * create any other page type to add to the stack, without needing to know
 * anything about the created page.
 */

class PageComponent : public juce::Component, public juce::Button::Listener
{
public:
    class PageFactoryInterface;

    enum ColourIds
    {
        backgroundColourId = 0x1900500
    };

    /**
     * @param name  The internal component name.
     */
    PageComponent(const juce::String& name = juce::String());

    virtual ~PageComponent() { }
    
    /**
     * Sets the layout of the page component.  If the page layout was set
     * previously, the old layout will be cleared, and its components will be
     * removed from the page.
     * 
     * @param layout      Defines the layout of all child components within the
     *                    page, not including the back button. These components 
     *                    will be added to the page as child components 
     *                    and made visible. If necessary, the layout margins 
     *                    will be resized to make room for the back button.
     */
    void setLayout(LayoutManager::Layout layout);
    
    //Options available for showing a back button on the page
    enum BackButtonType
    {
        leftBackButton,
        rightBackButton,
        noBackButton
    };
    
    /**
     * Sets the type of back button(if any) that should be shown on the page.
     * If necessary, page layout margins will be resized to make room for the
     * back button.
     * 
     * @param buttonType  The type of back button to show on the page.
     */
    void setBackButton(BackButtonType buttonType);

    /**
     * Sets a background image to draw behind all page components.
     * 
     * @param bgImage   The image to draw, or a null Image().  If a null
     *                   image is used, the background will be filled with the 
     *                   background color instead of an image.
     */
    void setBackgroundImage(juce::Image bgImage);

    /**
     * Adds all components in the layout to the page and makes them visible.
     */
    void addAndShowLayoutComponents();

    /**
     * Defines all pages that other pages can create to add to the page stack.
     */
    enum PageType
    {
        InputSettings,
        Keybinding,
        SetPassword,
        RemovePassword,
        Power,
        Fel,
        QuickSettings,
        BluetoothSettings,
        WifiSettings,
        UI,
        ColourSettings,
        SettingsList,
        DateTime,
        HomeSettings
    };

    /**
     * Defines the interface between individual PageComponents and the page
     * stack.  
     */
    class PageStackInterface
    {
        friend class PageComponent;
    public:

        virtual ~PageStackInterface() { }

        /**
         * Sets the initial page of the page stack, which will remain on the
         * stack until the stack is destroyed.
         * 
         * @param page  The initial page to add.
         */
        void setRootPage(PageComponent* page);

    protected:

        PageStackInterface() { }

        /**
         * The PageStack should call this to notify a PageComponent after 
         * pushing it on top of the page stack.
         *
         * @param page  The new top page on the stack.
         */
        void signalPageAdded(PageComponent* page);

        /**
         * When the top page is popped from the stack, the PageStack should
         * call this to notify the next page down that it's now the top page.
         *
         * @param page  The new top page on the stack.
         */
        void signalPageRevealed(PageComponent* page);

    private:
        /**
         * Pushes a new PageComponent on top of the stack, optionally animating
         * the transition. 
         * 
         * @param page        The new top page to add.
         * 
         * @param transition  The animation type to use for the page
         *                    transition.
         */
        virtual void pushPage
        (PageComponent* page,
                TransitionAnimator::Transition transition 
                = TransitionAnimator::moveLeft) = 0;

        /**
         * Removes the top page from the stack, optionally animating the 
         * transition.
         * 
         * @param transition  The animation type to use for the page
         *                    transition.
         */
        virtual void popPage(TransitionAnimator::Transition transition 
                = TransitionAnimator::moveRight) = 0;


        /**
         * Checks if a page is the top page on the stack.
         * 
         * @param page
         * 
         * @return true iff page is displayed on top of the page stack.
         */
        virtual bool isTopPage(PageComponent* page) = 0;
    };

    /**
     * Defines the interface between PageComponents and the PageFactory object
     * that creates new pages to add to the page stack.
     */
    class PageFactoryInterface
    {
    public:
        friend class PageComponent;

        virtual ~PageFactoryInterface() { }

    protected:

        PageFactoryInterface() { }
        
        /**
         * Assigns this PageFactory to a PageComponent.  The PageFactory should 
         * call this on every page it creates that should be allowed to open 
         * other pages.
         * 
         * @param page
         * 
         * @return page
         */
        PageComponent* setPageFactory(PageComponent* page);

    private:
        /**
         * Create a new page to push on top of the page stack.
         * 
         * @param PageType
         */
        virtual PageComponent* createPage(PageType type) = 0;
    };

protected:
    /**
     * Whenever this page is added to a page stack, the PageStack
     * will call this method. 
     */
    virtual void pageAddedToStack() { }

    /**
     * Whenever this page becomes the top page on the page stack, the 
     * PageStackComponent will call this method. 
     */
    virtual void pageRevealedOnStack() { }

    /**
     * Whenever this page is removed from the stack, the 
     * PageStackComponent will call this method. 
     */
    virtual void pageRemovedFromStack() { }

    /**
     * Handles any buttons besides the back button.  Inheriting classes
     * should override this instead of buttonClicked(Button*) to handle their 
     * own button actions.
     * 
     * @param button  Points to a button that was just clicked that added this
     *                 PageComponent as a listener.
     */
    virtual void pageButtonClicked(juce::Button* button) { }

    /**
     * Handles any actions necessary whenever the page is resized, besides
     * updating the layout manager and back button.  Inheriting classes should
     * override this instead of resized() to handle page resizing.
     */
    virtual void pageResized() { }

    /**
     * Checks if this page is on top of a page stack.
     * 
     * @return true iff the page is currently on the top of a page stack.
     */
    bool isStackTop();

    /**
     * If this page is currently on top of a page stack, this will remove it 
     * from the stack and destroy it.
     *
     * @param transition  This transition animation will run if the page that 
     *                    was on top of the stack and was removed successfully.
     */
    void removeFromStack(TransitionAnimator::Transition transition 
                = TransitionAnimator::moveLeft);

    /**
     * Creates and pushes a new page on top of the stack.
     *
     * @param pageType   The type of page to create and add.
    
     * @param transition This transition animation will run if the page is 
     *                   successfully added to the stack.
     */
    void pushPageToStack(PageType pageType,
            TransitionAnimator::Transition transition 
                = TransitionAnimator::moveLeft);

private:
    /**
     * Inheriting classes can override this method to change the behavior of the
     * back button. It will be called every time the back button is clicked, and
     * if it returns true, the back button will not remove the page.
     * 
     * @return true if the back button's action was replaced, false to allow
     *          the back button to remove the page as usual.
     */
    virtual bool overrideBackButton();


    /**
     * Recalculate component layout and back button bounds when the page is
     * resized.
     */
    void resized() final override;

    /**
     * Closes the page when the back button is clicked, and passes all other
     * button clicks to the pageButtonClicked method.
     *
     * @param button
     */
    void buttonClicked(juce::Button* button) final override;

    /**
     * Fills the page background with an image or color.
     *
     * @param g
     */
    virtual void paint(juce::Graphics& g) override;

    //A button that removes the page from the page stack may be held here.
    juce::ScopedPointer<NavButton> backButton = nullptr;

    //Layout manager and component margin/padding values.
    LayoutManager layoutManager;

    //Optional page background image.
    juce::Image backgroundImage;

    //Used by the page to create additional pages to display.
    PageFactoryInterface* pageFactory = nullptr;

    //Points to the page stack this page is on, if any.
    PageStackInterface* pageStack = nullptr;
};