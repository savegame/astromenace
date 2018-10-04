/************************************************************************************

	AstroMenace
	Hardcore 3D space scroll-shooter with spaceship upgrade possibilities.
	Copyright (c) 2006-2018 Mikhail Kurinnoi, Viewizard


	AstroMenace is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	AstroMenace is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with AstroMenace. If not, see <https://www.gnu.org/licenses/>.


	Website: https://viewizard.com/
	Project: https://github.com/viewizard/astromenace
	E-mail: viewizard@viewizard.com

*************************************************************************************/

// TODO translate comments

// NOTE should be tested with in-game resolution changes

#include "../core/core.h"
#include "../config/config.h"
#include "../assets/texture.h"
#include "../object3d/space_ship/space_ship.h"
#include "../game.h" // FIXME "game.h" should be replaced by individual headers

// NOTE switch to nested namespace definition (namespace A::B::C { ... }) (since C++17)
namespace viewizard {
namespace astromenace {

namespace {

constexpr int ProgressBarWidth{8};
constexpr int ProgressBarHeight{64};

constexpr int SlimBorder{2};
constexpr int SlimSeparator{2};

} // unnamed namespace

// FIXME should be fixed, don't allow global scope interaction for local variables
extern float CurrentAlert3;


/*
 * Calculate weapon ammo progress.
 *            -  |---|  -
 *            |  |   |  |
 *            |  |   |  |- should be calculated by this function
 *            |  |   |  |
 * BarHeight -|  |---|  -
 *            |  |---|
 *            |  |---|
 *            -  |---|
 */
static int WeaponAmmoProgress(std::shared_ptr<cWeapon> &sharedWeapon, int BarHeight)
{
	return (BarHeight * (sharedWeapon->AmmoStart - sharedWeapon->Ammo)) / sharedWeapon->AmmoStart;
}

/*
 * Calculate weapon reload progress.
 *            -  |---|  -
 *            |  |   |  |
 *            |  |   |  |- should be calculated by this function
 *            |  |   |  |
 * BarHeight -|  |---|  -
 *            |  |---|
 *            |  |---|
 *            -  |---|
 */
static int WeaponReloadProgress(std::shared_ptr<cWeapon> &sharedWeapon, float TimeLastUpdate, int BarHeight)
{
	int Reload = BarHeight -
		     static_cast<int>((BarHeight * (TimeLastUpdate - sharedWeapon->LastFireTime)) / sharedWeapon->NextFireTime);

	if ((sharedWeapon->InternalType == 17) && // swarm
	    (sharedWeapon->SwarmNum) > 0)
		Reload = 64;
	else if (Reload < 0)
		Reload = 0;

	return Reload;
}

/*
 * Draw slim weapon slot.
 */
static void DrawSlimWeaponSlot(int X, int Y, int AmmoOffsetX, int ReloadOffsetX,
			       int AmmoProgressBar, int ReloadProgressBar, bool WeaponDestroyed)
{
	sRECT SrcRect(0, 0, 2, 2);
	sRECT DstRect(X,
		      Y,
		      X + SlimBorder * 2 + SlimSeparator + ProgressBarWidth * 2,
		      Y + SlimBorder * 2 + ProgressBarHeight);
	GLtexture tmpBlackPoint = GetPreloadedTextureAsset("menu/blackpoint.tga");
	vw_Draw2D(DstRect, SrcRect, tmpBlackPoint, true, 0.2f);

	DstRect(X + SlimBorder + AmmoOffsetX,
		Y + SlimBorder,
		X + SlimBorder + AmmoOffsetX + ProgressBarWidth,
		Y + SlimBorder + ProgressBarHeight);
	vw_Draw2D(DstRect, SrcRect, tmpBlackPoint, true, 0.5f);

	DstRect(X + SlimBorder + ReloadOffsetX,
		Y + SlimBorder,
		X + SlimBorder + ReloadOffsetX + ProgressBarWidth,
		Y + SlimBorder + ProgressBarHeight);
	vw_Draw2D(DstRect, SrcRect, tmpBlackPoint, true, 0.5f);

	if (WeaponDestroyed)
		return;

	SrcRect(0, AmmoProgressBar, ProgressBarWidth, ProgressBarHeight);
	DstRect(X + SlimBorder + AmmoOffsetX,
		Y + SlimBorder + AmmoProgressBar,
		X + SlimBorder + AmmoOffsetX + ProgressBarWidth,
		Y + SlimBorder + ProgressBarHeight);
	vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("game/ammo.tga"), true, 1.0f);

