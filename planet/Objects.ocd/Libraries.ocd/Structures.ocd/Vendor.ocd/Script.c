/**
	Structure Vendor
	Basic library for structures, handles:
	* Selling objects in interaction menu
	
	@author Randrian, Marky (buy and sell logic), TODO (menu)
*/

local lib_vendor = {}; // proplist that avoids clashes in variables


// ----------------- Settings for the trading of objects ----------------
// --- these functions can be overloaded for vendors or special bases ---

// ----- Buying

func GetBuyValue(id def)
{
	// By default call the engine function
	return def->GetValue();
}

func GetBuyableItems(int iPlr)
{
	// By default get the base material
	var i, item;
	var items = [];
	while (item = GetBaseMaterial(GetOwner(), nil, i++))
	{
		PushBack(items, item);
	}
	return items;
}

func GetBuyableAmount(int iPayPlr, id idDef)
{
	// by default use the base material
	return GetBaseMaterial(iPayPlr, idDef);
}

func ChangeBuyableAmount(int iPayPlr, id idDef, int amount)
{
	// by default use base engine function
	DoBaseMaterial(iPayPlr, idDef, amount);
}

// ----- Selling

func GetSellableItems(object container)
{
	var items = []; // TODO
	return items;
}

func GetSellableAmount(int iPayPlr, id idDef)
{
	return 0; // TODO
}

// returns the value of the object if sold in this base
func GetSellValue(object pObj)
{
	// By default call the engine function
	return pObj->GetValue();
}

// ----- Menu entries

func Definition(id def)
{
	_inherited(...);
	
	// default design of a control menu item
	if (def.lib_vendor == nil) def.lib_vendor = {};
	def.lib_vendor.custom_entry = 
	{
		Right = "4em", Bottom = "2em",
		BackgroundColor = {Std = 0, OnHover = 0x50ff0000},
		image = {Right = "2em", Style = GUI_TextBottom | GUI_TextRight},
		price = {Left = "2em", Priority = 3}
	};
	
}


// -------------------------- Internal functions --------------------------
// --- these functions should not be overloaded,
// --- but offer more interfaces if necessary

// ------------------------ Buying -------------------------------------


func DoBuy(id idDef, int iForPlr, int iPayPlr, object pClonk, bool bRight, bool fShowErrors)
{
	// Tries to buy an object or all available objects for bRight == true
	// Returns the last bought object
	var num_available = GetBuyableAmount(iPayPlr, idDef);
	if(!num_available) return; //TODO
	var num_buy = 1, pObj = nil;
	if (bRight) num_buy = num_available;
	while (num_buy--)
	{
		var iValue = GetBuyValue(idDef);
		// Does the player have enough money?
		if(iValue > GetWealth(iPayPlr))
		{
			// TODO: get an errorsound
			if(fShowErrors)
			{
				Sound("Error", 0, 100, iForPlr+1);
				PlayerMessage(iForPlr, "$TxtNotEnoughtMoney$");
			}
			break;
		}
		// Take the cash
		DoWealth(iPayPlr, -iValue);
		Sound("UnCash", 0, 100, iForPlr+1); // TODO: get sound
		// Decrease the Basematerial
		ChangeBuyableAmount(iPayPlr, idDef, -1);
		// Deliver the object
		var pObj = CreateContents(idDef);
		pObj->SetOwner(iForPlr);
		if(pObj->GetOCF() & OCF_CrewMember) pObj->MakeCrewMember(iForPlr);
		if(pObj->GetOCF() & OCF_Collectible) pClonk->Collect(pObj);
	}
	return pObj;
}

// -------------------------- Selling -------------------------------------

func DoSell(object obj, int wealth_player)
{
	if (obj->~QueryOnSell(wealth_player)) return;

	// Sell contents first
	for(var contents in FindObjects(Find_Container(obj)))
	{
		DoSell(contents, wealth_player);
	}

	// Give the player the cash
	DoWealth(wealth_player, GetSellValue(obj));
	Sound("UI::Cash", nil, nil, wealth_player + 1);

	// OnSale callback to object e.g. for goal updates
	obj->~OnSale(wealth_player, this);

	// Remove object, but eject contents
	// ejecting contents may be important, because those contents
	// that return true in QueryOnSell are still in the object
	// and they should not be removed (why else would they have QueryOnSell)?
	if (obj) obj->RemoveObject(true);
	return true;
}

