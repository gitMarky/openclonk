/**
	Electronics
	Unit tests for the elecronics stations. Invokes tests by calling the 
	global function Test*_OnStart(int plr) and iterate through all 
	tests. The test is completed once Test*_Completed() returns
	true. Then Test*_OnFinished() is called, to be able to reset 
	the scenario for the next test.
	
	With LaunchTest(int nr) a specific test can be launched when
	called during runtime. A test can be skipped by calling the
	function SkipTest().
	
	@author Marky (tests), Maikel (test control)
*/


static script_plr;

protected func Initialize()
{
	// Add the no power rule, this is about liquids.
	CreateObject(Rule_NoPowerNeed);	
	// Create a script player for some tests.
	script_plr = nil;
	CreateScriptPlayer("Buddy", RGB(0, 0, 255), nil, CSPF_NoEliminationCheck);
	return;
}

protected func InitializePlayer(int plr)
{
	// Set zoom to full map size.
	SetPlayerZoomByViewRange(plr, LandscapeWidth(), nil, PLRZOOM_Direct);
	
	// No FoW to see everything happening.
	SetFoW(false, plr);
	
	// All players belong to the first team.
	// The second team only exists for testing.
	SetPlayerTeam(plr, 1);
		
	// Initialize script player.
	if (GetPlayerType(plr) == C4PT_Script)
	{
		// Store the player number.
		if (script_plr == nil)
			script_plr = plr;
		// No crew needed.
		GetCrew(plr)->RemoveObject();
		return;
	}	
	
	// Move player to the start of the scenario.
	GetCrew(plr)->SetPosition(120, 150);
	
	// Some knowledge to construct a flagpole.
	GetCrew(plr)->CreateContents(Hammer);
	SetPlrKnowledge(plr, Flagpole);
	
	// Add test control effect.
	var effect = AddEffect("IntTestControl", nil, 100, 2);
	effect.testnr = 1;
	effect.launched = false;
	effect.plr = plr;
	return;
}

protected func RemovePlayer(int plr)
{
	// Remove script player.
	if (GetPlayerType(plr) == C4PT_Script)
	{
		if (plr == script_plr)
			script_plr = nil;
		return;	
	}
	return;
}


/*-- Test Control --*/

// Aborts the current test and launches the specified test instead.
global func LaunchTest(int nr)
{
	// Get the control effect.
	var effect = GetEffect("IntTestControl", nil);
	if (!effect)
	{
		// Create a new control effect and launch the test.
		effect = AddEffect("IntTestControl", nil, 100, 2);
		effect.testnr = nr;
		effect.launched = false;
		effect.plr = GetPlayerByIndex(0, C4PT_User);
		return;
	}
	// Finish the currently running test.
	Call(Format("~Test%d_OnFinished", effect.testnr));
	// Start the requested test by just setting the test number and setting 
	// effect.launched to false, effect will handle the rest.
	effect.testnr = nr;
	effect.launched = false;
	effect.verdict = nil;
	return;
}

// Calling this function skips the current test, does not work if last test has been ran already.
global func SkipTest()
{
	// Get the control effect.
	var effect = GetTest();
	if (!effect)
		return;
	// Finish the previous test.
	Call(Format("~Test%d_OnFinished", effect.testnr));
	// Start the next test by just increasing the test number and setting 
	// effect.launched to false, effect will handle the rest.
	effect.testnr++;
	effect.launched = false;
	effect.verdict = nil;
	return;
}


global func GetTest()
{
	return GetEffect("IntTestControl", nil);
}


/*-- Test Effect --*/

global func FxIntTestControlStart(object target, proplist effect, int temporary)
{
	if (temporary)
		return FX_OK;
	// Set default interval.
	effect.Interval = 2;
	effect.failed = [];
	return FX_OK;
}

