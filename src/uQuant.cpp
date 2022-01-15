/*

uQuant is basically a bastard skin for AmalgamaticHarmonic's Scale Quantizer MKII:
https://github.com/jhoar/AmalgamatedHarmonics

*/

#include "RJModules.hpp"
#include "Core.hpp"
#include "UI.hpp"

#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>

// Module
struct uQuant : Module {

    enum ParamIds {
        KEY_PARAM,
        SCALE_PARAM,
        SHIFT_PARAM,
        TRANS_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        IN_INPUT,
        KEY_INPUT,
        SCALE_INPUT,
        TRANS_INPUT,
        HOLD_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        OUT_OUTPUT,
        TRIG_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    uQuant() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(KEY_PARAM, 0.0f, 11.0f, 0.0f, "Key");
        configParam(SCALE_PARAM, 0.0f, 11.0f, 0.0f, "Scale");
        configParam(SHIFT_PARAM, -3.0f, 3.0f, 0.0f, "Shift");
        configParam(TRANS_PARAM, -11.0f, 11.0f, 0.0f, "Transpose");
        configInput(IN_INPUT, "Main");
        configInput(KEY_INPUT, "Key CV");
        configInput(SCALE_INPUT, "Scale CV");
        configInput(TRANS_INPUT, "Transpose CV");
        configInput(HOLD_INPUT, "Hold");
        configOutput(OUT_OUTPUT, "Main");
        configOutput(TRIG_OUTPUT, "Trigger");
    }

    void process(const ProcessArgs& args) override;

    bool firstStep = true;
    int lastScale = 0;
    int lastRoot = 0;
    float lastTrans = -10000.0f;

    dsp::SchmittTrigger holdTrigger;
    float holdPitch = 0.0f;
    float lastPitch = 0.0f;
    AHPulseGenerator triggerPulse;

    std::string keys[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    std::string scales[12] = {"Ch", "Io", "Do", "Ph", "Ly", "Mi", "Ae", "Lo", "5M", "5m", "Hm", "Bl"};

    std::map<std::string, std::string> display_data {
        {"key_value", "C"},
        {"scale_value", "C"}
    };

    int currScale = 0;
    int currRoot = 0;

};

void uQuant::process(const ProcessArgs& args) {

    lastScale = currScale;
    lastRoot = currRoot;

    int currNote = 0;
    int currDegree = 0;

    if (inputs[KEY_INPUT].active) {
        float fRoot = inputs[KEY_INPUT].value;
        currRoot = CoreUtil().getKeyFromVolts(fRoot);
    } else {
        currRoot = params[KEY_PARAM].value;
    }

    if (inputs[SCALE_INPUT].active) {
        float fScale = inputs[SCALE_INPUT].value;
        currScale = CoreUtil().getScaleFromVolts(fScale);
    } else {
        currScale = params[SCALE_PARAM].value;
    }

    float trans = (inputs[TRANS_INPUT].value + params[TRANS_PARAM].value) / 12.0;
    if (trans != 0.0) {
        if (trans != lastTrans) {
            int i;
            int d;
            trans = CoreUtil().getPitchFromVolts(trans, Core::NOTE_C, Core::SCALE_CHROMATIC, &i, &d);
            lastTrans = trans;
        } else {
            trans = lastTrans;
        }
    }

    float holdInput = inputs[HOLD_INPUT].value;
    bool holdActive = inputs[HOLD_INPUT].active;
    bool holdStatus = holdTrigger.process(holdInput);

    float volts = inputs[IN_INPUT].value;
    float shift = params[SHIFT_PARAM].value;

    if (holdActive) {
        // Sample the pitch
        if (holdStatus && inputs[IN_INPUT].active) {
            holdPitch = CoreUtil().getPitchFromVolts(volts, currRoot, currScale, &currNote, &currDegree);
        }

    } else {
        if (inputs[IN_INPUT].active) {
            holdPitch = CoreUtil().getPitchFromVolts(volts, currRoot, currScale, &currNote, &currDegree);
        }
    }

    // If the quantised pitch has changed
    if (lastPitch != holdPitch) {
        // Pulse the gate
        triggerPulse.trigger(Core::TRIGGER);

        // Record the pitch
        lastPitch = holdPitch;
    }

    if (triggerPulse.process(args.sampleTime)) {
        outputs[TRIG_OUTPUT].value = 10.0f;
    } else {
        outputs[TRIG_OUTPUT].value = 0.0f;
    }

    outputs[OUT_OUTPUT].value = holdPitch + shift + trans;

    display_data["key_value"] = std::string(keys[currRoot]);
    display_data["scale_value"] = std::string(scales[currScale]);

    firstStep = false;

}

// Displays
struct uQuantDisplayWidget : TransparentWidget {

