/**
	Tutorial 02: Bombing Barriers
	Author: Maikel
	
	Again your wipf is lost: explains items, collection, inventory management, using them and throwing them.
	
	Following controls and concepts are explained:
	 * Collection of objects
	 * Inventory control
	 * Throwing items: firestones
	 * Dropping items: mushroom/berries
	 * Using items: shovel, loam
*/


static guide; // guide object.

protected func Initialize()
{
	// Tutorial goal.
	var goal = CreateObject(Goal_Tutorial);
	goal.Name = "$MsgGoalName$";
	goal.Description = "$MsgGoalDescription$";
	
	// Set the mood.
	//SetGamma(RGB(30, 25, 20), RGB(135, 130, 125), RGB(255, 250, 245));
	SetSkyParallax(0, 20, 20);
	
	// Place objects in different sections.
	InitCaveEntrance();
	InitCaveMiddle();
	InitCaveBottom();
	InitCliffTop();
	InitAcidLake();
	InitVegetation();
	InitAnimals();
	
	// Dialogue options -> repeat round.
	SetNextMission("Tutorials.ocf\\Tutorial02.ocs", "$MsgRepeatRound$", "$MsgRepeatRoundDesc$");
	return;
}

// Gamecall from goals, set next mission.
protected func OnGoalsFulfilled()
{
	// Dialogue options -> next round.
	SetNextMission("Tutorials.ocf\\Tutorial03.ocs", "$MsgNextTutorial$", "$MsgNextTutorialDesc$");
	// Normal scenario ending by goal library.
	return false;
}

private func InitCaveEntrance()
{
	CreateObject(WindGenerator, 40, 328);
	CreateObject(Armory, 112, 328);
	CreateObject(ToolsWorkshop, 192, 328);
	
	// Place a shovel for digging.
	var shovel = CreateObject(Shovel, 200, 328);
	shovel->SetR(Random(360));

	return;
}

private func InitCaveMiddle()
{
	// Make sure the brick overlaps the rock to blast.
	DrawMaterialQuad("Brick", 432, 536, 472, 536, 472, 542, 432, 542, true);
	
	// Some firestones on the way, to pick up.
	//PlaceObjects(Firestone, 2 + Random(2), "Earth", 248, 440, 40, 40);
		
	// Some mushrooms and ferns in the middle.
	Fern->Place(2 + Random(2), Rectangle(0, 480, 56, 40));
	Mushroom->Place(4 + Random(2), Rectangle(0, 480, 56, 40));

	// Some mushrooms and ferns in the middle.
	Fern->Place(3 + Random(2), Rectangle(128, 464, 104, 80));
	Mushroom->Place(6 + Random(2), Rectangle(128, 464, 104, 80));
	
	// Some mushrooms and ferns in the middle.
	Fern->Place(3 + Random(2), Rectangle(312, 496, 192, 136));
	Mushroom->Place(6 + Random(2), Rectangle(312, 496, 192, 136));
	
	// Seaweed in the two lakes.
	Seaweed->Place(5, Rectangle(48, 400, 96, 32));

	return;
}

private func InitCaveBottom()
{
	PlaceObjects(Loam, 2 + Random(2), "Earth", 496, 592, 40, 24);
	
	// Seaweed in the two lakes.
	Seaweed->Place(5, Rectangle(128, 688, 88, 48));
	Seaweed->Place(5, Rectangle(568, 688, 80, 48));
	return;
}

private func InitCliffTop()
{
	// Some trees and bushes on top of the cliff.
	SproutBerryBush->Place(3 + Random(3), Rectangle(240, 160, 400, 120));	
		
	// Berry bush on the right side of the cliff.
	SproutBerryBush->Place(1, Rectangle(580, 310, 60, 60));
	return;
}

private func InitAcidLake()
{
	// Ropebridge over the acid lake.
	var post1 = CreateObject(Ropebridge_Post, 816, 528);
	var post2 = CreateObject(Ropebridge_Post, 928, 528);
	post2->SetObjDrawTransform(-1000, 0, 0, 0, 1000);
	post2.Double->SetObjDrawTransform(-1000, 0, 0, 0, 1000);
	var bridge = CreateObject(Ropebridge, 872, 528);
	bridge->MakeBridge(post1, post2);
	bridge->SetFragile();
	
	// An acid power plant on the right with some dead trees.
	CreateObject(Tree_Coniferous_Burned, 754, 532)->DoCon(-30);
	CreateObject(Tree_Coniferous_Burned, 792, 532)->DoCon(-10);
	CreateObject(Tree_Coniferous_Burned, 942, 532)->DoCon(-35);	
	CreateObject(SteamEngine, 980, 528); // TODO: replace with the acid machine
	return;
}