global func FxIntTestControlTimer(object target, proplist effect)
{
	// Launch new test if needed.
	if (!effect.launched)
	{
		// Log test start.
		Log("=====================================");
		Log("Test %d started:", effect.testnr);
		// Start the test if available, otherwise finish test sequence.
		if (!Call(Format("~Test%d_OnStart", effect.testnr), effect.plr))
		{
			Log("Test %d not available, the previous test was the last test.", effect.testnr);
			Log("=====================================");
			if (GetLength(effect.failed) == 0)
			{
				Log("All tests have been successfully completed!");
			}
			else
			{
				Log("Tests failed: %v", effect.failed);
			}
			return FX_Execute_Kill;
		}
		effect.launched = true;
	}
	// Check whether the current test has been finished.
	if (Call(Format("Test%d_Completed", effect.testnr)))
	{
		effect.launched = false;
		//RemoveTest();
		// Call the test on finished function.
		Call(Format("~Test%d_OnFinished", effect.testnr));
		// Log result and increase test number.
		if (effect.verdict)
		{
			Log("Test %d successfully completed.", effect.testnr);
		}
		else
		{
			Log("Test %d failed", effect.testnr);
			PushBack(effect.failed, effect.testnr);
		}
		effect.testnr++;
		effect.verdict = nil;
	}
	return FX_OK;
}


/*-- Tests --*/

global func Test1_OnStart(int plr)
{	
	var station1 = CreateObject(ElectronicsStation, 135, 152, plr);
	var station2 = CreateObject(ElectronicsStation, 215, 152, plr);
	
	// Log what the test is about.
	Log("Connect pipe to station1.");
	var wire = CreateObject(Pipe, 150, 150, plr);
	wire->ConnectPipeTo(station1);
	
	Test("Connecting a pipe makes it the output. Got %v, expected %v", station1->GetWireOutput(0), wire->GetConnectedLine());
	Test("Output destination is %v, expected %v", station1->GetWireOutputDestinations()[0], wire);
	Test("Output signal is %v, should be %v", station1->GetWireOutputState(), false);
	
	Log("Connect pipe to station2");
	wire->ConnectPipeTo(station2);
	
	Test("Connecting a pipe makes it the input. Got %v, expected %v", station2->GetWireInput(0), wire->GetConnectedLine());
	Test("Input source is %v, expected %v", station2->GetWireInputSources()[0], station1);
	Test("Input signal is %v, should be %v", station2->GetWireInputState(0), false);
	
	return true;
}

global func Test1_Completed()
{
	return GetTest().verdict;
}

global func Test1_OnFinished()
{
	RemoveAll(Find_ID(ElectronicsStation));
}



