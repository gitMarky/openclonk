/**
	Switch
	
	Library for switches. Contains basic functionality:
	- setting the object that gets operated
	- switching the object on/off as a basic term for describing the operation
	
	@author Marky
*/


local switch_target;

// Legacy function, so that possible errors are avoided for now
private func SetStoneDoor(object target)
{
	return SetSwitchTarget(target);
}

/*-- Public Interface --*/

// Sets the object that is operated by this switch
public func SetSwitchTarget(object target)
{
	switch_target = target;
	return true;
}


// Gets the object that is operated by this switch
public func GetSwitchTarget()
{
	return switch_target;
}


/*
  Switches the object on

  Forwards the user = the object that is controlling the switch
  and the switch to the switch target. 

  Calls OpenDoor(by_user, switch) in the object at the moment.
*/
public func DoSwitchOn(object by_user)
{
	if (GetSwitchTarget())
	{
		GetSwitchTarget()->OpenDoor(by_user, this);
	}
}


/*
  Switches the object off

  Forwards the user = the object that is controlling the switch
  and the switch to the switch target. 

  Calls CloseDoor(by_user, switch) in the object at the moment.
*/
public func DoSwitchOff(object by_user)
{
	if (GetSwitchTarget())
	{
		GetSwitchTarget()->CloseDoor(by_user, this);
	}
}

/*-- Saving --*/

public func SaveScenarioObject(proplist props)
{
	if (!inherited(props, ...)) return false;
	if (switch_target) props->AddCall("Target", this, "SetSwitchTarget", switch_target);
	return true;
}


/*-- Editor --*/

public func Definition(proplist def)
{
	if (!def.EditorProps) def.EditorProps = {};
	def.EditorProps.switch_target = { Name = "$SwitchTarget$", Type = "object", Filter = "IsSwitchTarget" };
	return _inherited(def, ...);
}
