#pragma once
#include <glib.h>
#include <map>
#include "JuceHeader.h"

/**
 * @file GPPObject.h
 * 
 * @brief A C++ interface and RAII container for GLib GObject data.
 * 
 * GPPObject holds a GObject*, providing methods for setting and getting object
 * properties, and adding and removing object signal handlers.  
 * 
 *   GPPObjects ensure that GObject pointers are referenced when acquired, and
 * unreferenced when removed.  If the GObject data is null or otherwise invalid,
 * isVoid() will return true, and all other methods will return default data or 
 * do nothing.
 * 
 *   GPPObject is intended only for use as a base class for specialized GObject
 * containers.
 */

class GPPObject
{
protected:
    /**
     * Create a void GPPObject, with no internal GObject.
     */
    GPPObject();
    
    /**
     * Create a new GPPObject as a reference to existing object data.
     * 
     *@param toCopy  This object's GObject* will be shared, and its reference
     *               count will be increased if the GObject* is GPPObject-owned.
     */
    GPPObject(const GPPObject& toCopy);
    
    /**
     * Create a GPPObject from GObject* data.
     * 
     * @param toAssign  GObject data to store in this GPPObject.  If this 
     *                  GObject* has a floating reference, GPPObject will sink
     *                  the reference.  Otherwise, a new reference will be
     *                  added.
     */
    GPPObject(GObject* toAssign);
    
public:
    /**
     * When this GPPObject is destroyed, unreference its GObject data, and 
     * remove all signal handlers added through this GPPObject.
     */
    virtual ~GPPObject();
    
    /**
     * Checks if this object holds valid GObject data.
     * 
     * @return true iff this object's GObject* is null or otherwise invalid
     */
    bool isVoid() const;
    
    
    /**
     * An abstract base for classes that handle GObject signals.
     */
    class SignalHandler
    {
    protected:
        friend class GPPObject;
        
        /**
         * Adds the signal handler's address to the list of valid signal
         * handlers.
         */
        SignalHandler();
        
    public:
        /**
         * Removes all of its signal handling callback functions from within the
         * signal context thread, and remove this signal handler's address from
         * the list of valid signal handlers.
         */
        virtual ~SignalHandler();
        
    private:
        /**
         * Callback function for handling property change notification signals.
         * Subclasses should override this to handle all specific property 
         * change notifications that they support.
         * 
         * @param source    Holds the GObject that emitted the signal.
         * 
         * @param property  The name of the object property that changed.
         */
        virtual void propertyChanged(GPPObject* source, String property);
        
        /**
         * Track all signal sources handled by this object
         */
        Array<GPPObject*, CriticalSection> sources;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SignalHandler);
    };
    
    /**
     * Un-subscribe a signal handler from all of this object's signals
     * 
     * @param signalHandler  Any signal handler object that should no longer
     *                       receive signals from this GPPObject.
     */
    void removeSignalHandler(SignalHandler* signalHandler);
           
    /**
     * Checks if this GPPObject and another share the same GObject data.
     * 
     * @param rhs  Another GPPObject instance.
     * 
     * @return true iff both objects hold pointers to the same GObject data.
     */
    bool operator==(const GPPObject& rhs) const;
    
    /**
     * Checks if this GPPObject holds a particular GObject pointer.
     * 
     * @rhs  A pointer to GObject data, or nullptr.
     * 
     * @return true iff rhs holds the same address as this GPPObject.
     */
    bool operator==(GObject* rhs) const;
                
    /**
     * Checks if this GPPObject and another don't share the same GObject data.
     * 
     * @param rhs  Another GPPObject instance.
     * 
     * @return true iff both objects don't hold pointers to the same GObject 
     *         data.
     */
    bool operator!=(const GPPObject& rhs) const;
    
    /**
     * Checks if this GPPObject does not hold a particular GObject pointer.
     * 
     * @rhs  A pointer to GObject data, or nullptr.
     * 
     * @return true iff rhs does not hold the same address as this GPPObject.
     */
    bool operator!=(GObject* rhs) const;
    
    /**
     * Sets this GPPObject's stored data to a new reference of another 
     * GPPObject's stored GObject.
     * 
     * @param rhs  Another GPPObject instance.  If rhs is void, this object's
     *             data will be removed.
     */
    void operator=(const GPPObject& rhs);
    
    /**
     * Sets this GPPObject's stored GObject data.
     * 
     * @param rhs  A valid GObject to store, or nullptr to convert this 
     *             GPPObject into a void object. If this value is floating, 
     *             the floating reference will be claimed by the GPPObject. 
     *             Otherwise, the reference count will be increased.
     */
    void operator=(GObject* rhs);
    
