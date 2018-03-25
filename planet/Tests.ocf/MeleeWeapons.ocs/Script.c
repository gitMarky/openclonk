/**
	Melee Weapons
	
	Unit tests for the behavior of melee weapons.
	
	Invokes tests by calling the global function Test*_OnStart(int plr)
	and iterate through all tests.
	The test is completed once Test*_Completed() returns true.
	Then Test*_OnFinished() is called, to be able to reset the scenario
	for the next test.
	
	@author Maikel (test logic), Marky (test)
*/


protected func Initialize()
{
	return;
}

protected func InitializePlayer(int plr)
{
	// Set zoom and move player to the middle of the scenario.
	SetPlayerZoomByViewRange(plr, LandscapeWidth(), nil, PLRZOOM_Direct);
	SetFoW(false, plr);
	GetCrew(plr)->SetPosition(120, 190);
	GetCrew(plr)->MakeInvincible();
	
	// Add test control effect.
	var effect = AddEffect("IntTestControl", nil, 100, 2);
	effect.testnr = 1;
	effect.launched = false;
	effect.plr = plr;
	return true;
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
	return;
}

// Calling this function skips the current test, does not work if last test has been ran already.
global func SkipTest()
{
	// Get the control effect.
	var effect = GetEffect("IntTestControl", nil);
	if (!effect)
		return;
	// Finish the previous test.
	Call(Format("~Test%d_OnFinished", effect.testnr));
	// Start the next test by just increasing the test number and setting 
	// effect.launched to false, effect will handle the rest.
	effect.testnr++;
	effect.skipped++;
	effect.launched = false;
	return;
}


/*-- Tests --*/

global func FxIntTestControlStart(object target, proplist effect, int temporary)
{
	if (temporary)
		return FX_OK;
	// Set default interval.
	effect.Interval = 2;
	effect.result = true;
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
		if (!Call(Format("~Test%d_OnStart", effect.testnr), effect))
		{
			Log("Test %d not available, this was the last test.", effect.testnr);
			Log("=====================================");
			if (effect.result)
				Log("All tests have passed!");
			else
				Log("At least one test has failed!");
			Log("%d tests passed", effect.passed);
			Log("%d tests failed", effect.failed);
			Log("%d tests skipped", effect.skipped);
			return -1;
		}
		effect.launched = true;
	}
	else if (effect.wait <= 0)
	{
		var result = Call(Format("Test%d_Execute", effect.testnr), effect);
		if (result == nil)
		{
			return FX_OK;
		}
		else if (result == true)
		{
			++effect.passed;
		}
		else
		{
			++effect.failed;
		}
		effect.launched = false;
		effect.result &= result;
		// Call the test on finished function.
		Call(Format("~Test%d_OnFinished", effect.testnr), effect);
		// Log result and increase test number.
		if (result)
			Log(">> Test %d passed.", effect.testnr);
		else
			Log (">> Test %d failed.", effect.testnr);

		effect.testnr++;
	}
	else
	{
		effect.wait--;
	}
	return FX_OK;
}

// ----------------------------------

global func Test1_OnStart(effect test)
{
	Log("Sword - Strike in range hurts the target");
	setUp(test, Sword);
	return true;
}

global func Test1_Execute(effect test)
{
	if (test.attack_started)
	{
		var new_health = test.defender->GetEnergy();
		return doTest("Sword strike in range hurts the target. Health was %d, should be %d", new_health, test.health - 9);
	}
	else
	{
		if (test.attacker->IsWalking())
		{
			test.attack_started = true;	
			test.health = test.defender->GetEnergy();
			var click = getClick(test);
			test.weapon->ControlUse(test.attacker, click.x, click.y);
			wait(test, Sword_Standard_StrikingLength + 5);
		}
		return nil; // Test cannot be evaluated yet
	}
}

global func Test1_OnFinished(effect test)
{
	cleanUp(test);
}

// ----------------------------------

