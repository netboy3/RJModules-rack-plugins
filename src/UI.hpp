#pragma once
#include "RJModules.hpp"

struct ColorValueLight : ModuleLightWidget {
	NVGcolor baseColor;
	// Pixels to add for outer radius (either px or relative %).
	float outerRadiusHalo = 0.35;
	bool outerRadiusRelative = true;
	ColorValueLight() : ModuleLightWidget()
	{
		bgColor = nvgRGBA(0x20, 0x20, 0x20, 0xFF);
		borderColor = nvgRGBA(0, 0, 0, 0);
		return;
	};
	virtual ~ColorValueLight(){};
	// Set a single color
	void setColor(NVGcolor bColor)
	{
		color = bColor;
		baseColor = bColor;
		if (baseColors.size() < 1)
		{
			baseColors.push_back(bColor);
		}
		else
		{
			baseColors[0] = bColor;
		}
	}
	void drawHalo(const DrawArgs &args) override
	{
		float radius = box.size.x / 2.0;
		float oradius = radius + ((outerRadiusRelative) ? (radius*outerRadiusHalo) : outerRadiusHalo);

		nvgBeginPath(args.vg);
		nvgRect(args.vg, radius - oradius, radius - oradius, 2 * oradius, 2 * oradius);

		NVGpaint paint;
		NVGcolor icol = color::mult(color, 0.10);//colorMult(color, 0.10);
		NVGcolor ocol = nvgRGB(0, 0, 0);
		paint = nvgRadialGradient(args.vg, radius, radius, radius, oradius, icol, ocol);
		nvgFillPaint(args.vg, paint);
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		nvgFill(args.vg);
	}
};

//--------------------------------------------------------------
// RJ_PadSwitch
//--------------------------------------------------------------
struct RJ_PadSvgSwitch : SvgSwitch {
	int btnId = -1;
	// Group id (to match guys that should respond to mouse down drag).
	int groupId = -1;
	RJ_PadSvgSwitch() : SvgSwitch() {
		momentary = false;

		this->shadow->opacity = 0.0f; // Turn off the circular shadows that are everywhere.

		return;
	}
	RJ_PadSvgSwitch(Vec size) : RJ_PadSvgSwitch() {
		box.size = size;
		return;
	}
	void setValue(float val) {
		if (getParamQuantity())
		{
			getParamQuantity()->setValue(val);
		}
		return;
	}

	void toggleVal()
	{
		if (getParamQuantity())
		{
			float newVal = (getParamQuantity()->getValue() < getParamQuantity()->maxValue) ? getParamQuantity()->maxValue : getParamQuantity()->minValue;
			getParamQuantity()->setValue(newVal); // Toggle Value
		}
		return;
	}

	// Allow mouse-down & drag to set buttons (i.e. on Sequencer grid where there are many buttons).
	// Suggestion from @LKHSogpit, Solution from @AndrewBelt.
	// https://github.com/j4s0n-c/trowaSoft-VCV/issues/7
	// https://github.com/VCVRack/Rack/issues/607
	/** Called when a widget responds to `onMouseDown` for a left button press */
	void onDragStart(const event::DragStart &e) override {
		if (e.button != GLFW_MOUSE_BUTTON_LEFT)
			return;

		if (getParamQuantity())
		{
			// if (momentary)
			// {
			// 	DEBUG("RJ_PadSvgSwitch onDragStart(%d) - Momentary - Set Value to %3.1f.", btnId, getParamQuantity()->maxValue);
			// 	getParamQuantity()->setValue(getParamQuantity()->maxValue); // Trigger Value
			// }
			// else
			// {
			// 	float newVal = (getParamQuantity()->getValue() < getParamQuantity()->maxValue) ? getParamQuantity()->maxValue : getParamQuantity()->minValue;
			// 	DEBUG("RJ_PadSvgSwitch onDragStart(%d) - Set Value to %3.1f.", btnId, newVal);
			// 	getParamQuantity()->setValue(newVal); // Toggle Value
			// }

			float newVal = (getParamQuantity()->getValue() < getParamQuantity()->maxValue) ? getParamQuantity()->maxValue : getParamQuantity()->minValue;
			DEBUG("RJ_PadSvgSwitch onDragStart(%d) - Set Value to %3.1f.", btnId, newVal);
			getParamQuantity()->setValue(1.0); // Toggle Value

		}
		return;
	}
	/** Called when the left button is released and this widget is being dragged */
	// https://github.com/j4s0n-c/trowaSoft-VCV/issues/12
	// Last button keeps pressed down.
	// void onDragEnd(const event::DragEnd &e) override
	// {
		// return;
	// }