	SrcRect(0, ReloadProgressBar, ProgressBarWidth, ProgressBarHeight);
	DstRect(X + SlimBorder + ReloadOffsetX,
		Y + SlimBorder + ReloadProgressBar,
		X + SlimBorder + ReloadOffsetX + ProgressBarWidth,
		Y + SlimBorder + ProgressBarHeight);
	vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("game/energy.tga"), true, 1.0f);
}

//------------------------------------------------------------------------------------
// Прорисовка левого слота
//------------------------------------------------------------------------------------
static void DrawGameWeaponLeftSlot(std::shared_ptr<cSpaceShip> &sharedSpaceShip,
				   std::shared_ptr<cWeapon> &sharedWeapon, int &DrawLevelPos)
{
	sRECT SrcRect, DstRect;

	int Xpos = 0;
	int Ypos = 150;
	switch (DrawLevelPos) {
	case 1:
		Ypos = 80;
		break;
	case 2:
		Ypos = 150;
		break;
	case 3:
		Ypos = 220;
		break;
	}

	if (GameConfig().GameWeaponInfoType == 1) {
		Ypos += DrawLevelPos * 15 - 25;

		// выводим подложку меню - общую
		SrcRect(0, 0, 164, 88);
		DstRect(Xpos, Ypos, Xpos+164, Ypos+88);
		vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("game/weapon_panel_left.tga"), true, 1.0f);


		// подложка-состояния
		SrcRect(0, 0, 128, 64);
		DstRect(Xpos+24, Ypos+12, Xpos+24+128, Ypos+64+12);
		// пушка работает или нет?
		if (sharedWeapon->ArmorCurrentStatus <= 0.0f) {
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/weapon_on_icon.tga"), true, CurrentAlert3, 0.0f, sRGBCOLOR{eRGBCOLOR::red});

			// иконка оружия
			SrcRect(0, 0, 128, 64);
			DstRect(Xpos + 24, Ypos + 12, Xpos + 24 + 128, Ypos + 64 + 12);
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(GetWeaponIconName(sharedWeapon->InternalType)), true, 1.0f);
		} else {
			if (sharedWeapon->CurrentEnergyAccumulated < sharedWeapon->EnergyUse)
				vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/weapon_on_icon.tga"), true, CurrentAlert3, 0.0f, sRGBCOLOR{0.0f, 1.0f, 1.0f});
			else {
				if (sharedWeapon->Ammo == 0)
					vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/weapon_on_icon.tga"), true, CurrentAlert3, 0.0f, sRGBCOLOR{1.0f, 0.5f, 0.2f});
				else
					vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/weapon_on_icon.tga"), true, 1.0f, 0.0f, sRGBCOLOR{eRGBCOLOR::green});
			}

			// иконка оружия
			SrcRect(0, 0, 128, 64);
			DstRect(Xpos+24, Ypos+12, Xpos+24+128, Ypos+64+12);
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(GetWeaponIconName(sharedWeapon->InternalType)), true, 1.0f);

			// боекомплект
			int AmmoShow = WeaponAmmoProgress(sharedWeapon, 56);

			SrcRect(0, AmmoShow, 8, 56);
			DstRect(Xpos+2, Ypos+16+AmmoShow, Xpos+8+2, Ypos+56+16);
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("game/weapon_ammo.tga"), true, 1.0f);

			// перезарядка
			int ReloadShow = WeaponReloadProgress(sharedWeapon, sharedSpaceShip->TimeLastUpdate, 56);

			SrcRect(0,ReloadShow,8,56);
			DstRect(Xpos+12,Ypos+16+ReloadShow,Xpos+12+8,Ypos+56+16);
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("game/weapon_energy.tga"), true, 1.0f);
		}
	}
	if (GameConfig().GameWeaponInfoType == 2) {
		// выводим подложку меню - общую
		SrcRect(0,0,2,2);
		DstRect(Xpos,Ypos,Xpos+128+18+6,Ypos+64+4);
		vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/blackpoint.tga"), true, 0.2f);

		// выводим подложку меню - под иконку
		SrcRect(0,0,2,2);
		DstRect(Xpos+23,Ypos+1,Xpos+23+128,Ypos+64+3);
		vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/blackpoint.tga"), true, 0.5f);

		// выводим подложку меню - под боекомплект
		SrcRect(0,0,2,2);
		DstRect(Xpos+1,Ypos+1,Xpos+8+3,Ypos+64+3);
		vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/blackpoint.tga"), true, 0.5f);

		// выводим подложку меню - под перезарядка
		SrcRect(0,0,2,2);
		DstRect(Xpos+12,Ypos+1,Xpos+12+8+2,Ypos+64+3);
		vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/blackpoint.tga"), true, 0.5f);

		// подложка-состояния
		SrcRect(0,0,128,64);
		DstRect(Xpos+23,Ypos+2,Xpos+23+128,Ypos+64+2);

		// пушка работает или нет?
		if (sharedWeapon->ArmorCurrentStatus <= 0.0f) {
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/weapon_on_icon.tga"), true, CurrentAlert3, 0.0f, sRGBCOLOR{eRGBCOLOR::red});

			// иконка оружия
			SrcRect(0,0,128,64);
			DstRect(Xpos+23,Ypos+2,Xpos+23+128,Ypos+64+2);
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(GetWeaponIconName(sharedWeapon->InternalType)), true, 1.0f);
		} else {
			if (sharedWeapon->CurrentEnergyAccumulated < sharedWeapon->EnergyUse)
				vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/weapon_on_icon.tga"), true, CurrentAlert3, 0.0f, sRGBCOLOR{0.0f, 1.0f, 1.0f});
			else {
				if (sharedWeapon->Ammo == 0)
					vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/weapon_on_icon.tga"), true, CurrentAlert3, 0.0f, sRGBCOLOR{1.0f, 0.5f, 0.2f});
				else
					vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/weapon_on_icon.tga"), true, 1.0f, 0.0f, sRGBCOLOR{eRGBCOLOR::green});
			}
			// иконка оружия
			SrcRect(0,0,128,64);
			DstRect(Xpos+23,Ypos+2,Xpos+23+128,Ypos+64+2);
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(GetWeaponIconName(sharedWeapon->InternalType)), true, 1.0f);

			// боекомплект
			int AmmoShow = WeaponAmmoProgress(sharedWeapon, ProgressBarHeight);

			SrcRect(0, AmmoShow, 8, 64);
			DstRect(Xpos+2, Ypos+2+AmmoShow, Xpos+8+2, Ypos+64+2);
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("game/ammo.tga"), true, 1.0f);

			// перезарядка
			int ReloadShow = WeaponReloadProgress(sharedWeapon, sharedSpaceShip->TimeLastUpdate, ProgressBarHeight);

			SrcRect(0,ReloadShow,8,64);
			DstRect(Xpos+12+1,Ypos+2+ReloadShow,Xpos+12+8+1,Ypos+64+2);
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("game/energy.tga"), true, 1.0f);
		}
	}
	if (GameConfig().GameWeaponInfoType == 3) {
		DrawSlimWeaponSlot(Xpos, Ypos, 0, ProgressBarWidth + SlimSeparator,
				   WeaponAmmoProgress(sharedWeapon, ProgressBarHeight),
				   WeaponReloadProgress(sharedWeapon, sharedSpaceShip->TimeLastUpdate, ProgressBarHeight),
				   (sharedWeapon->ArmorCurrentStatus <= 0.0f));
	}

	DrawLevelPos++;
}