// Vegetation throughout the scenario.
private func InitVegetation()
{
	PlaceGrass(85);
	PlaceObjects(Firestone, 25 + Random(5), "Earth");
	PlaceObjects(Loam, 15 + Random(5), "Earth");
	return;
}

private func InitAnimals()
{
	// The wipf as your friend, controlled by AI.
	CreateObject(Wipf, 500, 536);
	
	// Some butterflies as atmosphere.
	for (var i = 0; i < 25; i++)
		PlaceAnimal(Butterfly);
	return;
}	

/*-- Player Handling --*/

protected func InitializePlayer(int plr)
{
	// Position player's clonk.
	var clonk = GetCrew(plr, 0);
	clonk->SetPosition(40, 318);
	var effect = AddEffect("ClonkRestore", clonk, 100, 10);
	effect.to_x = 40;
	effect.to_y = 318;
	
	// Add an effect to the clonk to track the goal.
	AddEffect("TrackGoal", clonk, 100, 2);

	// Standard player zoom for tutorials.
	SetPlayerViewLock(plr, true);
	SetPlayerZoomByViewRange(plr, 400, nil, PLRZOOM_Direct);

	// Create tutorial guide, add messages, show first.
	guide = CreateObject(TutorialGuide, 0, 0 , plr);
	guide->AddGuideMessage("$MsgTutorialIntro$");
	guide->ShowGuideMessage(0);
	AddEffect("TutorialShovel", nil, 100, 5);
	return;
}

/*-- Intro, Tutorial Goal & Outro --*/

global func FxTrackGoalTimer(object target, proplist effect, int time)
{
	if (Inside(target->GetX(), 982, 1002) && Inside(target->GetY(), 504, 528))
	{
		if (FindObject(Find_ID(Wipf), Find_Distance(15, target->GetX(), target->GetY())))
		{
			AddEffect("GoalOutro", target, 100, 5);
			return -1;	
		}		
	}
	return 1;
}

global func FxGoalOutroStart(object target, proplist effect, int temp)
{
	if (temp)
		return;
		
	// Show guide message congratulating.
	guide->AddGuideMessage("$MsgTutorialCompleted$");
	guide->ShowGuideMessage(10);
		
	return 1;
}

global func FxGoalOutroTimer(object target, proplist effect, int time)
{
	if (time >= 54)
	{
		var goal = FindObject(Find_ID(Goal_Tutorial));
		goal->Fulfill();
		return -1;
	}
	return 1;
}

global func FxGoalOutroStop(object target, proplist effect, int reason, bool temp)
{
	if (temp) 
		return;
}

/*-- Guide Messages --*/

global func FxTutorialShovelTimer()
{
	if (FindObject(Find_OCF(OCF_CrewMember), Find_InRect(120, 264, 64, 64)))
	{
		guide->AddGuideMessage("$MsgTutorialShovel$");
		guide->ShowGuideMessage(1);
		AddEffect("TutorialDigging", nil, 100, 5);
		return -1;
	}
	return 1;
}

global func FxTutorialDiggingTimer()
{
	if (FindObject(Find_OCF(OCF_CrewMember), Find_InRect(328, 304, 96, 64)))
	{
		guide->AddGuideMessage("$MsgTutorialDigging$");
		guide->ShowGuideMessage(2);
		AddEffect("TutorialFirestones", nil, 100, 5);
		return -1;
	}
	return 1;
}

global func FxTutorialFirestonesTimer()
{
	if (FindObject(Find_OCF(OCF_CrewMember), Find_InRect(288, 384, 24, 96)))
	{
		guide->AddGuideMessage("$MsgTutorialFirestones$");
		guide->ShowGuideMessage(3);
		AddEffect("TutorialWipfHole", nil, 100, 5);
		return -1;
	}
	return 1;
}

global func FxTutorialWipfHoleTimer()
{
	if (FindObject(Find_OCF(OCF_CrewMember), Find_InRect(368, 496, 72, 56)))
	{
		guide->AddGuideMessage("$MsgTutorialFoundWipf$");
		guide->ShowGuideMessage(4);
		AddEffect("TutorialBlastedRock", nil, 100, 5);
		return -1;
	}
	return 1;
}

global func FxTutorialBlastedRockTimer()
{
	if (FindObject(Find_OCF(OCF_CrewMember), Find_InRect(468, 496, 72, 48)))
	{
		guide->AddGuideMessage("$MsgTutorialBlastedRock$");
		guide->ShowGuideMessage(5);
		AddEffect("TutorialFedWipf", nil, 100, 5);
		return -1;
	}
	return 1;
}