	/** Called when a widget responds to `onMouseUp` for a left button release and a widget is being dragged */
	void onDragEnter(const event::DragEnter &e) override
	{
		if (e.button != GLFW_MOUSE_BUTTON_LEFT)
			return;
		// Set these no matter what because if you drag back onto your starting square, you want to toggle it again.
		RJ_PadSvgSwitch *origin = dynamic_cast<RJ_PadSvgSwitch*>(e.origin);
		// XXX THIS
		// if (origin && origin != this && origin->groupId == this->groupId && getParamQuantity())
		// {
		if (origin && origin->groupId == this->groupId && getParamQuantity())
		{
			float newVal = (getParamQuantity()->getValue() < getParamQuantity()->maxValue) ? getParamQuantity()->maxValue : getParamQuantity()->minValue;
			DEBUG("RJ_PadSvgSwitch onDragEnter(%d) - Set Value to %3.1f.", btnId, newVal);
			getParamQuantity()->setValue(1.0); // Toggle Value
		}
		return;
	}
	void onDragLeave(const event::DragLeave &e) override {
		if (e.button != GLFW_MOUSE_BUTTON_LEFT)
			return;
		SvgSwitch::onDragLeave(e);
		getParamQuantity()->setValue(0);
		return;
	}
	void onButton(const event::Button &e) override
	{
		this->ParamWidget::onButton(e); // Need to call this base method to be set as the touchedParam for MIDI mapping to work.
	}
};


//--------------------------------------------------------------
// RJ_PadSquare - A Square Pad button.
//--------------------------------------------------------------
struct RJ_PadSquare : RJ_PadSvgSwitch {
	RJ_PadSquare()
	{
		this->SvgSwitch::addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/TS_pad_0.svg")));
		sw->wrap();
		SvgSwitch::box.size = sw->box.size;
	}
	RJ_PadSquare(Vec size)
	{
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TS_pad_0.svg")));
		sw->box.size = size;
		SvgSwitch::box.size = size;
	}
};

struct RJ_LightSquare : ColorValueLight
{
	// Radius on corners
	float cornerRadius = 5.0;
	RJ_LightSquare()
	{
		bgColor = nvgRGBAf(0, 0, 0, /*alpha */ 0.5);
		baseColor = componentlibrary::SCHEME_RED;
	}
	void draw(const DrawArgs &args) override
	{
		float radius = box.size.x / 2.0;
		float oradius = radius*1.1;

		NVGcolor backColor = bgColor;
		NVGcolor outerColor = color;
		// Solid
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, cornerRadius);
		nvgFillColor(args.vg, backColor);
		nvgFill(args.vg);

		// Border
		nvgStrokeWidth(args.vg, 1.0);
		NVGcolor borderColor = bgColor;
		borderColor.a *= 0.5;
		nvgStrokeColor(args.vg, borderColor);
		nvgStroke(args.vg);

		// Inner glow
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		nvgFillColor(args.vg, color);
		nvgFill(args.vg);

		// Outer glow
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, /*x*/ radius - oradius, /*y*/ radius - oradius, /*w*/ 2*oradius, /*h*/ 2*oradius, cornerRadius);
		NVGpaint paint;
		NVGcolor icol = outerColor;// color;
		icol.a *= 0.25;
		NVGcolor ocol = outerColor;// color;
		ocol.a = 0.0;
		float feather = 1;
		// Feather defines how blurry the border of the rectangle is. // Fixed 01/19/2018, made it too tiny before
		paint = nvgBoxGradient(args.vg, /*x*/ radius - oradius, /*y*/ radius - oradius, /*w*/ 2 * oradius, /*h*/ 2 * oradius,  //args.vg, /*x*/ -5, /*y*/ -5, /*w*/ 2*oradius + 10, /*h*/ 2*oradius + 10,
			/*r: corner radius*/ cornerRadius, /*f: feather*/ feather,
			/*inner color*/ icol, /*outer color */ ocol);
		nvgFillPaint(args.vg, paint);
		nvgFill(args.vg);
		return;
	}
}; // end RJ_LightSquare

template <class TModuleLightWidget>
ColorValueLight * RJ_createColorValueLight(Vec pos,  Module *module, int lightId, Vec size, NVGcolor lightColor) {
	ColorValueLight *light = new TModuleLightWidget();
	light->box.pos = pos;
	light->module = module;
	light->firstLightId = lightId;
	//light->value = value;
	light->box.size = size;
	light->setColor(lightColor);
	//light->baseColor = lightColor;
	return light;
}
template <class TModuleLightWidget>
ColorValueLight * RJ_createColorValueLight(Vec pos, Module *module, int lightId, Vec size, NVGcolor lightColor, NVGcolor backColor) {
	ColorValueLight *light = new TModuleLightWidget();
	light->box.pos = pos;
	light->module = module;
	light->firstLightId = lightId;
	light->box.size = size;
	light->setColor(lightColor);
	light->bgColor = backColor;
	return light;
}
