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
