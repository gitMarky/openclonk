/**
	Switch device
	
	Library for devices that can be connected with wires. 
	
	Needs to call _inherited in the following functions:
	* Construction()
	* UpdateWireStatus()
	
	@author Marky
*/


local lib_electronics_device;


static const LIBRARY_ELECTRONICS_Menu_Action_Connect_Input = "connect_input";
static const LIBRARY_ELECTRONICS_Menu_Action_Connect_Output = "connect_output";
static const LIBRARY_ELECTRONICS_Menu_Action_Disconnect_Input = "disconnect_input";
static const LIBRARY_ELECTRONICS_Menu_Action_Disconnect_Output = "disconnect_output";
static const LIBRARY_ELECTRONICS_Menu_Action_Description = "description";
static const LIBRARY_ELECTRONICS_Menu_Action_None = "none";

static const LIBRARY_ELECTRONICS_GateType_OR = 0;


/*-- Engine callbacks --*/

public func Construction(object by_object)
{
	_inherited(by_object, ...);
	lib_electronics_device = {
		wire_output = [],
		wire_input = [],
		state_input = [],
		state_output = false,
		gate_type = LIBRARY_ELECTRONICS_GateType_OR,
		custom_entry = 
		{
			Right = "100%", Bottom = "2em",
			BackgroundColor = {Std = 0, OnHover = 0x50ff0000},
			image = {Right = "2em"},
			text = {Left = "2em"}
		},
		update_object_interactions = [],
	};
}

/*-- Connection logic --*/

/*
 Sets the input signal of this mechanism.
 This is used by switches usually, but it is also used by the 
 electronics device without an electronics device being a switch.
 
 Use the callback to activate or deactivate certain functions in the mechanism.
 For example, the stone door opens if you pass 'true', and closes if you pass 'false'
 
 @par operator this object is operating the mechanism or the object that sent the signal-
 @par sender this object sent the signal - this is usually a switch.
 @par value this value is sent.
*/
public func SetInputSignal(object operator, object sender, bool value)
{
	var input_index = GetIndexOf(GetWireInputSources(), sender);
	if (input_index >= 0)
	{
		SetWireInputState(input_index, value);
		UpdateOutputState();
	}
	_inherited(operator, sender, value, ...);
}

/*-- Public Interface --*/

public func GetWireOutputMaxAmount() { return 0; }
public func GetWireInputMaxAmount() { return 0; }

public func GetWireOutput(int index) { return lib_electronics_device.wire_output[index]; }
public func GetWireInput(int index) { return lib_electronics_device.wire_input[index]; }

public func GetWireOutputState() { return lib_electronics_device.state_output; }
public func GetWireInputState(input)
{
	if (GetType(input) == C4V_Int)
	{
		return lib_electronics_device.state_input[input];
	}
	else if (GetType(input) == C4V_C4Object)
	{
		// Get the saved state
		var index = GetIndexOf(lib_electronics_device.wire_input, GetConnectedLine(input));
		if (index >= 0)
		{
			return GetWireInputState(index);
		}
		// Get the state of a switch, if possible
		if (input.GetSwitchState)
		{
			return input->GetSwitchState();
		}
	}
	return false;
}


public func SetWireOutput(int index, object pipe)
{
	lib_electronics_device.wire_output[index] = GetConnectedLine(pipe);
	return lib_electronics_device.wire_output[index];
}


public func SetWireInput(int index, object pipe)
{
	lib_electronics_device.wire_input[index] = GetConnectedLine(pipe);
	if (pipe)
	{
		SetWireInputState(index, !!pipe->~GetSwitchState());
	}
	else
	{
		SetWireInputState(index, false);
	}
	return pipe;
}


public func SetWireOutputState(bool value, object operator)
{
	// Set output
	lib_electronics_device.state_output = value;
	var targets = GetWireOutputDestinations(); 
	for (var index = 0; index < GetLength(targets); ++index)
	{
		var output = targets[index];
		if (output)
		{
			output->~SetInputSignal(operator ?? this, this, value);
		}
	}
	return lib_electronics_device.state_output;
}


public func SetWireInputState(int index, bool value)
{
	lib_electronics_device.state_input[index] = value;
	this->~UpdateWireStatus();
}


/*-- Pipe / wire control --*/

public func CanConnectPipe(){ return true;}