  uQuant *module;
  std::string val_type;

  void draw(NVGcontext *vg) override
  {
    // Background
    NVGcolor backgroundColor = nvgRGB(0xC0, 0xC0, 0xC0);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);

    // text
    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/Pokemon.ttf"));
    if (font) {
    nvgFontSize(vg, 15);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2);
    }

    std::stringstream to_display;
    if (module) {
        to_display << std::setw(3) << module->display_data[val_type];
    } else {
        to_display << std::setw(3) << "C#";
    }

    Vec textPos = Vec(2.0f, 17.0f);
    NVGcolor textColor = nvgRGB(0x00, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};

struct uQTrimpotSnap : RoundKnob {
	uQTrimpotSnap() {
		snap = true;
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance,"res/ComponentLibrary/AHTrimpot.svg")));
	}
};

struct uQuantWidget : ModuleWidget {
    uQuantWidget(uQuant *module);
};

uQuantWidget::uQuantWidget(uQuant *module) {
		setModule(module);

    box.size = Vec(30, 380);

    {
        SvgPanel *panel = new SvgPanel();
        panel->box.size = box.size;
        panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/uQuant.svg")));
        addChild(panel);
    }

    int leftPad = 3;
    int knobLeftPad = 6;

    addInput(createInput<PJ301MPort>(Vec(leftPad, 41), module, uQuant::IN_INPUT));

    uQuantDisplayWidget *displayKey = new uQuantDisplayWidget();
    displayKey = new uQuantDisplayWidget();
    displayKey->box.pos = Vec(leftPad, 71);
    displayKey->box.size = Vec(25, 25);
    displayKey->module = module;
    displayKey->val_type = "key_value";
    addChild(displayKey);
    addParam(createParam<uQTrimpotSnap>(Vec(knobLeftPad, 101), module, uQuant::KEY_PARAM));  // 12 notes
    addInput(createInput<PJ301MPort>(Vec(leftPad, 125), module, uQuant::KEY_INPUT));

    uQuantDisplayWidget *displayScale = new uQuantDisplayWidget();
    displayScale = new uQuantDisplayWidget();
    displayScale->box.pos = Vec(leftPad, 155);
    displayScale->box.size = Vec(25, 25);
    displayScale->module = module;
    displayScale->val_type = "scale_value";
    addChild(displayScale);

    addParam(createParam<uQTrimpotSnap>(Vec(knobLeftPad, 185), module, uQuant::SCALE_PARAM)); // 12 notes
    addInput(createInput<PJ301MPort>(Vec(leftPad, 209), module, uQuant::SCALE_INPUT));

    // Octave
    addParam(createParam<uQTrimpotSnap>(Vec(knobLeftPad, 240), module, uQuant::SHIFT_PARAM));

    // Transpose
    addParam(createParam<uQTrimpotSnap>(Vec(knobLeftPad,265), module, uQuant::TRANS_PARAM)); // 12 notes
    addInput(createInput<PJ301MPort>(Vec(leftPad, 290), module, uQuant::TRANS_INPUT));

    // Outputs
    addOutput(createOutput<PJ301MPort>(Vec(leftPad, 320), module, uQuant::TRIG_OUTPUT));
    addOutput(createOutput<PJ301MPort>(Vec(leftPad, 350), module, uQuant::OUT_OUTPUT));

}

Model *modeluQuant = createModel<uQuant, uQuantWidget>("uQuant");
