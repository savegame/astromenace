/************************************************************************************

	AstroMenace (Hardcore 3D space shooter with spaceship upgrade possibilities)
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
	along with AstroMenace. If not, see <http://www.gnu.org/licenses/>.


	Web Site: http://www.viewizard.com/
	Project: https://github.com/viewizard/astromenace
	E-mail: viewizard@viewizard.com

*************************************************************************************/


#include "../game.h"


// временные данные для изменения и восстановления
int Options_TexturesCompressionType;
int Options_UseGLSL;
int Options_MSAA;
int Options_CSAA;
int Options_ShadowMap;
int Options_TexturesQuality;

extern cSpaceStars *psSpaceStatic;

const char *ButtonQuality[3] = {
	"3_High",
	"3_Medium",
	"3_Low"
};

const char *ShadowButtonQuality[10] = {
	"1_Off",
	"3_Low",
	"3_Low",
	"3_Low",
	"3_Medium",
	"3_Medium",
	"3_Medium",
	"3_High",
	"3_High",
	"3_High"
};

const char *ShadowButtonQualityBase[10] = {
	"%s",
	"%s, 2x2 PCF",
	"%s, 4x4 PCF",
	"%s, 8x8 PCF",
	"%s, 2x2 PCF",
	"%s, 4x4 PCF",
	"%s, 8x8 PCF",
	"%s, 2x2 PCF",
	"%s, 4x4 PCF",
	"%s, 8x8 PCF"
};


const char *ButtonTile[3] = {
	"3_Disabled",
	"3_1_Layer",
	"3_2_Layers"
};


const char *ButtonTextFiltr[2] = {
	"3_Bilinear",
	"3_Trilinear"
};


const char *ButtonTextCompression[3] = {
	"1_Off",
	"3_S3TC",
	"3_BPTC"
};


const char *ButtonPointLights[7] = {
	"3_Disabled",
	"3_1_Light",
	"3_2_Lights",
	"3_3_Lights",
	"3_4_Lights",
	"3_5_Lights",
	"3_6_Lights"
};


const char *ButtonTexturesQuality[3] = {
	"3_Low",
	"3_Medium",
	"3_High"
};