func GetSellableContents()
{
	return FindObjects(Find_Container(this), Find_Or(Find_Category(C4D_Object), Find_Category(C4D_Vehicle), Find_Category(65536/*C4D_TradeLiving*/)));
}


func CanStack(object pFirst, object pSecond)
{
	// Test if these Objects differ from each other
	if(!pFirst->CanConcatPictureWith(pSecond)) return false;
	if(GetSellValue(pFirst) != GetSellValue(pSecond)) return false;
	
	// ok they can be stacked
	return true;
}

// -------------------------- Vendor functionality -------------------------------------

func IsVendor()
{
	return lib_vendor.is_vendor;
}

// Makes this building a vendor or removes the base functionallity
public func MakeVendor(bool should_be_vendor)
{
	if (should_be_vendor)
	{
		if (!lib_vendor.is_vendor)
			lib_vendor.is_vendor = AddEffect("IntVendor", this, 1, 10, this);
	}
	else
	{
		if (lib_vendor.is_vendor)
			RemoveEffect(nil, nil, lib_vendor.vendor);

		lib_vendor.is_vendor = nil;
	}
}

func FxIntVendorTimer(object target, proplist effect, int time)
{
	// Search all objects for objects that want to be sold automatically
	for (var item in FindObjects(Find_Container(this), Find_Func("AutoSell")))
		Sell(item->GetOwner(), item, this);
}

// -------------------------- Menus -------------------------------------

// ----- generic things

// Provides an interaction menu for buying things.
public func HasInteractionMenu() { return true; }

// Interface for custom buy conditions
public func AllowBuyMenuEntries(){ return ObjectCount(Find_ID(Rule_BuyAtFlagpole));}

// Interface for custom sell conditions
public func AllowSellMenuEntries(){ return AllowBuyMenuEntries(); }


public func GetInteractionMenus(object clonk)
{
	var menus = _inherited() ?? [];
	// only open the menus if ready
	if (AllowBuyMenuEntries())
	{
		var buy_menu =
		{
			title = "$MsgBuy$",
			entries_callback = this.GetBuyMenuEntries,
			callback = "OnBuyMenuSelection",
			callback_target = this,
			BackgroundColor = RGB(50, 50, 0),
			Priority = 20
		};
		PushBack(menus, buy_menu);
	}
	
	if (AllowSellMenuEntries())
	{
		var sell_menu =
		{
			title = "$MsgSell$",
			entries_callback = this.GetSellMenuEntries,
			callback = "OnSellMenuSelection",
			callback_target = this,
			BackgroundColor = RGB(50, 50, 0),
			Priority = 10
		};
		PushBack(menus, sell_menu);
	}
	
	return menus;
}

func GetBuyOrSellMenuEntry(int index, object item, int amount, int value)
{
	var entry = 
	{
		Prototype = lib_vendor.custom_entry,
		image = {Prototype = lib_vendor.custom_entry.image},
		price = {Prototype = lib_vendor.custom_entry.price}
	};
	entry.image.Symbol = item;
	entry.image.Text = Format("%dx", amount);
	entry.price.Text = Format("<c ffff00>%d{{Icon_Wealth}}</c>", value);
	entry.Priority = 1000 * value + index; // Order by value and then by BaseMaterial index.

	return entry;
}

// ----- buying

public func GetBuyMenuEntries(object clonk)
{	
	// We need to know when exactly we should refresh the menu to prevent unecessary refreshs.
	var lowest_greyed_out_price = nil;

	// distinguish owners here. at the moment they are the same, but this may change
	var wealth_player = GetOwner();
	var material_player = GetOwner();

	var wealth = GetWealth(wealth_player); 
	var menu_entries = [];
	var i = 0, item, amount;
	
	for (item in GetBuyableItems(material_player))
	{
		amount = GetBuyableAmount(material_player, item);
		var value = GetBuyValue(item);
		var entry = GetBuyOrSellMenuEntry(i, item, amount, value);
		if (value > wealth) // If the player can't afford it, the item (except for the price) is overlayed by a greyish color.
		{
			entry.overlay = {Priority = 2, BackgroundColor = RGBa(50, 50, 50, 150)};
			if (lowest_greyed_out_price == nil || value < lowest_greyed_out_price)
				lowest_greyed_out_price = value;
		}
		PushBack(menu_entries, {symbol = item, extra_data = nil, custom = entry});
	}
	
	// At the top of the menu, we add the player's wealth.
	var entry = 
	{
		Bottom = "1.1em",
		BackgroundColor = RGBa(100, 100, 50, 100),
		Priority = -1,
		left_text =
		{
			Style = GUI_TextVCenter | GUI_TextLeft,
			Text = "<c 888888>$YourWealth$:</c>"
		},
		right_text = 
		{
			Style = GUI_TextVCenter | GUI_TextRight, 
			Text = Format("<c ffff00>%d{{Icon_Wealth}}</c>", wealth)
		}
	};
	var fx = AddEffect("UpdateWealthDisplay", this, 1, 5, nil, GetID());
	fx.lowest_greyed_out_price = lowest_greyed_out_price;
	fx.last_wealth = wealth;
	fx.plr = wealth_player;
	PushBack(menu_entries, {symbol = nil, extra_data = nil, custom = entry, fx = fx});

	return menu_entries;
}

