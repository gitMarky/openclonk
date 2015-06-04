/*
 * OpenClonk, http://www.openclonk.org
 *
 * Copyright (c) 2014-2015, The OpenClonk Team and contributors
 *
 * Distributed under the terms of the ISC license; see accompanying file
 * "COPYING" for details.
 *
 * "Clonk" is a registered trademark of Matthes Bender, used with permission.
 * See accompanying file "TRADEMARK" for details.
 *
 * To redistribute this file separately, substitute the full license texts
 * for the above references.
 */

#ifndef C4FOWAMBIENT_H
#define C4FOWAMBIENT_H

#include <C4Landscape.h>

/**
	This class manages a texture that holds the ambient light intensity
*/
class C4FoWAmbient
{
public:
	C4FoWAmbient();
	~C4FoWAmbient();

	GLuint Tex;

private:
	// Parameters
	double Resolution;
	double Radius;
	double FullCoverage;
	// Landscape size
	unsigned int LandscapeX;
	unsigned int LandscapeY;
	// Size of ambient map
	unsigned int SizeX;
	unsigned int SizeY;
	// Brightness (not premultiplied)
	double Brightness;
	// Color information
	uint32_t Color;
	float colorR;
	float colorG;
	float colorB;
	float colorV; // color value. 1.0 is maximum.
public:
	void Clear();

	void SetBrightness(double brightness) { Brightness = brightness; }
	double GetBrightness() const { return colorV * Brightness; }
	void SetColor(uint32_t Value);
	uint32_t GetColor() const { return Color; }
	float GetR() const { return colorR; }
	float GetG() const { return colorG; }
	float GetB() const { return colorB; }

	// High resolution will make the map coarser, but speed up the generation process
	// and save video memory.
	// The radius specifies the radius of landscape pixels that are sampled around each pixel
	// to obtain the ambient light.
	// full_coverage is a number between 0 and 1, and it specifies what portion of the full circle
	// needs to be illuminated for full ambient light intensity.
	void CreateFromLandscape(const C4Landscape& landscape, double resolution, double radius, double full_coverage);

	// Update the map after the landscape has changed in the region indicated by update.
	void UpdateFromLandscape(const C4Landscape& landscape, const C4Rect& update);

	// Fills a 2x3 matrix to transform fragment coordinates to ambient map texture coordinates
	void GetFragTransform(const FLOAT_RECT& vpRect, const C4Rect& clipRect, const C4Rect& outRect, float ambientTransform[6]) const;

	unsigned int GetLandscapeWidth() const { return LandscapeX; }
	unsigned int GetLandscapeHeight() const { return LandscapeY; }
};

#endif // C4FOWAMBIENT_H
