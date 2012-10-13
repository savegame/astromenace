/******************************************************************************

	This source file is part of Viewizard Game Engine
	For the latest info, see http://www.viewizard.com/

	File name: Font.cpp

	Copyright (c) 2012 Michael Kurinnoy, Viewizard
	All Rights Reserved.

	File Version: 3.1

******************************************************************************

	AstroMenace game source code available under "dual licensing" model.
	The licensing options available are:

	* Commercial Licensing. This is the appropriate option if you are
	  creating proprietary applications and you are not prepared to
	  distribute and share the source code of your application.
	  Contact us for pricing at viewizard@viewizard.com

	* Open Source Licensing. This is the appropriate option if you want
	  to share the source code of your application with everyone you
	  distribute it to, and you also want to give them the right to share
	  who uses it. You should have received a copy of the GNU General Public
	  License version 3 with this source codes.
	  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/


#include "Font.h"



void vw_AttachFontChar(eFontChar* FontChar);
void vw_DetachFontChar(eFontChar* FontChar);
extern FT_Face face;
extern int InternalFontSize;



//------------------------------------------------------------------------------------
// загрузка и генерация всех необходимых данных для символа (utf32)
//------------------------------------------------------------------------------------
eFontChar* vw_LoadFontChar(unsigned UTF32)
{
	// прежде всего, пытаемся загрузить символ, а уже потом создаем структуру


	// загрузка глифа нужного нам символа
	if (FT_Load_Char( face, UTF32, FT_LOAD_RENDER | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT))
	{
		fprintf(stderr, "Can't load Char: %u\n", UTF32);
		return 0;
	}


	// создаем структуру FontChar
	eFontChar* NewChar;
	NewChar = new eFontChar;

	NewChar->UTF32 = UTF32;
	NewChar->CharTexture = 0;
	NewChar->TexturePositionLeft = 0;
	NewChar->TexturePositionRight = face->glyph->bitmap.width; // в случае одной текстуры совпадают с шириной
	NewChar->TexturePositionTop = 0;
	NewChar->TexturePositionBottom = face->glyph->bitmap.rows; // в случае одной текстуры совпадают с высотой
	NewChar->Width = face->glyph->bitmap.width;
	NewChar->Height = face->glyph->bitmap.rows;
	NewChar->Left = face->glyph->bitmap_left;
	NewChar->Top = face->glyph->bitmap_top;
	NewChar->Prev = 0;
	NewChar->Next = 0;

	BYTE * pixels;
	pixels = new BYTE[NewChar->Width*NewChar->Height*4];

	// битмап идет в 1 канале градаций серого, делаем 32бита rgba
	int k=0;
	BYTE ColorRGB[3]={255,255,255};
	for (int j=0; j<NewChar->Height; j++)
	{
		for (int i=0; i<NewChar->Width; i++)
		{
			// RGB составляющую заполняем белым
			memcpy(pixels + k, ColorRGB, 3);
			k+=3;
			memcpy(pixels + k, face->glyph->bitmap.buffer+(NewChar->Height-j-1)*NewChar->Width+i, 1);
			k++;
		}
	}

	// называем текстуру номером символа в утф32
	char texturefilename[MAX_PATH];
	sprintf(texturefilename, "%i", UTF32);

	NewChar->CharTexture = vw_CreateTextureFromMemory(texturefilename, pixels, NewChar->Width, NewChar->Height, 4, false);
	// очищаем память
	delete [] pixels;

	// подключаем к менеджеру
	vw_AttachFontChar(NewChar);
	return NewChar;
}


//-----------------------------------------------------------------------------
// делаем генерацию нужных символов по списку генерируя одну текстуру
//-----------------------------------------------------------------------------
void vw_GenerateFontChars(int FontTextureWidth, int FontTextureHeight, const char * CharsList)
{

	printf("Font characters generation start.\n");

	// будем использовать последовательность как имя текстуры
	const char *TextureName = CharsList;
	// временный массив
	BYTE * DIB;
	DIB = new BYTE[FontTextureWidth*FontTextureHeight*4]; // всегда делаем rgba
	// устанавливаем 0 везде, чтобы потом не краcить rgb, а только формировать альфу
	memset(DIB, 0, FontTextureWidth*FontTextureHeight*4);

	// данные для работы с вклеиванием в текстуру
	int CurrentDIBX = 0;
	int CurrentDIBY = 0;
	int EdgingSpace = 2;
	int MaxHeightInCurrentLine = 0;


	// первый проход, формируем одну большую текстуру
	const char *CharsList2 = CharsList;
	while (strlen(CharsList) > 0)
	{
		unsigned CurrentChar;
		// преобразуем в утф32 и "сдвигаемся" на следующий символ в строке
		CharsList = utf8_to_utf32(CharsList, &CurrentChar);


		// загрузка глифа нужного нам символа
		if (FT_Load_Char( face, CurrentChar, FT_LOAD_RENDER | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT))
		{
			fprintf(stderr, "Can't load Char: %u\n", CurrentChar);
			return;
		}

		// создаем структуру FontChar
		eFontChar* NewChar;
		NewChar = new eFontChar;

		NewChar->UTF32 = CurrentChar;
		NewChar->CharTexture = 0;
		NewChar->TexturePositionLeft = 0;
		NewChar->TexturePositionRight = 0;
		NewChar->TexturePositionTop = 0;
		NewChar->TexturePositionBottom = 0;
		NewChar->Width = face->glyph->bitmap.width;
		NewChar->Height = face->glyph->bitmap.rows;
		NewChar->Left = face->glyph->bitmap_left;
		NewChar->Top = face->glyph->bitmap_top;
		NewChar->Prev = 0;
		NewChar->Next = 0;

		// делаем установку параметров для вклеивания

		// если в текущую строку символов уже не можем вписывать - смещаемся на новую, ниже
		if (CurrentDIBX + NewChar->Width > FontTextureWidth)
		{
			CurrentDIBX = 0;
			CurrentDIBY += MaxHeightInCurrentLine + EdgingSpace;
			MaxHeightInCurrentLine = 0;
		}
		// если в текущую строку не влазит уже по высоте - значит это фейл... кричим чтоб дали больше текстуру
		if (CurrentDIBY + NewChar->Height > FontTextureHeight)
		{
			fprintf(stderr, "!!! Can't generate all font chars in one texture. Too many chars or too small texture size!\n");
			delete NewChar;
			break;
		}

		// "вклеиваем" новый символ в массив
		BYTE ColorRGB[3]={255,255,255};
		for (int j=0; j<NewChar->Height; j++)
		for (int i=0; i<NewChar->Width; i++)
		{
			memcpy(DIB + (FontTextureHeight-CurrentDIBY-j-1)*FontTextureWidth*4 + (CurrentDIBX+i)*4,
					ColorRGB,
					3);
			memcpy(DIB + (FontTextureHeight-CurrentDIBY-j-1)*FontTextureWidth*4 + (CurrentDIBX+i)*4 + 3,
					face->glyph->bitmap.buffer+j*NewChar->Width+i,
					1);
		}

		// устанавливаем параметры текстуры для прорисовки нашему символу
		NewChar->TexturePositionLeft = CurrentDIBX;
		NewChar->TexturePositionRight = CurrentDIBX + NewChar->Width;
		NewChar->TexturePositionTop = CurrentDIBY;
		NewChar->TexturePositionBottom = CurrentDIBY + NewChar->Height;

		// выбираем наибольшую высоту символов
		if (MaxHeightInCurrentLine < NewChar->Height) MaxHeightInCurrentLine = NewChar->Height;
		// смещаем указатель
		CurrentDIBX += NewChar->Width + EdgingSpace;

		// подключаем к менеджеру
		vw_AttachFontChar(NewChar);
	}


/////////////////////////////////
/*
		// выводим в bmp файл сгенерированный DIB, если нужно проверить

        SDL_Surface *temp;
        temp = SDL_CreateRGBSurface(SDL_SWSURFACE, FontTextureWidth, FontTextureHeight, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
        0x000000FF, 0x0000FF00, 0x00FF0000, 0
#else
        0x00FF0000, 0x0000FF00, 0x000000FF, 0
#endif
        );
		memcpy(temp->pixels, DIB, FontTextureWidth*FontTextureHeight*4);
        SDL_SaveBMP(temp, "fontgenerationtest.bmp");
        SDL_FreeSurface(temp);
*/
/////////////////////////////////


	// создаем текстуру
	eTexture* FontTexture = vw_CreateTextureFromMemory(TextureName, DIB, FontTextureWidth, FontTextureHeight, 4, false);
	// освобождаем память
	delete [] DIB;
	if (FontTexture == 0)
	{
		fprintf(stderr, "Can't create font texture.\n");
		return;
	}


	// второй проход, всем FontChars из списка, присваиваем сгенерированную текстуру
	while (strlen(CharsList2) > 0)
	{
		unsigned CurrentChar;
		// преобразуем в утф32 и "сдвигаемся" на следующий символ в строке
		CharsList2 = utf8_to_utf32(CharsList2, &CurrentChar);
		// ставим нашу общую текстуру
		eFontChar* TMPChar = vw_FindFontCharByUTF32(CurrentChar);
		if (TMPChar != 0) TMPChar->CharTexture = FontTexture;
	}


	printf("Font characters generation end.\n\n");
}