public func OnBuyMenuSelection(id def, extra_data, object clonk)
{
	// distinguish owners here. at the moment they are the same, but this may change
	var wealth_player = GetOwner();
	var material_player = clonk->GetController();
	// Buy
	DoBuy(def, material_player, wealth_player, clonk);
	// Excess objects exit flag (can't get them out...)
	var i = ContentsCount();
	var obj;
	while (i--) 
		if (obj = Contents(i))
		{
			obj->Exit(0, GetDefHeight() / 2);
			// newly bought items do not fade out until they've been collected once
			if (obj && ObjectCount(Find_ID(Rule_ObjectFade)) && !obj.HasNoFadeOut)
			{
				obj.HasNoFadeOut = this.BuyItem_HasNoFadeout;
				obj.BuyOverload_Entrance = obj.Entrance;
				obj.Entrance = this.BuyItem_Entrance;
			}
		}
	UpdateInteractionMenus(this.GetBuyMenuEntries);
}


// ----- Selling

public func GetSellMenuEntries(object clonk)
{	
	var menu_entries = [];
	var i = 0, item, amount;
	
	for (item in GetSellableItems(clonk))
	{
		amount = GetSellableAmount(clonk, item);
		var value = GetSellValue(item);
		var entry = GetBuyOrSellMenuEntry(i, item, amount, value);
		PushBack(menu_entries, {symbol = item, extra_data = nil, custom = entry});
	}
	
	PushBack(menu_entries, {symbol = nil, extra_data = nil, custom = entry});

	return menu_entries;
}

public func OnSellMenuSelection(object item, extra_data, object clonk)
{
	// distinguish owners here. at the moment they are the same, but this may change
	var wealth_player = clonk->GetController();
	// Buy
	DoSell(item, wealth_player);
	UpdateInteractionMenus(this.GetSellMenuEntries);
}

// ----- Menu updates, misc

private func FxUpdateWealthDisplayTimer(object target, effect fx, int time)
{
	if (!fx.menu_target) return -1;
	if (fx.last_wealth == GetWealth(fx.wealth_player)) return FX_OK;
	fx.last_wealth = GetWealth(fx.wealth_player);
	// Do we need a full refresh? New objects might have become available.
	if (fx.lowest_greyed_out_price && fx.lowest_greyed_out_price <= fx.last_wealth)
	{
		target->UpdateInteractionMenus(target.GetBuyMenuEntries);
		return FX_OK;
	}
	// Just update the money display otherwise.
	GuiUpdate({right_text = {Text = Format("<c ffff00>%d{{Icon_Wealth}}</c>", fx.last_wealth)}}, fx.main_ID, fx.ID, fx.menu_target);
	return FX_OK;
}

public func FxUpdateWealthDisplayOnMenuOpened(object target, effect fx, int main_ID, int ID, object subwindow_target)
{
	fx.main_ID = main_ID;
	fx.menu_target = subwindow_target;
	fx.ID = ID;
}

// newly bought items do not fade out unless collected
func BuyItem_HasNoFadeout() { return true; }

func BuyItem_Entrance()
{
	// after first collection, fade out rule should be effective again
	var overloaded_fn = this.BuyOverload_Entrance;
	this.HasNoFadeOut = nil;
	this.BuyOverload_Entranc = nil;
	this.Entrance = overloaded_fn;
	if (overloaded_fn) return Call(overloaded_fn, ...);
}
