/* Switches */

static g_was_player_init;

func Initialize()
{
	// Environment
	SetSkyParallax(1, 20,20, 0,0, nil, nil);
	return true;
}

func InitializePlayer(int plr)
{
	var i;
	// First player init base
	if (!g_was_player_init)
	{
		InitBase(plr);
		g_was_player_init = true;
	}
	// Position and materials
	var crew;
	for (i=0; crew=GetCrew(plr,i); ++i)
	{
		crew->SetPosition(2100+Random(40), 233-10);
		crew->CreateContents(Shovel);
	}
	return true;
}


private func InitBase(int owner)
{
	// Create standard base owned by player
	var y=232;
	var lorry = CreateObjectAbove(Lorry, 2040,y-2, owner);
	if (lorry)
	{
		lorry->CreateContents(Loam, 6);
		lorry->CreateContents(Wood, 15);
		lorry->CreateContents(Metal, 4);
		lorry->CreateContents(Axe, 1);
		lorry->CreateContents(Pickaxe, 1);
		lorry->CreateContents(Hammer, 1);
		lorry->CreateContents(Dynamite, 2);
		lorry->CreateContents(Pipe, 10);
	}
	CreateObjectAbove(Pump, 2062, y, owner);
	CreateObjectAbove(Flagpole, 2085, y, owner);
	CreateObjectAbove(WindGenerator, 2110, y, owner);
	CreateObjectAbove(ToolsWorkshop, 2150, y, owner);
	CreateObjectAbove(WindGenerator, 2200, y, owner);
	CreateObjectAbove(WoodenCabin, 2250, y, owner);
	
	CreateObjectAbove(Foundry, 1793, y, owner);
	CreateObjectAbove(Flagpole, 1819, y, owner);
	CreateObjectAbove(Sawmill, 1845, y, owner);
	
	CreateObject(DynamiteBox, 1705, 230, owner);
	CreateObject(ElectronicsStation, 1772, 226, owner);
	CreateObject(ElectronicsSwitch, 2183, 227, owner);
	return true;
}