//------------------------------------------------------------------------------------
// прорисовка фонта
//------------------------------------------------------------------------------------
void vw_DrawFont(int X, int Y, float FlattenWidth, float MaxWidth, float FontScale, float R, float G, float B, float Transp, const char *Text, ...)
{
	if (Text == 0) return;

	// учитываем аспект рейшен
	float AW;
	float AH;
	bool ASpresent=false;
	ASpresent = vw_GetAspectWH(&AW, &AH);
	// получаем данные текущего вьюпорта
	int W, H;
	vw_GetViewport(0, 0, &W, &H);
	float AHw = H*1.0f;

	float RealTextYPos = AHw - 2*Y + 4 + InternalFontSize*FontScale;
	if (ASpresent) RealTextYPos = AH - 2*Y + 4 + InternalFontSize*FontScale;


	// если текст ниже чем ширина нашего окна - не рисуем
	if (ASpresent){ if (Y > AH) return;}
	else {if (Y > H) return;}
	// если текст выше чем ноль - тоже рисовать его смысла нет
	if (Y+InternalFontSize*FontScale < 0) return;


// FlattenWidth - выравнивать по ширине
// если FlattenWidth отрицателен, выравниваем по значению, "сжимая" буквы, если нужно
// MaxWidth - рисовать до ширины


	// смотрим значения параметров в строке
	char	text[1024];
	va_list		ap;
	va_start(ap, Text);
	vsprintf(text, Text, ap);
	va_end(ap);
	// в text уже полная строка
	if (strlen(text) == 0) return;

	float Xstart = X;
	// делаем пробел в 2/3 от размера фонта
	float SpaceWidth = InternalFontSize * 2 / 3;
	// коэф. изменения букв по ширине
	float FontWidthScale = 1.0f;

	if (Transp >= 1.0f) Transp = 1.0f;


	// если нужно выравнивать, считаем данные пробелов
	if (FlattenWidth > 0)
	{
		float LineWidth = 0;
		int SpaceCount = 0;

		const char *CountCheck = text;
		while (strlen(CountCheck) > 0)
		{
			unsigned UTF32;
			// преобразуем в утф32 и "сдвигаемся" на следующий символ в строке
			CountCheck = utf8_to_utf32(CountCheck, &UTF32);
			// находим наш текущий символ
			eFontChar* DrawChar = vw_FindFontCharByUTF32(UTF32);
			if (DrawChar == 0) DrawChar = vw_LoadFontChar(UTF32);

			// считаем кол-во пробелов
			if (UTF32 == 0x020)
				SpaceCount++;
			else
				LineWidth += DrawChar->Width + DrawChar->Left;
		}

		if (FlattenWidth > LineWidth)
			if (SpaceCount!=0) SpaceWidth = (FlattenWidth - LineWidth)/SpaceCount;
	}
	// если нужно сжать, считаем коэф. сжатия букв
	if (FlattenWidth < 0)
	{
		float LineWidth = 0;

		const char *CountCheck = text;
		while (strlen(CountCheck) > 0)
		{
			unsigned UTF32;
			// преобразуем в утф32 и "сдвигаемся" на следующий символ в строке
			CountCheck = utf8_to_utf32(CountCheck, &UTF32);
			// находим наш текущий символ
			eFontChar* DrawChar = vw_FindFontCharByUTF32(UTF32);
			if (DrawChar == 0) DrawChar = vw_LoadFontChar(UTF32);

			// считаем длину символов с пробелами
			if (UTF32 != 0x020)
				LineWidth += DrawChar->Width + DrawChar->Left;
			else
				LineWidth += SpaceWidth;
		}

		if (FlattenWidth*(-1.0f) < LineWidth) FontWidthScale = FlattenWidth/LineWidth*(-1.0f);
	}


	float LineWidth = 0;

	// установка свойств текстуры
	vw_SetTexAlpha(false, 0.01f);
	vw_SetTexBlend(RI_BLEND_SRCALPHA, RI_BLEND_INVSRCALPHA);
	// ставим цвет
	vw_SetColor(R, G, B, Transp);

	// для отрисовки
	eTexture* CurrentTexture = 0;
	int k=0;
	// буфер для последовательности RI_QUADS
	// войдет RI_2f_XYZ | RI_2f_TEX
	float *tmp = 0;
	tmp = new float[(2+2)*4*strlen(text)]; if (tmp == 0) return;

	// чтобы меньше делать операций умножения, включаем коэф. один в другой сразу для ширины символов
	FontWidthScale = FontScale*FontWidthScale;


	// прорисовка текста
	const char *textdraw = text;
	// прорисовываем все символы
	while (strlen(textdraw) > 0)
	{
		unsigned UTF32;
		// преобразуем в утф32 и "сдвигаемся" на следующий символ в строке
		textdraw = utf8_to_utf32(textdraw, &UTF32);
		// находим наш текущий символ
		eFontChar* DrawChar = vw_FindFontCharByUTF32(UTF32);
		if (DrawChar == 0) DrawChar = vw_LoadFontChar(UTF32);
		// первый символ - запоминаем его текстуру
		if (CurrentTexture == 0) CurrentTexture = DrawChar->CharTexture;


		// проверка на текстуру, если текстура поменялась - отрисовываем все что есть в буфере, если там что-то есть
		if (CurrentTexture != DrawChar->CharTexture)
		{
			// если что-то было в буфере - выводим
			if (k > 0)
			{
				// Установка текстуры и ее свойств...
				vw_SetTextureV(0, CurrentTexture);
				vw_SetTexAddressMode(0, RI_CLAMP);
				// не можем ставить другое! если нет мипмапа
				vw_SetTexFiltering(0, RI_MAGFILTER_LINEAR | RI_MINFILTER_LINEAR | RI_MIPFILTER_NONE, 1);
				// отрисовываем все что есть в буфере
				vw_SendVertices(RI_QUADS, 4*(k/16), RI_2f_XY | RI_1_TEX, tmp, 4*sizeof(float));
			}


			// запоминаем новую текстуру
			CurrentTexture = DrawChar->CharTexture;
			// сбрасываем счетчики
			k=0;
		}

		// если не пробел - рисуем
		if (UTF32 != 0x020)
		{

			float DrawX = Xstart + DrawChar->Left*FontWidthScale;
			float DrawY = Y + 2 + (InternalFontSize - DrawChar->Top)*FontScale; // 2 доп смещение ("привет" от старого фонта)

			// Вычисление поправки по У в зависимости от DrawCorner
			// - расположения угла начала координат
			float tmpPosY = 0;
			// изменяем только в случае RI_UL_CORNER
			if (ASpresent) tmpPosY = (AH - DrawY - DrawY - DrawChar->Height*FontScale);
			else tmpPosY = (AHw - DrawY - DrawY - DrawChar->Height*FontScale);

			float ImageHeight = DrawChar->CharTexture->Height*1.0f;
			float ImageWidth = DrawChar->CharTexture->Width*1.0f;

			float FrameHeight = (DrawChar->TexturePositionBottom*1.0f )/ImageHeight;
			float FrameWidth = (DrawChar->TexturePositionRight*1.0f )/ImageWidth;

			float Yst = (DrawChar->TexturePositionTop*1.0f)/ImageHeight;
			float Xst = (DrawChar->TexturePositionLeft*1.0f)/ImageWidth;

			tmp[k++] = DrawX;
			tmp[k++] = DrawY +tmpPosY + DrawChar->Height*FontScale;
			tmp[k++] = Xst;
			tmp[k++] = 1.0f-Yst;

			tmp[k++] = DrawX;
			tmp[k++] = DrawY +tmpPosY;
			tmp[k++] = Xst;
			tmp[k++] = 1.0f-FrameHeight;

			tmp[k++] = DrawX + DrawChar->Width*FontWidthScale;
			tmp[k++] = DrawY +tmpPosY;
			tmp[k++] = FrameWidth;
			tmp[k++] = 1.0f-FrameHeight;

			tmp[k++] = DrawX + DrawChar->Width*FontWidthScale;
			tmp[k++] = DrawY +tmpPosY + DrawChar->Height*FontScale;
			tmp[k++] = FrameWidth;
			tmp[k++] = 1.0f-Yst;


			Xstart += (DrawChar->Width + DrawChar->Left)*FontWidthScale;
			LineWidth += (DrawChar->Width + DrawChar->Left)*FontWidthScale;
		}
		else
		{
			Xstart += SpaceWidth*FontWidthScale;
			LineWidth += SpaceWidth*FontWidthScale;
		}

		// если нужно прорисовывать с ограничением по длине
		if (MaxWidth != 0.0f)
			if (LineWidth >= MaxWidth) break;
	}


	// если что-то было в буфере - выводим
	if (k > 0)
	{
		// Установка текстуры и ее свойств...
		vw_SetTextureV(0, CurrentTexture);
		vw_SetTexAddressMode(0, RI_CLAMP);
		// не можем ставить другое! если нет мипмапа
		vw_SetTexFiltering(0, RI_MAGFILTER_LINEAR | RI_MINFILTER_LINEAR | RI_MIPFILTER_NONE, 1);
		// отрисовываем все что есть в буфере
		vw_SendVertices(RI_QUADS, 4*(k/16), RI_2f_XY | RI_1_TEX, tmp, 4*sizeof(float));
	}



	if (tmp != 0){delete [] tmp; tmp = 0;}
	vw_SetTexAlpha(false, 0.5f);
	vw_SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	vw_SetTextureDef(0);
}