global func FxTutorialFedWipfTimer()
{
	var wipf = FindObject(Find_ID(Wipf));
	if (wipf->HadFood())
	{
		guide->AddGuideMessage("$MsgTutorialFedWipf$");
		guide->ShowGuideMessage(6);
		AddEffect("TutorialDigOutLoam", nil, 100, 5);
		return -1;
	}
	return 1;
}

global func FxTutorialDigOutLoamTimer()
{
	if (FindObject(Find_OCF(OCF_CrewMember), Find_InRect(424, 600, 32, 64)))
	{
		guide->AddGuideMessage("$MsgTutorialDigOutLoam$");
		guide->ShowGuideMessage(7);
		AddEffect("TutorialFragileBridge", nil, 100, 5);
		return -1;
	}
	return 1;
}

global func FxTutorialFragileBridgeTimer()
{
	if (FindObject(Find_OCF(OCF_CrewMember), Find_InRect(744, 480, 80, 48)))
	{
		guide->AddGuideMessage("$MsgTutorialFragileBridge$");
		guide->ShowGuideMessage(8);
		// Stop the controls of the clonk for a few seconds.
		AddEffect("TutorialWaitForBridge", nil, 100, 5);
		return -1;
	}
	return 1;
}

global func FxTutorialWaitForBridgeTimer(object target, proplist effect, int time)
{
	if (time > 2 * 36)
	{
		guide->AddGuideMessage("$MsgTutorialMakeLoamBridge$");
		guide->ShowGuideMessage(9);
		// Start the controls of the clonk again.
		return -1;
	}
	return 1;
}

protected func OnGuideMessageShown(int plr, int index)
{
	// Show the direction to move.
	if (index == 0)
		TutArrowShowPos(104, 312, 90);
	// Show the shovel.
	if (index == 1)
	{
		var shovel = FindObject(Find_ID(Shovel), Find_NoContainer());
		if (shovel)
			TutArrowShowTarget(shovel); 
	}
	// Show the direction to dig.
	if (index == 2)
		TutArrowShowPos(366, 396, 225);	
	// Show the firestone material.
	if (index == 3)
		TutArrowShowPos(268, 456);
	// Show the rock to blow up.
	if (index == 4)
		TutArrowShowPos(436, 520, 90);
	// Show the clonks friend: the wipf.	
	if (index == 5)
		TutArrowShowTarget(FindObject(Find_ID(Wipf))); 
	// Show to the way down to the settlement.
	if (index == 6)
		TutArrowShowPos(368, 592);
	// Show the loam pieces to collect.
	if (index == 7)
		TutArrowShowPos(504, 614, 90);
	// Show the fragile bridge.
	if (index == 8)
		TutArrowShowPos(848, 520);
	return;
}

protected func OnGuideMessageRemoved(int plr, int index)
{
	TutArrowClear();
	return;
}


/*-- Clonk restoring --*/

global func FxClonkRestoreTimer(object target, proplist effect, int time)
{
	// Respawn clonk to new location if reached certain position.
	if (FindObject(Find_OCF(OCF_CrewMember), Find_InRect(266, 382, 80, 100)))
	{
		effect.to_x = 304;
		effect.to_y = 444;             
	}
	if (FindObject(Find_OCF(OCF_CrewMember), Find_InRect(408, 574, 60, 58)))
	{
		effect.to_x = 428;
		effect.to_y = 624;             
	}
	if (FindObject(Find_OCF(OCF_CrewMember), Find_InRect(744, 464, 72, 64)))
	{
		effect.to_x = 776;
		effect.to_y = 518;             
	}
	return 1;
}

// Relaunches the clonk, from death or removal.
global func FxClonkRestoreStop(object target, effect, int reason, bool  temporary)
{
	if (reason == 3 || reason == 4)
	{
		var restorer = CreateObject(ObjectRestorer, 0, 0, NO_OWNER);
		var x = BoundBy(target->GetX(), 0, LandscapeWidth());
		var y = BoundBy(target->GetY(), 0, LandscapeHeight());
		restorer->SetPosition(x, y);
		var to_x = effect.to_x;
		var to_y = effect.to_y;
		// Respawn new clonk.
		var plr = target->GetOwner();
		var clonk = CreateObject(Clonk, 0, 0, plr);
		clonk->GrabObjectInfo(target);
		SetCursor(plr, clonk);
		clonk->DoEnergy(100000);
		restorer->SetRestoreObject(clonk, nil, to_x, to_y, 0, "ClonkRestore");
	}
	return 1;
}