protected:
    /**
     * Gets a pointer to this object's data in a thread-safe manner.
     * 
     * @return  a pointer to the held GObject data.  If this data is not null,
     *          the GObject's reference count will increase by one, and the
     *          caller is responsible for un-referencing the object once it is
     *          no longer needed. 
     */
    GObject* getGObject() const;
    
    /**
     * Assigns new GObject data to this GPPObject.  Unless the new object
     * already holds the same data as this object, any references or signal
     * handlers associated with previous GObject data will be removed from the
     * old GObject.
     * 
     * @param toAssign                 This should point to either a new, valid 
     *                                 GObject, or be set to nullptr. 
     *                                 Otherwise, this assignment will do 
     *                                 nothing.
     * 
     * @param transferSignalHandlers   If toAssign is not null and this
     *                                 parameter is true, any signal handlers
     *                                 attached to the old GObject data will
     *                                 be re-assigned to the new GObject data.
     */
    void setGObject(GObject* toAssign, 
            bool transferSignalHandlers = true);
    
    /**
     * Assigns new GObject data to this GPPObject.  Unless the new object
     * already holds the same data as this object, any references or signal
     * handlers associated with previous GObject data will be removed from the
     * old GObject.
     * 
     * @param toCopy                   Any other GPPObject with a compatible
     *                                 GType.  If toCopy holds the same data as 
     *                                 this GPPObject, nothing will happen.
     * 
     * @param transferSignalHandlers   If toCopy is not void and this
     *                                 parameter is true, any signal handlers
     *                                 attached to the old GObject data will
     *                                 be re-assigned to the new GObject data.
     * 
     * @param stealSignalHandlers      If this value is true, all signal
     *                                 held by toCopy will be moved to this
     *                                 GPPObject.
     */
    void setGObject(const GPPObject& toCopy,
            bool transferSignalHandlers = true,
            bool stealSignalHandlers = false);
    
    /**
     * Allows GPPObject subclasses to access GObject data within other 
     * GPPObjects. Avoid using this for anything other than calling library 
     * functions that need GObject* parameter data.
     * 
     * @param source  Another GPPObject.
     * 
     * @return  the GObject* data held by source, or nullptr if source is void.
     *          Any valid GObject returned by this function will have its
     *          reference count incremented, and the caller is responsible for
     *          unreferencing it when it is no longer needed.
     */
    void getOtherGObject(const GPPObject& source);
    
    /**
     * Remove this object's GObject data, clearing all associated references 
     * and signal handlers.
     */
    void removeData();
    
    /**
     * Call an arbitrary function from within the context assigned to this
     * GObject.  This defaults to using the global default context.  Override
     * this if the object should operate on a different context and thread.
     * 
     * @param call  This function will synchronously run on the GMainLoop
     *              that owns this object's GMainContext.
     */
    virtual void callInMainContext(std::function<void()> call);
    
    /**
     * Call an arbitrary function from within the context assigned to this
     * GObject, passing in GObject data as a parameter.  This makes use of the
     * zero parameter callInMainContext function, so it does not need to be
     * overridden to operate in a different context.
     * 
     * @param call            This function will synchronously run on the 
     *                        GMainLoop that owns this object's GMainContext.  
     *                        This object's GObject* data will be passed in as a
     *                        parameter. Unlike getGObject, the data provided 
     *                        through this parameter does not need to be 
     *                        unreferenced by the caller.
     * 
     * @param skipCallIfVoid  If this value is true and this GPPObject is void,
     *                        the call function parameter will not run.
     */
    void callInMainContext(std::function<void(GObject*)> call,
            bool skipCallIfVoid = true);
    
    /**
     * Get the GType assigned to this object.  This must be implemented by 
     * GPPObject subclasses to return the type of the GObject contained by
     * the subclass.  
     */
    virtual GType getType() const = 0;
    
    /**
     * Check if a GObject's type allows it to be held by this object.  This
     * must be implemented by GPPObject subclasses to check the data type
     * with the relevant GObject type checking macro.
     * 
     * @param toCheck  Any valid GObject, or nullptr.
     * 
     * @return  true iff toCheck shares a type with this object, or is null. 
     */
    virtual bool isValidType(GObject* toCheck) const = 0;
    
    /**
     * Gets a pointer to one of the property values stored by this object.
     * 
     * @tparam T        The property type that's being requested.
     * 
     * @param property  A name identifying some property held by the GObject.
     * 
     * @return  a pointer to a copy of the parameter data, or nullptr if the
     *          parameter wasn't found.  If this value isn't null, it must be
     *          freed with g_free() or g_object_unref(), depending on its type.
     */
    template<typename T> T* getProperty(const char* property)
    {
        gpointer object = getGObject();
        if(object != nullptr)
        {
            T* pValue = nullptr;
            g_object_get(object, property, &pValue);
            g_object_unref(object);
            return pValue;
        }
        return nullptr;
    }
    
    /**
     * Sets one of the property values stored by this object.
     * 
     * @tparam T        The type of the property that's being set.
     * 
     * @param property  A name identifying some property held by the GObject.
     * 
     * @param value     A valid property value to copy into the GObject.
     */
    template<typename T> void setProperty(const char* property, T value)
    {
        gpointer object = getGObject();
        if(object != nullptr)
        {
            g_object_set(object, property, value, nullptr);
            g_object_unref(object);
        }
    }
       
    /**
     * Register a signal handler for any signal the GObject can send.
     * 
     * @param handler     A signal handler that can provide a valid GCallback 
     *                    for the chosen signal.
     * 
     * @param signalName  The signal that handler wishes to receive.
     * 
     * @param callback    A signal-specific callback function that checks if the
     *                    signal handler and GPPObject are valid before running
     *                    the callback provided by handler->getSignalCallback().
     *                    Because callback parameters vary between different
     *                    signal types, GPPObject subclasses have to provide
     *                    their own callback functions for any signals they
     *                    support.
     */
    void addSignalHandler(SignalHandler* handler,
            const char* signalName, GCallback callback);
    
    /**
     * Check if a specific SignalHandler still exists.  Signal callback
     * functions should never dereference SignalHandler data without first
     * making sure this function returns true.
     * 
     * @param handler  A pointer that may or may not point to a valid 
     *                 SignalHandler.
     * 
     * @return  true iff the SignalHandler is valid.
     */
    static bool isSignalHandlerValid(SignalHandler* handler);
    
    /**
     * Use signal callback data to find the address of the GPPObject that
     * contains the signal source.
     * 
     * @param objectData    The GObject that emitted the signal.
     * 
     * @param signalHandler The SignalHandler registered to handle the signal.
     * 
     * @return  the address of the first signal source tracked by signalHandler
     *          that is storing objectData, or nullptr if none is found.
     */
    static GPPObject* findObjectWrapper(GObject* objectData,
            SignalHandler* signalHandler);
    
    /**
     * Add a signal handler to receive notifications when a specific object 
     * property changes.  
     * 
     * Signal handler callback function template:
     * 
     * static void handleNotify(SignalHandler* self, GPPObject* signalSource,
     *         String changedProperty);
     * 
     * @param signalHandler  A signalHandler that can provide a callback
     *                       function for the specific notification signal.
     * 
     * @param propertyName  A valid property of this object.
     */
    void addNotifySignalHandler(SignalHandler* signalHandler,
            const char* propertyName);

    
    /**
     * Callback to handle property change signals.
     * 
     * @param objectData    The GObject sending the signal.
     * 
     * @param pSpec         Specifications for the updated object property.
     * 
     * @param signalHandler The SignalHandler object being notified of the
     *                      property change.
     */
    static void notifyCallback(GObject* objectData, GParamSpec* pSpec,
            SignalHandler* signalHandler);
    