public func QueryConnectPipe(object pipe)
{
	// Do not allow connections from this object to itself
	if (pipe->~IsConnectedTo(this)
	 || pipe->~GetConnectedLine() && pipe->GetConnectedLine()->~IsConnectedTo(this))
	{
		return true;
	}
	// Connect only as long as there is a free input or output
	var free_output = GetUnusedWireOutputIndex();
	var free_input = GetUnusedWireInputIndex();
	if ((free_output != nil && pipe->IsNeutralPipe()) // Allow connecting neutral pipes to free outputs
	 || (free_input != nil && pipe->IsElectronicsPipe())) // Allow connecting electronics pipes to free inputs
	{
		return false;
	}
	// Forbid all other situations
	return true;
}


public func OnPipeConnect(object pipe, string specific_pipe_state)
{
	if (pipe->IsNeutralPipe())
	{
		var free_output = GetUnusedWireOutputIndex();

		if (free_output == nil)
		{
			pipe->CutLineConnection(this);
		}
		else
		{
			SetWireOutput(free_output, pipe);
			pipe->SetElectronicsPipe();
			pipe->Report("$MsgCreatedOutput$");
		}
	}
	else if (pipe->~IsElectronicsPipe())
	{
		var free_input = GetUnusedWireInputIndex();
		
		if (free_input == nil)
		{
			pipe->CutLineConnection(this);
		}
		else
		{
			SetWireInput(free_input, pipe);
			pipe->Report("$MsgCreatedInput$");
			
			var line = pipe->GetConnectedLine();
			if (line)
			{
				var output = line->GetConnectedObject(this);
				if (output && output.GetSwitchTarget && !output->GetSwitchTarget())
				{
					output->SetSwitchTarget(this);
				}
			}
		}
	}
	UpdateOutputState();
	GetConnectedObject(pipe, this)->~UpdateOutputState();
}


public func OnPipeDisconnect(object pipe)
{
	var index_output = GetIndexOf(lib_electronics_device.wire_output, GetConnectedLine(pipe));
	var index_input = GetIndexOf(lib_electronics_device.wire_input, GetConnectedLine(pipe));

	if (index_input >= 0)
	{
		SetWireInput(index_input);
		SetInputSignal(pipe, pipe, false);
	}
	if (index_output >= 0)
	{
		SetWireOutput(index_output);
	}
	UpdateOutputState();
}


private func GetUnusedWireOutputIndex()
{
	for (var index = 0; index < GetWireOutputMaxAmount(); ++index)
	{
		if (!GetWireOutput(index))
		{
			return index;
		}
	}
	return nil; // no free index
}

private func GetUnusedWireInputIndex()
{
	for (var index = 0; index < GetWireInputMaxAmount(); ++index)
	{
		if (!GetWireInput(index))
		{
			return index;
		}
	}
	return nil; // no free index
}


/*-- Menu Entries --*/

public func HasInteractionMenu() { return true; }

public func GetInteractionMenus(object clonk)
{
	var menus = _inherited(clonk, ...) ?? [];
	
	if (CanConnectPipe())
	{
		var menu_wire_input =
		{
			title = "$MenuWireControlInput$",
			entries_callback = this.GetWireControlInputMenuEntries,
			entries_callback_target = this,
			callback = "OnWireControl",
			callback_hover = "OnWireControlHover",
			callback_target = this,
			BackgroundColor = RGB(0, 50, 50),
			Priority = 41
		};
		var menu_wire_output =
		{
			title = "$MenuWireControlOutput$",
			entries_callback = this.GetWireControlOutputMenuEntries,
			entries_callback_target = this,
			callback = "OnWireControl",
			callback_hover = "OnWireControlHover",
			callback_target = this,
			BackgroundColor = RGB(0, 50, 50),
			Priority = 42
		};
		if (GetWireInputMaxAmount() > 0)  PushBack(menus, menu_wire_input);
		if (GetWireOutputMaxAmount() > 0) PushBack(menus, menu_wire_output);
	}
	return menus;
}

public func AddInteractionMenuListener(object listener)
{
	PushBack(lib_electronics_device.update_object_interactions, listener);
	RemoveDuplicates(lib_electronics_device.update_object_interactions);
	RemoveHoles(lib_electronics_device.update_object_interactions);
}

public func HasWireControlInputMenuEntriesReversed() { return false; } // Allows for inverse order

