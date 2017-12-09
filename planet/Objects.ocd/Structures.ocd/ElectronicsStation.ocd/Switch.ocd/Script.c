/*-- Switch --*/

#include Library_Switch

local last_controlling_clonk;
local handle_position;

public func Initialize()
{
	SetAction("Idle");
}

public func SaveScenarioObject(props)
{
	if (!inherited(props, ...)) return false;
	var pos = GetHandlePosition();
	if (pos) props->AddCall("Handle", this, "SetSwitchDir", (pos>0)-(pos<0));
	if (left_action || right_action) props->AddCall("Action", this, "SetActions", left_action, right_action);
	return true;
}

public func SetActions(new_left_action, new_right_action)
{
	left_action = new_left_action;
	right_action = new_right_action;
	return true;
}

public func ControlUp(object clonk)
{
	var dir = Rot2Dir(0,-1);
	if (!dir) return false;
	return ControlSwitchDir(clonk, dir);
}

public func ControlDown(object clonk)
{
	var dir = Rot2Dir(0,+1);
	if (!dir) return false;
	return ControlSwitchDir(clonk, dir);
}

public func ControlLeft(object clonk)
{
	var dir = Rot2Dir(-1,0);
	if (!dir) return false;
	return ControlSwitchDir(clonk, dir);
}

public func ControlRight(object clonk)
{
	var dir = Rot2Dir(+1,0);
	if (!dir) return false;
	return ControlSwitchDir(clonk, dir);
}

private func ControlSwitchDir(object clonk, int dir)
{
	if (!GetSwitchTarget() && !right_action && !left_action)
	{
		Sound("Structures::SwitchStuck");
		Message("$MsgStuck$");
		return false;
	}
	// remember clonk for clalbacks
	last_controlling_clonk = clonk;
	// move handle
	var handle_pos = GetHandlePosition();
	if (dir < 0)
	{
		if (GetAction() == "SwitchLeft") return false;
		if (handle_pos == -MaxHandleAngle) { Sound("Structures::SwitchStuck"); return false; }
		SetAction("SwitchLeft");
	}
	else
	{
		if (GetAction() == "SwitchRight") return false;
		if (handle_pos == MaxHandleAngle) { Sound("Structures::SwitchStuck"); return false; }
		SetAction("SwitchRight");
	}
	Sound("Structures::SwitchMove");
	return true;
}

private func Rot2Dir(int dx, int dy)
{
	// Convert direction dx, dy in world coordinates to horizontal direction in local coordinates
	var r = GetR();
	return BoundBy(dx*Cos(r, 3) + dy*Sin(r, 3), -1,+1);
}

private func GetHandlePosition()
{
	// returns position of handle relative to switch in degrees [-180 to +180]
	return (handle_position + 540) % 360 - 180;
}

public func SetSwitchDir(int to_dir)
{
	// Set angle of switch without doing any effects or callbacks
	handle_position = MaxHandleAngle * to_dir;
	return handle_position;
}

private func SwitchingTimer(int dir)
{
	if (!GetSwitchTarget() && !right_action && !left_action)
	{
		Sound("Structures::SwitchStuck");
		return SetAction("Idle");
	}
	var handle_pos = GetHandlePosition();
	var handle_pos_new = BoundBy(handle_pos + HandleSpeed * dir, -MaxHandleAngle, +MaxHandleAngle);
	if (!handle_pos_new) handle_pos_new = dir; // avoid direct central position, so player cannot force the same direction twice
	handle_position = handle_pos_new;
	// Reached end?
	if (handle_pos_new == MaxHandleAngle * dir) SetAction("Idle");
	// Passed by middle? (last call because callback might delete switch)
	if (handle_pos * handle_pos_new <= 0)
	{
		Sound("Structures::SwitchFlip*");
		DoSwitchFlip(last_controlling_clonk, dir);
	}
	var time = 2;
	TransformBone("lever", Trans_Rotate(handle_position, 0, 0, 1), 1, Anim_Linear(0, 0, 1000, time ?? 35, ANIM_Remove));
	return true;
}

private func DoSwitchFlip(object clonk, int dir)
{
	// Perform action associated to this switch
	if (dir > 0)
	{
		// Open/close should be aligned to vertical component of direction
		SetSwitchState(GetR() < 0, clonk); // switch on if rotation < 0
		// Action last; it may delete the door/clonk/etc.
		if (right_action)
			UserAction->EvaluateAction(right_action, this, clonk);
	}
	else
	{
		// Open/close should be aligned to vertical component of direction
		SetSwitchState(GetR() >= 0, clonk); // switch off if rotation < 0
		// Action last; it may delete the door/clonk/etc.	
		if (left_action)
			UserAction->EvaluateAction(left_action, this, clonk);
	}
	return false;
}

private func SwitchingLeft() { return SwitchingTimer(-1); }
private func SwitchingRight() { return SwitchingTimer(+1); }

local ActMap = {
	SwitchLeft = {
		Prototype = Action,
		Name = "SwitchLeft",
		Procedure = DFA_NONE,
		Length = 1,
		Delay = 1,
		NextAction = "SwitchLeft",
		StartCall = "SwitchingLeft",
		FacetBase = 1,
	},
	SwitchRight = {
		Prototype = Action,
		Name = "SwitchRight",
		Procedure = DFA_NONE,
		Length = 1,
		Delay = 1,
		NextAction = "SwitchRight",
		StartCall = "SwitchingRight",
		FacetBase = 1,
	},
};

local Name = "$Name$";
local Description = "$Description$";
local Touchable = 2;
local Plane = 270;
local MaxHandleAngle = 35;
local HandleSpeed = 6;
local Components = { Wood = 1, Metal = 1 };
local left_action, right_action; // Custom editor-selected actions on switch handling


func Definition(def)
{
	// Graphics
	SetProperty("PictureTransformation", Trans_Mul(Trans_Scale(800), Trans_Translate(0,0,0), Trans_Rotate(-20,1,0,0), Trans_Rotate(-30,0,1,0)), def);
	SetProperty("MeshTransformation", Trans_Rotate(-13,0,1,0), def);
	// Editor properties
	if (!def.EditorProps) def.EditorProps = {};
	def.EditorProps.left_action = new UserAction.Prop { Name="$LeftAction$" };
	def.EditorProps.right_action = new UserAction.Prop { Name="$RightAction$" };
	// Actions
	if (!def.EditorActions) def.EditorActions = {};
	def.EditorActions.SwitchLeft = { Name = "$SwitchLeft$", Command = "ControlSwitchDir(nil, -1)" };
	def.EditorActions.SwitchRight = { Name = "$SwitchRight$", Command = "ControlSwitchDir(nil, +1)" };
	def.EditorActions.Rotate = { Name = "$Rotate$", Command = "SetR((GetR()+135)/90*90)" };
	return _inherited(def, ...);
}