//------------------------------------------------------------------------------------
// Прорисовка правого слота
//------------------------------------------------------------------------------------
static void DrawGameWeaponRightSlot(std::shared_ptr<cSpaceShip> &sharedSpaceShip,
				    std::shared_ptr<cWeapon> &sharedWeapon, int &DrawLevelPos)
{
	sRECT SrcRect, DstRect;

	int Xpos = GameConfig().InternalWidth - 152;
	int Ypos = 80;
	switch (DrawLevelPos) {
	case 1:
		Ypos = 80;
		break;
	case 2:
		Ypos = 150;
		break;
	case 3:
		Ypos = 220;
		break;
	}

	if (GameConfig().GameWeaponInfoType == 1) {
		Ypos += DrawLevelPos*15 - 25;
		Xpos = GameConfig().InternalWidth - 164;

		// выводим подложку меню - общую
		SrcRect(0,0,164,88);
		DstRect(Xpos,Ypos,Xpos+164,Ypos+88);
		vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("game/weapon_panel_right.tga"), true, 1.0f);

		// подложка-состояния
		SrcRect(0,0,128,64);
		DstRect(Xpos+12,Ypos+12,Xpos+12+128,Ypos+64+12);
		// пушка работает или нет?
		if (sharedWeapon->ArmorCurrentStatus <= 0.0f) {
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/weapon_on_icon.tga"), true, CurrentAlert3, 0.0f, sRGBCOLOR{eRGBCOLOR::red});

			// иконка оружия
			SrcRect(0,0,128,64);
			DstRect(Xpos+12,Ypos+12,Xpos+12+128,Ypos+64+12);
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(GetWeaponIconName(sharedWeapon->InternalType)), true, 1.0f);
		} else {
			if (sharedWeapon->CurrentEnergyAccumulated < sharedWeapon->EnergyUse)
				vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/weapon_on_icon.tga"), true, CurrentAlert3, 0.0f, sRGBCOLOR{0.0f, 1.0f, 1.0f});
			else {
				if (sharedWeapon->Ammo == 0)
					vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/weapon_on_icon.tga"), true, CurrentAlert3, 0.0f, sRGBCOLOR{1.0f, 0.5f, 0.2f});
				else
					vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/weapon_on_icon.tga"), true, 1.0f, 0.0f, sRGBCOLOR{eRGBCOLOR::green});
			}
			// иконка оружия
			SrcRect(0,0,128,64);
			DstRect(Xpos+12,Ypos+12,Xpos+12+128,Ypos+64+12);
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(GetWeaponIconName(sharedWeapon->InternalType)), true, 1.0f);

			// боекомплект
			int AmmoShow = WeaponAmmoProgress(sharedWeapon, 56);

			SrcRect(0,AmmoShow,8,56);
			DstRect(Xpos+154,Ypos+16+AmmoShow,Xpos+8+154,Ypos+56+16);
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("game/weapon_ammo.tga"), true, 1.0f);

			// перезарядка
			int ReloadShow = WeaponReloadProgress(sharedWeapon, sharedSpaceShip->TimeLastUpdate, 56);

			SrcRect(0,ReloadShow,8,56);
			DstRect(Xpos+144,Ypos+16+ReloadShow,Xpos+144+8,Ypos+56+16);
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("game/weapon_energy.tga"), true, 1.0f);
		}
	}
	if (GameConfig().GameWeaponInfoType == 2) {
		// выводим подложку меню - общую
		SrcRect(0,0,2,2);
		DstRect(Xpos,Ypos,Xpos+128+18+6,Ypos+64+4);
		vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/blackpoint.tga"), true, 0.2f);

		// выводим подложку меню - под иконку
		SrcRect(0,0,2,2);
		DstRect(Xpos+1,Ypos+1,Xpos+1+128,Ypos+64+3);
		vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/blackpoint.tga"), true, 0.5f);

		// выводим подложку меню - под боекомплект
		SrcRect(0,0,2,2);
		DstRect(Xpos+13+128,Ypos+1,Xpos+13+8+2+128,Ypos+64+3);
		vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/blackpoint.tga"), true, 0.5f);

		// выводим подложку меню - под перезарядка
		SrcRect(0,0,2,2);
		DstRect(Xpos+2+128,Ypos+1,Xpos+8+4+128,Ypos+64+3);
		vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/blackpoint.tga"), true, 0.5f);

		// подложка-состояния
		SrcRect(0,0,128,64);
		DstRect(Xpos+1,Ypos+2,Xpos+1+128,Ypos+64+2);
		// пушка работает или нет?
		if (sharedWeapon->ArmorCurrentStatus <= 0.0f) {
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/weapon_on_icon.tga"), true, CurrentAlert3, 0.0f, sRGBCOLOR{eRGBCOLOR::red});

			// иконка оружия
			SrcRect(0,0,128,64);
			DstRect(Xpos+1,Ypos+2,Xpos+1+128,Ypos+64+2);
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(GetWeaponIconName(sharedWeapon->InternalType)), true, 1.0f);
		} else {
			if (sharedWeapon->CurrentEnergyAccumulated < sharedWeapon->EnergyUse)
				vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/weapon_on_icon.tga"), true, CurrentAlert3, 0.0f, sRGBCOLOR{0.0f, 1.0f, 1.0f});
			else {
				if (sharedWeapon->Ammo == 0)
					vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/weapon_on_icon.tga"), true, CurrentAlert3, 0.0f, sRGBCOLOR{1.0f, 0.5f, 0.2f});
				else
					vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("menu/weapon_on_icon.tga"), true, 1.0f, 0.0f, sRGBCOLOR{eRGBCOLOR::green});
			}
			// иконка оружия
			SrcRect(0,0,128,64);
			DstRect(Xpos+1,Ypos+2,Xpos+1+128,Ypos+64+2);
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(GetWeaponIconName(sharedWeapon->InternalType)), true, 1.0f);

			// боекомплект
			int AmmoShow = WeaponAmmoProgress(sharedWeapon, ProgressBarHeight);

			SrcRect(0,AmmoShow,8,64);
			DstRect(Xpos+12+2+128,Ypos+2+AmmoShow,Xpos+12+8+2+128,Ypos+64+2);
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("game/ammo.tga"), true, 1.0f);

			// перезарядка
			int ReloadShow = WeaponReloadProgress(sharedWeapon, sharedSpaceShip->TimeLastUpdate, ProgressBarHeight);

			SrcRect(0,ReloadShow,8,64);
			DstRect(Xpos+3+128,Ypos+2+ReloadShow,Xpos+8+3+128,Ypos+64+2);
			vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("game/energy.tga"), true, 1.0f);
		}
	}
	if (GameConfig().GameWeaponInfoType == 3) {
		Xpos = GameConfig().InternalWidth - (ProgressBarWidth * 2 + SlimSeparator + SlimBorder * 2);

		DrawSlimWeaponSlot(Xpos, Ypos, ProgressBarWidth + SlimSeparator, 0,
				   WeaponAmmoProgress(sharedWeapon, ProgressBarHeight),
				   WeaponReloadProgress(sharedWeapon, sharedSpaceShip->TimeLastUpdate, ProgressBarHeight),
				   (sharedWeapon->ArmorCurrentStatus <= 0.0f));
	}

	DrawLevelPos++;
}