global func Test2_OnStart(effect test)
{
	Log("Sword - Strike out of range does nothing");
	setUp(test, Sword);
	test.defender->MovePosition(10);
	return true;
}

global func Test2_Execute(effect test)
{
	if (test.attack_started)
	{
		var new_health = test.defender->GetEnergy();
		return doTest("Sword strike out of range does not hurt the target. Health was %d, should be %d", new_health, test.health);
	}
	else
	{
		if (test.attacker->IsWalking())
		{
			test.attack_started = true;	
			test.health = test.defender->GetEnergy();
			var click = getClick(test);
			test.weapon->ControlUse(test.attacker, click.x, click.y);
			wait(test, Sword_Standard_StrikingLength + 5);
		}
		return nil; // Test cannot be evaluated yet
	}
}

global func Test2_OnFinished(effect test)
{
	cleanUp(test);
}

// ----------------------------------

global func Test3_OnStart(effect test)
{
	Log("Sword - Strike after attack period does nothing");
	setUp(test, Sword);
	test.defender->MovePosition(+10);
	return true;
}

global func Test3_Execute(effect test)
{
	if (test.attack_started)
	{
		var new_health = test.defender->GetEnergy();
		return doTest("Sword after attack period does not hurt the target. Health was %d, should be %d", new_health, test.health);
	}
	else
	{
		if (test.attacker->IsWalking())
		{
			test.attack_started = true;	
			test.health = test.defender->GetEnergy();
			var click = getClick(test);
			test.weapon->ControlUse(test.attacker, click.x, click.y);
			test.defender->ScheduleCall(test.defender, Global.MovePosition, Sword_Standard_StrikingLength + 1, nil, -10);
			wait(test, Sword_Standard_StrikingLength + 5);
		}
		return nil; // Test cannot be evaluated yet
	}
}

global func Test3_OnFinished(effect test)
{
	cleanUp(test);
}

// ----------------------------------

global func Test4_OnStart(effect test)
{
	Log("Sword - Strike to the wrong side does nothing");
	setUp(test, Sword);
	test.defender->MovePosition(-20);
	return true;
}

global func Test4_Execute(effect test)
{
	if (test.attack_started)
	{
		var new_health = test.defender->GetEnergy();
		return doTest("Sword strike in range, but on the wrong side does not hurt the target. Health was %d, should be %d", new_health, test.health);
	}
	else
	{
		if (test.attacker->IsWalking())
		{
			test.attack_started = true;	
			test.health = test.defender->GetEnergy();
			var click = getClick(test);
			test.weapon->ControlUse(test.attacker, click.x, click.y);
			wait(test, Sword_Standard_StrikingLength + 5);
		}
		return nil; // Test cannot be evaluated yet
	}
}

global func Test4_OnFinished(effect test)
{
	cleanUp(test);
}

// ----------------------------------

global func wait(effect test, int amount)
{
	test.wait = amount;
}

global func doTest(description, returned, expected)
{
	var test = (returned == expected);
	
	var predicate = "[Fail]";
	if (test) predicate = "[Pass]";
	
	Log(Format("%s %s", predicate, description), returned, expected);
	return test;
}

global func setUp(effect test, id weapon)
{
	test.attacker = CreateObject(Clonk, 150, 190, NO_OWNER);	
	test.defender = CreateObject(Clonk, 160, 190, NO_OWNER);
	test.weapon = test.attacker->CreateContents(weapon);
	test.attack_started = false;
	test.attacker->SetDir(1);
	test.defender->SetDir(0);
}

global func cleanUp(effect test)
{
	if (test.weapon) test.weapon->RemoveObject();
	if (test.defender) test.defender->RemoveObject(false);
	if (test.attacker) test.attacker->RemoveObject(false);
}

global func getClick(effect test, int offset_x, int offset_y)
{
	return {
		x = test.defender->GetX() - test.defender->GetX() + offset_x,
		y = test.defender->GetY() - test.defender->GetY() + offset_y
	};
}