global func Test2_OnStart(int plr)
{	
	var station1 = CreateObject(ElectronicsStation, 135, 152, plr);
	var station2 = CreateObject(ElectronicsStation, 215, 152, plr);
	var dummy1 = CreateObject(Dummy);
	var dummy2 = CreateObject(Dummy);
	var dummy3 = CreateObject(Dummy);
	
	// Log what the test is about.
	Log("Connect pipe to station1.");
	var wire = CreateObject(Pipe, 150, 150, plr);
	wire->ConnectPipeTo(station1);
	wire->ConnectPipeTo(station2);
	station1->SetWireInput(0, dummy1);
	station2->SetWireOutput(0, dummy3);

	Log("Testing default state");

	Test("Station1 input signal 0 is %v, should be %v", station1->GetWireInputState(0), false);
	Test("Station1 output signal is %v, should be %v", station1->GetWireOutputState(), false);
	Test("Station2 input signal 0 is %v, should be %v", station2->GetWireInputState(0), false);
	Test("Station2 output signal is %v, should be %v", station2->GetWireOutputState(), false);

	Log("Set input signal 0 to 'true'");
	
	station1->SetInputSignal(GetHiRank(), dummy1, true);
	
	Test("Station1 input signal 0 is %v, should be %v", station1->GetWireInputState(0), true);
	Test("Station1 output signal is %v, should be %v", station1->GetWireOutputState(), true);
	Test("Station2 input signal 0 is %v, should be %v", station2->GetWireInputState(0), true);
	Test("Station2 output signal is %v, should be %v", station2->GetWireOutputState(), true);
	
	Log("Connect a second input - state should not change because this is an or-gate");
	station1->SetWireInput(1, dummy2);

	Test("Station1 input signal 0 is %v, should be %v", station1->GetWireInputState(0), true);
	Test("Station1 input signal 1 is %v, should be %v", station1->GetWireInputState(1), false);
	Test("Station1 output signal is %v, should be %v", station1->GetWireOutputState(), true);
	Test("Station2 input signal 0 is %v, should be %v", station2->GetWireInputState(0), true);
	Test("Station2 output signal is %v, should be %v", station2->GetWireOutputState(), true);
	
	Log("Set input signal 1 to 'true'");
	
	station1->SetInputSignal(GetHiRank(), dummy2, true);

	Test("Station1 input signal 0 is %v, should be %v", station1->GetWireInputState(0), true);
	Test("Station1 input signal 1 is %v, should be %v", station1->GetWireInputState(1), true);
	Test("Station1 output signal is %v, should be %v", station1->GetWireOutputState(), true);
	Test("Station2 input signal 0 is %v, should be %v", station2->GetWireInputState(0), true);
	Test("Station2 output signal is %v, should be %v", station2->GetWireOutputState(), true);

	Log("Set input signal 0 to 'false'");
	
	station1->SetInputSignal(GetHiRank(), dummy1, false);

	Test("Station1 input signal 0 is %v, should be %v", station1->GetWireInputState(0), false);
	Test("Station1 input signal 1 is %v, should be %v", station1->GetWireInputState(1), true);
	Test("Station1 output signal is %v, should be %v", station1->GetWireOutputState(), true);
	Test("Station2 input signal 0 is %v, should be %v", station2->GetWireInputState(0), true);
	Test("Station2 output signal is %v, should be %v", station2->GetWireOutputState(), true);

	Log("Set input signal 1 to 'false'");
	
	station1->SetInputSignal(GetHiRank(), dummy2, false);

	Test("Station1 input signal 0 is %v, should be %v", station1->GetWireInputState(0), false);
	Test("Station1 input signal 1 is %v, should be %v", station1->GetWireInputState(1), false);
	Test("Station1 output signal is %v, should be %v", station1->GetWireOutputState(), false);
	Test("Station2 input signal 0 is %v, should be %v", station2->GetWireInputState(0), false);
	Test("Station2 output signal is %v, should be %v", station2->GetWireOutputState(), false);
	
	Log("Set input signal 1 to 'true'");
	
	station1->SetInputSignal(GetHiRank(), dummy2, true);

	Test("Station1 input signal 0 is %v, should be %v", station1->GetWireInputState(0), false);
	Test("Station1 input signal 1 is %v, should be %v", station1->GetWireInputState(1), true);
	Test("Station1 output signal is %v, should be %v", station1->GetWireOutputState(), true);
	Test("Station2 input signal 0 is %v, should be %v", station2->GetWireInputState(0), true);
	Test("Station2 output signal is %v, should be %v", station2->GetWireOutputState(), true);
	
	return true;
}

global func Test2_Completed()
{
	return GetTest().verdict;
}

global func Test2_OnFinished()
{
	RemoveAll(Find_Or(Find_ID(ElectronicsStation), Find_ID(Dummy)));
}

/*-- Helper Functions --*/

global func RestoreWaterLevels()
{
	// Restore water levels.
	DrawMaterialQuad("Water", 144, 168, 208 + 1, 168, 208 + 1, 304, 144, 304, true);
	for (var x = 216; x <= 280; x++)
		for (var y = 24; y <= 120; y++)
			if (GetMaterial(x, y) != Material("BrickSoft"))
				ClearFreeRect(x, y, 1, 1);
	return;
}

global func DrawMatBasin(string mat, int x, int y)
{
	DrawMaterialQuad("Brick", x - 10, y - 10, x - 10, y + 10, x + 10, y + 10, x + 10, y - 10);
	DrawMaterialQuad(mat, x - 6, y - 6, x - 6, y + 6, x + 6, y + 6, x + 6, y - 6);
	return;
}

global func RemoveWater()
{
	for (var x = 144; x <= 208 + 1; x++)
		for (var y = 168; y <= 304; y++)
			if (GetMaterial(x, y) != Material("BrickSoft"))
				ClearFreeRect(x, y, 1, 1);
	return;
}




global func Test(description, returned, expected)
{
	var test = (returned == expected);
	
	var predicate = "[Fail]";
	if (test) predicate = "[Pass]";
	
	Log(Format("%s %s", predicate, description), returned, expected);
	GetTest().verdict = (GetTest().verdict ?? test) && test;
	return test;
}
