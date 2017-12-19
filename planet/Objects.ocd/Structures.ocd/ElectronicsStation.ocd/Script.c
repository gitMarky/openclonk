/**
	Electronics station
	
	Serves as an interface between switch targets and switches,
	if built by the player.
	
	@author Marky
*/


#include Library_Structure
#include Library_Ownable
#include Library_ElectronicsDevice

/*-- Properties --*/

private func Definition(def)
{
	def.PictureTransformation = Trans_Mul(Trans_Rotate(-30,1,0,0), Trans_Rotate(30,0,1,0));
	def.MeshTransformation = Trans_Mul(Trans_Rotate(20, 0, 1, 0), Trans_Scale(1500, 1500, 1500));
	return _inherited(def, ...);
}

local Name = "$Name$";
local Description = "$Description$";
local HitPoints = 20;
local Components = {Metal = 1};

private func IsHammerBuildable() { return true; }

/*-- Handle Connections --*/

public func GetWireOutputMaxAmount() { return 1; }
public func GetWireInputMaxAmount() { return 2; }

public func HasWireControlInputMenuEntriesReversed() { return true; } // Looks better than ordering the LEDs differently

private func UpdateWireStatus()
{
	_inherited(...);

	// Input or output is connected?
	var led2 = SetLEDactive(2, !!GetWireInput(1));
	var led1 = SetLEDactive(1, !!GetWireInput(0));
	var led0 = SetLEDactive(0, !!GetWireOutput(0));
	
	// Input or output state, only if active
	SetLEDstate(2, led2 && GetWireInputState(1));
	SetLEDstate(1, led1 && GetWireInputState(0));
	SetLEDstate(0, led0 && GetWireOutputState());
}

/*-- Displaying this object --*/

// Sets the ring around the LED on or off
private func SetLEDactive(int index, bool on)
{
	SetMeshMaterial(GetLEDmaterial(on), index + 1);
	return on;
}

// Sets the LED itself on or off
private func SetLEDstate(int index, bool on)
{
	SetMeshMaterial(GetLEDmaterial(on), index + 4);
}


private func GetLEDmaterial(bool on)
{
	if (on)
	{
		return "electronics_led_on";
	}
	else
	{
		return "electronics_led_off";
	}
}