void OptionsAdvMenu(float ContentTransp, float *ButtonTransp1, float *LastButtonUpdateTime1, float *ButtonTransp2, float *LastButtonUpdateTime2)
{


	sRECT SrcRect, DstRect;
	SetRect(&SrcRect,0,0,2,2);
	SetRect(&DstRect,0,0,Setup.iAspectRatioWidth,768);
	vw_DrawTransparent(&DstRect, &SrcRect, vw_FindTextureByName("menu/blackpoint.tga"), true, 0.5f*ContentTransp);







	int X1 = Setup.iAspectRatioWidth/2 - 375;
	int Y1 = 65;
	int Prir1 = 55;




	// качество визуальных эффектов
	vw_DrawFont(X1, Y1, -280, 0, 1.0f, 0.0f,1.0f,0.0f, ContentTransp, vw_GetText("3_Visual_Effects_Quality"));
	if (DrawButton128_2(X1+300, Y1-6, vw_GetText("1_Prev"), ContentTransp, Setup.VisualEffectsQuality==2)) {
		Setup.VisualEffectsQuality++;
		if (Setup.VisualEffectsQuality > 2) Setup.VisualEffectsQuality = 0;

		vw_InitParticleSystems(Setup.UseGLSL, Setup.VisualEffectsQuality+1.0f);

		if (psSpaceStatic != nullptr) {
			delete psSpaceStatic;
			psSpaceStatic = nullptr;
		}
		psSpaceStatic = new cSpaceStars;
	}
	if (DrawButton128_2(X1+616, Y1-6, vw_GetText("1_Next"), ContentTransp, Setup.VisualEffectsQuality==0)) {
		Setup.VisualEffectsQuality--;
		if (Setup.VisualEffectsQuality < 0) Setup.VisualEffectsQuality = 2;

		vw_InitParticleSystems(Setup.UseGLSL, Setup.VisualEffectsQuality+1.0f);

		if (psSpaceStatic != nullptr) {
			delete psSpaceStatic;
			psSpaceStatic = nullptr;
		}
		psSpaceStatic = new cSpaceStars;
	}
	int Size = vw_FontSize(vw_GetText(ButtonQuality[Setup.VisualEffectsQuality]));
	int SizeI = (170-Size)/2;//High, Medium, Low
	vw_DrawFont(X1+438+SizeI, Y1, 0, 0, 1.0f, 1.0f,1.0f,1.0f, ContentTransp, vw_GetText(ButtonQuality[Setup.VisualEffectsQuality]));







	// Максимальное кол-во источников света на 1 объекта
	Y1 += Prir1;
	vw_DrawFont(X1, Y1, -280, 0, 1.0f, 0.0f,1.0f,0.0f, ContentTransp, vw_GetText("3_Point_Lights_per_Object"));
	if (DrawButton128_2(X1+300, Y1-6, vw_GetText("1_Prev"), ContentTransp, Setup.MaxPointLights==0)) {
		Setup.MaxPointLights--;
		if (Setup.MaxPointLights < 0) Setup.MaxPointLights = 0;
	}
	if (DrawButton128_2(X1+616, Y1-6, vw_GetText("1_Next"), ContentTransp, Setup.MaxPointLights==6)) {
		Setup.MaxPointLights++;
		if (Setup.MaxPointLights > 6) Setup.MaxPointLights = 6;
	}
	Size = vw_FontSize(vw_GetText(ButtonPointLights[Setup.MaxPointLights]));
	SizeI = (170-Size)/2;
	vw_DrawFont(X1+438+SizeI, Y1, 0, 0, 1.0f, 1.0f,1.0f,1.0f, ContentTransp, vw_GetText(ButtonPointLights[Setup.MaxPointLights]));







	// качество текстур
	Y1 += Prir1;
	vw_DrawFont(X1, Y1, -280, 0, 1.0f, 0.0f,1.0f,0.0f, ContentTransp, vw_GetText("3_Textures_Quality"));
	if (DrawButton128_2(X1+300, Y1-6, vw_GetText("1_Prev"), ContentTransp, Options_TexturesQuality==1)) {
		Options_TexturesQuality--;
		if (Options_TexturesQuality < 1) Options_TexturesQuality = 3;
	}
	if (DrawButton128_2(X1+616, Y1-6, vw_GetText("1_Next"), ContentTransp, Options_TexturesQuality==3)) {
		Options_TexturesQuality++;
		if (Options_TexturesQuality > 3) Options_TexturesQuality = 1;
	}
	Size = vw_FontSize(vw_GetText(ButtonTexturesQuality[Options_TexturesQuality-1]));
	SizeI = (170-Size)/2;//High, Medium, Low
	vw_DrawFont(X1+438+SizeI, Y1, 0, 0, 1.0f, 1.0f,1.0f,1.0f, ContentTransp, vw_GetText(ButtonTexturesQuality[Options_TexturesQuality-1]));







	// тип фильтрации текстуры
	Y1 += Prir1;
	vw_DrawFont(X1, Y1, -280, 0, 1.0f, 0.0f,1.0f,0.0f, ContentTransp, vw_GetText("3_Texture_Filtering_Mode"));
	if (DrawButton128_2(X1+300, Y1-6, vw_GetText("1_Prev"), ContentTransp, Setup.TextureFilteringMode==1)) {
		Setup.TextureFilteringMode--;
		if (Setup.TextureFilteringMode < 1) Setup.TextureFilteringMode = 2;
	}
	if (DrawButton128_2(X1+616, Y1-6, vw_GetText("1_Next"), ContentTransp, Setup.TextureFilteringMode==2)) {
		Setup.TextureFilteringMode++;
		if (Setup.TextureFilteringMode > 2) Setup.TextureFilteringMode = 1;
	}
	Size = vw_FontSize(vw_GetText(ButtonTextFiltr[Setup.TextureFilteringMode-1]));
	SizeI = (170-Size)/2;//Bilinear, Trilinear
	vw_DrawFont(X1+438+SizeI, Y1, 0, 0, 1.0f, 1.0f,1.0f,1.0f, ContentTransp, vw_GetText(ButtonTextFiltr[Setup.TextureFilteringMode-1]));







	// анизотропия
	Y1 += Prir1;
	vw_DrawFont(X1, Y1, -280, 0, 1.0f, 0.0f,1.0f,0.0f, ContentTransp, vw_GetText("3_Anisotropy_Level"));
	if (DrawButton128_2(X1+300, Y1-6, vw_GetText("1_Prev"), ContentTransp, Setup.AnisotropyLevel==1)) {
		Setup.AnisotropyLevel = (int)(Setup.AnisotropyLevel/2);
		if (Setup.AnisotropyLevel < 1) Setup.AnisotropyLevel = 1;
	}
	if (DrawButton128_2(X1+616, Y1-6, vw_GetText("1_Next"), ContentTransp, Setup.AnisotropyLevel==vw_GetDevCaps()->MaxAnisotropyLevel)) {
		Setup.AnisotropyLevel = Setup.AnisotropyLevel*2;
		if (Setup.AnisotropyLevel > vw_GetDevCaps()->MaxAnisotropyLevel) Setup.AnisotropyLevel = vw_GetDevCaps()->MaxAnisotropyLevel;
	}
	if (Setup.AnisotropyLevel>1) {
		Size = vw_FontSize("x%i", Setup.AnisotropyLevel);
		SizeI = (170-Size)/2;
		vw_DrawFont(X1+438+SizeI, Y1, 0, 0, 1.0f, 1.0f,1.0f,1.0f, ContentTransp, "x%i",Setup.AnisotropyLevel);
	} else {
		if (vw_GetDevCaps()->MaxAnisotropyLevel > 1) {
			Size = vw_FontSize(vw_GetText("1_Off"));
			SizeI = (170-Size)/2;
			vw_DrawFont(X1+438+SizeI, Y1, 0, 0, 1.0f, 1.0f,1.0f,1.0f, ContentTransp, vw_GetText("1_Off"));
		} else {
			Size = vw_FontSize(vw_GetText("3_Not_available"));
			SizeI = (170-Size)/2;
			vw_DrawFont(X1+438+SizeI, Y1, 0, 0, 1.0f, 1.0f,0.5f,0.0f, ContentTransp, vw_GetText("3_Not_available"));
		}
	}




	// вкл-выкл компрессии текстур
	Y1 += Prir1;
	vw_DrawFont(X1, Y1, -280, 0, 1.0f, 0.0f,1.0f,0.0f, ContentTransp, vw_GetText("3_Textures_Compression"));
	int MaxCompressionCount = 1;
	if (vw_GetDevCaps()->TexturesCompressionBPTC) MaxCompressionCount = 2;
	if (DrawButton128_2(X1+300, Y1-6, vw_GetText("1_Prev"), ContentTransp, !vw_GetDevCaps()->TexturesCompression || (Options_TexturesCompressionType <= 0))) {
		Options_TexturesCompressionType--;
		if (Options_TexturesCompressionType < 0) Options_TexturesCompressionType = 0;
	}
	if (DrawButton128_2(X1+616, Y1-6, vw_GetText("1_Next"), ContentTransp, !vw_GetDevCaps()->TexturesCompression || (Options_TexturesCompressionType >= MaxCompressionCount))) {
		Options_TexturesCompressionType++;
		if (Options_TexturesCompressionType > MaxCompressionCount) Options_TexturesCompressionType = MaxCompressionCount;
	}
	Size = vw_FontSize(vw_GetText(ButtonTextCompression[Options_TexturesCompressionType]));
	SizeI = (170-Size)/2;
	vw_DrawFont(X1+438+SizeI, Y1, 0, 0, 1.0f, 1.0f,1.0f,1.0f, ContentTransp, vw_GetText(ButtonTextCompression[Options_TexturesCompressionType]));





	//Options_MultiSampleType
	Y1 += Prir1;
	vw_DrawFont(X1, Y1, -280, 0, 1.0f, 1.0f,0.5f,0.0f, ContentTransp, vw_GetText("3_Multisample_Antialiasing"));
	if (DrawButton128_2(X1+300, Y1-6, vw_GetText("1_Prev"), ContentTransp, (Options_MSAA == 0) || (vw_GetDevCaps()->MaxSamples == 0))) {
		// находим текущий режим
		int CurrentMode = 0;
		if (Options_MSAA == 0) CurrentMode = -1;
		else {
			for (int i=0; i<vw_GetDevCaps()->MaxMultisampleCoverageModes; i++) {
				if ((vw_GetDevCaps()->MultisampleCoverageModes[i].ColorSamples == Options_MSAA) &
				    (vw_GetDevCaps()->MultisampleCoverageModes[i].CoverageSamples == Options_CSAA)) {
					CurrentMode = i;
					break;
				}
			}
		}

		CurrentMode --;
		if (CurrentMode < -1) CurrentMode = vw_GetDevCaps()->MaxMultisampleCoverageModes-1;

		// -1 - делаем "выключение" антиалиасинга
		if (CurrentMode == -1) {
			Options_MSAA = 0;
			Options_CSAA = 0;
		} else {
			Options_MSAA = vw_GetDevCaps()->MultisampleCoverageModes[CurrentMode].ColorSamples;
			Options_CSAA = vw_GetDevCaps()->MultisampleCoverageModes[CurrentMode].CoverageSamples;
		}
	}
	bool TestStateButton = false;
	if (vw_GetDevCaps()->MaxMultisampleCoverageModes > 0) TestStateButton = (vw_GetDevCaps()->MultisampleCoverageModes[vw_GetDevCaps()->MaxMultisampleCoverageModes-1].ColorSamples == Options_MSAA) &
				(vw_GetDevCaps()->MultisampleCoverageModes[vw_GetDevCaps()->MaxMultisampleCoverageModes-1].CoverageSamples == Options_CSAA);
	if (DrawButton128_2(X1+616, Y1-6, vw_GetText("1_Next"), ContentTransp, TestStateButton || (vw_GetDevCaps()->MaxSamples == 0))) {
		// находим текущий режим
		int CurrentMode = 0;
		if (Options_MSAA == 0) CurrentMode = -1;
		else {
			for (int i=0; i<vw_GetDevCaps()->MaxMultisampleCoverageModes; i++) {
				if ((vw_GetDevCaps()->MultisampleCoverageModes[i].ColorSamples == Options_MSAA) &
				    (vw_GetDevCaps()->MultisampleCoverageModes[i].CoverageSamples == Options_CSAA)) {
					CurrentMode = i;
					break;
				}
			}
		}

		CurrentMode ++;
		if (CurrentMode > vw_GetDevCaps()->MaxMultisampleCoverageModes-1) CurrentMode = -1;

		// -1 - делаем "выключение" антиалиасинга
		if (CurrentMode == -1) {
			Options_MSAA = 0;
			Options_CSAA = 0;
		} else {
			Options_MSAA = vw_GetDevCaps()->MultisampleCoverageModes[CurrentMode].ColorSamples;
			Options_CSAA = vw_GetDevCaps()->MultisampleCoverageModes[CurrentMode].CoverageSamples;
		}
	}
	if (Options_MSAA == 0) {
		if (vw_GetDevCaps()->MaxSamples == 0) {
			Size = vw_FontSize(vw_GetText("3_Not_available"));
			SizeI = (170-Size)/2;
			vw_DrawFont(X1+438+SizeI, Y1, 0, 0, 1.0f, 1.0f,0.5f,0.0f, ContentTransp, vw_GetText("3_Not_available"));
		} else {
			Size = vw_FontSize(vw_GetText("1_Off"));
			SizeI = (170-Size)/2;
			vw_DrawFont(X1+438+SizeI, Y1, 0, 0, 1.0f, 1.0f,1.0f,1.0f, ContentTransp, vw_GetText("1_Off"));
		}
	} else {
		if ((Options_MSAA == Options_CSAA) || (Options_CSAA == 0)) {
			Size = vw_FontSize("%ix MS",Options_MSAA);
			SizeI = (170-Size)/2;//Off, 2x, 4x ...
			vw_DrawFont(X1+438+SizeI, Y1, 0, 0, 1.0f, 1.0f,1.0f,1.0f, ContentTransp, "%ix MS", Options_MSAA);
		} else {
			Size = vw_FontSize("%ix CS/%ix MS",Options_CSAA,Options_MSAA);
			SizeI = (170-Size)/2;//Off, 2x, 4x ...
			vw_DrawFont(X1+438+SizeI, Y1, 0, 0, 1.0f, 1.0f,1.0f,1.0f, ContentTransp, "%ix CS/%ix MS",Options_CSAA,Options_MSAA);
		}
	}




	// вкл-выкл шейдеров, если они поддерживаются
	Y1 += Prir1;
	vw_DrawFont(X1, Y1, -280, 0, 1.0f, 1.0f,0.0f,0.0f, ContentTransp, vw_GetText("3_OpenGL_Shading_Language"));
	if (DrawButton128_2(X1+300, Y1-6, vw_GetText("1_Prev"), ContentTransp, !vw_GetDevCaps()->GLSL100Supported || vw_GetDevCaps()->ShaderModel < 3.0) ||
	    DrawButton128_2(X1+616, Y1-6, vw_GetText("1_Next"), ContentTransp, !vw_GetDevCaps()->GLSL100Supported || vw_GetDevCaps()->ShaderModel < 3.0)) {
		Options_UseGLSL = !Options_UseGLSL;
		// если выключены шейдеры - выключаем и тени
		if (!Options_UseGLSL) Options_ShadowMap = 0;
		else Options_ShadowMap = Setup.ShadowMap;
	}
	if (vw_GetDevCaps()->GLSL100Supported && (vw_GetDevCaps()->ShaderModel >= 3.0)) {
		Size = vw_FontSize(Options_UseGLSL ? vw_GetText("1_On") : vw_GetText("1_Off"));
		SizeI = (170-Size)/2;
		vw_DrawFont(X1+438+SizeI, Y1, 0, 0, 1.0f, 1.0f,1.0f,1.0f, ContentTransp, Options_UseGLSL ? vw_GetText("1_On") : vw_GetText("1_Off"));
	} else {
		Size = vw_FontSize(vw_GetText("3_Not_available"));
		SizeI = (170-Size)/2;
		vw_DrawFont(X1+438+SizeI, Y1, 0, 0, 1.0f, 1.0f,0.5f,0.0f, ContentTransp, vw_GetText("3_Not_available"));
	}





	// качество теней
	Y1 += Prir1;
	vw_DrawFont(X1, Y1, -280, 0, 1.0f, 1.0f,0.0f,0.0f, ContentTransp, vw_GetText("3_Shadow_Quality"));
	if (DrawButton128_2(X1+300, Y1-6, vw_GetText("1_Prev"), ContentTransp, Options_ShadowMap==0 || !vw_GetDevCaps()->GLSL100Supported ||
			    vw_GetDevCaps()->ShaderModel < 3.0 || !vw_GetDevCaps()->FramebufferObject || !Options_UseGLSL || (vw_GetDevCaps()->FramebufferObjectDepthSize < 24))) {
		Options_ShadowMap--;
		if (Options_ShadowMap < 0) Options_ShadowMap = 0;
	}
	if (DrawButton128_2(X1+616, Y1-6, vw_GetText("1_Next"), ContentTransp, Options_ShadowMap==9 || !vw_GetDevCaps()->GLSL100Supported ||
			    vw_GetDevCaps()->ShaderModel < 3.0 || !vw_GetDevCaps()->FramebufferObject || !Options_UseGLSL || (vw_GetDevCaps()->FramebufferObjectDepthSize < 24))) {
		Options_ShadowMap++;
		if (Options_ShadowMap > 9) Options_ShadowMap = 9;
	}
	if (vw_GetDevCaps()->GLSL100Supported && (vw_GetDevCaps()->ShaderModel >= 3.0) && vw_GetDevCaps()->FramebufferObject && Options_UseGLSL && (vw_GetDevCaps()->FramebufferObjectDepthSize >= 24)) {
		Size = vw_FontSize(ShadowButtonQualityBase[Options_ShadowMap], vw_GetText(ShadowButtonQuality[Options_ShadowMap]));
		float WScale = 0;
		if (Size > 170) {
			Size = 170;
			WScale = -170;
		}
		SizeI = (170-Size)/2;//High, Medium, Low
		vw_DrawFont(X1+438+SizeI, Y1, WScale, 0, 1.0f, 1.0f,1.0f,1.0f, ContentTransp, ShadowButtonQualityBase[Options_ShadowMap], vw_GetText(ShadowButtonQuality[Options_ShadowMap]));
	} else {
		Size = vw_FontSize(vw_GetText("3_Not_available"));
		SizeI = (170-Size)/2;
		vw_DrawFont(X1+438+SizeI, Y1, 0, 0, 1.0f, 1.0f,0.5f,0.0f, ContentTransp, vw_GetText("3_Not_available"));
	}









	int Prir = 100;
	int X;
	int Y = 165+Prir*4;

	X = Setup.iAspectRatioWidth/2 - 366;
	if (DrawButton200_2(X,Y+28, vw_GetText("1_Interface"), ContentTransp, false)) {
		if (MenuStatus == eMenuStatus::GAME) {
			SetOptionsMenu(eMenuStatus::INTERFACE);
			GameMenuStatus = eGameMenuStatus::INTERFACE;
		} else
			ComBuffer = eCommand::SWITCH_TO_INTERFACE;
	}

	X = Setup.iAspectRatioWidth/2 - 100;
	if (DrawButton200_2(X,Y+28, vw_GetText("1_Video_&_Audio"), ContentTransp, false)) {
		if (MenuStatus == eMenuStatus::GAME) {
			SetOptionsMenu(eMenuStatus::OPTIONS);
			GameMenuStatus = eGameMenuStatus::OPTIONS;
		} else
			ComBuffer = eCommand::SWITCH_TO_OPTIONS;
	}

	X = Setup.iAspectRatioWidth/2 + 166;
	if (DrawButton200_2(X,Y+28, vw_GetText("1_Config_Controls"), ContentTransp, false)) {
		if (MenuStatus == eMenuStatus::GAME) {
			SetOptionsMenu(eMenuStatus::CONFCONTROL);
			GameMenuStatus = eGameMenuStatus::CONFCONTROL;
		} else
			ComBuffer = eCommand::SWITCH_TO_CONFCONTROL;
	}








	if (Options_TexturesCompressionType == Setup.TexturesCompressionType &&
	    Options_MSAA == Setup.MSAA &&
	    Options_CSAA == Setup.CSAA &&
	    Options_UseGLSL == Setup.UseGLSL &&
	    Options_TexturesQuality == Setup.TexturesQuality &&
	    Options_ShadowMap == Setup.ShadowMap) {
		X = (Setup.iAspectRatioWidth - 384)/2;
		Y = Y+Prir;
		if (MenuStatus == eMenuStatus::GAME) {
			if (DrawButton384(X,Y, vw_GetText("1_GAME_MENU"), ContentTransp, ButtonTransp1, LastButtonUpdateTime1))
				GameMenuStatus = eGameMenuStatus::GAME_MENU;
		} else {
			if (DrawButton384(X,Y, vw_GetText("1_MAIN_MENU"), ContentTransp, ButtonTransp1, LastButtonUpdateTime1))
				ComBuffer = eCommand::SWITCH_TO_MAIN_MENU;
		}
	} else {
		X = Setup.iAspectRatioWidth/2 - 256 - 38;
		Y = Y+Prir;
		if (MenuStatus == eMenuStatus::GAME) {
			if (DrawButton256(X,Y, vw_GetText("1_GAME_MENU"), ContentTransp, ButtonTransp1, LastButtonUpdateTime1))
				GameMenuStatus = eGameMenuStatus::GAME_MENU;
		} else {
			if (DrawButton256(X,Y, vw_GetText("1_MAIN_MENU"), ContentTransp, ButtonTransp1, LastButtonUpdateTime1))
				ComBuffer = eCommand::SWITCH_TO_MAIN_MENU;
		}
		X = Setup.iAspectRatioWidth/2 + 38;
		if (DrawButton256(X,Y, vw_GetText("1_APPLY"), ContentTransp, ButtonTransp2, LastButtonUpdateTime2)) {
			// проверяем, нужно перегружать или нет
			if (Options_TexturesCompressionType != Setup.TexturesCompressionType ||
			    Options_MSAA != Setup.MSAA ||
			    Options_CSAA != Setup.CSAA ||
			    Options_UseGLSL != Setup.UseGLSL ||
			    Options_TexturesQuality != Setup.TexturesQuality ||
			    Options_ShadowMap != Setup.ShadowMap) {
				if (MenuStatus == eMenuStatus::GAME)
					SetCurrentDialogBox(eDialogBox::RestartOnAdvOptChanged);
				else {
					CanQuit = false;
					Quit = true;
					NeedReCreate = true;
					SaveOptionsAdvMenuTmpData();
				}
			}
		}
	}
}


void SaveOptionsAdvMenuTmpData()
{
	Setup.UseGLSL = Options_UseGLSL;
	Setup.TexturesCompressionType = Options_TexturesCompressionType;
	Setup.MSAA = Options_MSAA;
	Setup.CSAA = Options_CSAA;
	Setup.ShadowMap = Options_ShadowMap;
	Setup.TexturesQuality = Options_TexturesQuality;
}