/*
 * Draw weapon slots in-game (HUD).
 */
void DrawGameWeaponSlots(std::weak_ptr<cSpaceShip> &SpaceShip)
{
	auto sharedSpaceShip = SpaceShip.lock();
	if (!sharedSpaceShip)
		return;

	// FIXME move this calculation to the "weapon slots" init, should be called one time only
	float tmpInvMatrix[9];
	memcpy(tmpInvMatrix,
	       sharedSpaceShip->CurrentRotationMat,
	       9 * sizeof(sharedSpaceShip->CurrentRotationMat[0]));
	vw_Matrix33InverseRotate(tmpInvMatrix);

	int RightDrawLevelPos{1};
	int LeftDrawLevelPos{1};
	for (auto &tmpWeaponSlot : sharedSpaceShip->WeaponSlots) {
		// revert back rotation + ship shaking effect, since we need initial weapon slot location
		// FIXME move this calculation to the "weapon slots" init, should be called one time only
		sVECTOR3D tmpInitialLocation = tmpWeaponSlot.Location;
		vw_Matrix33CalcPoint(tmpInitialLocation, tmpInvMatrix);

		if (auto sharedWeapon = tmpWeaponSlot.Weapon.lock()) {
			if (tmpInitialLocation.x > -0.01f) // denote a small quantity, which will be taken to zero
				DrawGameWeaponLeftSlot(sharedSpaceShip, sharedWeapon, LeftDrawLevelPos);
			else
				DrawGameWeaponRightSlot(sharedSpaceShip, sharedWeapon, RightDrawLevelPos);
		}
	}
}

} // astromenace namespace
} // viewizard namespace