public func GetWireControlInputMenuEntries(object clonk)
{
	var menu_entries = [];
	
	// Add info message about wire input control.
	PushBack(menu_entries, {symbol = this, extra_data = {action = LIBRARY_ELECTRONICS_Menu_Action_Description, target = this},
			custom =
			{
				Prototype = lib_electronics_device.custom_entry,
				Bottom = "1.2em",
				Priority = -1,
				BackgroundColor = RGB(25, 100, 100),
				text = {Prototype = lib_electronics_device.custom_entry.text, Text = Format("{{%i}} $MenuWireControlInput$ (%d/%d)", Icon_Enter, GetLength(GetWireInputSources()), GetWireInputMaxAmount())},
				image = {Prototype = lib_electronics_device.custom_entry.image, Symbol = Pipe}
			}});

	var electronics_pipes = FindAvailableWires(clonk, Find_Func("IsElectronicsPipe"));
	var free_input = GetUnusedWireInputIndex();

	// List connected inputs first
	for (var index = 0; index < GetWireInputMaxAmount(); ++index)
	{
		var priority;
		if (HasWireControlInputMenuEntriesReversed())
		{
			priority = GetWireInputMaxAmount() - index;
		}
		else
		{
			priority = index + 1;
		}
	
		var entry;
		var input = GetWireInput(index);
		// There is one!
		if (input)
		{
			var pipe_target = GetConnectedObject(input, this);
			var overlay, text, on_hover;
			if (input->~IsPipeLine()) // Can be cut or not
			{
				//var pipe_target = input->GetConnectedObject(this);
				if (input->GetPipeKit()->QueryCutLineConnection(this)) // disconnect not allowed?
				{
					overlay = Icon_Lock;
					text = Format("$MenuOptionFixedInput$: %s", pipe_target->GetName());
					on_hover = {action = LIBRARY_ELECTRONICS_Menu_Action_Description, target = input};
				}
				else // disconnect allowed
				{
					overlay = Icon_Cancel;
					text = Format("$MenuOptionDisconnectInput$: %s", pipe_target->GetName());
					on_hover = {action = LIBRARY_ELECTRONICS_Menu_Action_Disconnect_Input, target = input};
				}
			}
			else // there is an object, let's assume it cannot be connected/disconnected
			{
				overlay = Icon_Lock;
				text = Format("$MenuOptionFixedInput$: %s", pipe_target->GetName());
				on_hover = {action = LIBRARY_ELECTRONICS_Menu_Action_Description, target = input};
			}
			entry = GetWireMenuEntry(pipe_target, overlay, GetSignalState(GetWireInputState(input)), text, priority, on_hover);
		}
		else // display an empty symbol that hints at the possibility to connect something
		{
			entry = GetWireMenuEntry(Icon_Lightbulb, nil, GetSignalState(), "$MenuOptionInactiveInput$", priority, {action = LIBRARY_ELECTRONICS_Menu_Action_None});
		}
		PushBack(menu_entries, entry);
	}

	// Allow connecting additional inputs, displaying each source separately
	if (free_input != nil)
	{
		for (var electronics_pipe in electronics_pipes)
		{
			var pipe_source = GetConnectedObject(electronics_pipe);
			if (pipe_source && pipe_source != this)
			{
				PushBack(menu_entries, GetWireMenuEntry(pipe_source, Icon_Play, nil, Format("$MenuOptionConnectInput$: %s", pipe_source->GetName()), GetWireInputMaxAmount() + 1, {action = LIBRARY_ELECTRONICS_Menu_Action_Connect_Input, target = electronics_pipe}));
			}
		}
	}

	return menu_entries;
}