private:    
    
    /**
     * Get all signal handlers registered with this object's GObject data.
     * 
     * @return  the list of all signal handlers, without duplicate entries.
     */
    Array<SignalHandler*> getSignalHandlers();
    
    /**
     * Set this object's GObject data to a new value.  Unless the object data 
     * given equals the object's old data, any signal handlers attached to the
     * old GObject data will be removed.
     * 
     * @param data                A GObject of a type that is compatible with 
     *                            this GPPObject. 
     *
     * @param refNeeded           Indicates if the GPPObject needs to increment 
     *                            the GObject data reference count.  This should
     *                            only be set to false if an extra reference was
     *                            added prior to calling setData.
     * 
     * @param moveSignalHandlers  Iff this parameter is true and data is not
     *                            null, all signal handlers removed from the
     *                            old GObject data will be attached to the new
     *                            GObject data.
     */
    void setData(GObject* data, bool refNeeded, bool moveSignalHandlers);
    
    /**
     * Used to re-add a list of signal handlers to new GObject data.
     * Subclasses are responsible for implementing this to attach the signal 
     * handlers to the correct object signal(s).
     * 
     * @param toTransfer  A list of signal handler objects to add to this
     *                    GPPObject.
     */
    virtual void transferSignalHandlers(Array<SignalHandler*>& toTransfer);
    
    /**
     * Holds pointers to all existing SignalHandler objects.  SignalHandler
     * pointers will only be dereferenced if they're found in this list. All
     * signal handlers will ensure that they're removed from this list when
     * they're destroyed.
     */
    static Array<SignalHandler*, CriticalSection> signalHandlers;
    
    std::map<gulong,SignalHandler*> registeredSignals;
    ScopedPointer<GWeakRef> objectRef;
    Atomic<GObject*> objectData;
};