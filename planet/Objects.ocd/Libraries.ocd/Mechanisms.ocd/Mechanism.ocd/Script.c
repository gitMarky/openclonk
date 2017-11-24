/**
	Mechanism
	
	A mechanism is something that can be controlled via input signals.
	* the input signal is sent by another object, which is usually a switch
	* for simplicity, we currently allow one input signal
	* for simplicity, we currently allow only boolean input signal
	
	Needs to call _inherited in the following functions:
	* Construction()
	
	@author Marky
*/


local lib_mechanism;

/*-- Engine callbacks --*/

public func Construction(object by_object)
{
	_inherited(by_object, ...);
	lib_mechanism = {
		old_signal = nil,		// the last received signal
		invert_signal = false,  // setting this to true inverts the signal
	};
}

/*-- Public Interface --*/

/*
 Sets the input signal of this mechanism.
 
 The function issues two callbacks to the object:
 a) OnInputSignalChanged(operator, sender, value) => gets called only if the value changes
 b) OnInputSignalSet(operator, sender, value) => gets called every time the signal is set
 
 Use these callbacks to activate or deactivate certain functions in the mechanism.
 For example, the stone door opens if you pass 'true', and closes if you pass 'false'

 Usually only the signal changed callback should be relevant, but who knows. Callback b)
 can be removed at any time if it turns out to be unnecessary.

 @par operator this object is operating the mechanism or the object that sent the signal-
 @par sender this object sent the signal - this is usually a switch.
 @par value this value is sent.
*/
public func SetInputSignal(object operator, object sender, bool value)
{
	var new_signal = value != lib_mechanism.invert_signal;

	// Callback type 1: rising/falling edge
	if (lib_mechanism.old_signal != new_signal)
	{
		this->~OnInputSignalChanged(operator, sender, new_signal);
		lib_mechanism.old_signal = new_signal;
	}

	// Callback type 2: current signal
	this->~OnInputSignalSet(operator, sender, new_signal);
}


/*
 Determines whether the signal should be inverted.
 */
public func SetInvertInputSignal(bool invert)
{
	lib_mechanism.invert_signal = invert;
}


/*-- Saving --*/

public func SaveScenarioObject(proplist props)
{
	if (!inherited(props, ...)) return false;
	if (lib_mechanism.invert_signal) props->AddCall("Invert", this, "SetInvertInputSignal", lib_mechanism.invert_signal);
	return true;
}

/*-- Editor --*/

public func Definition(proplist def)
{
	if (!def.EditorProps) def.EditorProps = {};
	def.EditorProps.invert_signal = { Name = "$InvertSignal$", EditorHelp="$InvertSignalDesc$", Type="bool", Set="SetInvertInputSignal" };
	return _inherited(def, ...);
}