public func GetWireControlOutputMenuEntries(object clonk)
{
	var menu_entries = [];
	
	// Add info message about wire output control.
	PushBack(menu_entries, {symbol = this, extra_data = {action = LIBRARY_ELECTRONICS_Menu_Action_Description, target = this},
			custom =
			{
				Prototype = lib_electronics_device.custom_entry,
				Bottom = "1.2em",
				Priority = -1,
				BackgroundColor = RGB(25, 100, 100),
				text = {Prototype = lib_electronics_device.custom_entry.text, Text = Format("{{%i}} $MenuWireControlOutput$ (%d/%d)", Icon_Exit, GetLength(GetWireOutputDestinations()), GetWireOutputMaxAmount())},
				image = {Prototype = lib_electronics_device.custom_entry.image, Symbol = Pipe}
			}});

	var neutral_pipes = FindAvailableWires(clonk, Find_Func("IsNeutralPipe"));
	var free_output = GetUnusedWireOutputIndex();

	// List connected outputs first
	for (var index = 0; index < GetWireOutputMaxAmount(); ++index)
	{
		var entry;
		var output = GetWireOutput(index);
		var priority = index + 1;
		// There is one!
		if (output)
		{
			var pipe_target = GetConnectedObject(output, this);
			var overlay, text, on_hover;
			if (output->~IsPipeLine()) // Can be cut or not
			{
				//var pipe_target = output->GetConnectedObject(this);
				if (output->GetPipeKit()->QueryCutLineConnection(this)) // disconnect not allowed?
				{
					overlay = Icon_Lock;
					text = Format("$MenuOptionFixedOutput$: %s", pipe_target->GetName());
					on_hover = {action = LIBRARY_ELECTRONICS_Menu_Action_Description, target = output};
				}
				else // disconnect allowed
				{
					overlay = Icon_Cancel;
					text = Format("$MenuOptionDisconnectOutput$: %s", pipe_target->GetName());
					on_hover = {action = LIBRARY_ELECTRONICS_Menu_Action_Disconnect_Output, target = output};
				}
			}
			else // there is an object, let's assume it cannot be connected/disconnected
			{
				overlay = Icon_Lock;
				text = Format("$MenuOptionFixedOutput$: %s", pipe_target->GetName());
				on_hover = {action = LIBRARY_ELECTRONICS_Menu_Action_Description, target = output};
			}
			entry = GetWireMenuEntry(pipe_target, overlay, GetSignalState(GetWireOutputState()), text, priority, on_hover);
		}
		else // display an empty symbol that hints at the possibility to connect something
		{
			entry = GetWireMenuEntry(Icon_Lightbulb, nil, GetSignalState(), "$MenuOptionInactiveOutput$", priority, {action = LIBRARY_ELECTRONICS_Menu_Action_None});
		}
		PushBack(menu_entries, entry);
	}

	// Allow connecting additional outputs, displaying each target separately
	if (free_output != nil)
	{
		for (var neutral_pipe in neutral_pipes)
		{
			var pipe_target = GetConnectedObject(neutral_pipe);
			if (pipe_target && pipe_target != this)
			{
				PushBack(menu_entries, GetWireMenuEntry(pipe_target, Icon_Play, nil, Format("$MenuOptionConnectOutput$: %s", pipe_target->GetName()), GetWireOutputMaxAmount() + 1, {action = LIBRARY_ELECTRONICS_Menu_Action_Connect_Output, target = neutral_pipe}));
			}
		}
	}

	return menu_entries;
}

public func GetWireMenuEntry(symbol, symbol_overlay, string signal_state, string text, int priority, extra_data)
{
	var entry =  {
		symbol = symbol,
		extra_data = extra_data, 
		custom =
		{
			Prototype = lib_electronics_device.custom_entry,
			Priority = priority,
			text = {Prototype = lib_electronics_device.custom_entry.text, Text = text},
			image = {Prototype = lib_electronics_device.custom_entry.image, Symbol = symbol}
		}};
	if (symbol_overlay)
	{
		entry.custom.image.overlay_top =
		{
			Left = ToPercentString(500),
			Bottom = ToPercentString(500),
			Symbol = symbol_overlay,
		};
	}
	if (signal_state != nil)
	{			
		// Add it to the image
		entry.custom.image.overlay_bottom = 
		{
			Left = ToPercentString(500),
			Top = ToPercentString(500),
			Symbol = Icon_ElectricSignal,
			GraphicsName = signal_state,
		};
	}
	return entry;
}


private func GetSignalState(bool state)
{
	if (state == nil)
		return "";
	else if (state)
		return "On";
	else
		return "Off";
}

public func OnWireControlHover(id symbol, proplist data, desc_menu_target, menu_id)
{
	var text = "";
	     if (data.action == LIBRARY_ELECTRONICS_Menu_Action_Connect_Input) text = "$DescConnectInput$";
	else if (data.action == LIBRARY_ELECTRONICS_Menu_Action_Connect_Output) text = "$DescConnectOutput$";
	else if (data.action == LIBRARY_ELECTRONICS_Menu_Action_Disconnect_Input) text = "$DescDisconnectInput$";
	else if (data.action == LIBRARY_ELECTRONICS_Menu_Action_Disconnect_Input) text = "$DescDisconnectOutput$";
	GuiUpdateText(text, menu_id, 1, desc_menu_target);
}