//------------------------------------------------------------------------------------
// получаем размер строки
//------------------------------------------------------------------------------------
int vw_FontSize(const char *Text, ...)
{
	if (Text == 0) return 0;

	// смотрим значения параметров в строке
	char	text[1024];
	va_list		ap;

	va_start(ap, Text);
	vsprintf(text, Text, ap);
	va_end(ap);
	// в text уже полная строка
	if (strlen(text) == 0) return 0;

	const char *textdraw = text;
	// делаем пробел в 2/3 от размера фонта
	float SpaceWidth = InternalFontSize * 2 / 3;
	float LineWidth = 0;

	while (strlen(textdraw) > 0)
	{
		unsigned UTF32;
		// преобразуем в утф32 и "сдвигаемся" на следующий символ в строке
		textdraw = utf8_to_utf32(textdraw, &UTF32);
		// находим наш текущий символ
		eFontChar* DrawChar = vw_FindFontCharByUTF32(UTF32);
		if (DrawChar == 0) DrawChar = vw_LoadFontChar(UTF32);

		// считаем кол-во пробелов
		if (UTF32 == 0x020)
			LineWidth += SpaceWidth;
		else
			LineWidth += DrawChar->Width + DrawChar->Left;
	}


	return (int)LineWidth;
}





//------------------------------------------------------------------------------------
// прорисовка фонта в 3д пространстве
//------------------------------------------------------------------------------------
void vw_DrawFont3D(float X, float Y, float Z, const char *Text, ...)
{

	if (Text == 0) return;

	// смотрим значения параметров в строке
	char	text[1024];
	va_list		ap;
	va_start(ap, Text);
	vsprintf(text, Text, ap);
	va_end(ap);
	// в text уже полная строка
	if (strlen(text) == 0) return;
	// прорисовка текста
	const char *textdraw = text;


	float Xstart = 0.0f;
	// делаем пробел в 2/3 от размера фонта
	float SpaceWidth = InternalFontSize * 2 / 3;

	textdraw = text;


	// для отрисовки
	eTexture* CurrentTexture = 0;
	int k=0;
	// буфер для последовательности RI_QUADS
	// войдет RI_2f_XY | RI_2f_TEX
	float *tmp = 0;
	tmp = new float[(2+2)*4*strlen(textdraw)]; if (tmp == 0) return;

	// установка свойств текстуры
	vw_SetTexAlpha(false, 0.01f);
	vw_SetTexBlend(RI_BLEND_SRCALPHA, RI_BLEND_INVSRCALPHA);

	vw_PushMatrix();

	vw_Translate(VECTOR3D(X, Y, Z));
	VECTOR3D CurrentCameraRotation;
	vw_GetCameraRotation(&CurrentCameraRotation);

	// поворачиваем к камере
	vw_Rotate(0, CurrentCameraRotation.y, 0);
	vw_Rotate(CurrentCameraRotation.x, 0, 0);



	// прорисовываем все символы
	while (strlen(textdraw) > 0)
	{
		unsigned UTF32;
		// преобразуем в утф32 и "сдвигаемся" на следующий символ в строке
		textdraw = utf8_to_utf32(textdraw, &UTF32);
		// находим наш текущий символ
		eFontChar* DrawChar = vw_FindFontCharByUTF32(UTF32);
		if (DrawChar == 0) DrawChar = vw_LoadFontChar(UTF32);
		// первый символ - запоминаем его текстуру
		if (CurrentTexture == 0) CurrentTexture = DrawChar->CharTexture;


		// проверка на текстуру, если текстура поменялась - отрисовываем все что есть в буфере, если там что-то есть
		if (CurrentTexture != DrawChar->CharTexture)
		{
			// если что-то было в буфере - выводим
			if (k > 0)
			{
				// Установка текстуры и ее свойств...
				vw_SetTextureV(0, CurrentTexture);
				vw_SetTexAddressMode(0, RI_CLAMP);
				// не можем ставить другое! если нет мипмапа
				vw_SetTexFiltering(0, RI_MAGFILTER_LINEAR | RI_MINFILTER_LINEAR | RI_MIPFILTER_NONE, 1);
				// отрисовываем все что есть в буфере
				vw_SendVertices(RI_QUADS, 4*(k/16), RI_2f_XY | RI_1_TEX, tmp, 4*sizeof(float));
			}


			// запоминаем новую текстуру
			CurrentTexture = DrawChar->CharTexture;
			// сбрасываем счетчики
			k=0;
		}


		// если не пробел - рисуем
		if (UTF32 != 0x020)
		{
			float DrawX = Xstart + DrawChar->Left;
			float DrawY = InternalFontSize - DrawChar->Top;


			float ImageHeight = DrawChar->CharTexture->Height*1.0f;
			float ImageWidth = DrawChar->CharTexture->Width*1.0f;

			float FrameHeight = (DrawChar->TexturePositionBottom*1.0f )/ImageHeight;
			float FrameWidth = (DrawChar->TexturePositionRight*1.0f )/ImageWidth;

			float Yst = (DrawChar->TexturePositionTop*1.0f)/ImageHeight;
			float Xst = (DrawChar->TexturePositionLeft*1.0f)/ImageWidth;

			tmp[k++] = DrawX/10.0f;
			tmp[k++] = (DrawY + DrawChar->Height)/10.0f;
			tmp[k++] = Xst;
			tmp[k++] = 1.0f-Yst;

			tmp[k++] = DrawX/10.0f;
			tmp[k++] = DrawY/10.0f;
			tmp[k++] = Xst;
			tmp[k++] = 1.0f-FrameHeight;

			tmp[k++] = (DrawX + DrawChar->Width)/10.0f;
			tmp[k++] = DrawY/10.0f;
			tmp[k++] = FrameWidth;
			tmp[k++] = 1.0f-FrameHeight;

			tmp[k++] = (DrawX + DrawChar->Width)/10.0f;
			tmp[k++] = (DrawY + DrawChar->Height)/10.0f;
			tmp[k++] = FrameWidth;
			tmp[k++] = 1.0f-Yst;


			Xstart += DrawChar->Width + DrawChar->Left;
		}
		else
		{
			Xstart += SpaceWidth;
		}

	}


	// если что-то было в буфере - выводим
	if (k > 0)
	{
		// Установка текстуры и ее свойств...
		vw_SetTextureV(0, CurrentTexture);
		vw_SetTexAddressMode(0, RI_CLAMP);
		// не можем ставить другое! если нет мипмапа
		vw_SetTexFiltering(0, RI_MAGFILTER_LINEAR | RI_MINFILTER_LINEAR | RI_MIPFILTER_NONE, 1);
		// отрисовываем все что есть в буфере
		vw_SendVertices(RI_QUADS, 4*(k/16), RI_2f_XY | RI_1_TEX, tmp, 4*sizeof(float));
	}


	vw_PopMatrix();



	if (tmp != 0){delete [] tmp; tmp = 0;};
	vw_SetTexAlpha(false, 0.5f);
	vw_SetTextureDef(0);

}
