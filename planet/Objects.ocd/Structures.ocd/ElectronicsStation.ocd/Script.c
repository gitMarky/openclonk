/**
	Electronics station
	
	Serves as an interface between switch targets and switches,
	if built by the player.
	
	@author Marky
*/


#include Library_Structure
#include Library_Ownable

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

/*-- Pipe / wire control --*/

local wire_output;
local wire_inputA;
local wire_inputB;

public func CanConnectPipe(){ return true;}


public func QueryConnectPipe(object pipe)
{
	// Do not allow connections from this object to itself
	if (pipe->~IsConnectedTo(this))
	{
		return true;
	}
	// Allow neutral connection only if there is no output
	if (pipe->IsNeutralPipe() && GetWireOutput())
	{
		return true;
	}
	// Allow electronics connection only if there are free inputs
	// that is if not both are connected
	if (pipe->IsElectronicsPipe())
	{
		return GetWireInputA() && GetWireInputB();
	}
	// Allow connection of neutral or electronics pipes only
	return !(pipe->IsNeutralPipe() || pipe->IsElectronicsPipe());
}


public func OnPipeConnect(object pipe, string specific_pipe_state)
{
	if (pipe->IsNeutralPipe())
	{
		SetWireOutput(pipe);
		pipe->SetElectronicsPipe();
		pipe->Report("$MsgCreatedOutput$");
	}
	else if (pipe->~IsElectronicsPipe())
	{
		// Connect a second input?
		if (GetWireInputA())
		{
			if (!GetWireInputB())
			{
				SetWireInputB(pipe);
				pipe->Report("$MsgCreatedInputB$");
			}
		}
		// Create the first input
		else
		{
			SetWireInputA(pipe);
			pipe->Report("$MsgCreatedInputA$");
		}
	}
	UpdateStatus();
}


/*-- Handle Connections --*/

public func GetWireOutput() { return wire_output;}
public func GetWireInputA() { return wire_inputA;}
public func GetWireInputB() { return wire_inputB;}

public func SetWireOutput(object pipe)
{
	wire_output = pipe;
	return pipe;
}

public func SetWireInputA(object pipe)
{
	wire_inputA = pipe;
	return pipe;
}

public func SetWireInputB(object pipe)
{
	wire_inputB = pipe;
	return pipe;
}

private func UpdateStatus()
{
	SetLEDactive(2, !!GetWireInputB());
	SetLEDactive(1, !!GetWireInputA());
	SetLEDactive(0, !!GetWireOutput());
}

/*-- Displaying this object --*/

// Sets the ring around the LED on or off
private func SetLEDactive(int index, bool on)
{
	SetMeshMaterial(GetLEDmaterial(on), index + 1);
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