public func OnWireControl(symbol_or_object, proplist data, bool alt)
{
	if (data.action == LIBRARY_ELECTRONICS_Menu_Action_Connect_Input
	 || data.action == LIBRARY_ELECTRONICS_Menu_Action_Connect_Output)
	{
		this->DoConnectWire(data.target);
	}
	else if (data.action == LIBRARY_ELECTRONICS_Menu_Action_Disconnect_Input
	      || data.action == LIBRARY_ELECTRONICS_Menu_Action_Disconnect_Output)
	{
		this->DoDisconnectWire(data.target);
	}

	// Scheduled update, because some things take their time
	ScheduleCall(this, this.UpdateInteractionMenus, 1, nil, this.GetWireControlInputMenuEntries);
	ScheduleCall(this, this.UpdateInteractionMenus, 1, nil, this.GetWireControlOutputMenuEntries);
	
	// Notify the listeners
	for (var listener in lib_electronics_device.update_object_interactions)
	{
		if (listener)
		{
			ScheduleCall(listener, listener.UpdateInteractionMenus, 1, nil);
		}
	}
}


/*-- Menu Callbacks --*/

public func DoConnectWire(object pipe, string specific_pipe_state)
{
	pipe->ConnectPipeTo(this, specific_pipe_state);
}

public func DoDisconnectWire(object pipe)
{
	var kit = GetConnectedPipeKit(pipe);
	if (kit && kit->~IsPipe())
	{
		kit->CutLineConnection(this);
	}
}

public func FindAvailableWire(object container, find_state)
{
	for (var pipe in FindAvailableWires(container, find_state))
	{
		if (!this->~QueryConnectPipe(pipe))
			return pipe;
	}
	return nil;
}

private func FindAvailableWires(object container, find_state)
{
	return FindObjects(Find_ID(Pipe), Find_Container(container), find_state);
}


/*-- Internals --*/


private func GetConnectedLine(object pipe)
{
	if (pipe && pipe->~IsPipe())
	{
		return pipe->GetConnectedLine() ?? pipe;
	}
	else
	{
		return pipe;
	}
}

private func GetConnectedObject(object pipe, object connected_to)
{
	var source = pipe;
	pipe = GetConnectedLine(pipe);

	if (pipe && pipe->~IsPipeLine())
	{
		return pipe->GetConnectedObject(connected_to ?? source);
	}
	else
	{
		return pipe;
	}
}

private func GetConnectedPipeKit(object pipe)
{
	if (pipe && pipe->~IsPipeLine())
	{
		return pipe->GetPipeKit();
	}
	else
	{
		return pipe;
	}
}


private func UpdateOutputState(object operator)
{
	var fx = GetEffect("FxCalculateOutputState", this);
	if (!fx)
	{
		fx = CreateEffect(FxCalculateOutputState, 1, 1);
	}
	fx.operator = operator;
}


local FxCalculateOutputState = new Effect
{
	Timer = func ()
	{
		this.Target->CalculateOutputState(this.operator);
		return FX_Execute_Kill;
	},
};


private func CalculateOutputState(object operator)
{
	var outputs = GetWireOutputDestinations();
	RemoveHoles(outputs);
	var has_output = GetLength(outputs) > 0;
	// Outputs are off as long as no output is connected
	SetWireOutputState(has_output && GetLogicGateValue(), operator);
	this->~UpdateWireStatus();
}


private func GetLogicGateValue()
{
	// Calculate inputs
	var gate_status = false;
	
	for (var index = 0; index < GetWireInputMaxAmount(); ++index)
	{
		if (GetWireInput(index))
		{
			// Default gate: OR
			gate_status = gate_status || GetWireInputState(index);
		}
	}
	
	return gate_status;
}


private func GetWireInputSources()
{
	return GetWireConnectedObjects(GetWireInputMaxAmount(), this.GetWireInput);
}


private func GetWireOutputDestinations()
{
	return GetWireConnectedObjects(GetWireOutputMaxAmount(), this.GetWireOutput);
}


private func GetWireConnectedObjects(int max_amount, func get_pipe)
{
	var list = [];
	for (var index = 0; index < max_amount; ++index)
	{
		var pipe = Call(get_pipe, index);
		if (pipe)
		{
			list[index] = GetConnectedObject(pipe, this); // SetInputSignal relies on the indices being the same, so the holes need to be there
		}
	}
	return list;
}
