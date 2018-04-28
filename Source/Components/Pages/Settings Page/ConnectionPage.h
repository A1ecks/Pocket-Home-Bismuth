#pragma once
#include "ScalingLabel.h"
#include "SwitchComponent.h"
#include "ConfigurableImageButton.h"
#include "RelativeLayoutManager.h"
#include "PageComponent.h"
#include "PageStackComponent.h"

/**
 * @file ConnectionPage.h
 * 
 * Connection page is the abstract base for PageComponent classes that
 * show a list of connections and allow the user to connect or disconnect
 * from connections in the list.
 * 
 * ConnectionPtr must be a ReferenceCountedObjectPtr for a class inheriting from
 * juce::ReferenceCountedObject. In debug builds, the ReferenceCountedObject
 * inheriting class should also have a toString() method that returns some sort 
 * of identifying string for debug output.
 */

template<class ConnectionPtr>
class ConnectionPage : public PageComponent {
public:

    ConnectionPage();

    virtual ~ConnectionPage() { }

protected:
    /**
     * @return the selected connection pointer
     */
    ConnectionPtr getSelectedConnection();


    /**
     * Sets ConnectionPtr's connection as the selected connection.  When a
     * connection is selected, this page will show details and controls for that
     * connection, and the other connections on the list will be hidden.  When
     * set to nullptr, the full ConnectionPoint list will be shown again.
     * 
     * @param connection   This must be null, or equal to one of the 
     *                     connections in the connection list.
     */
    void setSelectedConnection(ConnectionPtr connection);


    /**
     * Remove all items from the connection list, and clear the page layout.
     */
    void clearConnectionList();

    /**
     * Reloads the list of connections and updates the page.
     */
    void updateConnectionList();

    /**
     * Reloads the page layout.  This method is responsible for updating the 
     * page when scrolling through the connection list or opening/closing 
     * connection details.
     */
    void layoutConnectionPage();
    
    /**
     * Replaces pageResized as the method inheriting classes should override
     * to execute code whenever the page bounds change.
     */
    virtual void connectionPageResized() {}

private:
    /**
     *Returns the list of all connection objects that should be listed.
     */
    virtual Array<ConnectionPtr> loadConnectionList() = 0;

    /**
     * @return true iff the system is connected to ConnectionPtr's connection.
     * 
     * @param connection
     */
    virtual bool isConnected(ConnectionPtr connection) = 0;

    /**
     * Construct a button component to represent a network connection point.
     * The generic ConnectionPage class will handle listening to button events,
     * and deallocating the button when it is no longer needed.
     * 
     * This button will be displayed in a list with other connection buttons, 
     * and when clicked, it will toggle the visibility of the connection 
     * controls.  
     * 
     * @param connection  One of the connection pointers listed on this page.
     * 
     * @return           a dynamically allocated Button component that displays
     *                    basic connection information.  
     */
    virtual Button* getConnectionButton(ConnectionPtr connection) = 0;

    /**
     * This is called whenever a button other than the navigation buttons
     * is clicked.
     * 
     * @param button
     */
    virtual void connectionButtonClicked(Button* button) = 0;

    /**
     * Get a set of layout parameters defining controls for a connection
     * list item.  Since connection controls are only shown for one connection
     * at a time, the ConnectionPage should reuse a a single set of control
     * components for every call to this method.
     * 
     * @param connection
     */
    virtual RelativeLayoutManager::Layout
    getConnectionControlsLayout(ConnectionPtr connection) = 0;

    /**
     * Update the connection control components for a given connection.
     * Call this whenever the selected connection changes state or
     * is replaced.
     *
     * @param connection 
     */
    virtual void updateConnectionControls(ConnectionPtr connection) = 0;
    
    /**
     * Update the connection list when the page is first added to the
     * page stack.
     */
    virtual void pageAddedToStack() override;

    /**
     * Update the connection list when the page is revealed on the page stack.
     */
    virtual void pageRevealedOnStack() override;

    /**
     * Handles connection list scrolling and connection selection.
     */
    void pageButtonClicked(Button* button) override;
    
        
    /**
     * When connection controls are open, override the back button to close
     * connection controls instead of closing the page.
     * 
     * @return true if connection controls were open when the back button was
     *          clicked.
     */
    bool overrideBackButton() override;

    /**
     * Close connection controls when esc is pressed.
     * 
     * @param key
     */
    virtual bool keyPressed(const KeyPress& key) override;
    
    /**
     * Update list navigation buttons to fit the page.
     */
    void pageResized() final override;


    /**
     * Holds the connection button and connection controls for a single
     * connection point.  Clicking the connection button toggles visibility
     * of connection controls.
     */
    class ConnectionListItem : public Component {
    public:

        /**
         * Create a new ConnectionListItem representing a ConnectionPtr.
         * 
         * @param connection        The represented connection.
         * 
         * @param connectionButton  This should be generated by the
         *                           ConnectionPage::getConnectionButton method 
         *                           using the connection.
         */
        ConnectionListItem
        (ConnectionPtr connection, Button* connectionButton);

        virtual ~ConnectionListItem();

        /**
         * @return the connection represented by this component.
         */
        ConnectionPtr getConnection();

        /**
         * Switch to connection point control mode, showing only the selected
         * connection.
         * 
         * @param detailLayout Contains the layout and component pointers to
         *                      add and show on this list item.
         */
        void setControlLayout(RelativeLayoutManager::Layout detailLayout);

        /**
         * Load the basic button layout used when showing the entire list.
         * This loads the initial layout, and restores it when closing
         * connection controls.
         */
        void setListItemLayout();

        /**
         * Checks if a button is the one attached to this list item.
         * 
         * @param button
         * 
         * @return true iff Button* button is this list item's 
         *          connectionButton.
         */
        bool ownsButton(Button* button);
        
    private:
        /**
         * @return the component's layout when not showing connection controls.
         */
        RelativeLayoutManager::Layout getListItemLayout();

        /**
         * Draws an outline around the entire component, with a width of 
         * ConnectionPage<ConnectionPoint>::ConnectionListItem::borderWidth
         * and color set by the ListBox backgroundColourId.
         * TODO: define a unique ColourId for the outline.
         * 
         * @param g
         */
        void paint(Graphics &g) override;

        /**
         * Scale the layout to fit the new bounds.
         */
        void resized() override;
        
        //Clicked to select this ConnectionPoint
        ScopedPointer<Button> connectionButton;
        RelativeLayoutManager listItemLayout;
        ConnectionPtr connection = nullptr;
        static const constexpr int borderWidth = 4;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConnectionListItem)
    };
    //scroll through the list of connections
    ConfigurableImageButton prevPageBtn;
    ConfigurableImageButton nextPageBtn;
    static const constexpr int connectionsPerPage = 4;
    int connectionIndex = 0;
    
    OwnedArray<ConnectionListItem> connectionItems;
    Array<ConnectionPtr> connections;
    ConnectionPtr selectedConnection;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConnectionPage)
};

#include "ConnectionPage.cpp"