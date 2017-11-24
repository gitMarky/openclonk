/**
	Switch
	
	Library for switches. Contains basic functionality:
	- setting the object that gets operated
	- switching the object on/off as a basic term for describing the operation
	
	Needs to call _inherited in the following functions:
	* Construction()
	
	@author Marky
*/


local lib_switch;

// Legacy function, so that possible errors are avoided for now
private func SetStoneDoor(object target)
{
	return SetSwitchTarget(target);
}


/*-- Engine callbacks --*/

public func Construction(object by_object)
{
	_inherited(by_object, ...);
	lib_switch = {
		switch_target = nil,
	};
}

/*-- Public Interface --*/

// Sets the object that is operated by this switch
public func SetSwitchTarget(object target)
{
	lib_switch.switch_target = target;
	return true;
}


// Gets the object that is operated by this switch
public func GetSwitchTarget()
{
	return lib_switch.switch_target;
}


/*
  Switches the object on.

  Forwards the user = the object that is controlling the switch
  and the switch to the switch target. 

  Calls OpenDoor(by_user, switch) in the object at the moment.
*/
public func DoSwitchOn(object by_user)
{
	if (GetSwitchTarget())
	{
		GetSwitchTarget()->SetInputSignal(by_user, this, true);
	}
}


/*
  Switches the object off.

  Forwards the user = the object that is controlling the switch
  and the switch to the switch target. 

  Calls CloseDoor(by_user, switch) in the object at the moment.
*/
public func DoSwitchOff(object by_user)
{
	if (GetSwitchTarget())
	{
		GetSwitchTarget()->SetInputSignal(by_user, this, false);
	}
}

/*-- Saving --*/

public func SaveScenarioObject(proplist props)
{
	if (!inherited(props, ...)) return false;
	if (GetSwitchTarget()) props->AddCall("Target", this, "SetSwitchTarget", GetSwitchTarget());
	return true;
}


/*-- Editor --*/

public func Definition(proplist def)
{
	if (!def.EditorProps) def.EditorProps = {};
	def.EditorProps.switch_target = { Name = "$SwitchTarget$", Type = "object", Filter = "IsSwitchTarget" };
	return _inherited(def, ...);
}
