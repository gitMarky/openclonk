func Initialize()
{
	var bed = CreateObjectAbove(Bed, 185, 185, NO_OWNER);
	bed->~SuperCoolExtraFunc1();
	bed->~SuperCoolExtraFunc2();
	Log("%v", bed->~IsFree());
}
